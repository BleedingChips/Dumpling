#include "syntax.h"
#include <assert.h>
#include <string>
#include <iostream>

namespace
{
	using namespace Potato;
	using storage_t = lr1::storage_t;
	std::set<storage_t> calculate_nullable_set(const std::vector<std::vector<storage_t>>& production)
	{
		std::set<storage_t> result;
		bool set_change = true;
		while (set_change)
		{
			set_change = false;
			for (auto& ite : production)
			{
				assert(ite.size() >= 1);
				if (ite.size() == 1)
				{
					set_change |= result.insert(ite[0]).second;
				}
				else {
					bool nullable_set = true;
					for (size_t index = 1; index < ite.size(); ++index)
					{
						storage_t symbol = ite[index];
						if (lr1::is_terminal(symbol) || result.find(symbol) == result.end())
						{
							nullable_set = false;
							break;
						}
					}
					if (nullable_set)
						set_change |= result.insert(ite[0]).second;
				}
			}
		}
		return result;
	}

	std::map<storage_t, std::set<storage_t>> calculate_noterminal_first_set(
		const std::vector<std::vector<storage_t>>& production,
		const std::set<storage_t>& nullable_set
	)
	{
		std::map<storage_t, std::set<storage_t>> result;
		bool set_change = true;
		while (set_change)
		{
			set_change = false;
			for (auto& ite : production)
			{
				assert(ite.size() >= 1);
				for (size_t index = 1; index < ite.size(); ++index)
				{
					auto head = ite[0];
					auto target = ite[index];
					if (lr1::is_terminal(target))
					{
						set_change |= result[head].insert(target).second;
						break;
					}
					else {
						if (nullable_set.find(target) == nullable_set.end())
						{
							auto& ref = result[head];
							auto find = result.find(target);
							if (find != result.end())
								for (auto& ite3 : find->second)
									set_change |= ref.insert(ite3).second;
							break;
						}
					}
				}
			}
		}
		return result;
	}

	std::pair<std::set<storage_t>, bool> calculate_production_first_set(
		std::vector<storage_t>::const_iterator begin, std::vector<storage_t>::const_iterator end,
		const std::set<storage_t>& nullable_set,
		const std::map<storage_t, std::set<storage_t>>& first_set
	)
	{
		std::set<storage_t> temporary;
		for (auto ite = begin; ite != end; ++ite)
		{
			if (lr1::is_terminal(*ite))
			{
				temporary.insert(*ite);
				return { std::move(temporary), false };
			}
			else {
				auto find = first_set.find(*ite);
				if (find != first_set.end())
				{
					temporary.insert(find->second.begin(), find->second.end());
					if (nullable_set.find(*ite) == nullable_set.end())
						return { std::move(temporary), false };
				}
				else
					throw lr1::production_head_missing{ *ite, 0 };
			}
		}
		return { std::move(temporary), true };
	}

	std::set<storage_t> calculate_production_first_set_forward(
		std::vector<storage_t>::const_iterator begin, std::vector<storage_t>::const_iterator end,
		const std::set<storage_t>& nullable_set,
		const std::map<storage_t, std::set<storage_t>>& first_set,
		const std::set<storage_t>& forward
	)
	{
		auto result = calculate_production_first_set(begin, end, nullable_set, first_set);
		if (result.second)
			result.first.insert(forward.begin(), forward.end());
		return std::move(result.first);
	}

	std::vector<std::set<storage_t>> calculate_productions_first_set(
		const std::multimap<storage_t, std::vector<storage_t>>& production,
		const std::set<storage_t>& nullable_set,
		const std::map<storage_t, std::set<storage_t>>& first_set_noterminal
	)
	{
		std::vector<std::set<storage_t>> temporary;
		temporary.reserve(production.size());
		for (auto& ite : production)
			temporary.push_back(std::move(calculate_production_first_set(ite.second.begin(), ite.second.end(), nullable_set, first_set_noterminal).first));
		return temporary;
	}

	bool compress_less()
	{
		return false;
	}

	template<typename T, typename K> int compress_less_implement(T&& t, K&& k)
	{
		if (t < k) return 1;
		if (t == k) return 0;
		return -1;
	}

	template<typename T, typename K, typename ...OT> bool compress_less(T&& t, K&& k, OT&&... ot)
	{
		int result = compress_less_implement(t, k);
		if (result == 1)
			return true;
		if (result == 0)
			return compress_less(std::forward<OT>(ot)...);
		return false;
	}

	struct production_index
	{
		storage_t m_production_index;
		storage_t m_production_element_index;
		bool operator<(const production_index& pe) const
		{
			return m_production_index < pe.m_production_index || (m_production_index == pe.m_production_index && m_production_element_index < pe.m_production_element_index);
		}
		bool operator==(const production_index& pe) const
		{
			return m_production_index == pe.m_production_index && m_production_element_index == pe.m_production_element_index;
		}
	};

	struct production_element
	{
		production_index m_index;
		std::set<std::set<storage_t>>::iterator m_forward_set;
		bool operator<(const production_element& pe) const
		{
			return compress_less(m_index, pe.m_index, &(*m_forward_set), &(*pe.m_forward_set));
		}
		bool operator==(const production_element& pe) const
		{
			return m_index == pe.m_index && m_forward_set == pe.m_forward_set;
		}
	};

	using temporary_state_map = std::map<production_index, std::set<std::set<storage_t>>::iterator>;

	bool insert_temporary_state_map(production_index index, temporary_state_map& handled, std::set<std::set<storage_t>>::iterator ite, std::set<std::set<storage_t>>& total_forward_set)
	{
		auto find_res = handled.insert({ index, ite });
		if (find_res.second)
			return true;
		else {
			if (find_res.first->second != ite)
			{
				std::set<storage_t> new_set = *find_res.first->second;
				bool change = false;
				for (auto& ite : *ite)
					change = new_set.insert(ite).second || change;
				find_res.first->second = total_forward_set.insert(new_set).first;
				return change;
			}
		}
		return false;
	}

	std::set<storage_t> remove_forward_set(std::set<storage_t> current, storage_t production_index, const std::vector<std::set<storage_t>>& remove_forward)
	{
		for (auto& ite : remove_forward[static_cast<size_t>(production_index)])
		{
			assert(lr1::is_terminal(ite));
			current.erase(ite);
		}
		return std::move(current);
	}

