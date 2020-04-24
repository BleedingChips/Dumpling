#include "lexical.h"
#include "syntax.h"
#include <optional>
#include <variant>
#include <string_view>
namespace Potato::Lexical
{
	using namespace Potato::Syntax;

	auto nfa_comsumer::comsume() -> travel
	{
		assert(*this);
		std::vector<std::tuple<size_t, size_t, size_t>> search_stack;
		search_stack.push_back({ 0,0, 0 });
		std::optional<std::tuple<size_t, size_t>> Acception;
		while (!search_stack.empty())
		{
			auto& [state, index, ite_shift] = *search_stack.rbegin();
			char32_t Input;
			if (ite_shift == length)
				Input = 0;
			else
				Input = ite[ite_shift];
			auto [s, c] = ref.Nodes[state];
			bool ForceBreak = false;
			while (!ForceBreak && index < c)
			{
				auto cur_index = index++;
				auto [Type, i1, i2] = ref.Edges[s + cur_index];
				switch (Type)
				{
				case nfa_storage::EdgeType::Comsume: {
					auto& edge_ref = ref.ComsumeEdge[i1];
					if (edge_ref.intersection_find({ Input, Input + 1 }))
					{
						search_stack.push_back({ i2, 0, ite_shift + 1 });
						ForceBreak = true;
					}
				} break;
				case nfa_storage::EdgeType::Acception: {
					search_stack.clear();
					search_stack.push_back({ i2, 0, ite_shift });
					Acception = { i1, ite_shift };
					ForceBreak = true;
				} break;
				default: {assert(false); } break;
				}
			}
			if (!ForceBreak && index == c)
				search_stack.pop_back();
		}
		if (Acception)
		{
			auto [i1, i2] = *Acception;
			travel result{ i1, std::u32string_view(ite,  i2), charactor_index,  line_count };
			auto count = i2;
			assert(count <= length);
			ite += count;
			length -= count;
			used += count;
			for(auto ite : result.capture_string)
				if (ite == U'\n')
				{
					charactor_index = 0;
					++line_count;
				}
				else
					++charactor_index;
			return result;
		}
		else
			throw error{ ite, charactor_index, line_count };
	}

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

	size_t nfa::back_construction(node&& n)
	{
		size_t size = nodes.size();
		nodes.push_back(std::move(n));
		return size;
	}

	const lr1_storage& rex_lr1()
	{
		static lr1_storage rex_lr1 = lr1::create(
			*DFASymbol::Statement,
			{
				{{*SYM::Statement, *SYM::Statement, *SYM::Statement}, 0},
				{{*SYM::Statement, *SYM::Statement, *SYM::Or, *SYM::Statement},{*SYM::Expression}, 7},
				{{*SYM::Statement, *SYM::Expression}, 14},

				{{*SYM::Expression, *SYM::ParenthesesLeft, *SYM::Expression, *SYM::ParenthesesRight}, 1},

				{{*SYM::Expression, *SYM::SquareBracketsLeft, *SYM::CharList}, 2},
				{{*SYM::Expression, *SYM::SquareBracketsLeft, *SYM::Not, *SYM::CharList}, 3},

				{{*SYM::Expression, *SYM::Expression, *SYM::Mulity},{*SYM::Question}, 4},
				{{*SYM::Expression, *SYM::Expression, *SYM::Mulity, *SYM::Question}, 16},

				{{*SYM::Expression, *SYM::Expression, *SYM::Question},{*SYM::Question}, 5},
				{{*SYM::Expression, *SYM::Expression, *SYM::Question, *SYM::Question}, 17},
				{{*SYM::Expression, *SYM::Expression, *SYM::Add}, {*SYM::Question}, 6},
				{{*SYM::Expression, *SYM::Expression, *SYM::Add, *SYM::Question}, 18},

				{{*SYM::Expression, *SYM::Char}, 8},
				{{*SYM::Expression, *SYM::Point}, 19},
				{{*SYM::Expression, *SYM::Min}, 15},
				{{*SYM::CharListStart, *SYM::Min}, 9},
				{{*SYM::CharListStart}, 10},

				{{*SYM::CharListStart, *SYM::CharListStart, *SYM::Char, *SYM::Min, *SYM::Char}, 11},
				{{*SYM::CharListStart,*SYM::CharListStart, *SYM::Char}, 12},
				{{*SYM::CharListStart,*SYM::CharListStart, *SYM::Point}, 21},
				{{*SYM::CharList,*SYM::CharListStart, *SYM::SquareBracketsRight}, 13},


			}, {
				{*SYM::Mulity, *SYM::Question, *SYM::Add}, {*SYM::Or}
			}
			);
		return rex_lr1;
	}

