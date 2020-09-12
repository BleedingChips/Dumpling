#include "../Public/Ebnf.h"
#include "../Public/StrFormat.h"
#include <assert.h>
#include <vector>

namespace PineApple::Ebnf
{

	enum class T
	{
		Empty = 0,
		Line,
		Terminal,
		Equal,
		Mask,
		Rex,
		NoTerminal,
		StartSymbol,
		LB_Brace,
		RB_Brace,
		LM_Brace,
		RM_Brace,
		LS_Brace,
		RS_Brace,
		Or,
		Number,
		Colon,
		Command,
		TokenMax,
	};

	constexpr Lr0::Symbol operator*(T sym) { return Lr0::Symbol{ static_cast<size_t>(sym), Lr0::TerminalT{} }; };

	enum class NT
	{
		Statement,
		FunctionEnum,
		ProductionHead,
		RemoveElement,
		RemoveExpression,
		AppendExpression,
		Expression,
		ExpressionStatement,
		LeftOrStatement,
		RightOrStatement,
		OrStatement,
	};

	constexpr Lr0::Symbol operator*(NT sym) { return Lr0::Symbol{ static_cast<size_t>(sym), Lr0::NoTerminalT{} }; };

	std::u32string_view Table::FindSymbolString(size_t input, bool IsTerminal) const noexcept {
		if (IsTerminal)
		{
			if (input < ter_count)
			{
				auto [s, size] = symbol_map[input];
				return std::u32string_view{ symbol_table.data() + s, size };
			}
		}
		else {
			input += ter_count;
			if (input < symbol_map.size())
			{
				auto [s, size] = symbol_map[input];
				return std::u32string_view{ symbol_table.data() + s, size };
			}
		}
		return {};
	}

	std::optional<size_t> Table::FindSymbolState(std::u32string_view sym, bool& IsTerminal) const noexcept
	{
		for (size_t i = 0; i < symbol_map.size(); ++i)
		{
			auto [s, size] = symbol_map[i];
			auto str = std::u32string_view{ symbol_table.data() + s, size };
			if (str == sym)
			{
				if (i > ter_count) {
					IsTerminal = false;
					return i;
				}
				else {
					IsTerminal = true;
					return i;
				}
			}
		}
		return std::nullopt;
	}

	std::tuple<std::vector<Symbol>, std::vector<Nfa::DocumenetMarchElement>> DefaultLexer(Nfa::Table const& table, std::u32string_view& input)
	{
		std::vector<Symbol> R1;
		std::vector<Nfa::DocumenetMarchElement> R2;
		Nfa::Location Loc;
		while (!input.empty())
		{
			auto Re = Nfa::DecumentComsume(table, input, Loc);
			assert(Re);
			input = Re->march.last_string;
			if (Re->march.acception != 0)
			{
				R1.push_back(Symbol(Re->march.acception, Lr0::TerminalT{}));
				R2.push_back(*Re);
			}
		}
		return { std::move(R1), std::move(R2) };
	}

	History Translate(Table const& Tab, Lr0::History const& Steps, std::vector<Nfa::DocumenetMarchElement> const& Datas)
	{
		std::vector<Step> AllStep;
		AllStep.reserve(Steps.steps.size());
		Nfa::Location LastLocation;
		size_t DataCount = 0;
		std::vector<std::tuple<size_t, size_t>> TemporaryNoTerminal;
		for (auto& Ite : Steps.steps)
		{
			Step Result{};
			Result.state = Ite.value.Index();
			Result.is_terminal = Ite.IsTerminal();
			Result.string = Tab.FindSymbolString(Result.state, Result.is_terminal);
			if (!Result.string.empty())
			{
				if (Result.is_terminal)
				{
					auto& DatasRef = Datas[Ite.shift.token_index];
					Result.loc = DatasRef.location;
					LastLocation = DatasRef.location;
					Result.shift.capture = DatasRef.march.capture;
					Result.shift.mask = DatasRef.march.mask;
					DataCount += 1;
				}
				else {
					Result.reduce.mask = Ite.reduce.mask;
					size_t ProCount = Ite.reduce.production_count;
					size_t Used = ProCount;
					while (!TemporaryNoTerminal.empty())
					{
						auto [Index, Count] = *TemporaryNoTerminal.rbegin();
						if (Index + ProCount >= DataCount)
						{
							TemporaryNoTerminal.pop_back();
							Used += Count;
							Used -= 1;
						}
						else
							break;
					}
					assert(DataCount >= ProCount);
					DataCount -= ProCount;
					DataCount += 1;
					Result.reduce.production_count = Used;
					Result.loc = LastLocation;
				}
				AllStep.push_back(Result);
			}
			else {
				assert(Result.IsNoterminal());
				TemporaryNoTerminal.push_back({ DataCount, Ite.reduce.production_count });
				assert(DataCount >= Ite.reduce.production_count);
				DataCount -= Ite.reduce.production_count;
				DataCount += 1;
			}
		}
		return { std::move(AllStep) };
	}