	std::set<production_element> search_direct_mapping(
		temporary_state_map handled,
		std::set<std::set<storage_t>>& total_forward,
		const std::map<storage_t, std::set<storage_t>>& production_map,
		const std::vector<std::vector<storage_t>>& production,
		const std::set<storage_t>& nullable_set,
		const std::map<storage_t, std::set<storage_t>>& symbol_first_set,
		const std::vector<std::set<storage_t>>& remove_forward
	)
	{
		std::set<production_index> stack;
		for (auto& ite : handled)
		{
			auto re = stack.insert(ite.first);
			assert(re.second);
		}

		while (!stack.empty())
		{
			auto ite = stack.begin();
			production_index current_index = *ite;
			stack.erase(ite);
			auto find_re = handled.find(current_index);
			assert(find_re != handled.end());

			auto pi = find_re->first;
			assert(production.size() > pi.m_production_index);
			auto& prod = production[static_cast<size_t>(pi.m_production_index)];
			storage_t ei = pi.m_production_element_index + 1;
			assert(prod.size() >= ei);
			if (prod.size() > ei)
			{
				storage_t target_symbol = prod[static_cast<size_t>(ei)];
				if (!lr1::is_terminal(target_symbol))
				{
					auto find_re2 = production_map.find(target_symbol);
					if (find_re2 != production_map.end())
					{
						assert(!find_re2->second.empty());
						auto forward_set = calculate_production_first_set_forward(prod.begin() + ei + 1, prod.end(), nullable_set, symbol_first_set, *(find_re->second));
						if (!forward_set.empty())
						{
							auto forward_set_ite = total_forward.insert(std::move(forward_set)).first;
							for (auto& current_index : find_re2->second)
							{
								production_index new_one{ current_index , 0 };
								auto o_ite = forward_set_ite;
								if (!remove_forward[current_index].empty())
								{
									auto current_set = remove_forward_set(*o_ite, current_index, remove_forward);
									if (current_set.empty())
										continue;
									o_ite = total_forward.insert(std::move(current_set)).first;
								}
								if (insert_temporary_state_map(new_one, handled, o_ite, total_forward))
									stack.insert(new_one);
							}
						}
					}
					else {
						assert(production.size() <= static_cast<size_t>(std::numeric_limits<storage_t>::max()));
						throw lr1::production_head_missing{ target_symbol, static_cast<storage_t>(production.size()) };
					}
				}
			}
		}

		std::set<production_element> result;
		for (auto& ite : handled)
		{
			result.insert({ ite.first, ite.second });
		}
		return result;
	}

	std::tuple<std::map<storage_t, std::set<production_element>>, std::map<storage_t, storage_t>> search_shift_and_reduce(
		const std::set<production_element>& input,
		std::set<std::set<storage_t>>& total_forward,
		const std::map<storage_t, std::set<storage_t>>& production_map,
		const std::vector<std::vector<storage_t>>& production,
		const std::set<storage_t>& nullable_set,
		const std::map<storage_t, std::set<storage_t>>& symbol_first_set,
		const std::vector<std::set<storage_t>>& remove_forward
	)
	{
		std::map<storage_t, temporary_state_map> temporary_shift;
		std::map<storage_t, storage_t> reduce;
		assert(!input.empty());
		for (auto& ite : input)
		{
			storage_t pi = ite.m_index.m_production_index;
			assert(production.size() > pi);
			auto& prod = production[ite.m_index.m_production_index];
			storage_t ei = ite.m_index.m_production_element_index + 1;
			assert(prod.size() >= ei);
			if (ei == prod.size())
			{
				for (auto& ite2 : *ite.m_forward_set)
				{
					auto re = reduce.insert({ ite2, pi });
					if (!re.second)
					{
						std::vector<std::tuple<storage_t, std::vector<storage_t>, std::set<storage_t>>> state;
						storage_t old_state = 0;
						storage_t new_state = 0;
						for (auto& ite : input)
						{
							assert(state.size() < std::numeric_limits<storage_t>::max());
							if (re.first->second == ite.m_index.m_production_index)
								old_state = static_cast<storage_t>(state.size());
							else if (pi == ite.m_index.m_production_index)
								new_state = static_cast<storage_t>(state.size());
							state.push_back({ ite.m_index.m_production_element_index, production[ite.m_index.m_production_index], *ite.m_forward_set });
						}
						throw lr1::reduce_conflict{ ite2, old_state, new_state, std::move(state) };
					}

				}
			}
			else {
				uint32_t target_symbol = prod[ei];
				auto cur = ite;
				cur.m_index.m_production_element_index += 1;
				auto& ref = temporary_shift[target_symbol];
				insert_temporary_state_map(cur.m_index, ref, cur.m_forward_set, total_forward);
			}
		}
		std::map<storage_t, std::set<production_element>> shift;
		for (auto& ite : temporary_shift)
		{
			auto re = shift.insert({ ite.first, search_direct_mapping(std::move(ite.second), total_forward, production_map, production, nullable_set, symbol_first_set, remove_forward) });
			assert(re.second);
		}
		return { shift, reduce };
	}

	

	std::map<storage_t, std::set<storage_t>> translate_operator_priority(
		const std::vector<lr1::ope_priority>& priority
	)
	{
		std::map<storage_t, std::set<storage_t>> ope_priority;
		storage_t index = 0;
		for (storage_t index = 0; index < priority.size(); ++index)
		{
			std::set<storage_t> current_remove;
			storage_t target = index;
			auto& [opes, left] = priority[index];
			if(!left)
				++target;
			for (; target > 0; --target)
			{
				auto& [opes, left] = priority[target - 1];
				for (auto& ite : opes)
					current_remove.insert(ite);
			}
			for (auto& ite : opes)
			{
				auto re = ope_priority.insert({ ite, current_remove });
				if (!re.second)
					throw lr1::operator_level_conflict{ ite };
			}
		}
		return std::move(ope_priority);
	}

	storage_t diff(const std::vector<storage_t>& l1, const std::vector<storage_t>& l2)
	{
		storage_t index = 0;
		while (l1.size() > index&& l2.size() > index)
		{
			if (l1[index] != l2[index])
				break;
			++index;
		}
		return index;
	}
}


namespace Potato
{
	lr1::reduce_conflict::reduce_conflict(uint32_t token, uint32_t old_state_index, uint32_t new_state_index, std::vector<std::tuple<uint32_t, std::vector<uint32_t>, std::set<uint32_t>>> state)
		: std::logic_error("reduce conflict"), m_conflig_token(token), m_old_state_index(old_state_index), m_new_state_index(new_state_index), m_state(std::move(state))
	{}

	lr1::production_head_missing::production_head_missing(uint32_t head, uint32_t production)
		: std::logic_error("unable to find proction head"), m_require_head(head), m_production_index(production)
	{}

