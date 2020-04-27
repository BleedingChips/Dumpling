#include "syntax.h"
#include <assert.h>
#include <string>
#include <sstream>
namespace
{
	using namespace Potato::Syntax;
	using storage_t = lr1::storage_t;
	std::set<storage_t> calculate_nullable_set(const std::vector<lr1::production_input>& production)
	{
		std::set<storage_t> result;
		bool set_change = true;
		while (set_change)
		{
			set_change = false;
			for (auto& ite : production)
			{
				assert(ite.production.size() >= 1);
				if (ite.production.size() == 1)
				{
					set_change |= result.insert(ite.production[0]).second;
				}
				else {
					bool nullable_set = true;
					for (size_t index = 1; index < ite.production.size(); ++index)
					{
						storage_t symbol = ite.production[index];
						if (lr1::is_terminal(symbol) || result.find(symbol) == result.end())
						{
							nullable_set = false;
							break;
						}
					}
					if (nullable_set)
						set_change |= result.insert(ite.production[0]).second;
				}
			}
		}
		return result;
	}

	std::map<storage_t, std::set<storage_t>> calculate_noterminal_first_set(
		const std::vector<lr1::production_input>& production,
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
				assert(ite.production.size() >= 1);
				for (size_t index = 1; index < ite.production.size(); ++index)
				{
					auto head = ite.production[0];
					auto target = ite.production[index];
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
					throw lr1::missing_noterminal_define_error(*ite);
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
		const std::vector<lr1::production_input>& production,
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
			assert(prod.production.size() >= ei);
			if (prod.production.size() > ei)
			{
				storage_t target_symbol = prod.production[static_cast<size_t>(ei)];
				if (!lr1::is_terminal(target_symbol))
				{
					auto find_re2 = production_map.find(target_symbol);
					if (find_re2 != production_map.end())
					{
						assert(!find_re2->second.empty());
						auto forward_set = calculate_production_first_set_forward(prod.production.begin() + ei + 1, prod.production.end(), nullable_set, symbol_first_set, *(find_re->second));
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
						throw lr1::missing_noterminal_define_error(target_symbol);
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
		const std::vector<lr1::production_input>& production,
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
			auto& prod = production[ite.m_index.m_production_index].production;
			storage_t ei = ite.m_index.m_production_element_index + 1;
			assert(prod.size() >= ei);
			if (ei == prod.size())
			{
				for (auto& ite2 : *ite.m_forward_set)
				{
					auto re = reduce.insert({ ite2, pi });
					if (!re.second)
						throw lr1::reduce_conflict_error(ite2, re.first->first, pi);
				}
			}
			else {
				storage_t target_symbol = prod[ei];
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
					throw lr1::operator_conflict_error(ite, re.first->first);
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


namespace Potato::Syntax
{

	char const* lr1::unacceptable_production_error::what() const noexcept
	{
		return "Unacceable Prodution";
	}

	char const* lr1::missing_noterminal_define_error::what() const noexcept
	{
		return "Missinig Noterminal Define";
	}

	char const* lr1::production_redefine_error::what() const noexcept
	{
		return "Production Redefine Error";
	}

	char const* lr1::reduce_conflict_error::what() const noexcept
	{
		return "Reduce Conflict Error";
	}

	char const* lr1::operator_conflict_error::what() const noexcept
	{
		return "Operator Conflict Error";
	}

	lr1 lr1::create(
		storage_t start_symbol,
		std::vector<production_input> production,
		std::vector<ope_priority> priority
	)
	{
		std::vector<std::tuple<storage_t, storage_t, storage_t>> m_production;
		std::vector<table> m_table;
		production.push_back({ { lr1::start_symbol(), start_symbol }, {}, lr1::no_function_enum() });
		m_production.reserve(production.size() + 1);
		for (auto& ite : production)
			m_production.push_back({ ite.production[0], static_cast<storage_t>(ite.production.size()) - 1, ite.function_state });

		auto null_set = calculate_nullable_set(production);
		auto first_set = calculate_noterminal_first_set(production, null_set);

		std::vector<std::set<storage_t>> remove;
		remove.resize(production.size());

		{

			auto insert_set = [](std::set<storage_t>& output, const std::map<storage_t, std::set<storage_t>>& first_set, storage_t symbol) {
				if (is_terminal(symbol))
					output.insert(symbol);
				else {
					auto ite = first_set.find(symbol);
					assert(ite != first_set.end());
					output.insert(ite->second.begin(), ite->second.end());
				}
			};


			std::map<storage_t, std::set<storage_t>> remove_map = translate_operator_priority(priority);
			for (storage_t x = 0; x < production.size(); ++x)
			{
				auto& ref = production[x];
				assert(!ref.production.empty());

				{
					auto& ref = production[x].remove_forward;
					auto& tar = remove[x];
					for (auto& ite : ref)
					{
						if (is_terminal(ite))
							tar.insert(ite);
						else {
							auto find = first_set.find(ite);
							assert(find != first_set.end());
							tar.insert(find->second.begin(), find->second.end());
						}
					}
				}
				

				if (ref.production.size() >= 2)
				{
					auto symbol = *(ref.production.rbegin() + 1);
					auto ite = remove_map.find(symbol);
					if (ite != remove_map.end())
						remove[x].insert(ite->second.begin(), ite->second.end());
				}

				for (storage_t y = x + 1; y < production.size(); ++y)
				{
					auto& ref2 = production[y];
					assert(!ref.production.empty());
					if (ref.production[0] == ref2.production[0])
					{
						storage_t index = diff(ref.production, ref2.production);
						if (index < ref.production.size() && index < ref2.production.size())
							continue;
						else if (index == ref.production.size() && index == ref2.production.size())
							throw production_redefine_error(x, y);
						else {
							if (index < ref.production.size())
							{
								if (ref.production[index] != ref.production[0])
									insert_set(remove[x], first_set, ref.production[index]);
							}
							else {
								if (ref2.production[index] != ref2.production[0])
									insert_set(remove[y], first_set, ref2.production[index]);
							}
						}
					}
				}
			}
		}

		std::map<storage_t, std::set<storage_t>> production_map;

		assert(production.size() < std::numeric_limits<uint32_t>::max());

		{

			for (uint32_t index = 0; index < production.size(); ++index)
			{
				auto& ref = production[index];
				assert(ref.production.size() >= 1);
				production_map[ref.production[0]].insert(index);
			}
		}

		std::set<std::set<storage_t>> all_forward_set;


		std::map<std::set<production_element>, storage_t> state_map_mapping;
		std::vector<decltype(state_map_mapping)::iterator> stack;
		storage_t current_state = 0;
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

	lr1::operator lr1_storage() const
	{
		lr1_storage storage;
		storage.productions = m_production;
		for (auto& ite : m_table)
		{
			storage_t start = static_cast<storage_t>(storage.reduce_shift_table.size());
			storage_t reduce_count = 0;
			storage_t shift_count = 0;
			for (auto& ite2 : ite.m_reduce)
			{
				storage.reduce_shift_table.push_back(ite2);
				++reduce_count;
			}
			for (auto& ite2 : ite.m_shift)
			{
				storage.reduce_shift_table.push_back(ite2);
				++shift_count;
			}
			storage.nodes.push_back({start, reduce_count, shift_count});
		}
		return storage;
	}

	char const* lr1_processor::unacceptable_error::what() const noexcept
	{
		return "UnAcceptable Token";
	}

	bool lr1_processor::try_reduce(storage_t symbol, bool (*Function)(void* Func, travel input), void* data)
	{
		assert(!m_state_stack.empty());
		m_input_buffer.push_back({ symbol, token_index++ });
		while (!m_input_buffer.empty())
		{
			auto [input, token_index] = *m_input_buffer.rbegin();
			storage_t state = *m_state_stack.rbegin();
			auto [start, reduce_s, shift_s] = m_table_ref.nodes[state];
			bool Reduce = false;
			for (size_t i = 0; i < reduce_s; ++i)
			{
				auto [in, pro_index] = m_table_ref.reduce_shift_table[start + i];
				if (input == in)
				{
					Reduce = true;
					storage_t production_index = pro_index;
					assert(production_index < m_table_ref.productions.size());
					storage_t head_symbol;
					storage_t production_count;
					storage_t function_state;
					std::tie(head_symbol, production_count, function_state) = m_table_ref.productions[production_index];
					assert(m_state_stack.size() >= production_count);

					if (head_symbol != lr1::start_symbol())
					{
						assert(Function != nullptr && data != nullptr);
						travel input;
						input.symbol = head_symbol;
						input.noterminal.production_index = production_index;
						input.noterminal.production_count = production_count;
						input.noterminal.symbol_array = m_state_stack.data() + m_state_stack.size() - production_count;
						input.noterminal.function_enum = function_state;
						auto re = (*Function)(data, input);
						if (!re)
							return false;
						m_state_stack.resize(m_state_stack.size() - production_count);
						m_input_buffer.push_back({ head_symbol, 0 });
					}
					else
					{
						m_state_stack.resize(m_state_stack.size() - production_count);
						assert(m_state_stack.size() == 1);
						m_input_buffer.clear();
					}
					break;
				}
			}
			bool Shift = false;
			if (!Reduce)
			{
				for (size_t i = 0; i < shift_s; ++i)
				{
					auto [in, state] = m_table_ref.reduce_shift_table[start + reduce_s + i];
					if (input == in)
					{
						Shift = true;
						auto [sym, index] = *m_input_buffer.rbegin();
						if (lr1::is_terminal(sym))
						{
							travel input;
							input.symbol = sym;
							input.terminal.token_index = index;
							(*Function)(data, input);
						}
						m_input_buffer.pop_back();
						m_state_stack.push_back(state);
						break;
					}
				}
			}

			if (!(Reduce || Shift))
			{
				assert(!m_state_stack.empty());
				throw unacceptable_error{ input, *m_state_stack.rbegin() };
			}
		}
		return true;
	}

}