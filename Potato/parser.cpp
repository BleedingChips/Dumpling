#include "parser.h"
#include <assert.h>
#include <vector>

namespace
{

	using namespace Potato::Lexical;
	using namespace Potato::Syntax;

	enum class SYM : lr1_storage::storage_t
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
		TokenMax,
		Command,

		Statement = lr1::noterminal_start(),
		FunctionEnum,
		ProductionHead,
		RemoveElement,
		RemoveExpression,
		Expression,
	};

	std::map<SYM, std::u32string_view> Rexs = {
		{SYM::Empty, UR"(\s)" },
		{SYM::Line, U"\r\n|\n"},
		{SYM::Terminal, UR"([a-zA-Z_][a-zA-Z_0-9]*)"},
		{SYM::Equal, UR"(:=)"},
		{SYM::Mask, UR"(%%%\s*?\n)"},
		{SYM::Rex, UR"('.*?[^\\]')"},
		{SYM::NoTerminal, UR"(\<[_a-zA-Z][_a-zA-Z0-9]*\>)"},
		{SYM::StartSymbol, UR"(\$)"},
		{SYM::LB_Brace, UR"(\{)"},
		{SYM::RB_Brace, UR"(\})"},
		{SYM::LM_Brace, UR"(\[)"},
		{SYM::RM_Brace, UR"(\])"},
		{SYM::LS_Brace, UR"(\()"},
		{SYM::RS_Brace, UR"(\))"},
		{SYM::Colon, UR"(:)"},
		{SYM::Or, UR"(\|)"},
		{SYM::Number, UR"([1-9][0-9]*)"},
		{SYM::Command, UR"(/\*[.\n]*?\*/|//.*?\n)"},
	};
	constexpr lr1::storage_t operator*(SYM i) { return static_cast<size_t>(i); }

}

namespace Potato::Parser
{

	std::u32string_view sbnf::find_symbol(storage_t input) const noexcept {
		if (lr1::is_terminal(input))
		{
			if (input < ter_count)
			{
				auto [s, size] = symbol_map[input];
				return std::u32string_view{ table.data() + s, size };
			}
		}
		else {
			auto Index = input - lr1::noterminal_start() + ter_count;
			if (Index < symbol_map.size())
			{
				auto [s, size] = symbol_map[Index];
				return std::u32string_view{ table.data() + s, size };
			}
		}
		return {};
	}

	char const* sbnf::unacceptable_token_error::what() const noexcept
	{
		return "Unacceprable Token Error";
	}

	char const* sbnf::undefine_terminal_error::what() const noexcept
	{
		return "Undefine Terminal Error";
	}

	char const* sbnf::miss_start_symbol::what() const noexcept
	{
		return "Missing Start Symbol Error";
	}

	auto sbnf::find_symbol(std::u32string_view sym) const noexcept -> std::optional<storage_t> {
		for (storage_t i = 0; i < symbol_map.size(); ++i)
		{
			auto [s, size] = symbol_map[i];
			auto str = std::u32string_view{ table.data() + s, size };
			if (str == sym)
			{
				if (i > ter_count)
					return i + lr1::noterminal_start();
				else
					return i;
			}
		}
		return std::nullopt;
	};

	struct DefultLexer : nfa_lexer
	{
		using nfa_lexer::nfa_lexer;
		nfa_lexer::travel stack() const noexcept { return input_stack; }
		void reset_remove(std::set<size_t> rm) { remove = std::move(rm); }
		std::optional<lr1_storage::storage_t> operator()() {
			while (true)
			{
				if (nfa_lexer::operator bool())
				{
					auto result = nfa_lexer::operator()();
					if (result)
					{
						auto ite = remove.find(*result);
						if (ite == remove.end())
						{
							input_stack = nfa_lexer::stack();
							return static_cast<lr1_storage::storage_t>(*result);
						}
					}
					else{
						auto re = nfa_lexer::last();
						throw sbnf::unacceptable_token_error(std::u32string(re.data(), re.size() < 10 ? re.size() : 10), current_line(), current_index());
					}
				}
				else
					return std::nullopt;
			}
		}
	private:
		nfa_lexer::travel input_stack;
		std::set<size_t> remove;
	};