	lr1::same_production::same_production(uint32_t old_index, uint32_t new_index, std::vector<uint32_t> production)
		: std::logic_error("same production"), m_old_production_index(old_index), m_new_production_index(new_index), m_production(std::move(production))
	{}

	lr1 lr1::create(
		uint32_t start_symbol,
		std::vector<std::vector<storage_t>> production,
		std::vector<ope_priority> priority
	)
	{
		std::vector<std::tuple<storage_t, storage_t>> m_production;
		std::vector<table> m_table;
		for (auto& ite : production)
		{
			assert(ite.size() <= std::numeric_limits<uint32_t>::max());
			if(is_terminal(ite[0]))
				throw unavailable_symbol{};
			for (auto& ite2 : ite)
			{
				if (ite2 == lr1::start_symbol() || ite2 == lr1::eof_symbol())
					throw unavailable_symbol{};
			}
		}
		production.push_back({ lr1::start_symbol(), start_symbol });
		m_production.reserve(production.size() + 1);
		for (auto& ite : production)
		{
			assert(ite.size() <= std::numeric_limits<uint32_t>::max());
			m_production.push_back({ ite[0], static_cast<uint32_t>(ite.size()) - 1 });
		}

		auto null_set = calculate_nullable_set(production);
		auto first_set = calculate_noterminal_first_set(production, null_set);

		std::vector<std::set<uint32_t>> remove;
		remove.resize(production.size());

		{

			auto insert_set = [](std::set<uint32_t>& output, const std::map<uint32_t, std::set<uint32_t>>& first_set, uint32_t symbol) {
				if (is_terminal(symbol))
					output.insert(symbol);
				else {
					auto ite = first_set.find(symbol);
					assert(ite != first_set.end());
					output.insert(ite->second.begin(), ite->second.end());
				}
			};


			std::map<uint32_t, std::set<uint32_t>> remove_map = translate_operator_priority(priority);
			for (uint32_t x = 0; x < production.size(); ++x)
			{
				auto& ref = production[x];
				assert(!ref.empty());

				if (ref.size() >= 2)
				{
					auto symbol = *(ref.rbegin() + 1);
					auto ite = remove_map.find(symbol);
					if (ite != remove_map.end())
						remove[x].insert(ite->second.begin(), ite->second.end());
				}

				for (uint32_t y = x + 1; y < production.size(); ++y)
				{
					auto& ref2 = production[y];
					assert(!ref.empty());
					if (ref[0] == ref2[0])
					{
						uint32_t index = diff(ref, ref2);
						if (index < ref.size() && index < ref2.size())
							continue;
						else if (index == ref.size() && index == ref2.size())
							throw same_production{ x, y, production[x] };
						else {
							if (index < ref.size())
							{
								if (ref[index] != ref[0])
									insert_set(remove[x], first_set, ref[index]);
							}
							else {
								if (ref2[index] != ref2[0])
									insert_set(remove[y], first_set, ref2[index]);
							}
						}
					}
				}
			}
		}

		std::map<uint32_t, std::set<uint32_t>> production_map;

		assert(production.size() < std::numeric_limits<uint32_t>::max());

		{

			for (uint32_t index = 0; index < production.size(); ++index)
			{
				auto& ref = production[index];
				assert(ref.size() >= 1);
				production_map[ref[0]].insert(index);
			}
		}

		std::set<std::set<storage_t>> all_forward_set;


		std::map<std::set<production_element>, storage_t> state_map_mapping;
		std::vector<decltype(state_map_mapping)::iterator> stack;
		uint32_t current_state = 0;
		{
			auto re = all_forward_set.insert({ eof_symbol() }).first;
			temporary_state_map temmap{ {production_index{static_cast<storage_t>(production.size()) - 1, 0}, re} };
			auto result = search_direct_mapping(std::move(temmap), all_forward_set, production_map, production, null_set, first_set, remove);
			auto result2 = state_map_mapping.insert({ std::move(result), current_state });
			assert(result2.second);
			++current_state;
			stack.push_back(result2.first);
		}
		std::map<storage_t, table> temporary_table;

		while (!stack.empty())
		{
			auto ite = *stack.rbegin();
			stack.pop_back();
			auto shift_and_reduce = search_shift_and_reduce(ite->first, all_forward_set, production_map, production, null_set, first_set, remove);
			std::map<storage_t, storage_t> shift;
			for (auto& ite2 : std::get<0>(shift_and_reduce))
			{
				auto result = state_map_mapping.insert({ std::move(ite2.second), current_state });
				if (result.second)
				{
					++current_state;
					stack.push_back(result.first);
				}
				auto result2 = shift.insert({ ite2.first, result.first->second });
				assert(result2.second);
			}
			auto re = temporary_table.insert({ ite->second, table{std::move(shift), std::move(std::get<1>(shift_and_reduce))} });
			assert(re.second);
		}

		m_table.reserve(temporary_table.size());
		for (auto& ite : temporary_table)
		{
			m_table.push_back(std::move(ite.second));
			assert(m_table.size() == static_cast<size_t>(ite.first) + 1);
		}
		lr1 result;
		result.m_production = std::move(m_production);
		result.m_table = std::move(m_table);
		return std::move(result);
	}

	lr1_processor::unacceptable_error::unacceptable_error(storage_t forward_token, error_state lpes)
		: std::logic_error("unacceptable token"), m_forward_token(forward_token), error_state(std::move(lpes)) {}

	lr1_processor::uncomplete_error::uncomplete_error(error_state lps)
		: std::logic_error("unacceptable eof"), error_state(std::move(lps)) {}

