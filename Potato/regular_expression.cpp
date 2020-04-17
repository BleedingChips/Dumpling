#include "regular_expression.h"
#include "syntax.h"
#include <optional>
#include <variant>
#include <string_view>
namespace Potato::Rex
{
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

	const lr1& rex_lr1()
	{
		static lr1 rex_lr1 = lr1::create(
			*DFASymbol::Statement,
			{
				{{*SYM::Statement, *SYM::Statement, *SYM::Expression}, 0},
				{{*SYM::Statement, *SYM::Statement, *SYM::Or, *SYM::Statement}, 7},
				{{*SYM::Statement, *SYM::Expression}, 14},

				{{*SYM::Expression, *SYM::ParenthesesLeft, *SYM::Expression, *SYM::ParenthesesRight}, 1},

				{{*SYM::Expression, *SYM::SquareBracketsLeft, *SYM::CharList, *SYM::SquareBracketsRight}, 2},
				{{*SYM::Expression, *SYM::SquareBracketsLeft, *SYM::Not, *SYM::CharList, *SYM::SquareBracketsRight}, 3},

				{{*SYM::Expression, *SYM::Expression, *SYM::Mulity}, 4},
				{{*SYM::Expression, *SYM::Expression, *SYM::Mulity, *SYM::Question}, 16},

				{{*SYM::Expression, *SYM::Expression, *SYM::Question}, 5},
				{{*SYM::Expression, *SYM::Expression, *SYM::Question, *SYM::Question}, 17},
				{{*SYM::Expression, *SYM::Expression, *SYM::Add}, 6},
				{{*SYM::Expression, *SYM::Expression, *SYM::Add, *SYM::Question}, 18},

				{{*SYM::Expression, *SYM::Char}, 8},
				{{*SYM::Expression, *SYM::Min}, 15},
				{{*SYM::CharListStart, *SYM::Min}, 9},

				{{*SYM::CharListStart}, 10},
				{{*SYM::CharListStart, *SYM::CharListStart, *SYM::Char, *SYM::Min, *SYM::Char}, 11},
				{{*SYM::CharListStart,*SYM::CharListStart,*SYM::Char}, 12},
				{ {*SYM::CharList,*SYM::CharListStart}, 13},


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
			case U'D': { range_set Tem({ U'0', U'9' + 1 }); Tem = Tem.supplementary({ 1, std::numeric_limits<char32_t>::max() });  return Tem;  };
			case U'f': return range_set({ U'\f' });
			case U'n': return range_set({ U'\n' });
			case U'r': return range_set({ U'\r' });
			case U't': return range_set({ U'\t' });
			case U'v': return range_set({ U'\v' });
			case U's':
			{
				range_set tem({ U'\f', U'\r' + 1 });
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

	nfa nfa::create_from_rex(std::u32string_view Rex, size_t acception)
	{
		
		std::vector<std::tuple<size_t, size_t>> StateStack;
		std::optional<range_set> TemporaryRange;
		std::vector<Tool::range_set<char32_t>> ScopeStack;

		nfa result;
		result.back_construction({});

		lr1_process(rex_lr1(), [&](std::u32string_view::const_iterator& ite) -> std::optional<lr1::storage_t> {
			if (TemporaryRange)
			{
				ScopeStack.push_back(std::move(*TemporaryRange));
				TemporaryRange = std::nullopt;
			}

			if (ite != Rex.end())
			{
				auto Re = rex_lexer(ite, Rex.end());
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
				case 15: ScopeStack.push_back({ U'-' });
				case 2: case 8:{
					assert(ScopeStack.size() >= 1);
					auto Cur = std::move(*ScopeStack.rbegin());
					ScopeStack.pop_back();
					auto s1 = result.back_construction({});
					auto s2 = result.back_construction({});
					result[s1].edge.push_back(comsume{ s2, std::move(Cur) });
					StateStack.push_back({s1, s2});
				}break;
				case 3: {
					assert(ScopeStack.size() >= 1);
					auto Cur = std::move(*ScopeStack.rbegin());
					ScopeStack.pop_back();
					Cur = Cur.supplementary({ 1, std::numeric_limits<char32_t>::max() });
					auto s1 = result.back_construction({});
					auto s2 = result.back_construction({});
					result[s1].edge.push_back(comsume{ s2, std::move(Cur) });
					StateStack.push_back({ s1, s2 });
				}break;
				case 4: {
					auto s1 = result.back_construction({});
					auto s2 = result.back_construction({});
					assert(StateStack.size() >= 1);
					auto [t1, t2] = *StateStack.rbegin();
					StateStack.pop_back();
					result[t2].edge.push_back(epsilon{ t1 });
					result[s1].edge.push_back(epsilon{t1});
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
					break;
				}break;
				case 13: {} break;
				default: assert(false); break;
				}
			}

















			/*
			if (!tra.is_terminal())
			{
				switch (tra.noterminal.function_enum)
				{
				case 0:
				{
					assert(StateStack.size() >= 2);
					auto& L1 = *(StateStack.rbegin() + 1);
					auto L2 = *(StateStack.rbegin());
					auto re = nodes[std::get<1>(L1)].edge.push_back(epsilon{ std::get<0>(L2) });
					assert(re.second);
					L1 = { std::get<0>(L1), std::get<1>(L2) };
					StateStack.pop_back();
					break;
				}
				case 7:
				{
					auto [l1s, &l1] = nfa.push({});

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
					auto [s1, e1] = *(StateStack.rbegin() + 1);
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
					ScopeStack.push_back({ U'-' });
					break;
				}
				case 10:
				{
					ScopeStack.push_back({});
					break;
				}
				case 11:
				{
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
			*/
		}, Rex.begin());
		assert(StateStack.size() == 1);
		auto [s1, s2] = *StateStack.rbegin();
		result[0].edge.push_back(epsilon{ s1 });
		return result;
	}

	/*
	nfa_gen nfa_gen::create_from_rexs(std::u32string_view const* code, size_t count)
	{
		static lr1 DFALr1 = lr1::create(
			*DFASymbol::Statement,
			{
				{{*SYM::Statement, *SYM::Statement, *SYM::Expression}, 0},
				{{*SYM::Expression, *SYM::ParenthesesLeft, *SYM::Expression, *SYM::ParenthesesRight}, 1},
				{{*SYM::Expression, *SYM::SquareBracketsLeft, *SYM::CharList, *SYM::SquareBracketsRight}, 2},
				{{*SYM::Expression, *SYM::SquareBracketsLeft, *SYM::Not, *SYM::CharList, *SYM::SquareBracketsRight}, 3},
				{{*SYM::Expression, *SYM::Expression, *SYM::Mulity}, 4},
				{{*SYM::Expression, *SYM::Expression, *SYM::Mulity, *SYM::Question}, 16},

				{{*SYM::Expression, *SYM::Expression, *SYM::Question}, 5},
				{{*SYM::Expression, *SYM::Expression, *SYM::Question, *SYM::Question}, 17},
				{{*SYM::Expression, *SYM::Expression, *SYM::Add}, 6},
				{{*SYM::Expression, *SYM::Expression, *SYM::Add, *SYM::Question}, 18},
				{{*SYM::Expression, *SYM::Expression, *SYM::Or, *SYM::Expression}, 7},
				{{*SYM::Expression, *SYM::Char}, 8},
				{{*SYM::Expression, *SYM::Min}, 15},
				{{*SYM::CharListStart, *SYM::Min}, 9},

				{{*SYM::CharListStart}, 10},
				{{*SYM::CharListStart, *SYM::CharListStart, *SYM::Char, *SYM::Min, *SYM::Char}, 11},
				{{*SYM::CharListStart,*SYM::CharListStart,*SYM::Char}, 12},
				{ {*SYM::CharList,*SYM::CharListStart}, 13},

				{{*SYM::Statement, *SYM::Expression}, 14}
			}, {
				{*SYM::Mulity, *SYM::Question, *SYM::Add}, {*SYM::Or}
			}
			);

		std::vector<std::tuple<size_t, size_t>> StateStack;
		std::optional<range_set> TemporaryRange;
		std::vector<Tool::range_set<char32_t>> ScopeStack;

		nfa_epsilon_gen

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
					auto [s1, e1] = *(StateStack.rbegin() + 1);
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
					ScopeStack.push_back({ U'-' });
					break;
				}
				case 10:
				{
					ScopeStack.push_back({});
					break;
				}
				case 11:
				{
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
	}
	*/







}