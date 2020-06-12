#include "parser_nfa.h"
#include "parser_lr0.h"
#include <optional>
#include <variant>
#include <string_view>
#include "parser_lr0.h"
namespace Potato::Parser::NFA
{

	auto Consumer::operator()(Table const& ref, std::u32string_view String)-> std::optional<Consumer::Travel>
	{
		std::vector<std::tuple<size_t, size_t, std::u32string_view>> search_stack;
		search_stack.push_back({ 0,0, String });
		std::optional<Travel> Acception;
		while (!search_stack.empty())
		{
			auto& [state, index, code] = *search_stack.rbegin();
			char32_t Input;
			if (code.empty())
				Input = 0;
			else
				Input = *code.begin();
			auto [s, c] = ref.Nodes[state];
			bool ForceBreak = false;
			while (!ForceBreak && index < c)
			{
				auto cur_index = index++;
				auto [Type, i1, i2] = ref.Edges[s + cur_index];
				switch (Type)
				{
				case Table::EdgeType::Comsume: {
					auto& edge_ref = ref.ComsumeEdge[i1];
					if (edge_ref.intersection_find({ Input, Input + 1 }))
					{
						search_stack.push_back({ i2, 0, {code.data() + 1, code.size() - 1} });
						ForceBreak = true;
					}
				} break;
				case Table::EdgeType::Acception: {
					search_stack.clear();
					search_stack.push_back({ i2, 0, code });
					Acception = { i1, {String.data(), String.size() - code.size()}, {String.data() + code.size(), String.size() - code.size()} };
					ForceBreak = true;
				} break;
				default: {assert(false); } break;
				}
			}
			if (!ForceBreak && index == c)
				search_stack.pop_back();
		}
		return Acception;
	}

	/*
	std::tuple<std::vector<size_t>, std::vector<Processer::Element>> Processer::operator()(Table const& table, std::u32string_view Input)
	{
		std::vector<Lr0::Symbol> Results;
		std::vector<Element> Elements;
		auto Ite = Input.begin();
		while (Ite != Input.end())
		{
			std::vector<std::tuple<size_t, size_t, std::u32string_view::iterator>> search_stack;
			search_stack.push_back({ 0,0, Input.begin() });
			std::optional<std::tuple<size_t, Processer::Element>> Acception;
			while (!search_stack.empty())
			{
				auto& [state, index, code] = *search_stack.rbegin();
				char32_t Input;
				if (code.empty())
					Input = 0;
				else
					Input = *code.begin();
				auto [s, c] = table.Nodes[state];
				bool ForceBreak = false;
				while (!ForceBreak && index < c)
				{
					auto cur_index = index++;
					auto [Type, i1, i2] = table.Edges[s + cur_index];
					switch (Type)
					{
					case Table::EdgeType::Comsume: {
						auto& edge_ref = table.ComsumeEdge[i1];
						if (edge_ref.intersection_find({ Input, Input + 1 }))
						{
							search_stack.push_back({ i2, 0, {code.data() + 1, code.size() - 1} });
							ForceBreak = true;
						}
					} break;
					case Table::EdgeType::Acception: {
						search_stack.clear();
						search_stack.push_back({ i2, 0, code });
						Acception = { i1, {LastCode.data(), LastCode.size() - code.size()} };
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
				auto Used = Acception->capture.size();
				LastCode = { LastCode.data() + Used, LastCode.size() - Used };
			}
			return Acception;
		}




		std::vector<std::tuple<size_t, size_t, std::u32string_view>> search_stack;
		search_stack.push_back({ 0,0, LastCode });
		std::optional<travel> Acception;
		while (!search_stack.empty())
		{
			auto& [state, index, code] = *search_stack.rbegin();
			char32_t Input;
			if (code.empty())
				Input = 0;
			else
				Input = *code.begin();
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
						search_stack.push_back({ i2, 0, {code.data() + 1, code.size() - 1} });
						ForceBreak = true;
					}
				} break;
				case nfa_storage::EdgeType::Acception: {
					search_stack.clear();
					search_stack.push_back({ i2, 0, code });
					Acception = { i1, {LastCode.data(), LastCode.size() - code.size()} };
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
			auto Used = Acception->capture.size();
			LastCode = { LastCode.data() + Used, LastCode.size() - Used };
		}
		return Acception;
	}
	*/