	void lr1_processor::try_reduce(storage_t symbol, size_t index, void (*Function)(void* Func, travel input), void* data)
	{
		assert(!m_state_stack.empty());
		m_input_buffer.push_back({ symbol, index });
		while (!m_input_buffer.empty())
		{
			auto [input, token_index] = *m_input_buffer.rbegin();
			storage_t state = *m_state_stack.rbegin();
			auto& ref = m_table_ref.m_table[state];
			if (auto reduce = ref.m_reduce.find(input); reduce != ref.m_reduce.end())
			{
				storage_t production_index = reduce->second;
				assert(production_index < m_table_ref.m_production.size());
				storage_t head_symbol;
				storage_t production_count;
				std::tie(head_symbol, production_count) = m_table_ref.m_production[production_index];
				assert(m_state_stack.size() >= production_count);
				m_state_stack.resize(m_state_stack.size() - production_count);
				if (head_symbol != lr1::start_symbol())
				{
					m_input_buffer.push_back({ head_symbol, 0 });
					if (head_symbol != lr1::start_symbol())
					{
						assert(Function != nullptr && data != nullptr);
						travel input;
						input.symbol = head_symbol;
						input.no_terminal_production_index = production_index;
						input.no_terminal_production_count = production_count;
						(*Function)(data, input);
					}
				}
				else
				{
					assert(m_state_stack.size() == 1);
					m_input_buffer.clear();
				}
			}
			else if (auto shift = ref.m_shift.find(input); shift != ref.m_shift.end())
			{
				auto [sym, index] = *m_input_buffer.rbegin();
				if (lr1::is_terminal(sym))
				{
					travel input;
					input.symbol = sym;
					input.terminal_token_index = index;
					(*Function)(data, input);
				}
				m_input_buffer.pop_back();
				m_state_stack.push_back(shift->second);
			}
			else {
				std::set<storage_t> total_shift;
				for (auto& ite : ref.m_shift)
					total_shift.insert(ite.first);
				throw unacceptable_error{ input, {std::move(total_shift), ref.m_reduce} };
			}
		}
	}

	void lr1_ast::imp::operator()(lr1_processor::travel input)
	{
		if (input.is_terminal())
		{
			lr1_ast ast{ input.symbol, input.terminal_token_index };
			ast_buffer.push_back(std::move(ast));
		}
		else {
			auto size = input.no_terminal_production_count;
			size_t start = ast_buffer.size() - size;
			std::vector<lr1_ast> list{ std::move_iterator(ast_buffer.begin() + start), std::move_iterator(ast_buffer.end()) };
			lr1_ast ast{ input.symbol, std::move(list) };
			ast_buffer.resize(start);
			ast_buffer.push_back(std::move(ast));
		}
	}

	lr1_ast lr1_ast::imp::result()
	{
		assert(ast_buffer.size() == 1);
		auto re = std::move(*ast_buffer.begin());
		ast_buffer.clear();
		return std::move(re);
	}


	/*
	auto lr1_processor::try_reduce()->std::optional<std::tuple<storage_t, storage_t, storage_t>>
	{
		assert(!m_state_stack.empty());
		while (!m_input_buffer.empty())
		{
			storage_t input = *m_input_buffer.rbegin();
			storage_t state = *m_state_stack.rbegin();
			auto& ref = m_table_ref.m_table[state];
			if (auto reduce = ref.m_reduce.find(input); reduce != ref.m_reduce.end())
			{
				storage_t production_index = reduce->second;
				assert(production_index < m_table_ref.m_production.size());
				storage_t head_symbol;
				storage_t production_count;
				std::tie(head_symbol, production_count) = m_table_ref.m_production[production_index];
				assert(m_state_stack.size() >= production_count);
				m_state_stack.resize(m_state_stack.size() - production_count);
				if (head_symbol != lr1::noterminal_start())
					m_input_buffer.push_back(head_symbol);
				else
				{
					assert(m_state_stack.size() == 1);
					m_input_buffer.clear();
				}
				return std::tuple<storage_t, storage_t, storage_t>{ head_symbol, production_index, production_count };
			}
			else if (auto shift = ref.m_shift.find(input); shift != ref.m_shift.end())
			{
				m_input_buffer.pop_back();
				m_state_stack.push_back(shift->second);
			}
			else {
				std::set<storage_t> total_shift;
				for (auto& ite : ref.m_shift)
					total_shift.insert(ite.first);
				throw unacceptable_error{ input, {std::move(total_shift), ref.m_reduce} };
			}
		}
		return std::nullopt;
	}
	*/

	/*
	auto lr1_processor::receive(storage_t symbol) -> std::vector<result>
	{
		assert(!m_state_stack.empty());
		std::vector<result> re;
		m_input_buffer.push_back(symbol);
		while (!m_input_buffer.empty())
		{
			storage_t input = *m_input_buffer.rbegin();
			storage_t state = *m_state_stack.rbegin();
			auto& ref = m_table_ref.m_table[state];
			if (auto reduce = ref.m_reduce.find(input); reduce != ref.m_reduce.end())
			{
				storage_t production_index = reduce->second;
				assert(production_index < m_table_ref.m_production.size());
				storage_t head_symbol;
				storage_t production_count;
				std::tie(head_symbol, production_count) = m_table_ref.m_production[production_index];
				assert(m_state_stack.size() >= production_count);
				m_state_stack.resize(m_state_stack.size() - production_count);

				re.push_back({ head_symbol, production_index, production_count });
				if (head_symbol != lr1::noterminal_start())
					m_input_buffer.push_back(head_symbol);
				else
				{
					assert(m_state_stack.size() == 1);
					m_input_buffer.clear();
				}
			}
			else if (auto shift = ref.m_shift.find(input); shift != ref.m_shift.end())
			{
				m_input_buffer.pop_back();
				m_state_stack.push_back(shift->second);
			}
			else {
				std::set<storage_t> total_shift;
				for (auto& ite : ref.m_shift)
					total_shift.insert(ite.first);
				throw unacceptable_error{ input, {std::move(total_shift), ref.m_reduce} };
			}
		}
		return std::move(re);
	}
	*/