	std::variant<nfa::range_set, SYM> rex_lexer(std::u32string_view::const_iterator& begin, std::u32string_view::const_iterator end)
	{
		using range_set = nfa::range_set;
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
			case U'd': return range_set({ U'0', U'9' + 1 });
			case U'D': { 
				range_set Tem({ U'0', U'9' + 1 }); 
				range_set total({ 1, std::numeric_limits<char32_t>::max() });
				Tem.intersection_cull(total);
				return total;
			};
			case U'f': return range_set({ U'\f' });
			case U'n': return range_set({ U'\n' });
			case U'r': return range_set({ U'\r' });
			case U't': return range_set({ U'\t' });
			case U'v': return range_set({ U'\v' });
			case U's':
			{
				range_set tem({ 1, 33 });
				tem |= 127;
				return tem;
			}
			case U'S':
			{
				range_set tem({ 1, 33 });
				tem |= range_set::range{127, 128};
				range_set total = range_set::range{ 1, std::numeric_limits<char32_t>::max() };
				auto re = tem.intersection_cull(total);
				return total;
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
				range_set total({ 1, std::numeric_limits<char32_t>::max() });
				tem.intersection_cull(total);
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

	nfa& nfa::append_rex(std::u32string_view rex, size_t accept_state)
	{
		auto& result = *this;
		bool SelfIndex = false;
		if (result)
			SelfIndex = true;
		else
			result.back_construction({});

		std::vector<std::tuple<size_t, size_t>> StateStack;
		std::optional<range_set> TemporaryRange;
		std::vector<Tool::range_set<char32_t>> ScopeStack;
		bool UseMinAtEnd = false;
		auto ite = rex.begin();
		lr1_process(rex_lr1(), [&]() -> std::optional<lr1::storage_t> {
			if (TemporaryRange)
			{
				ScopeStack.push_back(std::move(*TemporaryRange));
				TemporaryRange = std::nullopt;
			}

			if (ite != rex.end())
			{
				auto Re = rex_lexer(ite, rex.end());
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
				case 0: {
					assert(StateStack.size() >= 2);
					auto& L1 = *(StateStack.rbegin() + 1);
					auto L2 = *(StateStack.rbegin());
					result[std::get<1>(L1)].edge.push_back(epsilon{ std::get<0>(L2) });
					L1 = { std::get<0>(L1), std::get<1>(L2) };
					StateStack.pop_back();
				} break;
				case 7: {
					assert(StateStack.size() >= 2);
					auto [s1, e1] = *(StateStack.rbegin() + 1);
					auto [s2, e2] = *(StateStack.rbegin());
					StateStack.resize(StateStack.size() - 2);
					auto i1s = result.back_construction({});
					auto i2s = result.back_construction({});
					result[i1s].edge.push_back(epsilon{ s1 });
					result[i1s].edge.push_back(epsilon{ s2 });
					result[e1].edge.push_back(epsilon{ i2s });
					result[e2].edge.push_back(epsilon{ i2s });
					StateStack.push_back({ i1s, i2s });
					break;
				}break;
				case 1:case 14: break;
				case 2: case 8: case 15: case 19: {
					if(tra.noterminal.function_enum == 15)
						ScopeStack.push_back({ U'-' });
					else if (tra.noterminal.function_enum == 19)
					{
						range_set R = range_set::range{ 1, U'\n' };
						R |= range_set::range{ U'\n' + 1, std::numeric_limits<char32_t>::max() };
						ScopeStack.push_back(std::move(R));
					}
					assert(ScopeStack.size() >= 1);
					auto Cur = std::move(*ScopeStack.rbegin());
					ScopeStack.pop_back();
					auto s1 = result.back_construction({});
					auto s2 = result.back_construction({});
					result[s1].edge.push_back(comsume{ s2, std::move(Cur) });
					StateStack.push_back({ s1, s2 });
				}break;
				case 3: {
					assert(ScopeStack.size() >= 1);
					auto Cur = std::move(*ScopeStack.rbegin());
					ScopeStack.pop_back();
					range_set Total({ 1, std::numeric_limits<char32_t>::max() });
					Cur.intersection_cull(Total);
					auto s1 = result.back_construction({});
					auto s2 = result.back_construction({});
					result[s1].edge.push_back(comsume{ s2, std::move(Total) });
					StateStack.push_back({ s1, s2 });
				}break;
				case 4: {
					auto s1 = result.back_construction({});
					auto s2 = result.back_construction({});
					assert(StateStack.size() >= 1);
					auto [t1, t2] = *StateStack.rbegin();
					StateStack.pop_back();
					result[t2].edge.push_back(epsilon{ t1 });
					result[s1].edge.push_back(epsilon{ t1 });
					result[s1].edge.push_back(epsilon{ s2 });
					result[t2].edge.push_back(epsilon{ s2 });
					StateStack.push_back({ s1, s2 });
				}break;
				case 16: {
					auto s1 = result.back_construction({});
					auto s2 = result.back_construction({});
					assert(StateStack.size() >= 1);
					auto [t1, t2] = *StateStack.rbegin();
					StateStack.pop_back();
					result[s1].edge.push_back(epsilon{ s2 });
					result[s1].edge.push_back(epsilon{ t1 });
					result[t2].edge.push_back(epsilon{ s2 });
					result[t2].edge.push_back(epsilon{ t1 });
					StateStack.push_back({ s1, s2 });
				}break;
				case 5: {
					auto s1 = result.back_construction({});
					auto s2 = result.back_construction({});
					assert(StateStack.size() >= 1);
					auto [t1, t2] = *StateStack.rbegin();
					StateStack.pop_back();
					result[s1].edge.push_back(epsilon{ t1 });
					result[s1].edge.push_back(epsilon{ s2 });
					result[t2].edge.push_back(epsilon{ s2 });
					StateStack.push_back({ s1, s2 });
					break;
				} break;
				case 17: {
					auto s1 = result.back_construction({});
					auto s2 = result.back_construction({});
					assert(StateStack.size() >= 1);
					auto [t1, t2] = *StateStack.rbegin();
					StateStack.pop_back();
					result[s1].edge.push_back(epsilon{ s2 });
					result[s1].edge.push_back(epsilon{ t1 });
					result[t2].edge.push_back(epsilon{ s2 });
					StateStack.push_back({ s1, s2 });
				}break;
				case 6: {
					auto s1 = result.back_construction({});
					auto s2 = result.back_construction({});
					assert(StateStack.size() >= 1);
					auto [t1, t2] = *StateStack.rbegin();
					StateStack.pop_back();
					result[s1].edge.push_back(epsilon{ t1 });
					result[t2].edge.push_back(epsilon{ t1 });
					result[t2].edge.push_back(epsilon{ s2 });
					StateStack.push_back({ s1, s2 });
				}break;
				case 18: {
					auto s1 = result.back_construction({});
					auto s2 = result.back_construction({});
					assert(StateStack.size() >= 1);
					auto [t1, t2] = *StateStack.rbegin();
					StateStack.pop_back();
					result[s1].edge.push_back(epsilon{ t1 });
					result[t2].edge.push_back(epsilon{ s2 });
					result[t2].edge.push_back(epsilon{ t1 });
					StateStack.push_back({ s1, s2 });
				}break;
				case 9: {
					ScopeStack.push_back({ U'-' });
				}break;
				case 10: ScopeStack.push_back({}); break;
				case 11: {
					assert(ScopeStack.size() >= 3);
					auto i1 = std::move(*(ScopeStack.rbegin() + 1));
					auto i2 = std::move(*(ScopeStack.rbegin()));
					assert(i1.size() >= 1 && i2.size() >= 1);
					auto min = i1[0].left;
					auto big = i2[i2.size() - 1].right;
					assert(min < big);
					auto& i3 = *(ScopeStack.rbegin() + 2);
					i3 |= range_set::range{ min, big };
					ScopeStack.resize(ScopeStack.size() - 2);
				}break;
				case 12: {
					assert(ScopeStack.size() >= 2);
					auto& i1 = *(ScopeStack.rbegin() + 1);
					auto i2 = std::move(*(ScopeStack.rbegin()));
					i1 |= i2;
					ScopeStack.pop_back();
				}break;
				case 21: {
					assert(ScopeStack.size() >= 1);
					range_set R = range_set::range{ 1, U'\n' };
					R |= range_set::range{ U'\n' + 1, std::numeric_limits<char32_t>::max() };
					*ScopeStack.rbegin() |= R;
				}break;
				case 13: {} break;
				default: assert(false); break;
				}
			}
		});
		assert(StateStack.size() == 1);
		auto end = result.back_construction({});
		auto [s1, s2] = *StateStack.rbegin();
		result[s2].edge.push_back(acception{ end, accept_state});
		result[0].edge.push_back(epsilon{ s1 });
		return *this;
	}

	nfa nfa::create_from_rex(std::u32string_view rex, size_t accept_state)
	{
		nfa result;
		result.append_rex(rex, accept_state);
		return result;
	}


	nfa nfa::create_from_rexs(std::u32string_view const* code, size_t count)
	{
		nfa result;
		if (count > 0)
		{
			do
			{
				--count;
				result.append_rex(code[count], count);
			} while (count != 0);
		}
		return std::move(result);
	}

	std::tuple<std::vector<std::vector<size_t>>, std::vector<nfa::edge_t>, std::vector<nfa::edge_t>> translate(const std::vector<nfa::edge_t>& input)
	{
		bool MeetAcception = false;
		std::vector<nfa::edge_t> result;
		std::vector<nfa::edge_t> o_result;
		std::map<std::vector<size_t>, size_t> mapping;
		nfa::range_set UsedRange;
		for (auto ite = input.begin(); ite != input.end();)
		{
			if (std::holds_alternative<nfa::acception>(*ite))
			{
				if (!MeetAcception)
				{
					auto& acc = std::get<nfa::acception>(*ite);
					MeetAcception = true;
					auto find_re = mapping.insert({ {acc.state}, mapping.size() });
					result.push_back(nfa::acception{ find_re.first->second, acc.acception_state });
				}
				++ite;
			}
			else if (std::holds_alternative<nfa::comsume>(*ite))
			{
				auto ite2 = ite + 1;
				for (; ite2 != input.end(); ++ite2)
				{
					if (!std::holds_alternative<nfa::comsume>(*ite2))
						break;
				}
				std::map<std::vector<size_t>, nfa::range_set> Mapping;
				for (auto ite3 = ite; ite3 != ite2; ++ite3)
				{
					auto& ref = std::get<nfa::comsume>(*ite3);
					auto cur_range = ref.require;
					cur_range -= UsedRange;
					for (auto& ite4 : Mapping)
					{
						if (cur_range.empty())
							break;
						auto re = cur_range.intersection_cull(ite4.second);
						if (!re.empty())
						{
							std::vector<size_t> states = ite4.first;
							states.push_back(ref.state);
							Mapping.insert({ std::move(states), std::move(re) });
						}
					}
					if (!cur_range.empty())
						Mapping.insert({ {ref.state}, std::move(cur_range) });
				}
				for (auto& ite : Mapping)
				{
					if (!ite.second.empty())
					{
						auto find = mapping.insert({ ite.first, mapping.size() });
						if (!MeetAcception)
							result.push_back(nfa::comsume{ find.first->second, std::move(ite.second) });
						else
							o_result.push_back(nfa::comsume{ find.first->second, std::move(ite.second) });
					}
				}
				ite = ite2;
			}
			else assert(false);
		}
		std::vector<std::vector<size_t>> re2;
		re2.resize(mapping.size());
		for (auto& ite : mapping)
			re2[ite.second] = ite.first;
		return { std::move(re2), std::move(result), std::move(o_result) };
	}

	std::tuple<std::vector<size_t>, std::vector<nfa::edge_t>> search_epsilon(const nfa& input, std::vector<size_t> head)
	{
		std::vector<nfa::edge_t> edges;
		std::set<size_t> head_set;
		std::vector<size_t> head_vec;
		std::vector<std::tuple<size_t, size_t>> search_stack;
		for (auto ite = head.rbegin(); ite != head.rend(); ++ite)
		{
			assert(input.size() > *ite);
			search_stack.push_back({ *ite, 0 });
		}
		nfa::range_set comsumed_input;
		std::optional<size_t> MeetAcception;
		while (!search_stack.empty())
		{
			auto& [state, index] = *search_stack.rbegin();
			if (!MeetAcception || (state > MeetAcception.value()))
			{
				if (index == 0)
					head_vec.push_back(state);
			}
			else{
				search_stack.pop_back();
				continue;
			}
			auto& node = input[state];
			bool Add = false;
			while (index < node.edge.size())
			{
				auto& ite = node.edge[index++];
				if (std::holds_alternative<nfa::epsilon>(ite))
				{
					auto& edge = std::get<nfa::epsilon>(ite);
					auto re = head_set.insert(edge.state);
					if (re.second)
					{
						Add = true;
						search_stack.push_back({ edge.state, 0 });
					}
					break;
				}
				else if (std::holds_alternative<nfa::comsume>(ite))
				{
					edges.push_back(ite);
				}
				else if (std::holds_alternative<nfa::acception>(ite))
				{
					MeetAcception = state;
					edges.push_back(ite);
				}
				else assert(false);
			}
			if (!Add && index >= node.edge.size())
				search_stack.pop_back();
		}
		//auto [ref_state, edges_r, edges_ro] = translate(edges);
		return { std::move(head_vec), std::move(edges) };
	}

	nfa_storage nfa::simplify() const
	{
		nfa_storage re;
		if (*this)
		{
			nfa result;
			std::map<std::vector<size_t>, size_t> old_to_new;
			auto [zero_set, zero_edge] = search_epsilon(*this, { 0 });
			old_to_new.insert({ zero_set,old_to_new.size() });
			result.back_construction({});
			std::vector<std::tuple<size_t, std::vector<edge_t>>> search_stack;
			search_stack.push_back({ 0, std::move(zero_edge) });
			while (!search_stack.empty())
			{
				auto [cur_set, cur_edges] = std::move(*search_stack.rbegin());
				search_stack.pop_back();
				auto [ref_state, edges, edges_o] = translate(cur_edges);
				std::vector<size_t> mapping;
				mapping.resize(ref_state.size());
				for (size_t i= 0; i < ref_state.size(); ++i)
				{
					auto [find_set, find_edge] = search_epsilon(*this, ref_state[i]);
					auto re = old_to_new.insert({ std::move(find_set), old_to_new.size() });
					mapping[i] = re.first->second;
					if (re.second)
					{
						result.back_construction({});
						search_stack.push_back({ re.first->second, find_edge });
					}
				}
				auto& ref = result[cur_set];
				std::optional<size_t> Accept;
				for (auto& ite : edges)
				{
					if (std::holds_alternative<comsume>(ite))
					{
						assert(!Accept);
						auto ref2 = std::move(std::get<comsume>(ite));
						ref.edge.push_back(comsume{ mapping [ref2.state], std::move(ref2.require)});
					}
					else if (std::holds_alternative<acception>(ite))
					{
						assert(!Accept);
						
						auto ref2 = std::move(std::get<acception>(ite));
						Accept = mapping[ref2.state];
						ref.edge.push_back(acception{ *Accept, ref2.acception_state });
					}
				}
				if (Accept)
				{
					auto& ref = result[*Accept];
					if (!edges_o.empty())
					{
						for (auto& ite : edges_o)
						{
							if (std::holds_alternative<comsume>(ite))
							{
								auto ref2 = std::move(std::get<comsume>(ite));
								ref.edge.push_back(comsume{ mapping[ref2.state], std::move(ref2.require) });
							}
							else if (std::holds_alternative<acception>(ite))
							{
								assert(false);
							}
						}
					}
				}
			}
			for (auto& ite : result.nodes)
			{
				size_t start = re.Edges.size();
				size_t count = 0;
				for (auto& ite2 : ite.edge)
				{
					if (std::holds_alternative<comsume>(ite2))
					{
						auto& ref = std::get<comsume>(ite2);
						size_t comsume_index = re.ComsumeEdge.size();
						re.ComsumeEdge.push_back(std::move(ref.require));
						re.Edges.push_back({ nfa_storage::EdgeType::Comsume, comsume_index, ref.state});
					}
					else if (std::holds_alternative<acception>(ite2))
					{
						auto& ref = std::get<acception>(ite2);
						re.Edges.push_back({ nfa_storage::EdgeType::Acception, ref.acception_state, ref.state });
					}
					else
						assert(false);
					++count;
				}
				re.Nodes.push_back({start, count});
			}
		}
		return re;
	}
}