	History Process(Table const& Tab, std::u32string_view Code)
	{
		auto [Symbols, Datas] = DefaultLexer(Tab.nfa_table, Code);

		try{
			auto Steps = Lr0::Process(Tab.lr0_table, Symbols.data(), Symbols.size());
			return Translate(Tab, Steps, Datas);
		}
		catch (Lr0::Error::UnaccableSymbol const& Symbol)
		{
			auto his = Translate(Tab, Symbol.backup_step, Datas);
			auto Str = Tab.FindSymbolString(Symbol.symbol.Index(), Symbol.symbol.IsTerminal());
			if (Str.empty())
			{
				Nfa::Location loc = (Symbol.index > 0) ? Datas[Symbol.index - 1].location : Nfa::Location{};
				throw Error::UnacceptableSyntax{ U"$_Eof", U"$_Eof", loc, his.Expand() };
			}
			auto loc = Datas[Symbol.symbol.Index()].location;
			throw Error::UnacceptableSyntax{ std::u32string(Str), std::u32string(Datas[Symbol.index].march.capture),Datas[Symbol.index].location, his.Expand() };
		}
	}

	std::any History::operator()(std::any(*Function)(void*, Element&), void* FunctionBody) const
	{
		std::vector<Element::Property> Storage;
		int append = 0;
		for (auto& ite : steps)
		{
			if (ite.IsTerminal())
			{
				Element Re(ite);
				Re.datas = nullptr;
				auto Result = (*Function)(FunctionBody, Re);
				Storage.push_back({ ite, std::move(Result) });
			}
			else {
				Element Re(ite);
				size_t TotalUsed = ite.reduce.production_count + append;
				size_t CurrentAdress = Storage.size() - TotalUsed;
				Re.datas = Storage.data() + CurrentAdress;
				if (TotalUsed >= 1)
				{
					Re.loc = Re[0].loc;
					auto Tar = Re[TotalUsed - 1].loc;
					Re.loc.length = Tar.total_index - Re.loc.total_index + Tar.length;
				}
				else {
					Re.loc = ite.loc;
				}
				auto Result = (*Function)(FunctionBody, Re);
				Storage.resize(CurrentAdress);
				Storage.push_back({ ite, std::move(Result) });
			}
		}
		assert(Storage.size() == 1);
		return std::move(Storage[0].data);
	}

	std::u32string ExpandExe(std::vector<std::u32string_view> ts)
	{
		std::u32string result;
		static auto pattern = StrFormat::CreatePatternRef(U"{} ");
		for (auto& ite : ts)
		{
			result += StrFormat::Process(pattern, ite);
		}
		return result;
	}

	std::vector<std::u32string> History::Expand() const
	{
		std::vector<std::u32string> result;
		std::vector<std::u32string_view> all_token;
		bool InputTerminal = false;
		for (auto& ite : steps)
		{
			if (ite.IsTerminal())
			{
				all_token.push_back(ite.shift.capture);
				InputTerminal = true;
			}
			else {
				if (InputTerminal)
				{
					auto re = ExpandExe(all_token);
					result.push_back(std::move(re));
					InputTerminal = false;
				}
				assert(all_token.size() >= ite.reduce.production_count);
				all_token.resize(all_token.size() - ite.reduce.production_count);
				all_token.push_back(ite.string);
				auto re = ExpandExe(all_token);
				result.push_back(std::move(re));
			}
		}
		if (!all_token.empty())
			result.push_back(ExpandExe(all_token));
		return result;
	}

