#include "parser.h"
#include <fstream>
#include "character_encoding.h"
#include <assert.h>
namespace Potato
{

	parser_sbnf::parser_sbnf(
		std::wstring table,
		std::vector<std::tuple<std::size_t, std::size_t>> sym_list,
		size_t ter_count,
		std::vector<std::tuple<std::wstring, storage_t>> terminal_rex,
		std::set<storage_t> unused_terminal,
		size_t temporary_prodution_start,
		lr1 lr1imp
	) : table(std::move(table)), sym_list(std::move(sym_list)), ter_count(ter_count),
		terminal_rex(std::move(terminal_rex)), unused_terminal(std::move(unused_terminal)),
		temporary_prodution_start(temporary_prodution_start), lr1imp(std::move(lr1imp))
	{
		build_rex();
	}

	void parser_sbnf::build_rex()
	{
		auto flag = std::regex::optimize | std::regex::nosubs;
		terminal_rex_imp.clear();
		for (auto& ite : terminal_rex)
		{
			auto [rex, sym] = ite;
			std::wregex tem_rex{ rex, flag };
			terminal_rex_imp.push_back({ std::move(tem_rex), sym });
		}
	}

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
			size_t production_index = input.no_terminal_production_index;
			if (production_index < temporary_prodution_start)
			{
				auto [s, e] = sym_list[input.symbol - lr1::noterminal_start() + ter_count];
				std::wstring_view value = { table.data() + s, e - s };
				travel tra{ input.symbol, value };
				tra.noter_pro_index = production_index;
				tra.noter_pro_count = static_cast<size_t>(input.no_terminal_production_count + pro_count);
				pro_count = 0;
				return tra;
			}
			else {
				pro_count += input.no_terminal_production_count - 1;
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
	
	/*
	token 的正则表达式
	删除的token
	sbnf中的未命名Token
	产生式
	运算符优先级
	*/
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
		std::vector<std::vector<type>> production;
		size_t temporary_production_start = 0;
		std::vector<lr1::ope_priority> priority;
		

		{

			static lr1 imp(*TerSymbol::Statement, {
				{*TerSymbol::Statement, *TerSymbol::Terminal, *TerSymbol::Equal, *TerSymbol::Rex},
				{*TerSymbol::Statement, *TerSymbol::Terminal, *TerSymbol::Equal, *TerSymbol::Terminal},
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
						auto& ref = vec[input.terminal_token_index];
						back_buffer = std::get<1>(vec[input.terminal_token_index]);
					}
					else if (input.symbol == *TerSymbol::Rex || input.symbol == *TerSymbol::Terminal)
					{
						auto& ref = vec[input.terminal_token_index];
						auto rex = std::get<1>(vec[input.terminal_token_index]);
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


			static lr1 imp(*TerSymbol::Statement, {
			{*TerSymbol::Statement, *TerSymbol::Terminal},
			{*TerSymbol::Statement, *TerSymbol::Statement, *TerSymbol::Terminal},
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
						auto& ref = vec[input.terminal_token_index];
						auto sym_string = std::get<1>(vec[input.terminal_token_index]);
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
			static lr1 imp(
				*TerSymbol::Statement, {
				{ *TerSymbol::Statement, *TerSymbol::StartSymbol, *TerSymbol::Equal, *TerSymbol::NoTerminal },
				{ *TerSymbol::Statement, *TerSymbol::StartSymbol, *TerSymbol::Equal, *TerSymbol::Terminal },
				{ *TerSymbol::Statement, *TerSymbol::NoTerminal, *TerSymbol::Equal, *TerSymbol::TerList },
				{ *TerSymbol::Statement, *TerSymbol::NoTerminal, *TerSymbol::Equal},
				{ *TerSymbol::Statement, *TerSymbol::Equal, *TerSymbol::TerList },
				{ *TerSymbol::Statement, *TerSymbol::Equal },
				{ *TerSymbol::TerList, *TerSymbol::NoTerminal },
				{ *TerSymbol::TerList, *TerSymbol::Terminal },
				{ *TerSymbol::TerList, *TerSymbol::RexTerminal },
				{ *TerSymbol::TerList, *TerSymbol::TerList, *TerSymbol::TerList},
				{ *TerSymbol::TerList, *TerSymbol::TerList, *TerSymbol::Or, *TerSymbol::TerList },
				{ *TerSymbol::TerList, *TerSymbol::LS_Brace, *TerSymbol::TerList, *TerSymbol::RS_Brace },
				{ *TerSymbol::TerList, *TerSymbol::LM_Brace, *TerSymbol::TerList, *TerSymbol::RM_Brace },
				{ *TerSymbol::TerList, *TerSymbol::LB_Brace, *TerSymbol::TerList, *TerSymbol::RB_Brace },
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
						auto& ref = vec[input.terminal_token_index];
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
						switch (input.no_terminal_production_index)
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
							production.push_back({ *last_no_terminal });
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
								production.push_back(std::move(pre));
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
								production.push_back({ *last_no_terminal });
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
			production.insert(production.end(), std::move_iterator(temporary_production.begin()), std::move_iterator(temporary_production.end()));

			for (auto& ite : temporary_noterminal_buffer)
				temporary_noterminal.insert(std::get<1>(ite));

		}
		{
			lr1 imp(
				*TerSymbol::Statement, {
				{ *TerSymbol::TerList, *TerSymbol::RexTerminal },
				{ *TerSymbol::TerList, *TerSymbol::Terminal },
				{ *TerSymbol::TerList, *TerSymbol::TerList,  *TerSymbol::TerList},
				{ *TerSymbol::Statement, *TerSymbol::RexTerminal },
				{ *TerSymbol::Statement, *TerSymbol::Terminal },
				{ *TerSymbol::Statement, *TerSymbol::LS_Brace, *TerSymbol::TerList, *TerSymbol::RS_Brace },
				{ *TerSymbol::Statement, *TerSymbol::LM_Brace, *TerSymbol::TerList, *TerSymbol::RM_Brace },
				{ *TerSymbol::Statement, *TerSymbol::Statement, *TerSymbol::Statement },
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
						auto& ref = vec[input.terminal_token_index];
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
						switch (input.no_terminal_production_index)
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

			lr1 imp(
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
}