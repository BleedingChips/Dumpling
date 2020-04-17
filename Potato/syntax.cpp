#include "syntax.h"
#include <assert.h>
#include <string>
#include <iostream>

struct generated_epsilon_nfa
{
	using range_set = Potato::Tool::range_set<char32_t>;
	struct Epsilon { size_t state; };
	struct FirstEpsilon { size_t state; };
	struct LookAheadPositiveAssert { size_t state; };
	struct Comsume { size_t state; range_set set; };
};



struct  epsilon_nfa_generated
{
	struct node
	{

	};
};

















namespace Potato
{

	/*
	using range_set = Tool::range_set<char32_t>;

	enum class DFASymbol : lr1::storage_t
	{
		Char = 0,
		Min, // -
		SquareBracketsLeft, //[
		SquareBracketsRight, // ]
		ParenthesesLeft, //(
		ParenthesesRight, //)
		Mulity, //*
		Question, // ?
		Point, //.
		Or, // |
		Add, // +
		Not, // ^


		Statement = lr1::noterminal_start(),
		CharListStart,
		CharList,
		Expression,
	};

	constexpr lr1::storage_t operator*(DFASymbol sym) { return static_cast<lr1::storage_t>(sym); }

	using SYM = DFASymbol;


	std::variant<Tool::range_set<char32_t>, SYM> HandleSingleChar(std::u32string_view::const_iterator& begin, std::u32string_view::const_iterator end)
	{
		assert(begin != end);
		char32_t input = *(begin++);
		switch (input)
		{
		case U'-':return SYM::Min;
		case U'[': return SYM::SquareBracketsLeft;
		case U']':return  SYM::SquareBracketsRight;
		case U'(': return  SYM::ParenthesesLeft;
		case U')':return SYM::ParenthesesRight;
		case U'*': return SYM::Mulity;
		case U'?':return SYM::Question;
		case U'.': return SYM::Point;
		case U'|':return SYM::Or;
		case U'+':return SYM::Add;
		case U'^':return SYM::Not;
		case U'\\':
		{
			assert(begin != end);
			input = *(begin++);
			switch (input)
			{
			case U'd': return range_set({U'0', U'9' + 1});
			case U'D': { range_set Tem({ U'0', U'9' + 1 }); Tem = Tem.supplementary({ 1, std::numeric_limits<char32_t>::max() });  return Tem;  };
			case U'f': return range_set({ U'\f' });
			case U'n': return range_set({ U'\n' });
			case U'r': return range_set({ U'\r' });
			case U't': return range_set({ U'\t'});
			case U'v': return range_set({U'\v'});
			case U's': 
			{
				range_set tem({U'\f', U'\r' + 1});
				tem |= U'\t';
				return tem;
			}
			case U'S':
			{
				range_set tem({ U'\f', U'\r' + 1 });
				tem |= U'\t';
				tem = tem.supplementary({ 1, std::numeric_limits<char32_t>::max() });
				return tem;
			}
			case U'w':
			{
				range_set tem({ U'a', U'z' + 1 });
				tem |= range_set({ U'A', U'Z' + 1 });
				tem |= range_set({ U'_', U'_' + 1 });
				return tem;
			}
			case U'W':
			{
				range_set tem({ U'a', U'z' + 1 });
				tem |= range_set({ U'A', U'Z' + 1 });
				tem |= range_set({ U'_', U'_' + 1 });
				tem = tem.supplementary({ 1, std::numeric_limits<char32_t>::max() });
				return tem;
			}
			case U'u':
			{
				assert(begin + 4 <= end);
				size_t index = 0;
				for (size_t i = 0; i < 4; ++i)
				{
					char32_t in = *(begin++);
					index *= 16;
					if (in >= U'0' && in <= U'9')
						index += in - U'0';
					else if (in >= U'a' && in <= U'f')
						index += in - U'a' + 10;
					else if (in >= U'A' && in <= U'F')
						index += in - U'A' + 10;
					else
						assert(false);
				}
				return range_set({ static_cast<char32_t>(index), static_cast<char32_t>(index) + 1 });
			}
			default:
				range_set tem({ input,input + 1 });
				return tem;
				break;
			}
			break;
		}
			
		default:
			return range_set(input);
			break;
		}
	}

	std::tuple<size_t, size_t> nfa::create_single_rex(std::u32string_view Rex)
	{

		static lr1 DFALr1 = lr1::create(
			*DFASymbol::Statement,
			{
				{{*SYM::Statement, *SYM::Statement, *SYM::Expression}, 0},
				{{*SYM::Expression, *SYM::ParenthesesLeft, *SYM::Expression, *SYM::ParenthesesRight}, 1},
				{{*SYM::Expression, *SYM::SquareBracketsLeft, *SYM::CharList, *SYM::SquareBracketsRight}, 2},
				{{*SYM::Expression, *SYM::SquareBracketsLeft, *SYM::Not, *SYM::CharList, *SYM::SquareBracketsRight}, 3},
				{{*SYM::Expression, *SYM::Expression, *SYM::Mulity}, 4},

				{{*SYM::Expression, *SYM::Expression, *SYM::Question}, 5},
				{{*SYM::Expression, *SYM::Expression, *SYM::Add}, 6},
				{{*SYM::Expression, *SYM::Expression, *SYM::Or, *SYM::Expression}, 7},
				{{*SYM::Expression, *SYM::Char}, 8},
				{{*SYM::Expression, *SYM::Min}, 15},
				{{*SYM::CharListStart, *SYM::Min}, 9},

				{{*SYM::CharListStart}, 10},
				{{*SYM::CharListStart, *SYM::CharListStart, *SYM::Char, *SYM::Min, *SYM::Char}, 11},
				{{*SYM::CharListStart,* SYM::CharListStart,* SYM::Char}, 12},
				{ {*SYM::CharList,* SYM::CharListStart}, 13},

				{{*SYM::Statement, *SYM::Expression}, 14}
			}, {
				{*SYM::Mulity, *SYM::Question, *SYM::Add}, {*SYM::Or}
			}
			);

		std::vector<std::tuple<size_t, size_t>> StateStack;
		std::optional<Tool::range_set<char32_t>> TemporaryRange;
		std::vector<Tool::range_set<char32_t>> ScopeStack;

		lr1_process(DFALr1, [&](std::u32string_view::const_iterator& ite) -> std::optional<lr1::storage_t> {
			if (TemporaryRange)
			{
				ScopeStack.push_back(std::move(*TemporaryRange));
				TemporaryRange = std::nullopt;
			}
				
			if (ite != Rex.end())
			{
				auto Re = HandleSingleChar(ite, Rex.end());
				if (std::holds_alternative<range_set>(Re))
				{
					TemporaryRange = std::move(std::get<range_set>(Re));
					return *SYM::Char;
				}
				else
					return *std::get<SYM>(Re);
			}
			else {
				return std::nullopt;
			}
		}, [&](lr1_processor::travel tra) {
			if (!tra.is_terminal())
			{
				switch (tra.noterminal.function_enum)
				{
				case 0:
				{
					assert(StateStack.size() >= 2);
					auto& L1 = *(StateStack.rbegin() + 1);
					auto L2 = *(StateStack.rbegin());
					auto re = total_node[std::get<1>(L1)].table_null_shift.insert(std::get<0>(L2));
					assert(re.second);
					L1 = { std::get<0>(L1), std::get<1>(L2) };
					StateStack.pop_back();
					break;
				}
				case 1: break;
				case 2:
				{
					assert(ScopeStack.size() >= 1);
					auto Char = std::move(*ScopeStack.rbegin());
					ScopeStack.pop_back();
					size_t n1_state = total_node.size();
					size_t n2_state = n1_state + 1;
					node n1;
					node n2;
					auto re = n1.table_shift.insert({ n2_state, {} });
					re.first->second |= Char;
					total_node.push_back(std::move(n1));
					total_node.push_back(std::move(n2));
					StateStack.push_back({ n1_state, n2_state });
					break;
				}
				case 3:
				{
					assert(ScopeStack.size() >= 1);
					auto Char = std::move(*ScopeStack.rbegin());
					Char.supplementary({ 1, std::numeric_limits<char32_t>::max() });
					ScopeStack.pop_back();
					size_t n1_state = total_node.size();
					size_t n2_state = n1_state + 1;
					node n1;
					node n2;
					auto re = n1.table_shift.insert({ n2_state, {} });
					re.first->second |= Char;
					total_node.push_back(std::move(n1));
					total_node.push_back(std::move(n2));
					StateStack.push_back({ n1_state, n2_state });
					break;
				}
				case 4:
				{
					assert(StateStack.size() != 0);
					auto [s, e] = *StateStack.rbegin();
					auto& n1 = total_node[s];
					auto& n2 = total_node[e];
					n1.table_null_shift.insert({ e });
					n2.table_null_shift.insert({ s });
					break;
				}
				case 5:
				{
					assert(StateStack.size() != 0);
					auto [s, e] = *StateStack.rbegin();
					auto& n1 = total_node[s];
					n1.table_null_shift.insert({ e });
					break;
				}
				case 6:
				{
					assert(StateStack.size() != 0);
					auto [s, e] = *StateStack.rbegin();
					auto& n2 = total_node[e];
					n2.table_null_shift.insert({ s });
					break;
				}
				case 7:
				{
					assert(StateStack.size() >= 2);
					auto [s1, e1] = *(StateStack.rbegin()+1);
					auto [s2, e2] = *(StateStack.rbegin());
					StateStack.resize(StateStack.size() - 2);
					size_t n1_state = total_node.size();
					size_t n2_state = n1_state + 1;
					node n1; node n2;
					n1.table_null_shift.insert({ s1 });
					n1.table_null_shift.insert({ s2 });
					total_node[e1].table_null_shift.insert({ n2_state });
					total_node[e2].table_null_shift.insert({ n2_state });
					total_node.push_back(std::move(n1));
					total_node.push_back(std::move(n2));
					StateStack.push_back({ n1_state, n2_state });
					break;
				}
				case 15:
					ScopeStack.push_back({ U'-' });
				case 8:
				{
					assert(ScopeStack.size() >= 1);
					auto re = std::move(*ScopeStack.rbegin());
					ScopeStack.pop_back();
					size_t n1_state = total_node.size();
					size_t n2_state = n1_state + 1;
					node n1; node n2;
					n1.table_shift.insert({ n2_state, std::move(re) });
					total_node.push_back(std::move(n1));
					total_node.push_back(std::move(n2));
					StateStack.push_back({ n1_state, n2_state });
					break;
				}
				case 9:
				{
					ScopeStack.push_back({U'-'});
					break;
				}
				case 10:
				{
					ScopeStack.push_back({});
					break;
				}
				case 11:
				{
					assert(ScopeStack.size() >=3);
					auto i1 = std::move(*(ScopeStack.rbegin() + 1));
					auto i2 = std::move(*(ScopeStack.rbegin()));
					assert(i1.size() >= 1 && i2.size() >= 1);
					auto min = i1[0].left;
					auto big = i2[i2.size() - 1].right;
					assert(min < big);
					auto& i3 = *(ScopeStack.rbegin() + 2);
					i3 |= range_set::range{ min, big };
					ScopeStack.resize(ScopeStack.size() - 2);
					break;
				}
				case 12:
				{
					assert(ScopeStack.size() >= 2);
					auto& i1 = *(ScopeStack.rbegin() + 1);
					auto i2 = std::move(*(ScopeStack.rbegin()));
					i1 |= i2;
					ScopeStack.pop_back();
					break;
				}
				case 13: case 14: break;
				default:
					assert(false);
					break;
				}
			}
		}, Rex.begin());
		assert(StateStack.size() == 1);
		return StateStack[0];
	}

	nfa nfa::create_from_rex(std::u32string_view const* input, size_t length)
	{
		nfa result;
		result.total_node.push_back(node{});
		for (size_t index = 0; index < length; ++index)
		{
			auto [s, e] = result.create_single_rex(input[index]);
			result[e].is_accept = index;
			result[result.start_state()].table_null_shift.insert(s);
		}
		return std::move(result);
	}

	std::set<size_t> nfa::search_null_state_set(size_t start) const
	{
		std::set<size_t> current_set;
		std::vector<size_t> stack;
		stack.push_back(start);
		while (!stack.empty())
		{
			size_t cur = *stack.rbegin();
			stack.pop_back();
			current_set.insert(cur);
			for (auto& ite : total_node[cur].table_null_shift)
			{
				auto re = current_set.insert(ite);
				if (re.second)
					stack.push_back(ite);
			}
		}
		return current_set;
	}

	struct nfa_null_set_search
	{
		const nfa& ref;
		nfa_null_set_search(const nfa& ref) : ref(ref) {}
		using set_iterator_t = std::set<std::set<size_t>>::const_iterator;
		std::set<std::set<size_t>>::const_iterator search(size_t head)
		{
			std::set<size_t> result;
			result.insert(head);
			std::vector<size_t> search_stack;
			search_stack.push_back(head);
			while (!search_stack.empty())
			{
				size_t s = *search_stack.begin();
				search_stack.pop_back();
				auto ite = nfa_null_set_head_to_set.find(s);
				if (ite != nfa_null_set_head_to_set.end())
				{
					for (auto& ite : *(ite->second))
						result.insert(ite);
				}
				else {
					auto& node = ref[s];
					for (auto& ite : node.table_null_shift)
					{
						auto re = result.insert(ite);
						if (re.second)
							search_stack.push_back(ite);
					}
				}
			}
			auto re = nfa_null_set.insert(std::move(result));
			if (re.second)
				nfa_null_set_head_to_set.insert({ head, re.first });
			return re.first;
		}
		std::set<std::set<size_t>> nfa_null_set;
		std::map<size_t, decltype(nfa_null_set)::const_iterator> nfa_null_set_head_to_set;
	};

	dfa dfa::create_from_rexs(std::u32string_view const* rexs, size_t length)
	{

		using Tool::RangeLocation;

		nfa nfa_result = nfa::create_from_rex(rexs, length);
		nfa_null_set_search null_search(nfa_result);

		dfa dfa_result;
		auto& ref = dfa_result.nodes;

		std::map<std::set<size_t>, size_t> nfa_set_to_dfa;
		std::vector<std::set<size_t>> dfa_state_to_nfa;
		auto ite = null_search.search(nfa_result.start_state());
		dfa_state_to_nfa.push_back(*ite);
		std::vector<size_t> search_stack;
		ref.push_back(node{});
		search_stack.push_back(0);
		while (!search_stack.empty())
		{
			size_t cur_state = *search_stack.rbegin();
			search_stack.pop_back();
			const auto& TargetSet = dfa_state_to_nfa[cur_state];
			std::map<std::set<size_t>, range_set> mapping;

			for (auto& ite : TargetSet)
			{
				for (auto& ite2 : nfa_result[ite].table_shift)
				{
					auto re = mapping.insert({ {ite2.first}, ite2.second });
					if (!re.second)
						re.first->second |= ite2.second;
				}
			}

			{
				std::vector<decltype(mapping)::iterator> search_back;

				for (auto ite = mapping.begin(); ite != mapping.end(); ++ite)
					search_back.push_back(ite);

				while (!search_back.empty())
				{
					auto ite = std::move(*search_back.rbegin());
					search_back.pop_back();
					if (ite != mapping.end())
					{
						size_t start = 0;
						size_t end = search_back.size();
						while (start < end)
						{
							auto tar_ite = search_back[start];
							if (tar_ite != mapping.end())
							{
								auto& ref = tar_ite->second;
								auto res = ite->second.intersection_cull(ref);
								if (!res.empty())
								{
									std::set<size_t> tar = ite->first;
									tar.insert(tar_ite->first.begin(), tar_ite->first.end());
									auto re = mapping.insert({ std::move(tar), std::move(res) });
									assert(re.second);
									search_back.push_back(re.first);
									if (ref.empty())
									{
										mapping.erase(tar_ite);
										search_back[start] = mapping.end();
									}
									if (ite->second.empty())
									{
										mapping.erase(ite);
										break;
									}
								}
							}
							++start;
						}
					}
				}
			}

			{
				decltype(mapping) tem = std::move(mapping);
				for (auto& ite : tem)
				{
					std::set<size_t> tar = ite.first;
					for (auto& ite2 : ite.first)
					{
						auto ite3 = null_search.search(ite2);
						tar.insert(ite3->begin(), ite3->end());
					}
					auto re = mapping.emplace(std::move(tar), std::move(ite.second));
					if (!re.second)
						re.first->second |= ite.second;
				}

				for (auto& ite : mapping)
				{
					size_t state = ref.size();
					auto re = nfa_set_to_dfa.emplace(ite.first, state);
					if (re.second)
					{
						ref.push_back(node{});
						search_stack.push_back(state);
						dfa_state_to_nfa.push_back(ite.first);
					}
					auto re2 = ref[cur_state].shift.emplace(re.first->second, std::move(ite.second));
					assert(re2.second);
				}
			}
		}
		
		for (size_t i = 0; i < dfa_state_to_nfa.size(); ++i)
		{
			std::optional<size_t> accept;
			for (auto ite : dfa_state_to_nfa[i])
			{
				if (nfa_result[ite].is_accept.has_value())
				{
					if (accept.has_value())
						accept = std::max(*nfa_result[ite].is_accept, *accept);
					else
						accept = *nfa_result[ite].is_accept;
				}
			}
			dfa_result[i].is_accept = accept;
		}


		{
			std::map<size_t, std::set<size_t>> accept_state_map;
			for (size_t i = 0; i < dfa_result.size(); ++i)
			{
				auto& ref = dfa_result[i];
				if (ref.shift.empty())
				{
					assert(ref.is_accept);
					auto re = accept_state_map.insert({ *ref.is_accept, {} });
					re.first->second.insert(i);
				}
			}

			std::vector<std::tuple<std::set<size_t>, std::optional<size_t>, size_t>> search_stack;
			for (auto& ite : accept_state_map)
				search_stack.push_back({ std::move(ite.second), ite.first, 0 });

			struct OptionalLess
			{
				bool operator()(const std::optional<size_t>& i1, const std::optional<size_t>& i2) const noexcept
				{
					return !i1 && i2 || (i1 && i2 && *i1 < *i2);
				}
			};

			while (!search_stack.empty())
			{
				auto [tar_stack, accept, tar_index] = std::move(*search_stack.rbegin());
				search_stack.pop_back();
				if (tar_stack.size() >= 2)
				{
					std::map<std::optional<size_t>, std::set<size_t>, OptionalLess> stack;
					size_t tar_state = ref.size();
					for (size_t i = 0; i < ref.size(); ++i)
					{
						auto& ref_node = ref[i];
						std::optional<range_set> shift_range;
						for (auto ite = ref_node.shift.begin(); ite != ref_node.shift.end();)
						{
							auto re = tar_stack.find(ite->first);
							if (re != tar_stack.end())
							{
								if (shift_range)
									*shift_range |= ite->second;
								else
									shift_range = std::move(ite->second);
								ite = ref_node.shift.erase(ite);
							}
							else
								++ite;
						}
						if (shift_range)
						{
							ref_node.shift.insert({ tar_state , std::move(*shift_range) });
							if (ref_node.shift.size() == 1)
							{
								auto re = stack.insert({ ref_node.is_accept, {} });
								re.first->second.insert(i);
							}
						}
					}
					node cur;
					cur.is_accept = accept;
					std::optional<range_set> shift_range;
					for (auto& ite : tar_stack)
					{
						auto& ref_node = ref[ite];
						if (ref_node.shift.size() == 1)
						{
							if (shift_range)
								*shift_range |= ref_node.shift.begin()->second;
							else
								shift_range = std::move(ref_node.shift.begin()->second);
						}
						else if (ref_node.shift.size() > 1)
							assert(false);
						ref_node.is_accept = std::nullopt;
						ref_node.shift.clear();
					}

					if (shift_range)
						cur.shift.insert({ tar_index, std::move(*shift_range) });
					ref.push_back(std::move(cur));
					for (auto& ite : stack)
					{
						if (ite.second.size() >= 2)
							search_stack.push_back({ std::move(ite.second), ite.first, tar_state });
					}
				}
			}

			auto ite = ref.begin();
			for (auto ite = ref.begin();  ite != ref.end(); ++ite)
			{
				if (!ite->is_accept.has_value() && ite->shift.empty())
				{
					auto ite2 = ite + 1;
					bool Swap = false;
					for (auto ite2 = ite + 1; ite2 != ref.end(); ++ite2)
					{
						if (ite2->is_accept || !ite2->shift.empty())
						{
							std::swap(*ite, *ite2);
							size_t cur_index = ite2 - ref.begin();
							size_t swap_index = ite - ref.begin();
							for (auto& ite3 : ref)
							{
								auto find = ite3.shift.find(cur_index);
								if (find != ite3.shift.end())
								{
									auto range = std::move(find->second);
									ite3.shift.erase(find);
									auto re = ite3.shift.insert({ swap_index, std::move(range) });
									assert(re.second);
								}
							}
							Swap = true;
							break;
						}
					}
					if (!Swap){
						ref.erase(ite, ref.end());
						break;
					}
				}
			}
		}
		return dfa_result;
	}

	auto dfa_processer::comsume_analyze(const dfa& table, std::u32string_view input) ->std::optional<travel>
	{
		std::optional<travel> result;
		size_t state = table.start_symbol();
		auto ite_length = 0;
		do
		{
			auto& dfa_node = table[state];
			if (dfa_node.is_accept)
				result = travel{ *dfa_node.is_accept, std::u32string_view(input.data(), ite_length) };
			if (ite_length < input.size())
			{
				bool Change = false;
				for (auto& ite2 : dfa_node.shift)
				{
					if (ite2.second.intersection_find({ input[ite_length], input[ite_length] + 1 }))
					{
						state = ite2.first;
						++ite_length;
						Change = true;
						break;
					}
				}
				if (!Change)
					break;
			}
			else
				break;
		} while (true);
		return result;
	}
	*/

}



