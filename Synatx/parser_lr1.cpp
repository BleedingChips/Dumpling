#include "parser_lr1.h"
#include <set>
#include <map>
#include <algorithm>
namespace Potato::Lr1
{


	/*
	processer::history processer::operator()(const table& table, symbol const* token_array, size_t index)
	{
		std::vector<step> steps;
		struct search_element
		{
			size_t steps_index;
			size_t token_index;
			std::vector<size_t> state_stack;
		};
		std::vector<search_element> respond_stack;
		respond_stack.push_back({ 0, 0, {0} });
		while (!respond_stack.empty())
		{
			auto& [step_index, token_index, stack] = *respond_stack.rbegin();

		}
	}*/
























	std::set<symbol> calculate_nullable_set(const std::vector<production_input>& production)
	{
		std::set<symbol> result;
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
						symbol symbol = ite.production[index];
						if (symbol.is_terminal() || result.find(symbol) == result.end())
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

	std::map<symbol, std::set<symbol>> calculate_noterminal_first_set(
		const std::vector<production_input>& production,
		const std::set<symbol>& nullable_set
	)
	{
		std::map<symbol, std::set<symbol>> result;
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
					if (target.is_terminal())
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

	std::pair<std::set<symbol>, bool> calculate_production_first_set(
		std::vector<symbol>::const_iterator begin, std::vector<symbol>::const_iterator end,
		const std::set<symbol>& nullable_set,
		const std::map<symbol, std::set<symbol>>& first_set
	)
	{
		std::set<symbol> temporary;
		for (auto ite = begin; ite != end; ++ite)
		{
			if (ite->is_terminal())
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
					throw Error::noterminal_production_undefine{ *ite };
			}
		}
		return { std::move(temporary), true };
	}

	std::set<symbol> calculate_production_first_set_forward(
		std::vector<symbol>::const_iterator begin, std::vector<symbol>::const_iterator end,
		const std::set<symbol>& nullable_set,
		const std::map<symbol, std::set<symbol>>& first_set,
		const std::set<symbol>& forward
	)
	{
		auto result = calculate_production_first_set(begin, end, nullable_set, first_set);
		if (result.second)
			result.first.insert(forward.begin(), forward.end());
		return std::move(result.first);
	}

	std::vector<std::set<symbol>> calculate_productions_first_set(
		const std::multimap<symbol, std::vector<symbol>>& production,
		const std::set<symbol>& nullable_set,
		const std::map<symbol, std::set<symbol>>& first_set_noterminal
	)
	{
		std::vector<std::set<symbol>> temporary;
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
		size_t m_production_index;
		size_t m_production_element_index;
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
		std::set<std::set<symbol>>::iterator m_forward_set;
		bool operator<(const production_element& pe) const
		{
			return compress_less(m_index, pe.m_index, &(*m_forward_set), &(*pe.m_forward_set));
		}
		bool operator==(const production_element& pe) const
		{
			return m_index == pe.m_index && m_forward_set == pe.m_forward_set;
		}
	};

	using temporary_state_map = std::map<production_index, std::set<std::set<symbol>>::iterator>;

	bool insert_temporary_state_map(production_index index, temporary_state_map& handled, std::set<std::set<symbol>>::iterator ite, std::set<std::set<symbol>>& total_forward_set)
	{
		auto find_res = handled.insert({ index, ite });
		if (find_res.second)
			return true;
		else {
			if (find_res.first->second != ite)
			{
				std::set<symbol> new_set = *find_res.first->second;
				bool change = false;
				for (auto& ite : *ite)
					change = new_set.insert(ite).second || change;
				find_res.first->second = total_forward_set.insert(new_set).first;
				return change;
			}
		}
		return false;
	}

	std::set<symbol> remove_forward_set(std::set<symbol> current, size_t production_index, const std::vector<std::set<symbol>>& remove_forward)
	{
		for (auto& ite : remove_forward[production_index])
		{
			assert(ite.is_terminal());
			current.erase(ite);
		}
		return std::move(current);
	}

	std::set<production_element> search_direct_mapping(
		temporary_state_map handled,
		std::set<std::set<symbol>>& total_forward,
		const std::map<symbol, std::set<size_t>>& production_map,
		const std::vector<production_input>& production,
		const std::set<symbol>& nullable_set,
		const std::map<symbol, std::set<symbol>>& symbol_first_set,
		const std::vector<std::set<symbol>>& remove_forward
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
			auto& prod = production[pi.m_production_index];
			size_t ei = pi.m_production_element_index + 1;
			assert(prod.production.size() >= ei);
			if (prod.production.size() > ei)
			{
				symbol target_symbol = prod.production[ei];
				if (!target_symbol.is_terminal())
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
						throw Error::noterminal_production_undefine{ target_symbol };
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

	std::tuple<std::map<symbol, std::set<production_element>>, std::map<symbol, std::set<size_t>>> search_shift_and_reduce(
		const std::set<production_element>& input,
		std::set<std::set<symbol>>& total_forward,
		const std::map<symbol, std::set<size_t>>& production_map,
		const std::vector<production_input>& production,
		const std::set<symbol>& nullable_set,
		const std::map<symbol, std::set<symbol>>& symbol_first_set,
		const std::vector<std::set<symbol>>& remove_forward
	)
	{
		std::map<symbol, temporary_state_map> temporary_shift;
		std::map<symbol, std::set<size_t>> reduce;
		assert(!input.empty());
		for (auto& ite : input)
		{
			size_t pi = ite.m_index.m_production_index;
			assert(production.size() > pi);
			auto& prod = production[ite.m_index.m_production_index].production;
			size_t ei = ite.m_index.m_production_element_index + 1;
			assert(prod.size() >= ei);
			if (ei == prod.size())
			{
				for (auto& ite2 : *ite.m_forward_set)
				{
					auto re = reduce.insert({ ite2, {pi} });
					if (!re.second)
						re.first->second.insert({ pi });
				}
			}
			else {
				symbol target_symbol = prod[ei];
				auto cur = ite;
				cur.m_index.m_production_element_index += 1;
				auto& ref = temporary_shift[target_symbol];
				insert_temporary_state_map(cur.m_index, ref, cur.m_forward_set, total_forward);
			}
		}
		std::map<symbol, std::set<production_element>> shift;
		for (auto& ite : temporary_shift)
		{
			auto re = shift.insert({ ite.first, search_direct_mapping(std::move(ite.second), total_forward, production_map, production, nullable_set, symbol_first_set, remove_forward) });
			assert(re.second);
		}
		return { shift, reduce };
	}



	std::map<symbol, std::set<symbol>> translate_operator_priority(
		const std::vector<ope_priority>& priority
	)
	{
		std::map<symbol, std::set<symbol>> ope_priority;
		size_t index = 0;
		for (size_t index = 0; index < priority.size(); ++index)
		{
			std::set<symbol> current_remove;
			size_t target = index;
			auto& [opes, left] = priority[index];
			if (!left)
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
					throw Error::operator_priority_conflict{ ite, re.first->first };
			}
		}
		return std::move(ope_priority);
	}

	size_t diff(const std::vector<symbol>& l1, const std::vector<symbol>& l2)
	{
		size_t index = 0;
		while (l1.size() > index && l2.size() > index)
		{
			if (l1[index] != l2[index])
				break;
			++index;
		}
		return index;
	}

	table create(
		symbol start_symbol,
		std::vector<production_input> production,
		std::vector<ope_priority> priority
	)
	{
		std::vector<table::production> m_production;
		std::vector<table> m_table;
		production.push_back({ { symbol::start(), start_symbol } });
		m_production.reserve(production.size() + 1);
		for (auto& ite : production)
			m_production.push_back({ ite.production[0], ite.production.size() - 1, ite.function_mask });

		auto null_set = calculate_nullable_set(production);
		auto first_set = calculate_noterminal_first_set(production, null_set);

		std::vector<std::set<symbol>> remove;
		remove.resize(production.size());

		{

			std::map<symbol, std::set<symbol>> remove_map = translate_operator_priority(priority);
			for (size_t x = 0; x < production.size(); ++x)
			{
				auto& ref = production[x];
				assert(!ref.production.empty());

				{
					auto& ref = production[x].remove_forward;
					auto& tar = remove[x];
					for (auto& ite : ref)
					{
						if (ite.is_terminal())
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

				for (size_t y = x + 1; y < production.size(); ++y)
				{
					auto& ref2 = production[y];
					assert(!ref.production.empty());
					if (ref.production[0] == ref2.production[0])
					{
						size_t index = diff(ref.production, ref2.production);
						if (index < ref.production.size() && index < ref2.production.size())
							continue;
						else if (index == ref.production.size() && index == ref2.production.size())
							throw Error::production_redefine{ production[x].production, x, production[x].function_mask, y, production[y].function_mask };
						else {
							auto insert_set = [](std::set<symbol>& output, const std::map<symbol, std::set<symbol>>& first_set, symbol symbol) {
								if (symbol.is_terminal())
									output.insert(symbol);
								else {
									auto ite = first_set.find(symbol);
									if (ite == first_set.end())
										throw Error::noterminal_production_undefine{ symbol };
									output.insert(ite->second.begin(), ite->second.end());
								}
							};

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

		std::map<symbol, std::set<size_t>> production_map;

		assert(production.size() < std::numeric_limits<uint32_t>::max());

		{

			for (size_t index = 0; index < production.size(); ++index)
			{
				auto& ref = production[index];
				assert(ref.production.size() >= 1);
				production_map[ref.production[0]].insert(index);
			}
		}

		std::set<std::set<symbol>> all_forward_set;


		std::map<std::set<production_element>, size_t> state_map_mapping;
		std::vector<decltype(state_map_mapping)::iterator> stack;
		size_t current_state = 0;
		{
			auto re = all_forward_set.insert({ symbol::eof() }).first;
			temporary_state_map temmap{ {production_index{production.size() - 1, 0}, re} };
			auto result = search_direct_mapping(std::move(temmap), all_forward_set, production_map, production, null_set, first_set, remove);
			auto result2 = state_map_mapping.insert({ std::move(result), current_state });
			assert(result2.second);
			++current_state;
			stack.push_back(result2.first);
		}

		struct table_temporary
		{
			std::map<symbol, size_t> m_shift;
			std::map<symbol, size_t> m_reduce;
		};

		std::map<size_t, std::vector<table::respond>> temporary_table;

		while (!stack.empty())
		{
			auto ite = *stack.rbegin();
			stack.pop_back();
			auto [shift_r, reduce_r] = search_shift_and_reduce(ite->first, all_forward_set, production_map, production, null_set, first_set, remove);
			std::map<symbol, size_t> shift;
			for (auto& ite2 : shift_r)
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
			std::vector<table::respond> all_respond;
			for (auto& ite3 : reduce_r)
			{
				for (auto& ite4 : ite3.second)
					all_respond.push_back(table::respond{ RespondType::Reduce, ite3.first, ite4});
			}
			for(auto& ite3 : shift)
				all_respond.push_back(table::respond{ RespondType::Shift, ite3.first, ite3.second });
			auto re = temporary_table.insert({ ite->second, std::move(all_respond)});
			assert(re.second);
		}


		table result;
		result.productions = std::move(m_production);
		for (auto& ite : temporary_table)
		{
			result.nodes.push_back({ result.responds.size(), ite.second.size() });
			std::sort(ite.second.begin(), ite.second.end(), [](table::respond const& r1, table::respond const& r2) -> bool {
				return r1.require_token < r2.require_token || (r1.require_token == r2.require_token) && (r1.type > r2.type || (r1.type == r2.type && r1.state_or_index > r2.state_or_index));
			});
			result.responds.insert(result.responds.begin(), ite.second.begin(), ite.second.end());

			assert(result.nodes.size() == ite.first + 1);
		}
		return std::move(result);
	}
}