	/*
	LR1_implement::LR1_implement(uint32_t start_symbol, std::vector<std::vector<uint32_t>> production,
		std::vector<std::tuple<std::vector<std::variant<uint32_t, std::pair<uint32_t, uint32_t>>>, Associativity>> priority)
		//: m_production(std::move(production))
	{
		production.push_back({ noterminal_start(), start_symbol });

		m_production.reserve(production.size());
		for (auto& ite : production)
		{
			assert(ite.size() <= std::numeric_limits<uint32_t>::max());
			m_production.push_back({ ite[0], static_cast<uint32_t>(ite.size()) - 1 });
		}


		auto null_set = calculate_nullable_set(production);
		auto first_set = calculate_noterminal_first_set(production, null_set);

		std::vector<std::set<uint32_t>> remove;
		remove.resize(production.size());

		{

			auto insert_set = [](std::set<uint32_t>& output, const std::map<uint32_t, std::set<uint32_t>>& first_set, uint32_t symbol) {
				if (is_terminal(symbol))
					output.insert(symbol);
				else {
					auto ite = first_set.find(symbol);
					assert(ite != first_set.end());
					output.insert(ite->second.begin(), ite->second.end());
				}
			};


			std::map<uint32_t, std::set<uint32_t>> remove_map = translate_operator_priority(priority);
			for (uint32_t x = 0; x < production.size(); ++x)
			{
				auto& ref = production[x];
				assert(!ref.empty());

				if (ref.size() >= 2)
				{
					auto symbol = *(ref.rbegin() + 1);
					auto ite = remove_map.find(symbol);
					if (ite != remove_map.end())
						remove[x].insert(ite->second.begin(), ite->second.end());
				}

				for (uint32_t y = x + 1; y < production.size(); ++y)
				{
					auto& ref2 = production[y];
					assert(!ref.empty());
					if (ref[0] == ref2[0])
					{
						uint32_t index = diff(ref, ref2);
						if (index < ref.size() && index < ref2.size())
							continue;
						else if (index == ref.size() && index == ref2.size())
							throw lr1_same_production{ x, y, production[x] };
						else {
							if (index < ref.size())
							{
								if (ref[index] != ref[0])
									insert_set(remove[x], first_set, ref[index]);
							}
							else {
								if (ref2[index] != ref2[0])
									insert_set(remove[y], first_set, ref2[index]);
							}
						}
					}
				}
			}
		}

		std::map<uint32_t, std::set<uint32_t>> production_map;

		assert(production.size() < std::numeric_limits<uint32_t>::max());

		{

			for (uint32_t index = 0; index < production.size(); ++index)
			{
				auto& ref = production[index];
				assert(ref.size() >= 1);
				production_map[ref[0]].insert(index);
			}
		}

		std::set<std::set<uint32_t>> all_forward_set;


		std::map<std::set<production_element>, uint32_t> state_map_mapping;
		std::vector<decltype(state_map_mapping)::iterator> stack;
		uint32_t current_state = 0;
		{
			auto re = all_forward_set.insert({ terminal_eof() }).first;
			temporary_state_map temmap{ {production_index{static_cast<uint32_t>(production.size()) - 1, 0}, re} };
			auto result = search_direct_mapping(std::move(temmap), all_forward_set, production_map, production, null_set, first_set, remove);
			auto result2 = state_map_mapping.insert({ std::move(result), current_state });
			assert(result2.second);
			++current_state;
			stack.push_back(result2.first);
		}
		std::map<uint32_t, shift_reduce_description> temporary_table;

		while (!stack.empty())
		{
			auto ite = *stack.rbegin();
			stack.pop_back();
			auto shift_and_reduce = search_shift_and_reduce(ite->first, all_forward_set, production_map, production, null_set, first_set, remove);
			std::map<uint32_t, uint32_t> shift;
			for (auto& ite2 : std::get<0>(shift_and_reduce))
			{
				auto result = state_map_mapping.insert({ std::move(ite2.second), current_state });
				if (result.second)
				{
					++current_state;
					stack.push_back(result.first);
				}
				auto result2 = shift.insert({ ite2.first, result.first->second });
				assert(result2.second);
			}
			auto re = temporary_table.insert({ ite->second, shift_reduce_description{std::move(shift), std::move(std::get<1>(shift_and_reduce))} });
			assert(re.second);
		}

		m_table.reserve(temporary_table.size());
		for (auto& ite : temporary_table)
		{
			m_table.push_back(std::move(ite.second));
			assert(m_table.size() == ite.first + 1);
		}
	}
	*/

}






