namespace
{
	using namespace Potato;
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
							state.push_back({ ite.m_index.m_production_element_index, production[ite.m_index.m_production_index].production, *ite.m_forward_set });
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
		std::vector<production_input> production,
		std::vector<ope_priority> priority
	)
	{
		std::vector<std::tuple<storage_t, storage_t, storage_t>> m_production;
		std::vector<table> m_table;
		production.push_back({ { lr1::start_symbol(), start_symbol }, 0, {} });
		m_production.reserve(production.size() + 1);
		for (auto& ite : production)
			m_production.push_back({ ite.production[0], static_cast<uint32_t>(ite.production.size()) - 1, ite.function_state });

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

				for (uint32_t y = x + 1; y < production.size(); ++y)
				{
					auto& ref2 = production[y];
					assert(!ref.production.empty());
					if (ref.production[0] == ref2.production[0])
					{
						uint32_t index = diff(ref.production, ref2.production);
						if (index < ref.production.size() && index < ref2.production.size())
							continue;
						else if (index == ref.production.size() && index == ref2.production.size())
							throw same_production{ x, y, production[x].production };
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

		std::map<uint32_t, std::set<uint32_t>> production_map;

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

	std::vector<storage_t> lr1::serialization()
	{
		std::vector<storage_t> data;
		data.push_back(static_cast<storage_t>(m_production.size()));
		for (auto& ite : m_production)
		{
			auto [i1, i2, i3] = ite;
			data.push_back(i1);
			data.push_back(i2);
			data.push_back(i3);
		}
		data.push_back(static_cast<storage_t>(m_table.size()));
		for (auto& ite : m_table)
		{
			data.push_back(static_cast<storage_t>(ite.m_shift.size()));
			for (auto& ite2 : ite.m_shift)
			{
				data.push_back(ite2.first);
				data.push_back(ite2.second);
			}
			data.push_back(static_cast<storage_t>(ite.m_reduce.size()));
			for (auto& ite2 : ite.m_reduce)
			{
				data.push_back(ite2.first);
				data.push_back(ite2.second);
			}
		}
		return std::move(data);
	}

	lr1 lr1::unserialization(const storage_t* data, size_t length)
	{
		size_t ite = 0;
		std::vector<std::tuple<storage_t, storage_t, storage_t>> m_production;
		{
			size_t size = data[ite++];
			for (size_t i = 0; i < size; ++i)
			{
				storage_t i1 = data[ite + i * 3];
				storage_t i2 = data[ite + i * 3 + 1];
				storage_t i3 = data[ite + i * 3 + 2];
				m_production.push_back({ i1, i2, i3 });
			}
			ite += size * 3;
		}
		size_t size = data[ite++];
		std::vector<table> tab_v;
		for (size_t x = 0; x < size; ++x)
		{
			table tab;
			{
				size_t size_2 = data[ite++];
				for (size_t i = 0; i < size_2; ++i)
				{
					std::pair<storage_t, storage_t> i2 = { data[ite + i * 2], data[ite + i * 2 + 1] };
					tab.m_shift.insert(i2);
				}
				ite += size_2 * 2;
			}
			{
				size_t size_2 = data[ite++];
				for (size_t i = 0; i < size_2; ++i)
				{
					std::pair<storage_t, storage_t> i2 = { data[ite + i * 2], data[ite + i * 2 + 1] };
					tab.m_reduce.insert(i2);
				}
				ite += size_2 * 2;
			}
			tab_v.push_back(std::move(tab));
		}
		
		return lr1{std::move(m_production), std::move(tab_v)};
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
				storage_t function_state;
				std::tie(head_symbol, production_count, function_state) = m_table_ref.m_production[production_index];
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
					(*Function)(data, input);
					m_state_stack.resize(m_state_stack.size() - production_count);
					m_input_buffer.push_back({ head_symbol, 0 });
				}
				else
				{
					m_state_stack.resize(m_state_stack.size() - production_count);
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
					input.terminal.token_index = index;
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
			lr1_ast ast{ input.symbol, input.terminal.token_index };
			ast_buffer.push_back(std::move(ast));
		}
		else {
			auto size = input.noterminal.production_count;
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