	enum class T
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
	};

	constexpr Lr0::Symbol operator*(T sym) { return Lr0::Symbol{ static_cast<size_t>(sym), Lr0::TerminalT{} }; };

	enum class NT
	{
		Statement,
		OrStatement,
		CharListStart,
		CharList,
		Expression,
	};

	constexpr Lr0::Symbol operator*(NT sym) { return Lr0::Symbol{ static_cast<size_t>(sym), Lr0::NoTerminalT{} }; };

	const Lr0::Table& rex_lr0()
	{
		static Lr0::Table rex_lr0 = Lr0::CreateTable(
			*NT::Statement,
			{
				{{*NT::Statement, *NT::Statement, *NT::Statement}, 0},
				{{*NT::Statement, *NT::Statement, *T::Or, *NT::Statement},{*NT::Expression}, 7},
				{{*NT::Statement, *NT::Expression}, 14},
				{{*NT::Expression, *T::ParenthesesLeft, *NT::Statement, *T::ParenthesesRight}},
				{{*NT::Expression, *T::ParenthesesLeft, *NT::Expression, *T::ParenthesesRight}, 1},
				{{*NT::Expression, *T::SquareBracketsLeft, *NT::CharList}, 2},
				{{*NT::Expression, *T::SquareBracketsLeft, *T::Not, *NT::CharList}, 3},

				{{*NT::Expression, *NT::Expression, *T::Mulity},{*T::Question}, 4},
				{{*NT::Expression, *NT::Expression, *T::Mulity, *T::Question}, 16},

				{{*NT::Expression, *NT::Expression, *T::Question},{*T::Question}, 5},
				{{*NT::Expression, *NT::Expression, *T::Question, *T::Question}, 17},
				{{*NT::Expression, *NT::Expression, *T::Add}, {*T::Question}, 6},
				{{*NT::Expression, *NT::Expression, *T::Add, *T::Question}, 18},

				{{*NT::Expression, *T::Char}, 8},
				{{*NT::Expression, *T::Point}, 19},
				{{*NT::Expression, *T::Min}, 15},
				{{*NT::CharListStart, *T::Min}, 9},
				{{*NT::CharListStart}, 10},

				{{*NT::CharListStart, *NT::CharListStart, *T::Char, *T::Min, *T::Char}, 11},
				{{*NT::CharListStart,*NT::CharListStart, *T::Char}, 12},
				{{*NT::CharListStart,*NT::CharListStart, *T::Point}, 21},
				{{*NT::CharList,*NT::CharListStart, *T::SquareBracketsRight}, 13},
				{{*NT::CharList,*NT::CharListStart, *T::Min, *T::SquareBracketsRight}, 22},
			}, {
				{{*T::Mulity, *T::Question, *T::Add}}, {{*T::Or}}
			}
			);
		return rex_lr0;
	}



	std::variant<T, Table::RangeSet> rex_element_lexer(std::u32string_view::const_iterator& begin, std::u32string_view::const_iterator end)
	{
		using range_set = Table::RangeSet;
		assert(begin != end);
		char32_t input = *(begin++);
		switch (input)
		{
		case U'-':return T::Min;
		case U'[': return T::SquareBracketsLeft;
		case U']':return  T::SquareBracketsRight;
		case U'(': return  T::ParenthesesLeft;
		case U')':return T::ParenthesesRight;
		case U'*': return T::Mulity;
		case U'?':return T::Question;
		case U'.': return T::Point;
		case U'|':return T::Or;
		case U'+':return T::Add;
		case U'^':return T::Not;
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
				tem |= range_set::range{ 127, 128 };
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

	std::tuple<std::vector<Lr0::Symbol>, std::vector<Table::RangeSet>> rex_lexer(std::u32string_view Input)
	{
		auto begin = Input.begin();
		auto end = Input.end();
		std::vector<Lr0::Symbol> Symbols;
		std::vector<Table::RangeSet> RangeSets;
		while (begin != end)
		{
			auto re = rex_element_lexer(begin, end);
			if(std::holds_alternative<T>(re))
			{
				Symbols.push_back(*std::get<T>(re));
				RangeSets.push_back({});
			}
			else {
				Symbols.push_back(*T::Char);
				RangeSets.push_back(std::move(std::get<Table::RangeSet>(re)));
			}
		}
		return { std::move(Symbols), std::move(RangeSets) };
	}

	struct nfa
	{
		struct unacceptable_rex_error : std::exception
		{
			std::u32string rex;
			size_t acception_state;
			size_t index;
			unacceptable_rex_error(std::u32string rex, size_t acception_state, size_t index) : rex(rex), acception_state(acception_state), index(index) {}
			char const* what() const noexcept override;
		};
		using range_set = Table::RangeSet;

		struct epsilon { size_t state; };
		struct comsume { size_t state; range_set require; };
		struct acception { size_t state; size_t acception_state; };

		using edge_t = std::variant<epsilon, comsume, acception>;

		static constexpr size_t no_accepted = std::numeric_limits<size_t>::max();

		struct node { std::vector<edge_t> edge; };

		nfa& append_rex(std::u32string_view rex, size_t accept_state);
		static nfa create_from_rexs(std::u32string_view const* input, size_t length = 0);
		static nfa create_from_rex(std::u32string_view Rex, size_t);
		const node& operator[](size_t index) const { return nodes[index]; }
		operator bool() const noexcept { return !nodes.empty(); }
		operator Table() const { return simplify(); }
		size_t start_state() const noexcept { return 0; }
		nfa(const nfa&) = default;
		nfa(nfa&&) = default;
		nfa& operator=(const nfa&) = default;
		nfa& operator=(nfa&&) = default;
		nfa() = default;
		node& operator[](size_t index) { return nodes[index]; }
		Table simplify() const;
		size_t back_construction(node&& n);
		size_t size() const noexcept { return nodes.size(); }
	private:
		std::vector<node> nodes;
	};

	size_t nfa::back_construction(node&& n)
	{
		size_t size = nodes.size();
		nodes.push_back(std::move(n));
		return size;
	}

	/*
	auto nfa_processer::operator()()-> std::optional<travel>
	{
		assert(*this);
		std::vector<std::tuple<size_t, size_t, std::u32string_view>> search_stack;
		search_stack.push_back({ 0,0, LastCode});
		std::optional<travel> Acception;
		while (!search_stack.empty())
		{
			auto& [state, index, code] = *search_stack.rbegin();
			char32_t Input;
			if (code.empty())
				Input = 0;
			else
				Input = *code.begin();
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
						search_stack.push_back({ i2, 0, {code.data() + 1, code.size() - 1} });
						ForceBreak = true;
					}
				} break;
				case nfa_storage::EdgeType::Acception: {
					search_stack.clear();
					search_stack.push_back({ i2, 0, code });
					Acception = { i1, {LastCode.data(), LastCode.size() - code.size()} };
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
			auto Used = Acception->capture.size();
			LastCode = { LastCode.data() + Used, LastCode.size() - Used };
		}
		return Acception;
	}
	*/
	/*
	auto nfa_lexer::operator()() noexcept -> std::optional<size_t>
	{
		assert(*this);
		auto re = processer->operator()();
		if (re)
		{
			processer_stack = travel( *re, line, index, total_index );
			total_index += re->capture.size();
			for (auto& ite : re->capture)
			{
				if (ite == U'\n')
				{
					index = 0;
					++line;
				}
				else
					++index;
			}
			return re->acception;
		}
		return std::nullopt;
	}
	*/

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
		try {
			auto [Symbols, Datas] = rex_lexer(rex);
			auto Hist = Lr0::Processer{}(rex_lr0(), Symbols.data(), Symbols.size());
			Lr0::Processer{}(rex_lr0(), Hist, [&](Lr0::Processer::Step tra, Lr0::Processer::Element* datas) -> std::any {
				if (!tra.IsTerminal())
				{
					switch (tra.reduce.mask)
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
						if (tra.reduce.mask == 15)
							ScopeStack.push_back({ U'-' });
						else if (tra.reduce.mask == 19)
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
					case 22: {
						assert(ScopeStack.size() >= 1);
						auto& i1 = *(ScopeStack.rbegin());
						i1 |= { U'-' };
					}break;
					case Lr0::ProductionInput::default_mask(): {} break;
					default: assert(false); break;
					}
				}
				return {};
			});
			assert(StateStack.size() == 1);
			auto end = result.back_construction({});
			auto [s1, s2] = *StateStack.rbegin();
			result[s2].edge.push_back(acception{ end, accept_state });
			auto& TopNode = result[0];
			TopNode.edge.push_back(epsilon{ s1 });
			return *this;
		}
		catch (Lr0::Error::unacceptable_symbol const& UAS)
		{
			throw;
			//throw unacceptable_rex_error(std::u32string(rex), accept_state, static_cast<size_t>(ite - rex.begin()));
		}
		
	}

	char const* nfa::unacceptable_rex_error::what() const noexcept
	{
		return "Unacceptable Regular Expression Error";
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
		for (size_t i = 0; i < count; ++i)
			result.append_rex(code[i], i);
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

	Table nfa::simplify() const
	{
		Table re;
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
						re.Edges.push_back({ Table::EdgeType::Comsume, comsume_index, ref.state});
					}
					else if (std::holds_alternative<acception>(ite2))
					{
						auto& ref = std::get<acception>(ite2);
						re.Edges.push_back({ Table::EdgeType::Acception, ref.acception_state, ref.state });
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

	Table CreateTable(std::u32string_view const* input, size_t input_length)
	{
		nfa Result = nfa::create_from_rexs(input, input_length);
		return Result.simplify();
	}
}