	void sbnf_processer::analyze_imp(std::u32string_view code, void(*Func)(void* data, travel), void* data)
	{
		assert(Func != nullptr);
		DefultLexer Wrapper(ref.nfa_s, code);
		Wrapper.reset_remove({ ref.unused_terminal });
		Syntax::lr1_processor lp(ref.lr1_s);
		std::vector<storage_t> ProductionStorage;
		std::vector<storage_t> TemporaryProductionCountReserve;
		std::vector<size_t> TemporaryProductionCount;
		size_t TemporarySymbolCount = 0;
		lp.analyze(Wrapper, [&](Syntax::lr1_processor::travel tra) {
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

	struct LexerWrapper : nfa_lexer
	{
		using nfa_lexer::nfa_lexer;
		size_t state = 0;
		nfa_lexer::travel stack() const noexcept { return input_stack; }
		void reset_remove(std::set<size_t> rm) { remove = std::move(rm); }
		std::optional<lr1_storage::storage_t> operator()() {
			if (state == 1)
			{
				state = 0;
				return std::nullopt;
			}
			while (true)
			{
				if (nfa_lexer::operator bool())
				{
					auto result = nfa_lexer::operator()();
					if (result)
					{
						switch (*result)
						{
						case *SYM::Command:
						case *SYM::Empty:
							break;
						case *SYM::Mask:
							return std::nullopt;
						default:
							input_stack = nfa_lexer::stack();
							return static_cast<lr1_storage::storage_t>(*result);
						}
					}
					else {
						auto re = nfa_lexer::last();
						throw sbnf::unacceptable_token_error(std::u32string(re.data(), re.size() < 10 ? re.size() : 10), current_line(), current_index());
					}
				}
				else
					return std::nullopt;
			}
		}
	private:
		nfa_lexer::travel input_stack;
		std::set<size_t> remove;
	};


	sbnf sbnf::create(std::u32string_view code)
	{
		using namespace Lexical;
		using namespace Syntax;
		using storage_t = lr1::storage_t;

		std::map<std::u32string_view, storage_t> symbol_to_index;
		std::vector<std::tuple<std::u32string, storage_t>> symbol_rex;
		lr1_storage::storage_t unused_terminal = std::numeric_limits<lr1_storage::storage_t>::max();

		LexerWrapper Generator(code);

		// step1
		{
			static nfa_storage nfa_instance = ([]() -> nfa_storage {
				std::vector<SYM> RequireList = { SYM::Terminal, SYM::Equal, SYM::Mask, SYM::Rex, SYM::Line, SYM::Command, SYM::Empty };
				nfa tem;
				for (auto& ite : RequireList)
					tem.append_rex(Rexs[ite], *ite);
				return tem.simplify();
			}());

			static lr1_storage lr1_instance = lr1::create(
				*SYM::Statement, {
					{{*SYM::Statement, *SYM::Statement, *SYM::Terminal, *SYM::Equal, *SYM::Rex, *SYM::Line}, 1},
					{{*SYM::Statement}, 3},
					//{{*SYM::Statement,*SYM::Statement,  *SYM::Mask}},
					{{*SYM::Statement,*SYM::Statement,  *SYM::Line}}
				}, {}
			);


			std::u32string_view Token;
			std::u32string_view Rex;
			Generator.reset_nfa(nfa_instance);
			Generator.reset_remove({ *SYM::Empty, *SYM::Command });
			lr1_processor lp(lr1_instance);
			lp.controlable_analyze(Generator, [&](lr1_processor::travel input) {
				if (input.is_terminal())
				{
					switch (input.symbol)
					{
					case* SYM::Terminal: {
						Token = Generator.stack().capture;
					}break;
					case* SYM::Rex: {
						auto re = Generator.stack().capture;
						Rex = { re.data() + 1, re.size() -2 };
					}break;
					default:break;
					}
				}
				else {
					switch (input.noterminal.function_enum)
					{
					case 1: {
						auto re = symbol_to_index.insert({ Token, static_cast<storage_t>(symbol_to_index.size()) });
						symbol_rex.push_back({ std::u32string(Rex) , re.first->second});
					}break;
					case 4:
						return false;
						break;
					default: break;
					}
				}
				return true;
			});
			auto Find = symbol_to_index.find(U"_IGNORE");
			if (Find != symbol_to_index.end())
				unused_terminal = static_cast<lr1::storage_t>(Find->second);
		}

		std::map<std::u32string_view, storage_t> noterminal_symbol_to_index;
		std::vector<lr1::production_input> productions;
		std::optional<storage_t> start_symbol;
		storage_t noterminal_temporary = lr1::start_symbol() - 1;

		// step2
		{

			static nfa_storage nfa_instance = ([]() -> nfa_storage {
				std::vector<SYM> RequireList = {
					SYM::StartSymbol, SYM::Colon, SYM::Terminal, SYM::Equal, SYM::Number, SYM::NoTerminal, SYM::Mask, SYM::Rex, SYM::Line, 
					SYM::LS_Brace, SYM::RS_Brace, SYM::LM_Brace, SYM::RM_Brace, SYM::LB_Brace, SYM::RB_Brace, SYM::Command, SYM::Empty 
				};
				nfa tem;
				for (auto& ite : RequireList)
					tem.append_rex(Rexs[ite], *ite);
				return tem.simplify();
			}());


			static lr1_storage imp = lr1::create(
				*SYM::Statement, {
				{{*SYM::Expression, *SYM::NoTerminal }, 1},
				{{*SYM::Expression, *SYM::Terminal }, 2},
				{{*SYM::Expression, *SYM::Rex }, 3},
				{{*SYM::Expression, *SYM::Expression, *SYM::Expression }, 4},
				{{*SYM::Expression, *SYM::LS_Brace, *SYM::Expression, *SYM::RS_Brace}},
				{{*SYM::Expression, *SYM::LB_Brace, *SYM::Expression, *SYM::RB_Brace}, 5},
				{{*SYM::Expression, *SYM::LM_Brace, *SYM::Expression, *SYM::RM_Brace}, 6},
				{{*SYM::Expression, *SYM::Expression, *SYM::Or, *SYM::Expression}, {*SYM::Expression}, 7},

				{{*SYM::FunctionEnum}},
				{{*SYM::FunctionEnum, *SYM::LM_Brace, *SYM::Number, *SYM::RM_Brace}},

				{{*SYM::ProductionHead}},
				{{*SYM::ProductionHead, *SYM::NoTerminal}, 9},

				{{*SYM::RemoveElement, *SYM::Terminal}, 10},
				{{*SYM::RemoveElement, *SYM::NoTerminal}, 10},
				{{*SYM::RemoveElement, *SYM::Rex}, 10},
				{{*SYM::RemoveElement, *SYM::RemoveElement, *SYM::RemoveElement}, 11},

				{{*SYM::RemoveExpression, *SYM::Colon, *SYM::RemoveElement}},
				{{*SYM::RemoveExpression}},

				{{*SYM::Statement, *SYM::Statement, *SYM::StartSymbol, *SYM::Equal, *SYM::NoTerminal, *SYM::Line }, 12},
				{{*SYM::Statement, *SYM::Statement, *SYM::ProductionHead, *SYM::Equal, *SYM::Expression, *SYM::RemoveExpression, *SYM::FunctionEnum, *SYM::Line}, 13},
				{{*SYM::Statement, *SYM::Statement, *SYM::ProductionHead, *SYM::Equal, *SYM::RemoveExpression, *SYM::FunctionEnum, *SYM::Line}, 13},

				{{*SYM::Statement}},
				{{*SYM::Statement, *SYM::Statement, *SYM::Line}},
				//{{*SYM::Statement, *SYM::Statement, *SYM::Mask}},
				},
				{}
			);

			std::optional<storage_t> LastHead;
			storage_t Input;
			std::vector<storage_t> Tokens;
			std::optional<storage_t> FunctionEnum;
			//std::map<std::u32string_view, storage_t> Mapping;
			std::vector<std::vector<storage_t>> tem_production;
			std::vector<std::vector<storage_t>> tem_remove;

			Generator.reset_nfa(nfa_instance);
			lr1_processor lp(imp);
			lp.controlable_analyze(Generator, [&](lr1_processor::travel tra) {
				if (tra.is_terminal())
				{
					auto InPutString = Generator.stack().capture;
					switch (tra.symbol)
					{
					case* SYM::NoTerminal: {
						auto Find = noterminal_symbol_to_index.insert({ InPutString, static_cast<storage_t>(noterminal_symbol_to_index.size() + lr1::noterminal_start()) });
						Input = Find.first->second;
					}break;
					case* SYM::Terminal: {
						auto Find = symbol_to_index.find(InPutString);
						if (Find != symbol_to_index.end())
							Input = Find->second;
						else
							throw error{ std::u32string(U"Undefined Terminal : ") + std::u32string(InPutString), Generator.current_line(), Generator.current_index() };
					}break;
					case* SYM::Rex: {
						static const std::u32string SpecialChar = UR"($()*+.[]?\^{}|,\)";
						assert(InPutString.size() >= 2);
						auto re = symbol_to_index.insert({ InPutString, static_cast<storage_t>(symbol_to_index.size()) });
						if (re.second)
						{
							std::u32string rex;
							for (size_t i = 1; i < InPutString.size() - 1; ++i)
							{
								for (auto& ite : SpecialChar)
									if (ite == InPutString[i])
									{
										rex.push_back(U'\\');
										break;
									}
								rex.push_back(InPutString[i]);
							}
							symbol_rex.push_back({ std::move(rex), re.first->second });
						}
						Input = re.first->second;
					}break;
					case* SYM::Number: {
						storage_t Number = 0;
						for (auto ite : InPutString)
							Number = Number * 10 + ite - U'0';
						FunctionEnum = Number;
					}break;
					case* SYM::Mask: return false;
					default: break;
					}
				}
				else
				{
					switch (tra.noterminal.function_enum)
					{
					case 1: case 2: case 3:{
						tem_production.push_back({ Input });
					}break;
					case 4: {
						assert(tem_production.size() >=2);
						auto& Ref = *(tem_production.rbegin() + 1);
						auto& Ref2 = *(tem_production.rbegin());
						Ref.insert(Ref.end(), Ref2.begin(), Ref2.end());
						tem_production.pop_back();
					}break;
					case 5: {
						assert(tem_production.size() >= 1);
						storage_t TemProduction = noterminal_temporary--;
						assert(TemProduction > noterminal_symbol_to_index.size() + lr1::noterminal_start());
						std::vector<storage_t> Pro = { TemProduction };
						productions.push_back(lr1::production_input{ Pro });
						auto& ref = *tem_production.rbegin();
						Pro.push_back(TemProduction);
						Pro.insert(Pro.end(), ref.begin(), ref.end());
						ref = { TemProduction };
						productions.push_back(lr1::production_input{ std::move(Pro) });
					} break;
					case 8:
					case 6: {
						assert(tem_production.size() >= 1);
						storage_t TemProduction = noterminal_temporary--;
						assert(TemProduction > noterminal_symbol_to_index.size() + lr1::noterminal_start());
						std::vector<storage_t> Pro = { TemProduction };
						productions.push_back(lr1::production_input{ Pro });
						auto& ref = *tem_production.rbegin();
						Pro.insert(Pro.end(), ref.begin(), ref.end());
						ref = { TemProduction };
						productions.push_back(lr1::production_input{ std::move(Pro) });
					}break;
					case 7: {
						assert(tem_production.size() >= 2);
						auto& Ref = *tem_production.rbegin();
						auto& Ref2 = *(tem_production.rbegin() + 1);
						storage_t TemProduction = noterminal_temporary--;
						assert(TemProduction > noterminal_symbol_to_index.size() + lr1::noterminal_start());
						std::vector<storage_t> Pro = { TemProduction };
						Pro.insert(Pro.end(), Ref.begin(), Ref.end());
						productions.push_back(lr1::production_input{ std::move(Pro) });
						std::vector<storage_t> Pro2 = { TemProduction };
						Pro2.insert(Pro2.end(), Ref2.begin(), Ref2.end());
						productions.push_back(lr1::production_input{ std::move(Pro2) });
						Ref2 = { TemProduction };
						tem_production.pop_back();
					}break;
					case 9: {
						LastHead = Input;
					}break;
					case 10: {tem_remove.push_back({ Input }); } break;
					case 11: {
						assert(tem_remove.size() >= 2);
						auto& Ref = *tem_production.rbegin();
						auto& Ref2 = *(tem_production.rbegin() + 1);
						Ref2.insert(Ref2.end(), Ref.begin(), Ref.end());
						tem_production.pop_back();
					}break;
					case 12: {
						if (!start_symbol)
						{
							start_symbol = Input;
							LastHead = std::nullopt;
						}
						else
							throw error{ U"Start Symbol Is Already Seted!", Generator.current_line(), 0 };
					}break;
					case 13: {
						if (LastHead)
						{
							assert(tem_production.size() <= 1);
							assert(tem_remove.size() <= 1);
							std::vector<storage_t> Productions{ *LastHead };
							if (!tem_production.empty())
							{
								Productions.insert(Productions.end(), tem_production[0].begin(), tem_production[0].end());
								tem_production.clear();
							}
							std::vector<storage_t> RemoveSet;
							if (!tem_remove.empty())
							{
								RemoveSet = std::move(std::move(tem_remove[0]));
								tem_remove.clear();
							}
							storage_t TargetFunctionEnum = lr1::no_function_enum();
							if (FunctionEnum)
							{
								TargetFunctionEnum = *FunctionEnum;
								FunctionEnum = std::nullopt;
							}
							productions.push_back({ std::move(Productions), std::move(RemoveSet), TargetFunctionEnum });
						}
						else
							throw error{ U"Production Head Symbol Is Missing", Generator.current_line(), 0 };
					}break;
					case 14: {return false; };
					default:
						break;
					}
				}
				return true;
			});
		}


		std::vector<lr1::ope_priority> operator_priority;
		// step3
		{
			static nfa_storage nfa_instance = ([]() -> nfa_storage {
				std::vector<SYM> RequireList = { SYM::Terminal, SYM::Rex, SYM::Command,
					SYM::LS_Brace, SYM::RS_Brace, SYM::LM_Brace, SYM::RM_Brace, SYM::Empty };
				nfa tem;
				for (auto& ite : RequireList)
					tem.append_rex(Rexs[ite], *ite);
				return tem.simplify();
			}());

			static lr1_storage lr1_instance = lr1::create(
				*SYM::Statement, {
					{{*SYM::Expression, *SYM::Terminal}, 1},
					{{*SYM::Expression, *SYM::Rex}, 1},
					{{*SYM::Expression, *SYM::Expression, *SYM::Expression}, 2},
					{{*SYM::Statement, *SYM::Statement, *SYM::Terminal}, 3},
					{{*SYM::Statement, *SYM::Statement, *SYM::Rex}, 3},
					{{*SYM::Statement, *SYM::Statement, *SYM::LS_Brace, *SYM::Expression, *SYM::RS_Brace}, 4},
					{{*SYM::Statement, *SYM::Statement, *SYM::LM_Brace, *SYM::Expression, *SYM::RM_Brace}, 5},
					{{*SYM::Statement}},
				}, {}
			);

			std::vector<storage_t> tokens;
			Generator.reset_nfa(nfa_instance);
			lr1_processor lp(lr1_instance);
			lp.analyze(Generator, [&](lr1_processor::travel tra) {
				if (tra.is_terminal())
				{
					if (tra.symbol == *SYM::Terminal || tra.symbol == *SYM::Rex)
					{
						auto Find = symbol_to_index.find(Generator.stack().capture);
						if (Find != symbol_to_index.end())
							tokens.push_back(Find->second);
						else
							throw undefine_terminal_error(std::u32string(Generator.stack().capture), Generator.current_line(), Generator.current_index());
					}
				}
				else {
					switch (tra.noterminal.function_enum)
					{
					case 3: case 4:{
						assert(tokens.size() >= 1);
						operator_priority.push_back(lr1::ope_priority{ std::move(tokens), true });
					} break;
					case 5: {
						assert(tokens.size() >= 1);
						operator_priority.push_back(lr1::ope_priority{ std::move(tokens), false });
					} break;
					}
				}
			});
		}

		if (!start_symbol)
			throw miss_start_symbol();

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
			symbol_map[ite.second + TerminalCount - lr1::noterminal_start()] = { start, ite.first.size() };
		}
		Lexical::nfa nfa_temporary;
		for (auto ite = symbol_rex.rbegin(); ite != symbol_rex.rend(); ++ite)
		{
			auto& [str, index] = *ite;
			nfa_temporary.append_rex(str, index);
		}
		Syntax::lr1 lr1_temporay = Syntax::lr1::create(
			*start_symbol, std::move(productions), std::move(operator_priority)
		);

		return {std::move(table), std::move(symbol_map), TerminalCount, unused_terminal, noterminal_temporary, nfa_temporary, lr1_temporay};
	}
}