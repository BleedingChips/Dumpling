#include "parser_ebnf.h"
#include <assert.h>
#include <vector>

namespace Potato::Parser::Ebnf
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

	std::map<T, std::u32string_view> Rexs = {
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
		{T::Number, UR"([1-9][0-9]*)"},
		{T::Command, UR"(/\*[.\n]*?\*/|//.*?\n)"},
	};

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

	std::optional<std::tuple<size_t, bool>> Table::FindSymbolState(std::u32string_view sym) const noexcept
	{
		for (size_t i = 0; i < symbol_map.size(); ++i)
		{
			auto [s, size] = symbol_map[i];
			auto str = std::u32string_view{ symbol_table.data() + s, size };
			if (str == sym)
			{
				if (i > ter_count)
					return std::tuple<size_t, bool>{i, true};
				else
					return std::tuple<size_t, bool>{i, false};
			}
		}
		return std::nullopt;
	}

	char const* Table::unacceptable_token_error::what() const noexcept
	{
		return "Unacceprable Token Error";
	}

	char const* Table::undefine_terminal_error::what() const noexcept
	{
		return "Undefine Terminal Error";
	}

	char const* Table::miss_start_symbol::what() const noexcept
	{
		return "Missing Start Symbol Error";
	}

	std::tuple<std::vector<Symbol>, std::vector<NFA::DocumenetMarchElement>> DefaultLexer(NFA::Table const& table, std::u32string_view& input)
	{
		std::vector<Symbol> R1;
		std::vector<NFA::DocumenetMarchElement> R2;
		NFA::Location Loc;
		while (!input.empty())
		{
			auto Re = NFA::DecumentComsume(table, input, Loc);
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

	History Process(Table const& Tab, std::u32string_view Code)
	{
		auto [Symbols, Datas] = DefaultLexer(Tab.nfa_table, Code);
		auto Steps = Lr0::Process(Tab.lr0_table, Symbols.data(), Symbols.size());
		
		std::vector<Step> AllStep;
		AllStep.reserve(Steps.steps.size());
		for (auto& Ite : Steps.steps)
		{
			Step Result{};
			Result.state = Ite.value.Index();
			Result.is_terminal = Ite.IsTerminal();
			Result.string = Tab.FindSymbolString(Result.state, Result.is_terminal);
			if (Result.is_terminal)
			{
				auto& DatasRef = Datas[Ite.shift.token_index];
				Result.shift.loc = DatasRef.location;
				Result.shift.capture = DatasRef.march.capture;
			}
			else {
				Result.reduce.mask = Ite.reduce.mask;
				Result.reduce.production_count = Ite.reduce.production_count;
			}
			AllStep.push_back(Result);
		}
		return { std::move(AllStep) };
	}

	std::any History::operator()(std::any(*Function)(void*, Element&), void* FunctionBody) const
	{
		std::vector<std::tuple<size_t, std::u32string_view, std::any>> Storage;
		int append = 0;
		for (auto& ite : steps)
		{
			if (ite.IsTerminal())
			{
				Element Re(ite);
				Re.datas = nullptr;
				auto Result = (*Function)(FunctionBody, Re);
				Storage.push_back({ ite.state, ite.string, std::move(Result) });
			}
			else {
				Element Re(ite);
				if (Re.string.empty())
				{
					assert(Re.reduce.production_count < std::numeric_limits<int>::max());
					int used = static_cast<int>(Re.reduce.production_count);
					used -= 1;
					append += used;
				}
				else {
					size_t CurrentAdress = Storage.size() - (ite.reduce.production_count + append);
					Re.datas = Storage.data() + CurrentAdress;
					auto Result = (*Function)(FunctionBody, Re);
					Storage.resize(CurrentAdress);
					Storage.push_back({Re.state, Re.string, std::move(Result)});
				}
			}
		}
		assert(Storage.size() == 1);
		return std::move(std::get<2>(Storage[0]));
	}
	
	/*
	std::any sbnf_processer::analyze_imp(Table const& ref, std::u32string_view code, std::any(*Func)(void* data, travel), void* data)
	{
		assert(Func != nullptr);
		DefultLexer Wrapper(ref.nfa_s, code);
		Wrapper.reset_remove({ ref.unused_terminal });
		std::vector<storage_t> ProductionStorage;
		std::vector<storage_t> TemporaryProductionCountReserve;
		std::vector<size_t> TemporaryProductionCount;
		size_t TemporarySymbolCount = 0;
		try {
			Syntax::lr1_processor{}(ref.lr1_s, Wrapper, [&](Syntax::lr1_processor::travel tra) {
				if (tra.is_terminal())
				{
					travel re;
					re.sym_str = ref.find_symbol(tra.symbol);
					re.sym = tra.symbol;
					auto stack = Wrapper.stack();
					re.token_data = stack.capture;
					re.terminal.line = stack.line;
					re.terminal.index = stack.index;
					Func(data, re);
				}
				else {
					if (tra.symbol > ref.temporary_prodution_start && tra.symbol != lr1::start_symbol())
					{
						std::vector<storage_t> Storage;
						for (size_t i = tra.noterminal.production_count; i > 0; --i)
						{
							if (tra.noterminal.symbol_array[i - 1] > ref.temporary_prodution_start)
							{
								assert(TemporaryProductionCount.size() > 0);
								auto count = *TemporaryProductionCount.rbegin();
								TemporaryProductionCount.pop_back();
								size_t k = TemporaryProductionCount.size();
								size_t target = k - count;
								for (size_t j = target; j < k; ++j)
									Storage.push_back(TemporaryProductionCountReserve[j]);
								TemporaryProductionCountReserve.resize(target);
							}
							else
								Storage.push_back(tra.symbol);
						}
						TemporaryProductionCount.push_back(Storage.size());
						TemporaryProductionCountReserve.insert(TemporaryProductionCountReserve.end(), Storage.begin(), Storage.end());
					}
					else {
						ProductionStorage.clear();
						for (size_t i = tra.noterminal.production_count; i > 0; --i)
						{
							auto sym = tra.noterminal.symbol_array[i - 1];
							if (sym > ref.temporary_prodution_start)
							{
								assert(TemporaryProductionCount.size() > 0);
								auto Size = *TemporaryProductionCount.rbegin();
								TemporaryProductionCount.pop_back();
								auto Target = TemporaryProductionCountReserve.size() - Size;
								for (size_t ite = Target; ite < TemporaryProductionCountReserve.size(); ++ite)
									ProductionStorage.push_back(TemporaryProductionCountReserve[ite]);
								TemporaryProductionCountReserve.resize(Target);
							}
							else
								ProductionStorage.push_back(tra.symbol);
						}
						std::reverse(ProductionStorage.begin(), ProductionStorage.end());
						travel re;
						re.sym_str = ref.find_symbol(tra.symbol);
						re.sym = tra.symbol;
						auto stack = Wrapper.stack();
						re.noterminal.array_count = ProductionStorage.size();
						re.noterminal.function_enum = tra.noterminal.function_enum;
						re.noterminal.symbol_array = ProductionStorage.data();
						Func(data, re);
					}
				}
			});
		}
		catch (lr1_processor::unacceptable_error const& Ue)
		{
			auto Symbol = ref.find_symbol(Ue.token);
			auto LastSymbol = ref.find_symbol(Ue.last_symbol);

			std::u32string Result = std::u32string(U"Unacceptable Token [") + std::u32string(Symbol) + U"] And Token Data [" + std::u32string(Wrapper.stack().capture) + U"]";
			if (LastSymbol.size() != 0)
				Result += U" With Follow Symbol [" + std::u32string(LastSymbol) + U"].";

			throw sbnf::error{
				std::move(Result),
				Wrapper.stack().line,
				Wrapper.stack().index
			};
		}
		
	}
	*/

	std::tuple<std::vector<Symbol>, std::vector<NFA::DocumenetMarchElement>> EbnfLexer(NFA::Table const& table, std::u32string_view& input, std::set<size_t> const& Remove, NFA::Location& Loc)
	{
		std::vector<Symbol> R1;
		std::vector<NFA::DocumenetMarchElement> R2;
		while (!input.empty())
		{
			auto Re = NFA::DecumentComsume(table, input, Loc);
			assert(Re);
			input = Re->march.last_string;
			if (Re->march.acception == static_cast<size_t>(T::Mask)) { break; }
			auto Ite = Remove.find(Re->march.acception);
			if (Ite == Remove.end())
			{
				R1.push_back(Symbol(Re->march.acception, Lr0::TerminalT{}));
				R2.push_back(*Re);
			}
		}
		return { std::move(R1), std::move(R2) };
	}

	Table CreateTable(std::u32string_view code)
	{

		std::map<std::u32string, size_t> symbol_to_index;
		std::vector<std::tuple<std::u32string, size_t>> symbol_rex;

		symbol_to_index.insert({ U"_IGNORE", 0 });

		NFA::Location Loc;

		// step1
		{

			static NFA::Table nfa_table = ([]() -> NFA::Table {
				std::vector<T> RequireList = { T::Terminal, T::Equal, T::Mask, T::Rex, T::Line, T::Command, T::Empty };
				std::vector<size_t> States;
				std::vector<std::u32string_view> RexStroage;
				for (auto& ite : RequireList)
				{
					States.push_back(static_cast<size_t>(ite));
					RexStroage.push_back(Rexs[ite]);
				}
				return NFA::CreateTableReversal(RexStroage.data(), States.data(), RequireList.size());
			}());

			static Lr0::Table lr0_instance = Lr0::CreateTable(
				*NT::Statement, {
					{{*NT::Statement, *NT::Statement, *T::Terminal, *T::Equal, *T::Rex, *T::Line}, 1},
					{{*NT::Statement}, 3},
					//{{*SYM::Statement,*SYM::Statement,  *SYM::Mask}},
					{{*NT::Statement,*NT::Statement,  *T::Line}, 4}
				}, {}
			);


			auto [Symbols, Elements] = EbnfLexer(nfa_table, code, { static_cast<size_t>(T::Empty), static_cast<size_t>(T::Command) }, Loc);
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
						return  std::u32string_view( re.data() + 1, re.size() - 2 );
					}break;
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
						symbol_rex.push_back({ std::u32string(Rex) , re.first->second });
					}break;
					case 4:
						return false;
						break;
					default: break;
					}
				}
				return {};
			});
		}

		std::map<std::u32string, size_t> noterminal_symbol_to_index;
		std::vector<Lr0::ProductionInput> productions;
		std::optional<Symbol> start_symbol;
		size_t noterminal_temporary = Lr0::SymbolStorageMax - 1;


		struct Token
		{
			Symbol sym;
			NFA::DocumenetMarchElement march;
		};

		//std::map<lr1::storage_t, std::tuple<std::variant<OrRelationShift, MBraceRelationShift, BBraceRelationShift>, size_t>> temporary_noterminal_production_debug;

		// step2
		{

			static NFA::Table nfa_instance = ([]() -> NFA::Table {
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
				return NFA::CreateTableReversal(RexStroage.data(), States.data(), RequireList.size());
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
								throw Error::ErrorMessage{ std::u32string(U"Undefined Terminal : ") + string, element.location };
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
								symbol_rex.push_back({ std::move(rex), re.first->second });
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
								throw Error::ErrorMessage{ U"Redefine Start Symbol !",  P1.march.location };
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
								throw Error::ErrorMessage{ U"Unreferenced Production Head !", {} };
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
							/*
							auto TemSym = Symbol(noterminal_temporary--, Lr0::NoTerminalT{});
							auto P = std::move(datas[1].cast<SymbolList>());
							P.insert(P.begin(), TemSym);
							productions.push_back(std::move(P));
							return Token{ TemSym, {} };
							*/
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
			catch (Lr0::Error::unacceptable_symbol const& US)
			{
				std::u32string TokenName;
				std::optional<NFA::Location> Loc;
				if (US.index >= Elements.size())
					TokenName = U"&EOF";
				else {
					auto Element = Elements[US.index];
					TokenName = Element.march.capture;
					Loc = Element.location;
				}
				__debugbreak();
			}
			

			

			

		}


		std::vector<Lr0::OpePriority> operator_priority;
		// step3
		{
			static NFA::Table nfa_instance = ([]() -> NFA::Table {
				std::vector<T> RequireList = { T::Terminal, T::Rex, T::Command,
					T::LS_Brace, T::RS_Brace, T::LM_Brace, T::RM_Brace, T::Empty };
				std::vector<size_t> States;
				std::vector<std::u32string_view> RexStroage;
				for (auto& ite : RequireList)
				{
					States.push_back(static_cast<size_t>(ite));
					RexStroage.push_back(Rexs[ite]);
				}
				return NFA::CreateTableReversal(RexStroage.data(), States.data(), RequireList.size());
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
							throw Error::ErrorMessage{ U"Undefine Terminal " + std::u32string(element.march.capture), element.location };
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

		if (!start_symbol)
			throw Error::ErrorMessage{ U"Missing Start Symbol !", {} };

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
			for (auto ite = symbol_rex.rbegin(); ite != symbol_rex.rend(); ++ite)
			{
				auto& [str, index] = *ite;
				Rexs.push_back(str);
				MappingSize.push_back(index);
			}
			NFA::Table NFATable = NFA::CreateTableReversal(Rexs.data(), MappingSize.data(), MappingSize.size());
			Lr0::Table Lr0Table = Lr0::CreateTable(
				*start_symbol, productions, std::move(operator_priority)
			);
			return { std::move(table), std::move(symbol_map), TerminalCount, std::move(NFATable), std::move(Lr0Table) };
		}
		catch (...)
		{
			throw;
		}
		/*
		catch (lr1::missing_noterminal_define_error const& ref)
		{
			if (ref.noterminal_symbol < noterminal_symbol_to_index.size() + lr1::noterminal_start())
			{
				auto [start, size] = symbol_map[ref.noterminal_symbol - lr1::noterminal_start() + symbol_to_index.size()];
				std::u32string Name(table.data() + start, size);
				throw sbnf::error{ U"Missing NoTerminal Symbol Define :" + Name, 0, 0 };
			}
			throw;
		}
		catch (lr1::reduce_conflict_error const& error)
		{
			assert(lr1::is_terminal(error.token));
			auto [start, size] = symbol_map[error.token];
			std::u32string Symbol(table.data() + start, size);

			std::u32string Production1;

			{
				auto Productions = productions[error.possible_production_1];
			}

			auto SymbolToString = [&](lr1::storage_t Head) -> std::optional<std::u32string> {

				if (Head == lr1::eof_symbol())
					return U"[eof]";
				else if (Head == lr1::start_symbol())
					return U"$";

				size_t index = 0;
				if (lr1::is_terminal(Head))
					index = Head;
				else if (Head < noterminal_symbol_to_index.size() + lr1::noterminal_start())
					index = Head - lr1::noterminal_start() + symbol_to_index.size();
				else
					return std::nullopt;
				auto [start, size] = symbol_map[index];
				std::u32string Name(table.data() + start, size);
				return Name;
			};

			auto ToString = [](size_t input) -> std::u32string {
				if (input != 0)
				{
					std::u32string Str;
					do
					{
						char32_t Cur = (input % 10) + U'0';
						Str.insert(Str.begin(), Cur);
						input /= 10;
					} while (input != 0);
					return Str;
				}
				return U"0";
			};

			auto ProductionIndexToString = [&](lr1::storage_t ProductionIndex)
			{
				assert(ProductionIndex < productions.size());
				auto& ProductionRef = productions[ProductionIndex].production;
				auto Symbol = ProductionRef[0];
				std::u32string Result;
				auto Re = SymbolToString(Symbol);
				std::vector<std::tuple<bool, size_t, size_t>> SearchStack;
				if (Re) {
					Result += *Re;
					SearchStack.push_back({ true, ProductionIndex, 1 });
				}
				else {
					Result += U"@Production" + ToString(std::get<0>(noterminal_temporary_debug_production[noterminal_temporary_start - Symbol]));
					SearchStack.push_back({ false, Symbol, 0 });
				}
				Result += U" := ";
				while (!SearchStack.empty())
				{
					auto& [IsNormalProduction, Index, Count] = *SearchStack.rbegin();
					if (IsNormalProduction)
					{
						auto& Ref = productions[Index].production;
						if (Count < Ref.size())
						{
							auto Index = ++Count;
							auto Da = SymbolToString(Ref[Index]);
							if (Da) {
								Result += *Da;
								Result += U' ';
							}
							else {
								SearchStack.push_back({ false, Ref[Index], 0 });
							}
						}
						else {
							SearchStack.pop_back();
						}
					}
					else {
						auto& [ProIndex, Ref] = noterminal_temporary_debug_production[noterminal_temporary_start - Index];
						if (Count < Ref.size())
						{
							auto CurIndex = ++Count;
							switch (Ref[CurIndex])
							{
							case* DebugSymbol::BBrace_L: Result += U" { "; break;
							case* DebugSymbol::BBrace_R: Result += U" } "; break;
							case* DebugSymbol::MBrace_L: Result += U" [ "; break;
							case* DebugSymbol::MBrace_R: Result += U" ] "; break;
							case* DebugSymbol::SBrace_L: Result += U" ( "; break;
							case* DebugSymbol::SBrace_R: Result += U" ) "; break;
							case* DebugSymbol::Or: Result += U" | "; break;
							default: {
								auto Da = SymbolToString(Ref[CurIndex]);
								if (Da) {
									Result += *Da;
									Result += U' ';
								}
								else {
									SearchStack.push_back({ false, Ref[CurIndex], 0 });
								}
							}
								break;
							}
						}
						else {
							SearchStack.pop_back();
						}
					}
				}
				return Result;
			};

			auto S1String = ProductionIndexToString(error.possible_production_1);
			auto S2String = ProductionIndexToString(error.possible_production_2);
			throw sbnf::error{ U"reduce conflig " + Symbol + U" with production " + S1String + U" and " + S2String + U";", 0, 0 };
		}
		*/
	}
}