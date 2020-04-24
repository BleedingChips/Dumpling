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
		{SYM::Mask, UR"(\%\%\%)"},
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
	void sbnf_processer::analyze_imp(std::u32string_view code, void(*Func)(void* data, travel), void* data)
	{
		Lexical::nfa_comsumer nfa(ref.nfa_s, code);
		Syntax::lr1_processor lp(ref.lr1_s);
		Lexical::nfa_comsumer::travel current_token;
		Lexical::nfa_comsumer::travel next_token;
		lp.analyze([&]()  -> std::optional<storage_t> {
			while (true)
			{
				if (nfa)
				{
					auto re = nfa.comsume();
					if (re.acception_state != ref.unused_terminal)
					{
						std::swap(next_token, current_token);
						next_token = nfa.comsume();
						return static_cast<storage_t>(next_token.acception_state);
					}

				}
				else
					return std::nullopt;
			}
		}, [=](Syntax::lr1_processor::travel tra) {
			travel re;
			re.sym = tra.symbol;
			if (tra.is_terminal())
			{
				auto [s, e] = ref.sym_list[tra.symbol];
				re.sym_str = std::u32string_view(ref.table.data() + s, e);
				re.token_data = current_token.capture_string;
				re.terminal = {current_token.start_line_count, current_token.start_charactor_index};
			}
			else {
				auto [s, e] = ref.sym_list[tra.symbol + ref.ter_count];
				re.sym_str = std::u32string_view(ref.table.data() + s, e);
				re.noterminal = {tra.noterminal.function_enum, tra.noterminal.production_index, tra.noterminal.symbol_array, tra.noterminal.production_count};
			}
			Func(data, re);
		});
	}

	struct TokenGenerator
	{
		TokenGenerator(nfa_storage const& ref, std::u32string_view code) : comsumer(ref, code) {}
		nfa_comsumer::travel input;
		nfa_comsumer comsumer;
		std::optional<lr1::storage_t> operator()()
		{
			while (true)
			{
				if (comsumer)
				{
					auto re = comsumer.comsume();
					if (re.acception_state != *SYM::Empty && re.acception_state != *SYM::Command)
					{
						input = re;
						return static_cast<lr1::storage_t>(input.acception_state);
					}
				}
				else
					return std::nullopt;
			}
		}
	};


	sbnf sbnf::create(std::u32string_view code)
	{
		using namespace Lexical;
		using namespace Syntax;
		auto CurrentCode = code;
		using storage_t = lr1::storage_t;

		sbnf result;

		std::map<std::u32string_view, storage_t> symbol_to_index;
		std::vector<std::tuple<std::u32string, storage_t>> symbol_rex;
		lr1_storage::storage_t unused_terminal = std::numeric_limits<lr1_storage::storage_t>::max();

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
					{{*SYM::Statement, *SYM::Statement, *SYM::Mask, *SYM::Line}, 4},
					{{*SYM::Statement,*SYM::Statement,*SYM::Line}, 5}
				}, {}
			);


			std::u32string_view Token;
			std::u32string_view Rex;
			TokenGenerator Generator(nfa_instance, CurrentCode);
			lr1_processor lp(lr1_instance);
			lp.controlable_analyze(Generator, [&](lr1_processor::travel input) {
				if (input.is_terminal())
				{
					switch (input.symbol)
					{
					case* SYM::Terminal: {
						Token = Generator.input.capture_string;
					}break;
					case* SYM::Rex: {
						auto re = Generator.input.capture_string;
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
			CurrentCode = Generator.comsumer.last();
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
				{{*SYM::Statement, *SYM::Statement, *SYM::Mask, *SYM::Line}, 14},
				{{*SYM::Statement}},
				{{*SYM::Statement, *SYM::Statement, *SYM::Line}},
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

			TokenGenerator Generator(nfa_instance, CurrentCode);
			lr1_processor lp(imp);
			auto& InPutString = Generator.input.capture_string;
			lp.controlable_analyze(Generator, [&](lr1_processor::travel tra) {
				if (tra.is_terminal())
				{
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
							throw error{ std::u32string(U"Undefined Terminal : ") + std::u32string(InPutString), Generator.input.start_line_count, Generator.input.start_charactor_index };
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
							throw error{ U"Start Symbol Is Already Seted!", Generator.comsumer.lines(), 0 };
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
							productions.push_back({std::move(Productions), std::move(RemoveSet), TargetFunctionEnum });
						}
						else
							throw error{ U"Production Head Symbol Is Missing", Generator.comsumer.lines(), 0 };
					}break;
					case 14: {return false; };
					default:
						break;
					}
				}
				return true;
			});
		}

		return result;
	}

	/*
	std::tuple<std::wstring::const_iterator, std::wstring::const_iterator> read_lind(std::wstring::const_iterator begin, std::wstring::const_iterator end)
	{
		auto search_ite = begin;
		bool FindLine = false;
		for (; search_ite != end; ++search_ite)
		{
			if (*search_ite != L'\n')
				continue;
			else
			{
				auto Available = search_ite;
				if (Available != begin && *(Available - 1) == L'\r')
					Available = Available - 1;
				return { Available, search_ite + 1 };
			}
		}
		return { end, end };
	}


	sbnf translate_sbnf(std::u32string_view code)
	{
		sbnf result;
		return result;
	}
	*/

	/*
	auto parser_sbnf::translate(const lr1_processor::travel& input, const token_generator& tokens, int64_t& pro_count) const -> std::optional<travel>
	{
		if (input.is_terminal())
		{
			assert(tokens.last_tokens.has_value());
			auto [s, e] = sym_list[input.symbol];
			std::wstring_view value = { table.data() + s, e - s };
			travel tra{ input.symbol, value };
			tra.ter_data = tokens.last_tokens->token;
			tra.line = tokens.last_tokens->line;
			tra.charactor_index = tokens.last_tokens->charactor_index;
			//tra.ter_data = *tokens.last_tokens;
			return tra;
		}
		else {
			size_t production_index = input.noterminal.production_index;
			if (production_index < temporary_prodution_start)
			{
				auto [s, e] = sym_list[input.symbol - lr1::noterminal_start() + ter_count];
				std::wstring_view value = { table.data() + s, e - s };
				travel tra{ input.symbol, value };
				tra.noter_pro_index = production_index;
				tra.noter_pro_count = static_cast<size_t>(input.noterminal.production_count + pro_count);
				pro_count = 0;
				return tra;
			}
			else {
				pro_count += input.noterminal.production_count - 1;
				return std::nullopt;
			}
		}
	}

	std::optional<lr1::storage_t> parser_sbnf::token_generator::operator()(std::tuple<const wchar_t*, size_t>& ite)
	{
		if (cur_tokens)
			last_tokens = cur_tokens;

		std::vector<std::tuple<lr1::storage_t, std::wstring_view>> SearchBuffer;
		for (auto& [str, len] = ite; len > 0; )
		{
			for (auto& ite : ref.terminal_rex_imp)
			{
				auto& [rex, sym] = ite;
				std::wcmatch match;
				if (std::regex_search(str, str + len, match, rex, std::regex_constants::match_continuous))
					SearchBuffer.push_back({ sym, std::wstring_view{ match[0].first, static_cast<size_t>(match[0].second - match[0].first) } });
			}

			std::optional<std::tuple<lr1::storage_t, std::wstring_view>> Result;

			for (auto& ite : SearchBuffer)
			{
				if (Result.has_value())
				{
					if (std::get<1>(ite).size() >= std::get<1>(*Result).size())
						Result = ite;
				}
			}

			if (Result.has_value())
			{
				auto [index, tem_str] = *Result;
				size_t last_line = current_line;
				size_t last_index = charactor_index;
				for (auto ite : tem_str)
				{
					if (ite != L'\n')
						++charactor_index;
					else {
						++current_line;
						charactor_index = 0;
					}
				}
				str += tem_str.size();
				len -= tem_str.size();
				auto unused = ref.unused_terminal.find(index);
				if (unused == ref.unused_terminal.end())
				{
					cur_tokens = element{ tem_str, last_line, last_index };
					return index;
				}
			}
			else
				throw Error::SBNFError{ current_line, L"Unregenize Tokens", {str, len} };
		}
		return std::nullopt;
	}

	auto parser_sbnf::serialization()->std::vector<storage_t>
	{
		std::vector<lr1::storage_t> result;
		result.push_back(static_cast<storage_t>(table.size()));
		for (auto& ite : table)
			result.push_back(ite);
		result.push_back(static_cast<storage_t>(sym_list.size()));
		for (auto& ite : sym_list)
		{
			auto [i1, i2] = ite;
			result.push_back(static_cast<storage_t>(i1));
			result.push_back(static_cast<storage_t>(i2));
		}
		result.push_back(static_cast<storage_t>(ter_count));
		result.push_back(static_cast<storage_t>(terminal_rex.size()));
		for (auto& ite : terminal_rex)
		{
			auto& [str, index] = ite;
			result.push_back(static_cast<storage_t>(str.size()));
			for(auto& ite2 : str)
				result.push_back(ite2);
			result.push_back(index);
		}
		result.push_back(static_cast<storage_t>(unused_terminal.size()));
		for (auto& ite : unused_terminal)
			result.push_back(ite);
		result.push_back(static_cast<storage_t>(temporary_prodution_start));
		auto lr1ser = lr1imp.serialization();
		result.insert(result.end(), lr1ser.begin(), lr1ser.end());
		return std::move(result);
	}

	parser_sbnf parser_sbnf::unserialization(const storage_t* data, size_t length)
	{
		assert(length > 0);
		size_t ite = 0;
		std::wstring table;
		{
			auto size = data[ite++];
			table.reserve(size);
			for (size_t i = 0; i <size; ++i)
				table.push_back(data[ite+i]);
			ite += size;
		}
		std::vector<std::tuple<std::size_t, std::size_t>> sym_list;
		{
			auto size = data[ite++];
			sym_list.reserve(size);
			for (size_t i = 0; i < size; ++i)
			{
				std::tuple<size_t, size_t> tup = { data[ite + i * 2], data[ite + i * 2 + 1] };
				sym_list.push_back(tup);
			}
			ite += size * 2;
		}
		size_t ter_count = data[ite++];
		std::vector<std::tuple<std::wstring, storage_t>> terminal_rex;
		{
			auto size = data[ite++];
			for (size_t i = 0; i < size; ++i)
			{
				std::wstring re;
				size_t str_size = data[ite++];
				re.reserve(str_size);
				for (size_t k = 0; k < str_size; ++k)
					re.push_back(data[ite++]);
				storage_t index = data[ite++];
				terminal_rex.push_back({std::move(re), index});
			}
		}
		std::set<storage_t> unused_terminal;
		{
			auto size = data[ite++];
			for (size_t i = 0; i < size; ++i)
				unused_terminal.insert(data[ite++]);
		}
		size_t temporary_prodution_start = data[ite++];

		lr1 lr1imp = lr1::unserialization(data + ite, length - ite);
		return parser_sbnf{
			std::move(table),
			std::move(sym_list),
			ter_count,
			std::move(terminal_rex),
			std::move(unused_terminal),
			temporary_prodution_start,
			std::move(lr1imp)
		};
	}



	// Available Space, NextLine
	std::tuple<std::wstring::const_iterator, std::wstring::const_iterator> read_lind(std::wstring::const_iterator begin, std::wstring::const_iterator end)
	{
		auto search_ite = begin;
		bool FindLine = false;
		for (; search_ite != end; ++search_ite)
		{
			if (*search_ite != L'\n')
				continue;
			else
			{
				auto Available = search_ite;
				if (Available != begin && *(Available - 1) == L'\r')
					Available = Available - 1;
				return { Available, search_ite + 1};
			}
		}
		return { end, end };
	}

	enum class TerSymbol : lr1::storage_t
	{
		Empty = 0,
		Terminal = 1,
		Equal = 2,
		Mask = 3,
		Rex = 4,
		RexTerminal = 5,
		NoTerminal = 6,
		StartSymbol = 7,
		LB_Brace = 8,
		RB_Brace = 9,
		LM_Brace = 10,
		RM_Brace = 11,
		LS_Brace = 12,
		RS_Brace = 13,
		Or,

		Statement = lr1::noterminal_start(),
		TerList,
	};

	constexpr lr1::storage_t operator*(TerSymbol input) { return static_cast<lr1::storage_t>(input); }

	std::wstring_view view(std::wstring::const_iterator s, std::wstring::const_iterator e) {
		assert(s <= e);
		return std::wstring_view{&*s, static_cast<size_t>(e -s)};
	}
	*/
	
	/*
	token 的正则表达式
	删除的token
	sbnf中的未命名Token
	产生式
	运算符优先级
	*/
	/*
	std::tuple<
		std::vector<std::tuple<std::vector<std::tuple<TerSymbol, std::wstring_view>>, std::size_t>>,
		std::array<std::size_t, 4>
	> DetectCodeToSymbol(const std::wstring& code)
	{
		auto flag = std::regex::optimize | std::regex::nosubs;

		static std::wregex Lexical[] = {
			//std::wregex {LR"([^\n]*))", flag}, // line sperate
			std::wregex {LR"(\s+)", flag}, // Empty
			std::wregex {LR"([a-zA-Z_][a-zA-Z_0-9]*)", flag}, // Terminal
			std::wregex {LR"(\:\=)", flag}, // Equal
			std::wregex {LR"(\%\%\%)", flag}, // Mask
			std::wregex {LR"(\S+)", flag}, // Rex
			std::wregex {LR"(\'.+?\')", flag}, // RexTernimal
			std::wregex {LR"(\<[a-zA-Z_][a-zA-Z_0-9]*\>)", flag}, // NoTerminal 
			std::wregex {LR"(\$)", flag}, // StartSymbole
			std::wregex {LR"(\{)", flag}, // LB_Brace {
			std::wregex {LR"(\})", flag}, // RB_Brace {
			std::wregex {LR"(\[)", flag}, // LM_Brace [
			std::wregex {LR"(\])", flag}, // RM_Brace ]
			std::wregex {LR"(\()", flag}, // LS_Brace (
			std::wregex {LR"(\))", flag}, // RS_Brace )
			std::wregex {LR"(\|)", flag}, // Or
		};

		auto ite = code.begin();
		auto code_end = code.end();
		size_t line_count = 1;
		size_t state = 0;

		size_t SymbolIndex = 0;

		std::vector<std::tuple<std::vector<std::tuple<TerSymbol, std::wstring_view>>, std::size_t>> AllToken;
		std::array<std::size_t, 4> Size = { 0, 0, 0, 0 };
		while (ite != code_end)
		{
			auto line_start = ite;
			auto [line_end, line_next] = read_lind(ite, code_end);
			if (ite != line_end)
			{
				std::vector<TerSymbol> TargetLexical[4] =
				{
					{TerSymbol::Empty, TerSymbol::Terminal, TerSymbol::Equal, TerSymbol::Mask, TerSymbol::Rex},
					{TerSymbol::Empty, TerSymbol::Terminal, TerSymbol::Mask},
					{TerSymbol::Empty, TerSymbol::Terminal, TerSymbol::NoTerminal, TerSymbol::RexTerminal, TerSymbol::Equal, TerSymbol::StartSymbol, 
					TerSymbol::LB_Brace,TerSymbol::RB_Brace,TerSymbol::LM_Brace,TerSymbol::RM_Brace,TerSymbol::LS_Brace,TerSymbol::RS_Brace, TerSymbol::Or, TerSymbol::Mask},
					{TerSymbol::Empty, TerSymbol::RexTerminal, TerSymbol::Terminal, TerSymbol::LM_Brace, TerSymbol::RM_Brace, TerSymbol::LS_Brace, TerSymbol::RS_Brace}
				};

				std::vector<std::tuple<TerSymbol, std::wstring_view>> LineSymbol;
				while (ite != line_end)
				{
					std::wstring Debug{ ite, line_end };
					std::vector<std::tuple<TerSymbol, std::wstring_view>> Result;
					for (auto sym : TargetLexical[state])
					{
						std::wsmatch match;
						if (std::regex_search(ite, line_end, match, Lexical[static_cast<std::size_t>(sym)], std::regex_constants::match_continuous))
								Result.push_back({sym,  view(match[0].first, match[0].second) });
					}
					if (!Result.empty())
					{
						std::sort(Result.begin(), Result.end(), [](const std::tuple<TerSymbol, std::wstring_view>& i1, const std::tuple<TerSymbol, std::wstring_view>& i2) {
							return std::get<1>(i1).size() > std::get<1>(i2).size();
						});
						auto [sym, str] = Result[0];
						if (sym != TerSymbol::Empty)
							LineSymbol.push_back(Result[0]);
						ite += std::get<1>(Result[0]).size();
					}else
						throw Error::SBNFError{ line_count, LR"(Unregenized token)", {ite, line_end} };
				}
				if (LineSymbol.size() > 0)
				{
					if (LineSymbol.size() == 1 && std::get<0>(LineSymbol[0]) == TerSymbol::Mask)
					{
						++state;
						if (state >= 5)
							throw Error::SBNFError{ line_count, LR"(Section overfall)" , {} };
					}
					else {
						AllToken.push_back({ std::move(LineSymbol), line_count });
						++Size[state];
					}
				}
			}
			++line_count;
			ite = line_next;
		}
		return  { std::move(AllToken), Size };
	}

	parser_sbnf LoadSBNFCode(const std::wstring& code)
	{
		using type = lr1::storage_t;
		auto [AllToken,  Size] = DetectCodeToSymbol(code);

		std::array<std::size_t, 4> SectionStart = { 0, 0, 0, 0};
		std::array<std::size_t, 4> SectionEnd = { 0, 0, 0, 0 };
		for (size_t i = 1; i < 4; ++i)
			SectionStart[i] = SectionStart[i - 1] + Size[i - 1];
		for (size_t i = 0; i < 4; ++i)
			SectionEnd[i] = SectionStart[i] + Size[i];

		std::map<std::wstring_view, type> symbol_to_index;
		std::map<type, std::wstring_view> index_to_symbol;
		std::vector<std::tuple<std::wstring, type>> rexs;
		std::set<type> unused_terminal;
		std::set<type> temporary_noterminal;
		type terminal_start = 0;
		type noterminal_start = lr1::noterminal_start();
		type temporary_noterminal_start = lr1::start_symbol() - 1;
		std::optional<type> start_symbol;
		std::vector<lr1::production_input> production;
		size_t temporary_production_start = 0;
		std::vector<lr1::ope_priority> priority;
		

		{

			static lr1 imp = lr1::create(*TerSymbol::Statement, {
				{{*TerSymbol::Statement, *TerSymbol::Terminal, *TerSymbol::Equal, *TerSymbol::Rex}},
				{{*TerSymbol::Statement, *TerSymbol::Terminal, *TerSymbol::Equal, *TerSymbol::Terminal}},
				},
				{}
			);


			std::wstring_view back_buffer;
			for (size_t i = SectionStart[0]; i < SectionEnd[0]; ++i)
			{
				auto& [vec_r, lin_r] = AllToken[i];
				auto& vec = vec_r;
				auto lin = lin_r;
				lr1_process(imp, [](auto in) {return *std::get<0>(*in); }, [&](lr1_processor::travel input) {
					if (input.symbol == *TerSymbol::Terminal)
					{
						auto& ref = vec[input.terminal.token_index];
						back_buffer = std::get<1>(vec[input.terminal.token_index]);
					}
					else if (input.symbol == *TerSymbol::Rex || input.symbol == *TerSymbol::Terminal)
					{
						auto& ref = vec[input.terminal.token_index];
						auto rex = std::get<1>(vec[input.terminal.token_index]);
						auto re = symbol_to_index.insert({ back_buffer, terminal_start });
						if (re.second)
						{
							index_to_symbol.insert({ terminal_start , back_buffer });
							++terminal_start;
						}
						rexs.push_back({ std::wstring{rex}, re.first->second });
					}
				}, vec.begin(), vec.end());
			}
		}

		{


			static lr1 imp = lr1::create(*TerSymbol::Statement, {
				{{*TerSymbol::Statement, *TerSymbol::Terminal}},
				{{*TerSymbol::Statement, *TerSymbol::Statement, *TerSymbol::Terminal}},
				},
				{}
			);

			for (size_t i = SectionStart[1]; i < SectionEnd[1]; ++i)
			{
				auto& [vec_r, lin_r] = AllToken[i];
				auto& vec = vec_r;
				auto lin = lin_r;
				lr1_process(imp, [](auto in) {return *std::get<0>(*in); }, [&](lr1_processor::travel input) {
					if (input.symbol == *TerSymbol::Terminal)
					{
						auto& ref = vec[input.terminal.token_index];
						auto sym_string = std::get<1>(vec[input.terminal.token_index]);
						auto find = symbol_to_index.find(sym_string);
						if (find != symbol_to_index.end())
							unused_terminal.insert(find->second);
						else
							throw Error::SBNFError{ lin, L"Removed Terminal Is Not Defined", std::wstring{sym_string} };

					}
				}, vec.begin(), vec.end());
			}
		}

		{
			static lr1 imp = lr1::create(
				*TerSymbol::Statement, {
					{{ *TerSymbol::Statement, *TerSymbol::StartSymbol, *TerSymbol::Equal, *TerSymbol::NoTerminal }},
				{{ *TerSymbol::Statement, *TerSymbol::StartSymbol, *TerSymbol::Equal, *TerSymbol::Terminal }},
				{{ *TerSymbol::Statement, *TerSymbol::NoTerminal, *TerSymbol::Equal, *TerSymbol::TerList }},
				{{ *TerSymbol::Statement, *TerSymbol::NoTerminal, *TerSymbol::Equal}},
				{{ *TerSymbol::Statement, *TerSymbol::Equal, *TerSymbol::TerList }},
				{{ *TerSymbol::Statement, *TerSymbol::Equal }},
				{{ *TerSymbol::TerList, *TerSymbol::NoTerminal }},
				{{ *TerSymbol::TerList, *TerSymbol::Terminal }},
				{{ *TerSymbol::TerList, *TerSymbol::RexTerminal }},
				{{ *TerSymbol::TerList, *TerSymbol::TerList, *TerSymbol::TerList}},
				{{ *TerSymbol::TerList, *TerSymbol::TerList, *TerSymbol::Or, *TerSymbol::TerList }},
				{{ *TerSymbol::TerList, *TerSymbol::LS_Brace, *TerSymbol::TerList, *TerSymbol::RS_Brace }},
				{{ *TerSymbol::TerList, *TerSymbol::LM_Brace, *TerSymbol::TerList, *TerSymbol::RM_Brace }},
				{{ *TerSymbol::TerList, *TerSymbol::LB_Brace, *TerSymbol::TerList, *TerSymbol::RB_Brace }},
				},
				{}
			);

			struct temporary_symbol_pair
			{
				std::vector<type> left;
				std::vector<type> right;
				// 1 -> or; 2 -> zero or one; 3 -> any
				size_t ope;
				bool operator<(const temporary_symbol_pair& pair) const noexcept {
					if (ope < pair.ope)
						return true;
					else if (ope == pair.ope)
					{
						if (left < pair.left)
							return true;
						else if (left == pair.left)
						{
							if (right < pair.right)
								return true;
						}
					}
					return false;
				}
			};

			std::vector<std::vector<type>> stack;
			std::map<temporary_symbol_pair, type> temporary_noterminal_buffer;
			std::optional<type> last_no_terminal;
			std::vector<std::vector<type>> temporary_production;

			for (size_t i = SectionStart[2]; i < SectionEnd[2]; ++i)
			{
				auto& [vec_r, lin_r] = AllToken[i];
				auto& vec = vec_r;
				auto lin = lin_r;
				lr1_process(imp, [](auto in) {return *std::get<0>(*in); }, [&](lr1_processor::travel input) {
					if (input.is_terminal())
					{
						auto& ref = vec[input.terminal.token_index];
						auto sym_string = std::get<1>(ref);
						switch (input.symbol)
						{
						case* TerSymbol::NoTerminal:
						{
							auto result = symbol_to_index.insert({ sym_string, noterminal_start });
							if (result.second)
							{
								index_to_symbol.insert({ noterminal_start, sym_string });
								++noterminal_start;
							}
							stack.push_back({ result.first->second });
							break;
						}
						case* TerSymbol::RexTerminal:
						{
							auto result = symbol_to_index.insert({ sym_string, terminal_start });
							if (result.second)
							{
								index_to_symbol.insert({ terminal_start, sym_string });
								assert(sym_string.size() >= 3);
								static const std::wstring SpecialChar = LR"($()*+.[]?\^{}|,\)";
								std::wstring Rex;
								Rex.reserve(sym_string.size());
								for (auto ite = sym_string.begin() + 1; ite != sym_string.end(); ++ite)
								{
									for (auto ite2 : SpecialChar)
									{
										if (*ite == ite2)
										{
											Rex.push_back(L'\\');
											break;
										}
									}
									Rex.push_back(*ite);
								}
								rexs.push_back({ std::move(Rex), result.first->second });
								++terminal_start;
							}
							stack.push_back({ result.first->second });
							break;
						}
						case* TerSymbol::Terminal:
						{
							auto ite = symbol_to_index.find(sym_string);
							if (ite != symbol_to_index.end())
							{
								if (unused_terminal.find(ite->second) == unused_terminal.end())
									stack.push_back({ ite->second });
								else
									throw Error::SBNFError{ lin, L"Removed Terminal used in Production", std::wstring{sym_string} };
							}
							else
								throw Error::SBNFError{ lin, L"Undefined Terminal", std::wstring{sym_string} };
							break;
						}
						default:
							break;

						}
					}
					else {
						switch (input.noterminal.production_index)
						{
						case 0:
						case 1:
						{
							last_no_terminal = std::nullopt;
							assert(stack.size() == 1 && stack[0].size() == 1);
							if (!start_symbol)
								start_symbol = stack[0][0];
							else
							{
								auto Find = index_to_symbol.find(*start_symbol);
								assert(Find != index_to_symbol.end());
								throw Error::SBNFError{ lin, L"Mulity Define Start Symbol", std::wstring{Find->second} };
							}
							stack.clear();
							break;
						}
						case 2:
						{
							assert(stack.size() == 2 && stack[0].size() == 1);
							last_no_terminal = stack[0][0];
							auto ite = std::move(stack[0]);
							ite.insert(ite.end(), std::move_iterator(stack[1].begin()), std::move_iterator(stack[1].end()));
							production.push_back(std::move(ite));
							stack.clear();
							break;
						}
						case 3:
						{
							assert(stack.size() == 1 && stack[0].size() == 1);
							last_no_terminal = stack[0][0];
							production.push_back({ { *last_no_terminal }, 0, {} });
							stack.clear();
							break;
						}
						case 4:
						{
							assert(stack.size() == 1);
							if (last_no_terminal)
							{
								std::vector<type> pre = { *last_no_terminal };
								pre.insert(pre.end(), stack[0].begin(), stack[0].end());
								production.push_back({ std::move(pre), 0 , {} });
								stack.clear();
							}
							else
								throw Error::SBNFError{ lin, L"Miss No Terminal Define", std::wstring{} };
							break;
						}
						case 5:
						{
							assert(stack.size() == 0);
							if (last_no_terminal)
								production.push_back({ { *last_no_terminal }, 0, {} });
							else
								throw Error::SBNFError{ lin, L"Miss No Terminal Define", std::wstring{} };
							break;
						}
						case 6:
						case 7:
						case 8:
						{
							break;
						}
						case 9:
						{
							assert(stack.size() >= 2);
							auto ite = std::move(*stack.rbegin());
							stack.pop_back();
							auto& ite2 = *stack.rbegin();
							ite2.insert(ite2.end(), std::move_iterator(ite.begin()), std::move_iterator(ite.end()));
							break;
						}
						case 10: // or
						{
							assert(stack.size() >= 2);
							auto ite = std::move(*stack.rbegin());
							stack.pop_back();
							auto ite2 = std::move(*stack.rbegin());
							stack.pop_back();
							temporary_symbol_pair pair{ std::move(ite), std::move(ite2), 1 };
							auto re = temporary_noterminal_buffer.insert({ std::move(pair), temporary_noterminal_start });
							if (re.second)
							{
								std::vector<type> tem = { re.first->second };
								auto& l = re.first->first.left;
								auto& r = re.first->first.right;
								tem.insert(tem.end(), l.begin(), l.end());
								temporary_production.push_back(std::move(tem));
								tem = { re.first->second };
								tem.insert(tem.end(), r.begin(), r.end());
								temporary_production.push_back(std::move(tem));
								--temporary_noterminal_start;
								assert(temporary_noterminal_start > noterminal_start);
							}
							stack.push_back({ re.first->second });
						}
						case 11: // ()
							break;
						case 12: // []
						{
							assert(stack.size() >= 1);
							auto ite = std::move(*stack.rbegin());
							stack.pop_back();
							temporary_symbol_pair pair{ std::move(ite), {}, 1 };
							auto re = temporary_noterminal_buffer.insert({ std::move(pair), temporary_noterminal_start });
							if (re.second)
							{
								std::vector<type> tem = { re.first->second };
								auto& l = re.first->first.left;
								tem.insert(tem.end(), l.begin(), l.end());
								temporary_production.push_back(std::move(tem));
								tem = { re.first->second };
								temporary_production.push_back(std::move(tem));
								--temporary_noterminal_start;
								assert(temporary_noterminal_start > noterminal_start);
							}
							stack.push_back({ re.first->second });
							break;
						}
						case 13://{}
						{
							assert(stack.size() >= 1);
							auto ite = std::move(*stack.rbegin());
							stack.pop_back();
							temporary_symbol_pair pair{ std::move(ite), {}, 2 };
							auto re = temporary_noterminal_buffer.insert({ std::move(pair), temporary_noterminal_start });
							if (re.second)
							{
								std::vector<type> tem = { re.first->second, re.first->second };
								auto& l = re.first->first.left;
								tem.insert(tem.end(), l.begin(), l.end());
								temporary_production.push_back(std::move(tem));
								tem = { re.first->second };
								temporary_production.push_back(std::move(tem));
								--temporary_noterminal_start;
								assert(temporary_noterminal_start > noterminal_start);
							}
							stack.push_back({ re.first->second });
							break;
						}
						// todo
						default:
							break;
						}
					}
				}, vec.begin(), vec.end());

			}

			temporary_production_start = production.size();
			for (auto& ite : temporary_production)
				production.push_back({ std::move(ite), 0, {} });

			for (auto& ite : temporary_noterminal_buffer)
				temporary_noterminal.insert(std::get<1>(ite));

		}
		{
			lr1 imp = lr1::create(
				*TerSymbol::Statement, {
					{{ *TerSymbol::TerList, *TerSymbol::RexTerminal }},
				{{ *TerSymbol::TerList, *TerSymbol::Terminal }},
				{{ *TerSymbol::TerList, *TerSymbol::TerList,  *TerSymbol::TerList}},
				{{ *TerSymbol::Statement, *TerSymbol::RexTerminal }},
				{{ *TerSymbol::Statement, *TerSymbol::Terminal }},
				{{ *TerSymbol::Statement, *TerSymbol::LS_Brace, *TerSymbol::TerList, *TerSymbol::RS_Brace }},
				{{ *TerSymbol::Statement, *TerSymbol::LM_Brace, *TerSymbol::TerList, *TerSymbol::RM_Brace }},
				{{ *TerSymbol::Statement, *TerSymbol::Statement, *TerSymbol::Statement }},
				},
				{}
			);

			for (size_t i = SectionStart[4]; i < SectionEnd[4]; ++i)
			{
				auto& [vec_r, lin_r] = AllToken[i];
				auto& vec = vec_r;
				auto lin = lin_r;
				// size -> 0 : left; 1 : Right
				std::vector<type> stack;
				lr1_process(imp, [](auto in) {return *std::get<0>(*in); }, [&](lr1_processor::travel input) {
					if (input.is_terminal())
					{
						auto& ref = vec[input.terminal.token_index];
						auto [type, sym_string] = (ref);
						if (type == TerSymbol::RexTerminal || type == TerSymbol::Terminal)
						{
							auto ite = symbol_to_index.find(sym_string);
							if (ite != symbol_to_index.end())
								stack.push_back(ite->second);
							else
								throw Error::SBNFError{ lin, L"No Operator Define", std::wstring{sym_string} };
						}
					}
					else {
						switch (input.noterminal.production_index)
						{
						case 0:case 1: case 2:case 7:break;
						case 3: case 4:
						{
							assert(stack.size() == 1);
							priority.push_back(stack[0]);
							stack.clear();
							break;
						}
						case 5:
						{
							priority.push_back(std::move(stack));
							stack.clear();
							break;
						}
						case 6:
						{
							priority.push_back({ std::move(stack), false });
							stack.clear();
							break;
						}
						}
					}
				}, vec.begin(), vec.end());
			}
		}

		

		if (start_symbol)
		{
			std::wstring table;
			std::vector<std::tuple<size_t, size_t>> sym_list;
			sym_list.resize(index_to_symbol.size());
			for (auto& ite : index_to_symbol)
			{
				auto [index, str] = ite;
				size_t size = table.size();
				table.insert(table.end(), str.begin(), str.end());
				if (lr1::is_terminal(index))
				{
					assert(index < terminal_start);
					sym_list[index] = { size, str.size() };
				}
				else {
					size_t true_index = index - lr1::noterminal_start() + terminal_start;
					assert(true_index < sym_list.size());
					sym_list[true_index] = { size, str.size() };
				}
			}

			lr1 imp = lr1::create(
				*start_symbol, 
				std::move(production),
				std::move(priority)
			);

			parser_sbnf sbnf{
				std::move(table),
				std::move(sym_list),
				terminal_start,
				std::move(rexs),
				std::move(unused_terminal),
				temporary_production_start,
				std::move(imp)
			};

			return std::move(sbnf);
		}
		else
			throw Error::SBNFError{ 0, LR"(No Define Start Synbol)", {} };

		
	}

	std::optional<parser_sbnf> LoadSBNFFile(const std::filesystem::path& Path, std::wstring& storage)
	{
		using namespace Potato::Encoding;
		std::ifstream input(Path, std::ios::binary);
		if (input.is_open())
		{
			storage.clear();
			char Bom[4] = { 0,0, 0, 0 };
			input.read(Bom, 4);
			auto [type, size] = translate_binary_to_bomtype(reinterpret_cast<const std::byte*>(Bom), 4);
			auto buffer_size = std::filesystem::file_size(Path);
			assert(buffer_size <= std::numeric_limits<size_t>::max() && buffer_size > size);
			buffer_size -= size;
			std::byte* Read = new std::byte[buffer_size];
			input.seekg(size, std::ios::beg);
			input.read(reinterpret_cast<char*>(Read), buffer_size);
			std::wstring result;
			switch (type)
			{
			case BomType::UTF8:
			case BomType::None:
			{
				string_encoding<char> utf8_string(reinterpret_cast<char*>(Read), buffer_size);
				storage = utf8_string.to_string<wchar_t>();
				break;
			}
			default:
				assert(false);
				break;
			}
			return { LoadSBNFCode(storage) };
		}
		return std::nullopt;
	}
	*/
}