/*
namespace Potato
{
	namespace Implement
	{
		std::set<uint32_t> calculate_nullable_set(const std::vector<std::vector<uint32_t>>& production)
		{
			std::set<uint32_t> result;
			bool set_change = true;
			while (set_change)
			{
				set_change = false;
				for (auto& ite : production)
				{
					assert(ite.size() >= 1);
					if (ite.size() == 1)
					{
						set_change |= result.insert(ite[0]).second;
					}
					else {
						bool nullable_set = true;
						for (size_t index = 1; index < ite.size(); ++index)
						{
							uint32_t symbol = ite[index];
							if (is_terminal(symbol) || result.find(symbol) == result.end())
							{
								nullable_set = false;
								break;
							}
						}
						if (nullable_set)
							set_change |= result.insert(ite[0]).second;
					}
				}
			}
			return result;
		}

		std::map<uint32_t, std::set<uint32_t>> calculate_noterminal_first_set(
			const std::vector<std::vector<uint32_t>>& production,
			const std::set<uint32_t>& nullable_set
		)
		{
			std::map<uint32_t, std::set<uint32_t>> result;
			bool set_change = true;
			while (set_change)
			{
				set_change = false;
				for (auto& ite : production)
				{
					assert(ite.size() >= 1);
					for (size_t index = 1; index < ite.size(); ++index)
					{
						auto head = ite[0];
						auto target = ite[index];
						if (is_terminal(target))
						{
							set_change |= result[head].insert(target).second;
							break;
						}
						else {
							if (nullable_set.find(target) == nullable_set.end())
							{
								auto& ref = result[head];
								auto find = result.find(target);
								if (find != result.end())
									for (auto& ite3 : find->second)
										set_change |= ref.insert(ite3).second;
								break;
							}
						}
					}
				}
			}
			return result;
		}

		std::pair<std::set<uint32_t>, bool> calculate_production_first_set(
			std::vector<uint32_t>::const_iterator begin, std::vector<uint32_t>::const_iterator end,
			const std::set<uint32_t>& nullable_set,
			const std::map<uint32_t, std::set<uint32_t>>& first_set
		)
		{
			std::set<uint32_t> temporary;
			for (auto ite = begin; ite != end; ++ite)
			{
				if (Implement::is_terminal(*ite))
				{
					temporary.insert(*ite);
					return { std::move(temporary), false };
				}
				else {
					auto find = first_set.find(*ite);
					if (find != first_set.end())
					{
						temporary.insert(find->second.begin(), find->second.end());
						if (nullable_set.find(*ite) == nullable_set.end())
							return { std::move(temporary), false };
					}
					else
						throw Implement::lr1_production_head_missing{ *ite, 0 };
				}
			}
			return { std::move(temporary), true };
		}

		std::set<uint32_t> calculate_production_first_set_forward(
			std::vector<uint32_t>::const_iterator begin, std::vector<uint32_t>::const_iterator end,
			const std::set<uint32_t>& nullable_set,
			const std::map<uint32_t, std::set<uint32_t>>& first_set,
			const std::set<uint32_t>& forward
		)
		{
			auto result = calculate_production_first_set(begin, end, nullable_set, first_set);
			if (result.second)
				result.first.insert(forward.begin(), forward.end());
			return std::move(result.first);
		}

		std::vector<std::set<uint32_t>> calculate_productions_first_set(
			const std::multimap<uint32_t, std::vector<uint32_t>>& production,
			const std::set<uint32_t>& nullable_set,
			const std::map<uint32_t, std::set<uint32_t>>& first_set_noterminal
		)
		{
			std::vector<std::set<uint32_t>> temporary;
			temporary.reserve(production.size());
			for (auto& ite : production)
				temporary.push_back(std::move(calculate_production_first_set(ite.second.begin(), ite.second.end(), nullable_set, first_set_noterminal).first));
			return temporary;
		}

		bool compress_less()
		{
			return false;
		}

		template<typename T, typename K> int compress_less_implement(T&& t, K&& k)
		{
			if (t < k) return 1;
			if (t == k) return 0;
			return -1;
		}

		template<typename T, typename K, typename ...OT> bool compress_less(T&& t, K&& k, OT&&... ot)
		{
			int result = compress_less_implement(t, k);
			if (result == 1)
				return true;
			if (result == 0)
				return compress_less(std::forward<OT>(ot)...);
			return false;
		}

		struct production_element
		{
			production_index m_index;
			std::set<std::set<uint32_t>>::iterator m_forward_set;
			bool operator<(const production_element& pe) const
			{
				return compress_less(m_index, pe.m_index, &(*m_forward_set), &(*pe.m_forward_set));
			}
			bool operator==(const production_element& pe) const
			{
				return m_index == pe.m_index && m_forward_set == pe.m_forward_set;
			}
		};

		using temporary_state_map = std::map<production_index, std::set<std::set<uint32_t>>::iterator>;

		bool insert_temporary_state_map(production_index index, temporary_state_map& handled, std::set<std::set<uint32_t>>::iterator ite, std::set<std::set<uint32_t>>& total_forward_set)
		{
			auto find_res = handled.insert({ index, ite });
			if (find_res.second)
				return true;
			else {
				if (find_res.first->second != ite)
				{
					std::set<uint32_t> new_set = *find_res.first->second;
					bool change = false;
					for (auto& ite : *ite)
						change = new_set.insert(ite).second || change;
					find_res.first->second = total_forward_set.insert(new_set).first;
					return change;
				}
			}
			return false;
		}

		std::set<uint32_t> remove_forward_set(std::set<uint32_t> current, uint32_t production_index, const std::vector<std::set<uint32_t>>& remove_forward)
		{
			for (auto& ite : remove_forward[static_cast<size_t>(production_index)])
			{
				assert(is_terminal(ite));
				current.erase(ite);
			}
			return std::move(current);
		}

		std::set<production_element> search_direct_mapping(
			temporary_state_map handled,
			std::set<std::set<uint32_t>>& total_forward,
			const std::map<uint32_t, std::set<uint32_t>>& production_map,
			const std::vector<std::vector<uint32_t>>& production,
			const std::set<uint32_t>& nullable_set,
			const std::map<uint32_t, std::set<uint32_t>>& symbol_first_set,
			const std::vector<std::set<uint32_t>>& remove_forward
		)
		{
			std::set<production_index> stack;
			for (auto& ite : handled)
			{
				auto re = stack.insert(ite.first);
				assert(re.second);
			}

			while (!stack.empty())
			{
				auto ite = stack.begin();
				production_index current_index = *ite;
				stack.erase(ite);
				auto find_re = handled.find(current_index);
				assert(find_re != handled.end());

				auto pi = find_re->first;
				assert(production.size() > pi.m_production_index);
				auto& prod = production[static_cast<size_t>(pi.m_production_index)];
				uint32_t ei = pi.m_production_element_index + 1;
				assert(prod.size() >= ei);
				if (prod.size() > ei)
				{
					uint32_t target_symbol = prod[static_cast<size_t>(ei)];
					if (!is_terminal(target_symbol))
					{
						auto find_re2 = production_map.find(target_symbol);
						if (find_re2 != production_map.end())
						{
							assert(!find_re2->second.empty());
							auto forward_set = calculate_production_first_set_forward(prod.begin() + ei + 1, prod.end(), nullable_set, symbol_first_set, *(find_re->second));
							if (!forward_set.empty())
							{
								auto forward_set_ite = total_forward.insert(std::move(forward_set)).first;
								for (auto& current_index : find_re2->second)
								{
									production_index new_one{ current_index , 0 };
									auto o_ite = forward_set_ite;
									if (!remove_forward[current_index].empty())
									{
										auto current_set = remove_forward_set(*o_ite, current_index, remove_forward);
										if (current_set.empty())
											continue;
										o_ite = total_forward.insert(std::move(current_set)).first;
									}
									if (insert_temporary_state_map(new_one, handled, o_ite, total_forward))
										stack.insert(new_one);
								}
							}
						}
						else {
							assert(production.size() <= static_cast<size_t>(std::numeric_limits<uint32_t>::max()));
							throw lr1_production_head_missing{ target_symbol, static_cast<uint32_t>(production.size()) };
						}
					}
				}
			}

			std::set<production_element> result;
			for (auto& ite : handled)
			{
				result.insert({ ite.first, ite.second });
			}
			return result;
		}

		std::tuple<std::map<uint32_t, std::set<production_element>>, std::map<uint32_t, uint32_t>> search_shift_and_reduce(
			const std::set<production_element>& input,
			std::set<std::set<uint32_t>>& total_forward,
			const std::map<uint32_t, std::set<uint32_t>>& production_map,
			const std::vector<std::vector<uint32_t>>& production,
			const std::set<uint32_t>& nullable_set,
			const std::map<uint32_t, std::set<uint32_t>>& symbol_first_set,
			const std::vector<std::set<uint32_t>>& remove_forward
		)
		{
			std::map<uint32_t, temporary_state_map> temporary_shift;
			std::map<uint32_t, uint32_t> reduce;
			assert(!input.empty());
			for (auto& ite : input)
			{
				uint32_t pi = ite.m_index.m_production_index;
				assert(production.size() > pi);
				auto& prod = production[ite.m_index.m_production_index];
				uint32_t ei = ite.m_index.m_production_element_index + 1;
				assert(prod.size() >= ei);
				if (ei == prod.size())
				{
					for (auto& ite2 : *ite.m_forward_set)
					{
						auto re = reduce.insert({ ite2, pi });
						if (!re.second)
						{
							std::vector<std::tuple<uint32_t, std::vector<uint32_t>, std::set<uint32_t>>> state;
							uint32_t old_state = 0;
							uint32_t new_state = 0;
							for (auto& ite : input)
							{
								assert(state.size() < std::numeric_limits<uint32_t>::max());
								if (re.first->second == ite.m_index.m_production_index)
									old_state = static_cast<uint32_t>(state.size());
								else if (pi == ite.m_index.m_production_index)
									new_state = static_cast<uint32_t>(state.size());
								state.push_back({ ite.m_index.m_production_element_index, production[ite.m_index.m_production_index], *ite.m_forward_set });
							}
							throw lr1_reduce_conflict{ ite2, old_state, new_state, std::move(state) };
						}

					}
				}
				else {
					uint32_t target_symbol = prod[ei];
					auto cur = ite;
					cur.m_index.m_production_element_index += 1;
					auto& ref = temporary_shift[target_symbol];
					insert_temporary_state_map(cur.m_index, ref, cur.m_forward_set, total_forward);
				}
			}
			std::map<uint32_t, std::set<production_element>> shift;
			for (auto& ite : temporary_shift)
			{
				auto re = shift.insert({ ite.first, search_direct_mapping(std::move(ite.second), total_forward, production_map, production, nullable_set, symbol_first_set, remove_forward) });
				assert(re.second);
			}
			return { shift, reduce };
		}

		lr1_reduce_conflict::lr1_reduce_conflict(uint32_t token, uint32_t old_state_index, uint32_t new_state_index, std::vector<std::tuple<uint32_t, std::vector<uint32_t>, std::set<uint32_t>>> state)
			: std::logic_error("reduce conflict"), m_conflig_token(token), m_old_state_index(old_state_index), m_new_state_index(new_state_index), m_state(std::move(state))
		{}

		lr1_production_head_missing::lr1_production_head_missing(uint32_t head, uint32_t production)
			: std::logic_error("unable to find proction head"), m_require_head(head), m_production_index(production)
		{}

		lr1_same_production::lr1_same_production(uint32_t old_index, uint32_t new_index, std::vector<uint32_t> production)
			: std::logic_error("same production"), m_old_production_index(old_index), m_new_production_index(new_index), m_production(std::move(production))
		{}

		std::map<uint32_t, std::set<uint32_t>> translate_operator_priority(
			const std::vector<std::tuple<std::vector<std::variant<uint32_t, std::pair<uint32_t, uint32_t>>>, Associativity>>& priority
		)
		{
			std::map<uint32_t, std::set<uint32_t>> ope_priority;
			uint32_t index = 0;
			for (uint32_t index = 0; index < priority.size(); ++index)
			{
				std::set<uint32_t> current_remove;
				uint32_t target = index;
				if (std::get<1>(priority[index]) == Associativity::Right)
					++target;
				for (; target > 0; --target)
				{
					auto& ref = std::get<0>(priority[target - 1]);
					for (auto& ite : ref)
					{
						if (std::holds_alternative<uint32_t>(ite))
							current_remove.insert(std::get<uint32_t>(ite));
						else
							current_remove.insert(std::get<std::pair<uint32_t, uint32_t>>(ite).first);
					}
				}
				for (auto& ite : std::get<0>(priority[index]))
				{
					uint32_t symbol;
					if (std::holds_alternative<uint32_t>(ite))
						symbol = std::get<uint32_t>(ite);
					else
						symbol = std::get<std::pair<uint32_t, uint32_t>>(ite).second;
					auto re = ope_priority.insert({ symbol, current_remove });
					if (!re.second)
						throw Error::LR1_operator_level_conflict{ symbol };
				}
			}
			return std::move(ope_priority);
		}

		uint32_t diff(const std::vector<uint32_t>& l1, const std::vector<uint32_t>& l2)
		{
			uint32_t index = 0;
			while (l1.size() > index && l2.size() > index)
			{
				if (l1[index] != l2[index])
					break;
				++index;
			}
			return index;
		}

		LR1_implement::LR1_implement(uint32_t start_symbol, std::vector<std::vector<uint32_t>> production,
			std::vector<std::tuple<std::vector<std::variant<uint32_t, std::pair<uint32_t, uint32_t>>>, Associativity>> priority)
			//: m_production(std::move(production))
		{
			production.push_back({ noterminal_start(), start_symbol });

			m_production.reserve(production.size());
			for (auto& ite : production)
			{
				assert(ite.size() <= std::numeric_limits<uint32_t>::max());
				m_production.push_back({ ite[0], static_cast<uint32_t>(ite.size()) - 1 });
			}
				

			auto null_set = calculate_nullable_set(production);
			auto first_set = calculate_noterminal_first_set(production, null_set);

			std::vector<std::set<uint32_t>> remove;
			remove.resize(production.size());

			{

				auto insert_set = [](std::set<uint32_t>& output, const std::map<uint32_t, std::set<uint32_t>>& first_set, uint32_t symbol) {
					if (is_terminal(symbol))
						output.insert(symbol);
					else {
						auto ite = first_set.find(symbol);
						assert(ite != first_set.end());
						output.insert(ite->second.begin(), ite->second.end());
					}
				};


				std::map<uint32_t, std::set<uint32_t>> remove_map = translate_operator_priority(priority);
				for (uint32_t x = 0; x < production.size(); ++x)
				{
					auto& ref = production[x];
					assert(!ref.empty());

					if (ref.size() >= 2)
					{
						auto symbol = *(ref.rbegin() + 1);
						auto ite = remove_map.find(symbol);
						if (ite != remove_map.end())
							remove[x].insert(ite->second.begin(), ite->second.end());
					}

					for (uint32_t y = x + 1; y < production.size(); ++y)
					{
						auto& ref2 = production[y];
						assert(!ref.empty());
						if (ref[0] == ref2[0])
						{
							uint32_t index = diff(ref, ref2);
							if (index < ref.size() && index < ref2.size())
								continue;
							else if (index == ref.size() && index == ref2.size())
								throw lr1_same_production{ x, y, production[x] };
							else {
								if (index < ref.size())
								{
									if (ref[index] != ref[0])
										insert_set(remove[x], first_set, ref[index]);
								}
								else {
									if (ref2[index] != ref2[0])
										insert_set(remove[y], first_set, ref2[index]);
								}
							}
						}
					}
				}
			}

			std::map<uint32_t, std::set<uint32_t>> production_map;

			assert(production.size() < std::numeric_limits<uint32_t>::max());

			{
				
				for (uint32_t index = 0; index < production.size(); ++index)
				{
					auto& ref = production[index];
					assert(ref.size() >= 1);
					production_map[ref[0]].insert(index);
				}
			}

			std::set<std::set<uint32_t>> all_forward_set;


			std::map<std::set<production_element>, uint32_t> state_map_mapping;
			std::vector<decltype(state_map_mapping)::iterator> stack;
			uint32_t current_state = 0;
			{
				auto re = all_forward_set.insert({ terminal_eof() }).first;
				temporary_state_map temmap{ {production_index{static_cast<uint32_t>(production.size()) - 1, 0}, re} };
				auto result = search_direct_mapping(std::move(temmap), all_forward_set, production_map, production, null_set, first_set, remove);
				auto result2 = state_map_mapping.insert({ std::move(result), current_state });
				assert(result2.second);
				++current_state;
				stack.push_back(result2.first);
			}
			std::map<uint32_t, shift_reduce_description> temporary_table;

			while (!stack.empty())
			{
				auto ite = *stack.rbegin();
				stack.pop_back();
				auto shift_and_reduce = search_shift_and_reduce(ite->first, all_forward_set, production_map, production, null_set, first_set, remove);
				std::map<uint32_t, uint32_t> shift;
				for (auto& ite2 : std::get<0>(shift_and_reduce))
				{
					auto result = state_map_mapping.insert({ std::move(ite2.second), current_state });
					if (result.second)
					{
						++current_state;
						stack.push_back(result.first);
					}
					auto result2 = shift.insert({ ite2.first, result.first->second });
					assert(result2.second);
				}
				auto re = temporary_table.insert({ ite->second, shift_reduce_description{std::move(shift), std::move(std::get<1>(shift_and_reduce))} });
				assert(re.second);
			}

			m_table.reserve(temporary_table.size());
			for (auto& ite : temporary_table)
			{
				m_table.push_back(std::move(ite.second));
				assert(m_table.size() == ite.first + 1);
			}
		}

		struct input_index_generator
		{
			input_index_generator(const uint32_t* input, size_t length) : m_input(input), m_length(length) { assert(m_input != nullptr); assert(m_length != 0); }
			uint32_t operator()() noexcept
			{
				assert(m_index < m_length);
				return m_input[m_index++];
			}
		private:
			const uint32_t* m_input;
			size_t m_length;
			size_t m_index = 0;
		};

		LR1_implement::LR1_implement(const uint32_t* input, size_t length)
		{
			input_index_generator ig(input, length);
			uint32_t production_count = ig();
			m_production.resize(production_count);
			for (auto& ite : m_production)
			{
				uint32_t symbol = ig();
				uint32_t length = ig();
				ite = { symbol, length };
			}
			uint32_t table_count = ig();
			m_table.resize(table_count);
			for (auto& ite : m_table)
			{
				uint32_t shift_count = ig();
				for (uint32_t i = 0; i < shift_count; ++i)
				{
					uint32_t s1 = ig();
					uint32_t s2 = ig();
					auto re = ite.m_shift.insert({ s1,s2 }).second;
					assert(re);
				}
				uint32_t reduce_count = ig();
				for (uint32_t i = 0; i < reduce_count; ++i)
				{
					uint32_t r1 = ig();
					uint32_t r2 = ig();
					auto re = ite.m_reduce.insert({ r1,r2 }).second;
					assert(re);
				}
			}
		}

		size_t LR1_implement::calculate_data_length() const noexcept
		{
			size_t count = 0;
			count += m_production.size() * 2 + 1;
			count += 1;
			for (auto& ite : m_table)
			{
				count += ite.m_shift.size() * 2 + 1;
				count += ite.m_reduce.size() * 2 + 1;
			}
			return count;
		}

		void LR1_implement::output_data(uint32_t* output) const noexcept
		{
			size_t index = 0;
			assert(m_production.size() <= std::numeric_limits<uint32_t>::max());
			output[index++] = static_cast<uint32_t>(m_production.size());
			for (auto& ite : m_production)
			{
				output[index++] = std::get<0>(ite);
				output[index++] = std::get<1>(ite);
			}
			assert(m_table.size() <= std::numeric_limits<uint32_t>::max());
			output[index++] = static_cast<uint32_t>(m_table.size());
			for (auto& ite : m_table)
			{
				assert(ite.m_shift.size() <= std::numeric_limits<uint32_t>::max());
				output[index++] = static_cast<uint32_t>(ite.m_shift.size());
				for (auto& ite2 : ite.m_shift)
				{
					output[index++] = ite2.first;
					output[index++] = ite2.second;
				}
				assert(ite.m_reduce.size() <= std::numeric_limits<uint32_t>::max());
				output[index++] = static_cast<uint32_t>(ite.m_reduce.size());
				for (auto& ite2 : ite.m_reduce)
				{
					output[index++] = ite2.first;
					output[index++] = ite2.second;
				}
			}
		}

		lr1_process_unacceptable_error::lr1_process_unacceptable_error(uint32_t forward_token, lr1_process_error_state lpes)
			: std::logic_error("unacceptable token"), m_forward_token(forward_token), lr1_process_error_state(std::move(lpes)) {}

		lr1_process_uncomplete_error::lr1_process_uncomplete_error(lr1_process_error_state lps)
			: std::logic_error("unacceptable eof"), lr1_process_error_state(std::move(lps)) {}

		lr1_processor::lr1_processor(const LR1_implement& syntax) : m_syntax(syntax), m_state_stack({ 0 }) {}

		auto lr1_processor::receive(uint32_t symbol) -> std::vector<result>
		{
			assert(!m_state_stack.empty());
			std::vector<result> re;
			m_input_buffer.push_back(symbol);
			while (!m_input_buffer.empty())
			{
				uint32_t input = *m_input_buffer.rbegin();
				uint32_t state = *m_state_stack.rbegin();
				auto& ref = m_syntax.m_table[state];
				if (auto reduce = ref.m_reduce.find(input); reduce != ref.m_reduce.end())
				{
					uint32_t production_index = reduce->second;
					assert(production_index < m_syntax.m_production.size());
					uint32_t head_symbol;
					uint32_t production_count;
					std::tie(head_symbol, production_count) = m_syntax.m_production[production_index];
					assert(m_state_stack.size() >= production_count);
					m_state_stack.resize(m_state_stack.size() - production_count);

					re.push_back({ head_symbol, production_index, production_count });
					if (head_symbol != noterminal_start())
						m_input_buffer.push_back(head_symbol);
					else
					{
						assert(m_state_stack.size() == 1);
						m_input_buffer.clear();
					}
				}
				else if (auto shift = ref.m_shift.find(input); shift != ref.m_shift.end())
				{
					m_input_buffer.pop_back();
					m_state_stack.push_back(shift->second);
				}
				else {
					std::set<uint32_t> total_shift;
					for (auto& ite : ref.m_shift)
						total_shift.insert(ite.first);
					throw lr1_process_unacceptable_error{ input, {std::move(total_shift), ref.m_reduce} };
				}
			}
			return std::move(re);
		}

	}
}
*/