	std::tuple<std::vector<Symbol>, std::vector<Nfa::DocumenetMarchElement>> EbnfLexer(Nfa::Table const& table, std::u32string_view& input, std::set<size_t> const& Remove, Nfa::Location& Loc)
	{
		std::vector<Symbol> R1;
		std::vector<Nfa::DocumenetMarchElement> R2;
		while (!input.empty())
		{
			auto Re = Nfa::DecumentComsume(table, input, Loc);
			if (Re)
			{
				input = Re->march.last_string;
				if (Re->march.acception == static_cast<size_t>(T::Mask)) { break; }
				auto Ite = Remove.find(Re->march.acception);
				if (Ite == Remove.end())
				{
					R1.push_back(Symbol(Re->march.acception, Lr0::TerminalT{}));
					R2.push_back(*Re);
				}
			}
			else
				throw Error::UnacceptableToken{ {}, Loc };
		}
		return { std::move(R1), std::move(R2) };
	}

	Table CreateTable(std::u32string_view code)
	{

		static std::map<T, std::u32string_view> Rexs = {
			{T::Empty, UR"(\s)" },
			{T::Line, U"\r\n|\n"},
			{T::Terminal, UR"([a-zA-Z_][a-zA-Z_0-9]*)"},
			{T::Equal, UR"(:=)"},
			{T::Mask, UR"(%%%\s*?\n)"},
			{T::Rex, UR"('.*?[^\\]')"},
			{T::NoTerminal, UR"(\<[_a-zA-Z][_a-zA-Z0-9]*\>)"},
			{T::StartSymbol, UR"(\$)"},
			{T::LB_Brace, UR"(\{)"},
			{T::RB_Brace, UR"(\})"},
			{T::LM_Brace, UR"(\[)"},
			{T::RM_Brace, UR"(\])"},
			{T::LS_Brace, UR"(\()"},
			{T::RS_Brace, UR"(\))"},
			{T::Colon, UR"(:)"},
			{T::Or, UR"(\|)"},
			{T::Number, UR"([1-9][0-9]*|0)"},
			{T::Command, UR"(/\*[.\n]*?\*/|//.*?\n)"},
		};

		std::map<std::u32string, size_t> symbol_to_index;
		std::vector<std::tuple<std::u32string, size_t, size_t>> symbol_rex;

		symbol_to_index.insert({ U"_IGNORE", 0 });

		Nfa::Location Loc;

		// step1
		{

			static Nfa::Table nfa_table = ([]() -> Nfa::Table {
				std::vector<T> RequireList = { T::LM_Brace, T::RM_Brace, T::Number, T::Colon, T::Terminal, T::Equal, T::Mask, T::Rex, T::Line, T::Command, T::Empty };
				std::vector<size_t> States;
				std::vector<std::u32string_view> RexStroage;
				for (auto& ite : RequireList)
				{
					States.push_back(static_cast<size_t>(ite));
					RexStroage.push_back(Rexs[ite]);
				}
				return Nfa::CreateTableFromRex(RexStroage.data(), States.data(), States.data(), RequireList.size());
			}());

			static Lr0::Table lr0_instance = Lr0::CreateTable(
				*NT::Statement, {
					{{*NT::Statement, *NT::Statement, *T::Terminal, *T::Equal, *T::Rex, *NT::FunctionEnum, *T::Line}, 1},
					{{*NT::Statement}, 3},
					{{*NT::FunctionEnum, *T::Colon, *T::LM_Brace, *T::Number, *T::RM_Brace}, 5},
					{{*NT::FunctionEnum}, 6},
					//{{*SYM::Statement,*SYM::Statement,  *SYM::Mask}},
					{{*NT::Statement,*NT::Statement,  *T::Line}, 4}
				}, {}
			);

			auto [Symbols, Elements] = EbnfLexer(nfa_table, code, { static_cast<size_t>(T::Empty), static_cast<size_t>(T::Command) }, Loc);
			try {
				auto History = Lr0::Process(lr0_instance, Symbols.data(), Symbols.size());
				Lr0::Process(History, [&](Lr0::Element& input) -> std::any {
					if (input.IsTerminal())
					{
						switch (input.value)
						{
						case* T::Terminal: {
							return Elements[input.shift.token_index].march.capture;
						}break;
						case* T::Rex: {
							auto re = Elements[input.shift.token_index].march.capture;
							return  std::u32string_view(re.data() + 1, re.size() - 2);
						}break;
						case* T::Number: {
							size_t Number = 0;
							for (auto ite : Elements[input.shift.token_index].march.capture)
								Number = Number * 10 + ite - U'0';
							return Number;
						} break;
						default:break;
						}
					}
					else {
						switch (input.reduce.mask)
						{
						case 1: {
							auto Token = input.GetData<std::u32string_view>(1);
							auto Rex = input.GetData<std::u32string_view>(3);
							auto re = symbol_to_index.insert({ std::u32string(Token), static_cast<size_t>(symbol_to_index.size()) });
							symbol_rex.push_back({ std::u32string(Rex), re.first->second, input.GetData<size_t>(4) });
						}break;
						case 5: {
							return input.GetData<size_t>(2);
						}break;
						case 6: {
							return std::numeric_limits<size_t>::max();
						} break;
						case 4:
							return false;
							break;
						default: break;
						}
					}
					return {};
				});
			}
			catch (Lr0::Error::UnaccableSymbol const& US)
			{
				assert(Elements.size() > US.index);
				auto P = Elements[US.index];
				throw Error::UnacceptableToken{ std::u32string(P.march.capture), P.location};
			}
			
		}

		std::map<std::u32string, size_t> noterminal_symbol_to_index;
		std::vector<Lr0::ProductionInput> productions;
		std::optional<Symbol> start_symbol;
		size_t noterminal_temporary = Lr0::SymbolStorageMax - 1;


		struct Token
		{
			Symbol sym;
			Nfa::DocumenetMarchElement march;
		};

		//std::map<lr1::storage_t, std::tuple<std::variant<OrRelationShift, MBraceRelationShift, BBraceRelationShift>, size_t>> temporary_noterminal_production_debug;

		// step2
		{

			static Nfa::Table nfa_instance = ([]() -> Nfa::Table {
				std::vector<T> RequireList = {
					T::Or, T::StartSymbol, T::Colon, T::Terminal, T::Equal, T::Number, T::NoTerminal, T::Mask, T::Rex, T::Line,
					T::LS_Brace, T::RS_Brace, T::LM_Brace, T::RM_Brace, T::LB_Brace, T::RB_Brace, T::Command, T::Empty
				};
				std::vector<size_t> States;
				std::vector<std::u32string_view> RexStroage;
				for (auto& ite : RequireList)
				{
					States.push_back(static_cast<size_t>(ite));
					RexStroage.push_back(Rexs[ite]);
				}
				return Nfa::CreateTableFromRex(RexStroage.data(), States.data(), States.data(), RequireList.size());
			}());


			static Lr0::Table imp = Lr0::CreateTable(
				*NT::Statement, {
				{{*NT::Statement}, 0},
				{{*NT::Statement, *NT::Statement, *T::Line}, 0},
				{{*NT::Statement, *NT::Statement, *T::StartSymbol, *T::Equal, *T::NoTerminal, *T::Line}, 1},

				{{*NT::LeftOrStatement, *NT::Expression}, 2},
				{{*NT::LeftOrStatement, *NT::LeftOrStatement, *NT::Expression}, 3},
				{{*NT::RightOrStatement, *NT::Expression}, 2},
				{{*NT::RightOrStatement, *NT::Expression, *NT::RightOrStatement}, 3},
				{{*NT::OrStatement, *NT::LeftOrStatement, *T::Or, *NT::RightOrStatement}, 5},
				{{*NT::OrStatement, *NT::OrStatement, *T::Or, *NT::RightOrStatement}, 5},

				{{*NT::ProductionHead, *T::NoTerminal}, 6},
				{{*NT::ProductionHead}, 7},

				{{*NT::Statement, *NT::Statement, *NT::ProductionHead, *T::Equal, *NT::OrStatement, *NT::AppendExpression, *T::Line}, 8},
				{{*NT::Statement, *NT::Statement, *NT::ProductionHead, *T::Equal, *NT::ExpressionStatement, *NT::AppendExpression, *T::Line}, 8},
				{{*NT::Statement, *NT::Statement, *NT::ProductionHead, *T::Equal, *NT::AppendExpression, *T::Line}, 19},

				{{*NT::ExpressionStatement, *NT::Expression}, 2},
				{{*NT::ExpressionStatement, *NT::ExpressionStatement, *NT::Expression}, 3},

				{{*NT::Expression, *T::LS_Brace, *NT::OrStatement, *T::RS_Brace}, 9},
				{{*NT::Expression, *T::LS_Brace, *NT::ExpressionStatement, *T::RS_Brace}, 9},
				{{*NT::Expression, *T::LM_Brace, *NT::OrStatement, *T::RM_Brace}, 10},
				{{*NT::Expression, *T::LM_Brace, *NT::ExpressionStatement, *T::RM_Brace}, 10},
				{{*NT::Expression, *T::LB_Brace, *NT::OrStatement, *T::RB_Brace}, 11},
				{{*NT::Expression, *T::LB_Brace, *NT::ExpressionStatement, *T::RB_Brace}, 11},
				{{*NT::Expression, *T::NoTerminal}, 12},
				{{*NT::Expression, *T::Terminal}, 12},
				{{*NT::Expression, *T::Rex}, 12},

				{{*NT::FunctionEnum, *T::LM_Brace, *T::Number, *T::RM_Brace}, 13},
				{{*NT::FunctionEnum}, 14},
				{{*NT::RemoveElement, *NT::RemoveElement, *T::Terminal}, 16},
				{{*NT::RemoveElement, *NT::RemoveElement, *T::Rex}, 16},
				{{*NT::RemoveElement}, 15},

				{{*NT::AppendExpression, *T::Colon, *NT::RemoveElement, *NT::FunctionEnum}, 17},
				{{*NT::AppendExpression}, 18},
				},
				{}
			);

			std::optional<Symbol> LastHead;
			size_t LastTemporaryNoTerminal = Lr0::SymbolStorageMax - 1;

			using SymbolList = std::vector<Symbol>;

			auto [Symbols, Elements] = EbnfLexer(nfa_instance, code, { static_cast<size_t>(T::Empty), static_cast<size_t>(T::Command) }, Loc);


			try {
				auto History = Lr0::Process(imp, Symbols.data(), Symbols.size());
				auto Total = Lr0::Process(History, [&](Lr0::Element & tra) -> std::any {
					if (tra.IsTerminal())
					{
						auto& element = Elements[tra.shift.token_index];
						auto string = std::u32string(element.march.capture);
						switch (tra.value)
						{
						case* T::Terminal: {
							auto Find = symbol_to_index.find(string);
							if (Find != symbol_to_index.end())
								return Token{ Symbol(Find->second, Lr0::TerminalT{}), element };
							else
								throw Error::UndefinedTerminal{ string, element.location };
						}break;
						case* T::NoTerminal: {
							auto Find = noterminal_symbol_to_index.insert({ string, noterminal_symbol_to_index.size() });
							return Token{ Symbol(Find.first->second, Lr0::NoTerminalT{}), element };
						}break;
						case* T::Rex: {
							static const std::u32string SpecialChar = UR"($()*+.[]?\^{}|,\)";
							assert(string.size() >= 2);
							auto re = symbol_to_index.insert({ string, symbol_to_index.size() });
							if (re.second)
							{
								std::u32string rex;
								for (size_t i = 1; i < string.size() - 1; ++i)
								{
									for (auto& ite : SpecialChar)
										if (ite == string[i])
										{
											rex.push_back(U'\\');
											break;
										}
									rex.push_back(string[i]);
								}
								symbol_rex.push_back({ std::move(rex), re.first->second, std::numeric_limits<size_t>::max() });
							}
							return Token{ Symbol(re.first->second, Lr0::TerminalT{}), element };
						}break;
						case* T::Number: {
							size_t Number = 0;
							for (auto ite : string)
								Number = Number * 10 + ite - U'0';
							return Number;
						}break;
						case* T::Mask: assert(false);
						default:
							break;
						}
					}
					else {
						switch (tra.reduce.mask)
						{
						case 0: {
						} break;
						case 1: {
							LastHead = std::nullopt;
							auto P1 = std::move(tra.GetData<Token>(3));
							if (!start_symbol)
								start_symbol = P1.sym;
							else
								throw Error::RedefinedStartSymbol{ P1.march.location };
							return std::vector<Lr0::ProductionInput>{};
						}break;
						case 2: {
							return std::move(tra.GetRawData(0));
						} break;
						case 3: {
							auto P1 = std::move(tra.GetData<SymbolList>(0));
							auto P2 = std::move(tra.GetData<SymbolList>(1));
							P1.insert(P1.end(), P2.begin(), P2.end());
							return std::move(P1);
						}break;
						case 5: {
							auto TemSym = Symbol(noterminal_temporary--, Lr0::NoTerminalT{});
							auto P1 = std::move(tra.GetData<SymbolList>(0));
							auto P2 = std::move(tra.GetData<SymbolList>(2));
							P1.insert(P1.begin(), TemSym);
							P2.insert(P2.begin(), TemSym);
							productions.push_back({ std::move(P1) });
							productions.push_back({ std::move(P2) });
							return SymbolList{ TemSym };
						} break;
						case 6: {
							auto P1 = std::move(tra.GetData<Token>(0));
							LastHead = P1.sym;
							return P1;
						}
						case 7: {
							if (!LastHead)
								throw Error::UnsetDefaultProductionHead{};
							return Token{ *LastHead, {} };
						}
						case 8: {
							auto Head = tra.GetData<Token>(1);
							auto Expression = std::move(tra.GetData<SymbolList>(3));
							auto [RemoveRe, Enum] = std::move(tra.GetData<std::tuple<std::set<Symbol>, size_t>>(4));
							Expression.insert(Expression.begin(), Head.sym);
							Lr0::ProductionInput re(std::move(Expression), std::move(RemoveRe), Enum);
							productions.push_back(std::move(re));
							return {};
						}break;
						case 19: {
							auto Head = tra.GetData<Token>(1);
							auto [RemoveRe, Enum] = std::move(tra.GetData<std::tuple<std::set<Symbol>, size_t>>(3));
							SymbolList Expression({ Head.sym });
							Lr0::ProductionInput re(std::move(Expression), std::move(RemoveRe), Enum);
							productions.push_back(std::move(re));
							return {};
						}break;
						case 9: {
							return std::move(tra.GetRawData(1));
						}break;
						case 10: {
							auto TemSym = Symbol(noterminal_temporary--, Lr0::NoTerminalT{});
							auto P = std::move(tra.GetData<SymbolList>(1));
							P.insert(P.begin(), TemSym);
							productions.push_back(std::move(P));
							productions.push_back(Lr0::ProductionInput({ TemSym }));
							return SymbolList{ TemSym };
						}break;
						case 11: {
							auto TemSym = Symbol(noterminal_temporary--, Lr0::NoTerminalT{});
							auto P = std::move(tra.GetData<SymbolList>(1));
							auto List = { TemSym, TemSym };
							P.insert(P.begin(), List.begin(), List.end());
							productions.push_back(std::move(P));
							productions.push_back(Lr0::ProductionInput({ TemSym }));
							return SymbolList{ TemSym };
						} break;
						case 12: {
							auto P = std::move(tra.GetData<Token>(0));
							SymbolList SL({ P.sym });
							return std::move(SL);
						} break;
						case 13: {
							return std::move(tra.GetRawData(1));
						} break;
						case 14: {
							return size_t(Lr0::ProductionInput::default_mask());
						} break;
						case 15: {
							return std::set<Symbol>{};
						}break;
						case 16: {
							auto P = std::move(tra.GetData<std::set<Symbol>>(0));
							auto P2 = std::move(tra.GetData<Token>(1));
							P.insert(P2.sym);
							return std::move(P);
						}break;
						case 17: {
							return std::tuple<std::set<Symbol>, size_t>{tra.GetData<std::set<Symbol>>(1), tra.GetData<size_t>(2)};
						} break;
						case 18: {
							return std::tuple<std::set<Symbol>, size_t>{ std::set<Symbol>{}, Lr0::ProductionInput::default_mask()};
						} break;
						}
					}
					return {};
				});
			}
			catch (Lr0::Error::UnaccableSymbol const& US)
			{
				auto P = Elements[US.index];
				throw Error::UnacceptableToken{ std::u32string(P.march.capture), P.location };
			}

		}

		std::vector<Lr0::OpePriority> operator_priority;
		// step3
		{
			static Nfa::Table nfa_instance = ([]() -> Nfa::Table {
				std::vector<T> RequireList = { T::Terminal, T::Rex, T::Command,
					T::LS_Brace, T::RS_Brace, T::LM_Brace, T::RM_Brace, T::Empty };
				std::vector<size_t> States;
				std::vector<std::u32string_view> RexStroage;
				for (auto& ite : RequireList)
				{
					States.push_back(static_cast<size_t>(ite));
					RexStroage.push_back(Rexs[ite]);
				}
				return Nfa::CreateTableFromRex(RexStroage.data(), States.data(), States.data(), RequireList.size());
			}());

			static Lr0::Table lr0_instance = Lr0::CreateTable(
				*NT::Statement, {
					{{*NT::Expression, *T::Terminal}, 1},
					{{*NT::Expression, *T::Rex}, 1},
					{{*NT::Expression, *NT::Expression, *NT::Expression}, 2},
					{{*NT::Statement, *NT::Statement, *T::Terminal}, 3},
					{{*NT::Statement, *NT::Statement, *T::Rex}, 3},
					{{*NT::Statement, *NT::Statement, *T::LS_Brace, *NT::Expression, *T::RS_Brace}, 4},
					{{*NT::Statement, *NT::Statement, *T::LM_Brace, *NT::Expression, *T::RM_Brace}, 5},
					{{*NT::Statement}},
				}, {}
			);

			auto [Symbols, Elements] = EbnfLexer(nfa_instance, code, { static_cast<size_t>(T::Empty), static_cast<size_t>(T::Command) }, Loc);
			try {
				auto History = Lr0::Process(lr0_instance, Symbols.data(), Symbols.size());
				Lr0::Process(History, [&](Lr0::Element& step) -> std::any {
					if (step.IsTerminal())
					{
						if (step.value == *T::Terminal || step.value == *T::Rex)
						{
							auto element = Elements[step.shift.token_index];
							auto Find = symbol_to_index.find(std::u32string(element.march.capture));
							if (Find != symbol_to_index.end())
								return Token{ Symbol(Find->second, Lr0::TerminalT{}) };
							else
								throw Error::UndefinedTerminal{std::u32string(element.march.capture), element.location };
						}
					}
					else {
						switch (step.reduce.mask)
						{
						case 1: {
							std::vector<Symbol> List;
							List.push_back(step.GetData<Token>(0).sym);
							return std::move(List);
						}break;
						case 2: {
							auto P1 = std::move(step.GetData<std::vector<Symbol>>(0));
							auto P2 = std::move(step.GetData<std::vector<Symbol>>(1));
							P1.insert(P1.end(), P2.begin(), P2.end());
							return std::move(P1);
						} break;
						case 3: {
							auto P = step.GetData<Token>(1).sym;
							operator_priority.push_back({ {P}, Lr0::Associativity::Left });
							return {};
						} break;
						case 4: {
							auto P = step.GetData<std::vector<Symbol>>(2);
							operator_priority.push_back({ std::move(P), Lr0::Associativity::Left });
							return {};
						} break;
						case 5: {
							auto P = step.GetData<std::vector<Symbol>>(2);
							operator_priority.push_back({ std::move(P), Lr0::Associativity::Right });
							return {};
						} break;
						}
					}
					return {};
				});
			}
			catch (Lr0::Error::UnaccableSymbol const& US)
			{
				auto P = Elements[US.index];
				throw Error::UnacceptableToken{ std::u32string(P.march.capture), P.location };
			}
		}

		if (!start_symbol)
			throw Error::MissingStartSymbol{};

		size_t DefineProduction_count = productions.size();
		//productions.insert(productions.end(), std::move_iterator(productions_for_temporary.begin()), std::move_iterator(productions_for_temporary.end()));
		std::u32string table;
		std::vector<std::tuple<size_t, size_t>> symbol_map;
		symbol_map.resize(symbol_to_index.size() + noterminal_symbol_to_index.size(), {0, 0});
		for (auto ite : symbol_to_index)
		{
			auto start = table.size();
			table += ite.first;
			symbol_map[ite.second] = {start, ite.first.size()};
		}
		size_t TerminalCount = symbol_to_index.size();
		for (auto ite : noterminal_symbol_to_index)
		{
			auto start = table.size();
			table += ite.first;
			symbol_map[ite.second + TerminalCount] = { start, ite.first.size() };
		}
		
		try {
			std::vector<std::u32string_view> Rexs;
			std::vector<size_t> MappingSize;
			std::vector<size_t> MappingMask;
			for (auto ite = symbol_rex.rbegin(); ite != symbol_rex.rend(); ++ite)
			{
				auto& [str, index, mask] = *ite;
				Rexs.push_back(str);
				MappingSize.push_back(index);
				MappingMask.push_back(mask);
			}
			Nfa::Table NFATable = Nfa::CreateTableFromRex(Rexs.data(), MappingSize.data(), MappingMask.data(), MappingSize.size());
			Lr0::Table Lr0Table = Lr0::CreateTable(
				*start_symbol, productions, std::move(operator_priority)
			);
			return { std::move(table), std::move(symbol_map), TerminalCount, std::move(NFATable), std::move(Lr0Table) };
		}
		catch (Nfa::Error::UnaccaptableRexgex const& ref)
		{
			throw Error::UnacceptableRegex{ref.Regex, ref.AccepetableMask};
		}
		catch (Lr0::Error::NoterminalUndefined const NU)
		{
			for(auto& ite : noterminal_symbol_to_index)
				if(ite.second == NU.value.Index())
					throw Error::UndefinedNoterminal{std::u32string(ite.first)};
			throw Error::UndefinedNoterminal{ std::u32string(U"$UnknowSymbol") };
		}
	}
}

