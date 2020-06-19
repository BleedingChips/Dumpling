#include "parser_nfa.h"
#include "parser_lr0.h"
#include <optional>
#include <variant>
#include <string_view>
#include "parser_lr0.h"
namespace Potato::Parser::NFA
{

	std::optional<MarchElement> Consume(Table const& Ref, std::u32string_view String)
	{
		std::vector<std::tuple<size_t, size_t, std::u32string_view>> search_stack;
		search_stack.push_back({ 0,0, String });
		std::optional<MarchElement> Acception;
		while (!search_stack.empty())
		{
			auto& [state, index, code] = *search_stack.rbegin();
			char32_t Input;
			if (code.empty())
				Input = 0;
			else
				Input = *code.begin();
			auto [s, c] = Ref.Nodes[state];
			bool ForceBreak = false;
			while (!ForceBreak && index < c)
			{
				auto cur_index = index++;
				auto [Type, i1, i2] = Ref.Edges[s + cur_index];
				switch (Type)
				{
				case static_cast<size_t>(Table::EdgeType::Comsume): {
					auto& edge_ref = Ref.ComsumeEdge[i1];
					if (edge_ref.intersection_find({ Input, Input + 1 }))
					{
						search_stack.push_back({ i2, 0, {code.data() + 1, code.size() - 1} });
						ForceBreak = true;
					}
				} break;
				case static_cast<size_t>(Table::EdgeType::Acception): {
					search_stack.clear();
					search_stack.push_back({ i2, 0, code });
					Acception = MarchElement{ i1, {String.data(), String.size() - code.size()}, code };
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

	std::vector<MarchElement> Process(Table const& Ref, std::u32string_view String)
	{
		std::vector<MarchElement> Result;
		while (String.size() != 0)
		{
			auto Re = Consume(Ref, String);
			if (Re)
			{
				Result.push_back(*Re);
				String = Re->last_string;
			}
			else
				throw Error::UnAccaptableString{std::u32string(String)};
		}
		return std::move(Result);
	}

	std::optional<DocumenetMarchElement> DecumentComsume(Table const& Ref, std::u32string_view String, Location& Loc)
	{
		auto Re = Consume(Ref, String);
		if (Re)
		{
			DocumenetMarchElement Result{ *Re, Loc };
			for (auto& ite : Re->capture)
			{
				++Loc.total_index;
				++Loc.start_index;
				if (ite == U'\n')
				{
					Loc.start_index = 0;
					++Loc.line;
				}
			}
			return Result;
		}
		return std::nullopt;
	}

	std::vector<DocumenetMarchElement> DecumentProcess(Table const& Ref, std::u32string_view String)
	{
		Location Loc;
		std::vector<DocumenetMarchElement> Result;
		while (String.size() != 0)
		{
			auto Re = DecumentComsume(Ref, String, Loc);
			if (Re)
			{
				Result.push_back(*Re);
				String = Re->march.last_string;
			}
			else
				throw Error::UnAccaptableString{ std::u32string(String) };
		}
		return std::move(Result);
	}


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
		Or, // |
		Add, // +
		Not, // ^
	};

	constexpr Lr0::Symbol operator*(T sym) { return Lr0::Symbol{ static_cast<size_t>(sym), Lr0::TerminalT{} }; };

	enum class NT
	{
		Statement,
		ExpressionStatement,
		LeftOrStatement,
		RightOrStatement,
		OrStatement,
		CharList,
		CharListNotable,
		Expression,
		NoGreedAppend,
	};

	constexpr Lr0::Symbol operator*(NT sym) { return Lr0::Symbol{ static_cast<size_t>(sym), Lr0::NoTerminalT{} }; };

	const Lr0::Table& rex_lr0()
	{
		static Lr0::Table rex_lr0 = Lr0::CreateTable(
			*NT::Statement,
			{
				Lr0::ProductionInput({*NT::Statement, *NT::ExpressionStatement}, 0),
				Lr0::ProductionInput({*NT::Statement, *NT::OrStatement}, 0),
				Lr0::ProductionInput({*NT::LeftOrStatement, *NT::LeftOrStatement, *NT::Expression}, 1),
				Lr0::ProductionInput({*NT::LeftOrStatement, *NT::Expression}, 0),
				Lr0::ProductionInput({*NT::RightOrStatement, *NT::Expression, *NT::RightOrStatement}, 1),
				Lr0::ProductionInput({*NT::RightOrStatement, *NT::Expression}, 0),
				Lr0::ProductionInput({*NT::OrStatement, *NT::LeftOrStatement, *T::Or, *NT::RightOrStatement}, 2),
				Lr0::ProductionInput({*NT::Expression, *T::ParenthesesLeft, *NT::OrStatement, *T::ParenthesesRight}, 3),
				Lr0::ProductionInput({*NT::Expression, *T::ParenthesesLeft, *NT::Expression, *T::ParenthesesRight}, 3),
				Lr0::ProductionInput({*NT::CharList, *T::SquareBracketsLeft, *NT::CharListNotable, *T::Min}, 3),
				Lr0::ProductionInput({*NT::CharList, *T::SquareBracketsLeft, *NT::CharListNotable}, 4),
				Lr0::ProductionInput({*NT::CharList, *NT::CharList, *T::Char}, 5),
				Lr0::ProductionInput({*NT::CharList, *NT::CharList, *T::Char, *T::Min, *T::Char}, 6),
				Lr0::ProductionInput({*NT::Expression, *NT::CharList, *T::Min, *T::SquareBracketsRight}, 7),
				Lr0::ProductionInput({*NT::Expression, *NT::CharList, *T::SquareBracketsRight}, 8),
				Lr0::ProductionInput({*NT::NoGreedAppend, *T::Question}, 0),
				Lr0::ProductionInput({*NT::NoGreedAppend}, {*T::Question}, Lr0::ProductionInput::default_mask()),
				Lr0::ProductionInput({*NT::Expression, *T::Char}, 12),
				Lr0::ProductionInput({*NT::Expression, *T::Min}, 12),
				Lr0::ProductionInput({*NT::Expression, *T::Not}, 12),
				Lr0::ProductionInput({*NT::Expression, *NT::Expression, *T::Mulity, *NT::NoGreedAppend}, {*T::Question}, 9),
				Lr0::ProductionInput({*NT::Expression, *NT::Expression, *T::Add, *NT::NoGreedAppend}, {*T::Question}, 10),
				Lr0::ProductionInput({*NT::Expression, *NT::Expression, *T::Question, *NT::NoGreedAppend}, {*T::Question}, 11),
				Lr0::ProductionInput({*NT::ExpressionStatement, *NT::ExpressionStatement, *NT::Expression}, 1),
				Lr0::ProductionInput({*NT::ExpressionStatement, *NT::Expression}, 0),
			{{*NT::CharListNotable, *T::Not}, 13},
			{{*NT::CharListNotable}, 14},
			}, {}
		);
		return rex_lr0;
	}

	std::tuple<T, Table::RangeSet> RexLexerTranslater(std::u32string_view::const_iterator& begin, std::u32string_view::const_iterator end)
	{
		using RangeSet = Table::RangeSet;
		assert(begin != end);
		char32_t input = *(begin++);
		switch (input)
		{
		case U'-':return { T::Min, RangeSet({input, input + 1}) };
		case U'[': return { T::SquareBracketsLeft, RangeSet({input, input + 1}) };
		case U']':return  {T::SquareBracketsRight, RangeSet({input, input + 1}) };
		case U'(': return  {T::ParenthesesLeft , RangeSet({input, input + 1}) };
		case U')':return {T::ParenthesesRight, RangeSet({input, input + 1}) };
		case U'*': return {T::Mulity, RangeSet({input, input + 1}) };
		case U'?':return {T::Question, RangeSet({input, input + 1}) };
		case U'.': return { T::Char, RangeSet({1, std::numeric_limits<char32_t>::max()}) };
		case U'|':return {T::Or, RangeSet({input, input + 1}) };
		case U'+':return {T::Add, RangeSet({input, input + 1}) };
		case U'^':return {T::Not, RangeSet({input, input + 1}) };
		case U'\\':
		{
			assert(begin != end);
			input = *(begin++);
			switch (input)
			{
			case U'd': return { T::Char, RangeSet({ U'0', U'9' + 1 }) };
			case U'D': {
				RangeSet Tem({ U'0', U'9' + 1 });
				RangeSet total({ 1, std::numeric_limits<char32_t>::max() });
				Tem.intersection_cull(total);
				return { T::Char, std::move(total) };
			};
			case U'f': return { T::Char, RangeSet({ U'\f' }) };
			case U'n': return { T::Char, RangeSet({ U'\n' }) };
			case U'r': return { T::Char, RangeSet({ U'\r' }) };
			case U't': return { T::Char, RangeSet({ U'\t' }) };
			case U'v': return { T::Char, RangeSet({ U'\v' }) };
			case U's':
			{
				RangeSet tem({ 1, 33 });
				tem |= 127;
				return { T::Char, tem };
			}
			case U'S':
			{
				RangeSet tem({ 1, 33 });
				tem |= RangeSet::range{ 127, 128 };
				RangeSet total = RangeSet::range{ 1, std::numeric_limits<char32_t>::max() };
				auto re = tem.intersection_cull(total);
				return { T::Char, total };
			}
			case U'w':
			{
				RangeSet tem({ U'a', U'z' + 1 });
				tem |= RangeSet({ U'A', U'Z' + 1 });
				tem |= RangeSet({ U'_', U'_' + 1 });
				return { T::Char, tem };
			}
			case U'W':
			{
				RangeSet tem({ U'a', U'z' + 1 });
				tem |= RangeSet({ U'A', U'Z' + 1 });
				tem |= RangeSet({ U'_', U'_' + 1 });
				RangeSet total({ 1, std::numeric_limits<char32_t>::max() });
				tem.intersection_cull(total);
				return { T::Char, tem };
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
				return { T::Char, RangeSet({ static_cast<char32_t>(index), static_cast<char32_t>(index) + 1 }) };
			}
			default:
				RangeSet tem({ input,input + 1 });
				return { T::Char, tem };
				break;
			}
			break;
		}

		default:
			return { T::Char, RangeSet(input) };
			break;
		}
	}

	std::tuple<std::vector<Lr0::Symbol>, std::vector<Table::RangeSet>> RexLexer(std::u32string_view Input)
	{
		auto begin = Input.begin();
		auto end = Input.end();
		std::vector<Lr0::Symbol> Symbols;
		std::vector<Table::RangeSet> RangeSets;
		while (begin != end)
		{
			auto [Sym, Rs] = RexLexerTranslater(begin, end);
			Symbols.push_back(*Sym);
			RangeSets.push_back(Rs);
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

	nfa& nfa::append_rex(std::u32string_view rex, size_t accept_state)
	{
		auto& result = *this;
		bool SelfIndex = false;
		if (result)
			SelfIndex = true;
		else
			result.back_construction({});
		bool NotAble = false;
		using RangeSet = Table::RangeSet;
		try {
			auto [Symbols, Datas] = RexLexer(rex);
			auto Hist = Lr0::Process(rex_lr0(), Symbols.data(), Symbols.size());
			auto Result = Lr0::Process(Hist, [&](Lr0::Element tra) -> std::any {
				if (tra.IsTerminal())
				{
					return Datas[tra.shift.token_index];
				}
				else {
					switch (tra.reduce.mask)
					{
					case 0: return std::move(tra.GetRawData(0));
					case 1: {
						auto [N1s, N1e] = tra.GetData<std::tuple<size_t, size_t>>(0);
						auto [N2s, N2e] = tra.GetData<std::tuple<size_t, size_t>>(1);
						result[N1e].edge.push_back(epsilon{ N2s });
						return std::tuple<size_t, size_t>{N1s, N2e};
					}
					case 2: {
						auto [N1s, N1e] = tra.GetData<std::tuple<size_t, size_t>>(0);
						auto [N2s, N2e] = tra.GetData<std::tuple<size_t, size_t>>(2);
						auto i1s = result.back_construction({});
						auto i2s = result.back_construction({});
						result[i1s].edge.push_back(epsilon{ N1s });
						result[i1s].edge.push_back(epsilon{ N2s });
						result[N1e].edge.push_back(epsilon{ i2s });
						result[N2e].edge.push_back(epsilon{ i2s });
						return std::tuple<size_t, size_t>{i1s, i2s};
					}
					case 3: {
						return std::move(tra.GetRawData(2));
					}
					case 4: {
						return RangeSet{};
					}
					case 5: {
						auto P1 = tra.GetData<RangeSet>(0);
						auto P2 = tra.GetData<RangeSet>(1);
						P1 |= P2;
						return std::move(P1);
					}
					case 6: {
						auto P1 = tra.GetData<RangeSet>(0);
						auto P2 = tra.GetData<RangeSet>(1);
						auto P3 = tra.GetData<RangeSet>(3);

						assert(!P2.empty() && !P3.empty());

						auto min = P2[0].left;
						auto big = P3[P3.size() - 1].right;
						P1 |= range_set::range{ min, big };
						return std::move(P1);
					}
					case 7 : {
						auto P1 = tra.GetData<RangeSet>(0);
						auto P2 = tra.GetData<RangeSet>(1);
						P1 |= P2;
						if (NotAble)
						{
							RangeSet total({ 1, std::numeric_limits<char32_t>::max() });
							P1.intersection_cull(total);
							P1 = std::move(total);
						}
						auto i1s = result.back_construction({});
						auto i2s = result.back_construction({});
						result[i1s].edge.push_back(comsume{ i2s, std::move(P1) });
						return std::tuple<size_t, size_t>(i1s, i2s);
					}
					case 8: {
						auto P1 = tra.GetData<RangeSet>(0);
						if (NotAble)
						{
							RangeSet total({ 1, std::numeric_limits<char32_t>::max() });
							P1.intersection_cull(total);
							P1 = std::move(total);
						}
						auto i1s = result.back_construction({});
						auto i2s = result.back_construction({});
						result[i1s].edge.push_back(comsume{ i2s, std::move(P1) });
						return std::tuple<size_t, size_t>(i1s, i2s);
					}
					case 9: {
						auto [N1s, N1e] = tra.GetData<std::tuple<size_t, size_t>>(0);
						auto s1 = result.back_construction({});
						auto s2 = result.back_construction({});
						if (tra.GetRawData(2).has_value())
						{
							result[s1].edge.push_back(epsilon{ s2 });
							result[s1].edge.push_back(epsilon{ N1s });
							result[N1e].edge.push_back(epsilon{ s2 });
							result[N1e].edge.push_back(epsilon{ N1s });
						}
						else {
							result[N1e].edge.push_back(epsilon{ N1s });
							result[s1].edge.push_back(epsilon{ N1s });
							result[s1].edge.push_back(epsilon{ s2 });
							result[N1e].edge.push_back(epsilon{ s2 });
						}
						return std::tuple<size_t, size_t>(s1, s2);
					}
					case 10: {
						auto [t1, t2] = tra.GetData<std::tuple<size_t, size_t>>(0);
						auto s1 = result.back_construction({});
						auto s2 = result.back_construction({});
						if (tra.GetRawData(2).has_value())
						{
							result[s1].edge.push_back(epsilon{ t1 });
							result[t2].edge.push_back(epsilon{ s2 });
							result[t2].edge.push_back(epsilon{ t1 });
						}
						else {
							result[s1].edge.push_back(epsilon{ t1 });
							result[t2].edge.push_back(epsilon{ t1 });
							result[t2].edge.push_back(epsilon{ s2 });
						}
						return std::tuple<size_t, size_t>(s1, s2);
					}
					case 11: {
						auto [t1, t2] = tra.GetData<std::tuple<size_t, size_t>>(0);
						auto s1 = result.back_construction({});
						auto s2 = result.back_construction({});
						if (tra.GetRawData(2).has_value())
						{
							result[s1].edge.push_back(epsilon{ s2 });
							result[s1].edge.push_back(epsilon{ t1 });
							result[t2].edge.push_back(epsilon{ s2 });
						}
						else {
							result[s1].edge.push_back(epsilon{ t1 });
							result[s1].edge.push_back(epsilon{ s2 });
							result[t2].edge.push_back(epsilon{ s2 });
						}
						return std::tuple<size_t, size_t>(s1, s2);
					}
					case 12: {
						auto P1 = tra.GetData<RangeSet>(0);
						auto i1s = result.back_construction({});
						auto i2s = result.back_construction({});
						result[i1s].edge.push_back(comsume{ i2s, std::move(P1) });
						return std::tuple<size_t, size_t>(i1s, i2s);
					} break;
					case 13: {NotAble = true; } break;
					case 14: {NotAble = false; } break;
					}
				}
				return {};
			});
			assert(Result.has_value());
			auto end = result.back_construction({});
			auto [s1, s2] = std::any_cast<std::tuple<size_t, size_t>>(Result);
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
						re.Edges.push_back({ static_cast<size_t>(Table::EdgeType::Comsume), comsume_index, ref.state});
					}
					else if (std::holds_alternative<acception>(ite2))
					{
						auto& ref = std::get<acception>(ite2);
						re.Edges.push_back({ static_cast<size_t>(Table::EdgeType::Acception), ref.acception_state, ref.state });
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
		nfa result;
		for (size_t i = 0; i < input_length; ++i)
			result.append_rex(input[i], i);
		return result.simplify();
	}

	Table CreateTable(std::u32string_view const* input, size_t const* state, size_t input_length)
	{
		nfa result;
		for (size_t i = 0; i < input_length; ++i)
			result.append_rex(input[i], state[i]);
		return result.simplify();
	}

	Table CreateTableReversal(std::u32string_view const* input, size_t input_length)
	{
		nfa result;
		for (size_t i = input_length; i >= 1; --i)
			result.append_rex(input[i -  1], i -  1);
		return result.simplify();
	}

	Table CreateTableReversal(std::u32string_view const* input, size_t const* state, size_t input_length)
	{
		nfa result;
		for (size_t i = input_length; i >= 1; --i)
			result.append_rex(input[i - 1], state[i - 1]);
		return result.simplify();
	}
	
}

namespace Potato::Parser::Format
{

	auto RengePattern = CreatePatternRef(U"{{0x{-hex}, 0x{-hex}}},");

	template<typename Type, typename T1> struct Formatter<std::vector<Type, T1>>
	{
		std::u32string operator()(std::u32string_view, std::vector<Type, T1> const& RS)
		{
			static auto pat = CreatePatternRef(U"{}, ");
			std::u32string Result;
			Result += U'{';
			for (auto& ite : RS)
				Result += Process(pat, ite);
			Result += U'}';
			return std::move(Result);
		}
	};

	template<> struct Formatter<NFA::Table::RangeSet>
	{
		std::u32string operator()(std::u32string_view par, NFA::Table::RangeSet const& RS)
		{
			return Formatter<std::remove_cv_t<std::remove_reference_t<decltype(RS.storage())>>>{}(par, RS.storage());
		}
	};

	template<> struct Formatter<NFA::Table::RangeSet::range>
	{
		std::u32string operator()(std::u32string_view, NFA::Table::RangeSet::range RS)
		{
			static auto pat = CreatePatternRef(U"{{0x{-hex}, 0x{-hex}}}");
			return Process(pat, static_cast<uint32_t>(RS.left), static_cast<uint32_t>(RS.right));
		}
	};

	template<> struct Formatter<std::tuple<size_t, size_t>>
	{
		std::u32string operator()(std::u32string_view, std::tuple<size_t, size_t> RS)
		{
			auto [a, b] = RS;
			static auto pat = CreatePatternRef(U"{{0x{-hex}, 0x{-hex}}}");
			return Process(pat, a, b);
		}
	};

	template<> struct Formatter<std::tuple<size_t, size_t, size_t>>
	{
		std::u32string operator()(std::u32string_view, std::tuple<size_t, size_t, size_t> RS)
		{
			static auto pat = CreatePatternRef(U"{{0x{-hex}, 0x{-hex}, 0x{-hex}}}");
			auto [a, b, c] = RS;
			return Process(pat, a, b ,c);
		}
	};

	std::u32string Formatter<NFA::Table>::operator()(std::u32string_view, NFA::Table const& input)
	{
		static auto pat = CreatePatternRef(U"{{{}, {}, {}}");
		return Process(pat, input.ComsumeEdge, input.Edges, input.Nodes);
	}
}