namespace PineApple::StrFormat
{

	struct Wrap { std::u32string const& ref; };

	template<> struct Formatter<Wrap>
	{
		std::u32string operator()(std::u32string_view, Wrap const& ref)
		{
			static auto pat = CreatePatternRef(U"\\x{-hex}");
			std::u32string W = U"U\"";
			for (auto& ite : ref.ref)
				W += Process(pat, static_cast<uint32_t>(ite));
			W += U"\"";
			return W;
		}
	};

	template<> struct Formatter<std::vector<std::tuple<std::size_t, std::size_t>>>
	{
		std::u32string operator()(std::u32string_view, std::vector<std::tuple<std::size_t, std::size_t>> const& ref)
		{
			static auto pat = CreatePatternRef(U"{{{}, {}}}, ");
			std::u32string W = U"{";
			for (auto [i1, i2] : ref)
				W += Process(pat, i1, i2);
			W += U"}";
			return W;
		}
	};

	std::u32string Formatter<Ebnf::Table>::operator()(std::u32string_view, Ebnf::Table const& ref)
	{
		static auto pat = CreatePatternRef(U"{{{}, {}, {}, {}, {}}}");
		return Process(pat, Wrap{ ref.symbol_table }, ref.symbol_map, ref.ter_count, ref.nfa_table, ref.lr0_table);
	}
}