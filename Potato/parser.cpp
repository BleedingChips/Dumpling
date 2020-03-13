#include "parser.h"
#include <fstream>
#include "character_encoding.h"
#include <assert.h>

namespace Potato
{
















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
		DefineTerminal = 1,
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
		Terminal,
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
			std::wregex {LR"([a-zA-Z_][a-zA-Z_0-9]*(?=\s|$))", flag}, // DefineTerminal
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
			std::wregex {LR"([a-zA-Z_][a-zA-Z_0-9]*(?=[^a-zA-Z_0-9]|$))", flag}, // Terminal
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
					{TerSymbol::Empty, TerSymbol::DefineTerminal, TerSymbol::Equal, TerSymbol::Mask, TerSymbol::Rex},
					{TerSymbol::Empty, TerSymbol::Terminal, TerSymbol::Mask},
					{TerSymbol::Empty, TerSymbol::Terminal, TerSymbol::NoTerminal, TerSymbol::RexTerminal, TerSymbol::Equal, TerSymbol::StartSymbol, 
					TerSymbol::LB_Brace,TerSymbol::RB_Brace,TerSymbol::LM_Brace,TerSymbol::RM_Brace,TerSymbol::LS_Brace,TerSymbol::RS_Brace, TerSymbol::Or, TerSymbol::Mask},
					{TerSymbol::Empty, TerSymbol::RexTerminal, TerSymbol::Terminal, TerSymbol::LM_Brace, TerSymbol::RM_Brace, TerSymbol::LS_Brace, TerSymbol::RS_Brace}
				};

				std::vector<std::tuple<TerSymbol, std::wstring_view>> LineSymbol;
				while (ite != line_end)
				{
					std::wstring Debug{ ite, line_end };
					bool Done = false;
					for (auto sym : TargetLexical[state])
					{
						std::wsmatch match;
						if (std::regex_search(ite, line_end, match, Lexical[static_cast<std::size_t>(sym)], std::regex_constants::match_continuous))
						{
							if (sym != TerSymbol::Empty)
								LineSymbol.push_back({ sym, view(match[0].first, match[0].second) });
							Done = true;
							ite = match[0].second;
							break;
						}
					}
					if (!Done)
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

	std::optional<parser> LoadSBNFCode(const std::wstring& code)
	{
		using type = lr1::storage_t;
		auto [AllToken,  Size] = DetectCodeToSymbol(code);

		std::array<std::size_t, 4> SectionStart = { 0, 0, 0, 0 };
		std::array<std::size_t, 4> SectionEnd = { 0, 0, 0, 0 };
		for (size_t i = 1; i < 4; ++i)
			SectionStart[i] = SectionStart[i - 1] + Size[i - 1];
		for (size_t i = 0; i < 4; ++i)
			SectionEnd[i] = SectionStart[i] + Size[i];

		

		std::map<std::wstring_view, type> symbol_to_index;
		std::map<type, std::wstring_view> index_to_symbol;
		std::vector<std::tuple<std::wstring, type>> rexs;
		std::set<type> unused_terminal;
		type terminal_start = 0;
		type noterminal_start = 0;
		std::optional<type> start_symbol;
		std::vector<std::vector<type>> production;

		{

			static lr1 imp(*TerSymbol::Statement, {
				{*TerSymbol::Statement, *TerSymbol::DefineTerminal, *TerSymbol::Equal, *TerSymbol::Rex},
				{*TerSymbol::Statement, *TerSymbol::DefineTerminal, *TerSymbol::Equal, *TerSymbol::DefineTerminal},
				},
				{}
			);


			std::wstring_view back_buffer;
			for (size_t i = SectionStart[0]; i < SectionEnd[0]; ++i)
			{
				auto& [vec, lin] = AllToken[i];
				try {
					lr1_process(imp, vec.begin(), vec.end(), [](auto in) {return *std::get<0>(in); }, [&](lr1_processor::travel input) {
						if (input.symbol == *TerSymbol::DefineTerminal)
						{
							auto& ref = vec[input.terminal_token_index];
							back_buffer = std::get<1>(vec[input.terminal_token_index]);
						}
						else if (input.symbol == *TerSymbol::Rex || input.symbol == *TerSymbol::DefineTerminal)
						{
							auto& ref = vec[input.terminal_token_index];
							auto rex = std::get<1>(vec[input.terminal_token_index]);
							auto re = symbol_to_index.insert({ back_buffer, terminal_start });
							if (re.second)
							{
								index_to_symbol.insert({ terminal_start , back_buffer});
								++terminal_start;
							}
							rexs.push_back({ std::wstring{rex}, re.first->second });
						}
					});
				}
				catch (...)
				{
					volatile int i = 0;
				}
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
				auto& [vec, lin] = AllToken[i];
				try {
					lr1_process(imp, vec.begin(), vec.end(), [](auto in) {return *std::get<0>(in); }, [&](lr1_processor::travel input) {
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
					});
				}
				catch (...)
				{
					volatile int i = 0;
				}
				//lr1_processor pro(imp[k - 1]);
				//AllAst.push_back(pro.generate_ast(vec.begin(), vec.end(), [](auto in) {return *std::get<0>(in); }));
			}
		}

		{
			static lr1 imp(
				*TerSymbol::Statement, {
				{ *TerSymbol::Statement, *TerSymbol::StartSymbol, *TerSymbol::Equal, *TerSymbol::NoTerminal },
				{ *TerSymbol::Statement, *TerSymbol::StartSymbol, *TerSymbol::Equal, *TerSymbol::Terminal },
				{ *TerSymbol::Statement, *TerSymbol::NoTerminal, *TerSymbol::Equal, *TerSymbol::TerList },
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

			std::vector<std::wstring_view> rex_terminal;
			std::vector<std::vector<type>> stack;
			std::map<std::vector<type>, type> temporary_terminal;
			std::optional<type> last_no_terminal;

			for (size_t i = SectionStart[2]; i < SectionEnd[2]; ++i)
			{
				auto& [vec, lin] = AllToken[i];
				try {
					lr1_process(imp, vec.begin(), vec.end(), [](auto in) {return *std::get<0>(in); }, [&](lr1_processor::travel input) {
						if (input.is_terminal())
						{
							auto& ref = vec[input.terminal_token_index];
							auto sym_string = std::get<1>(ref);
							switch (input.symbol)
							{
							case *TerSymbol::NoTerminal:
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
							case *TerSymbol::RexTerminal:
							{
								auto result = symbol_to_index.insert({ sym_string, terminal_start });
								if (result.second)
								{
									index_to_symbol.insert({ terminal_start, sym_string });
									++terminal_start;
									rex_terminal.push_back(sym_string);
								}
								stack.push_back({ result.first->second });
								break;
							}
							case *TerSymbol::Terminal:
							{
								auto ite = symbol_to_index.find(sym_string);
								if (ite != symbol_to_index.end())
								{
									if (unused_terminal.find(ite->second) != unused_terminal.end())
										stack.push_back({ ite->second });
									else
										throw Error::SBNFError{ lin, L"Removed Terminal used in Production", std::wstring{sym_string} };
								}else
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
								assert(stack.size() == 2);
								last_no_terminal = stack[0];
								production.push_back(std::move(stack));
								break;
							}
							case 3:
							{
								assert(stack.size() == 1);
								if(last_no_terminal)
									production.push_back({*last_no_terminal, stack[0]});
								else
								{
									auto Find = index_to_symbol.find(*start_symbol);
									assert(Find != index_to_symbol.end());
									throw Error::SBNFError{ lin, L"Miss No Terminal Define", std::wstring{} };
								}
								break;
							}
							case 4:
							{
								assert(stack.size() == 0);
								if (last_no_terminal)
									production.push_back({ *last_no_terminal });
								else
								{
									auto Find = index_to_symbol.find(*start_symbol);
									assert(Find != index_to_symbol.end());
									throw Error::SBNFError{ lin, L"Miss No Terminal Define", std::wstring{} };
								}
								break;
							}
							case 5:
							case 6:
							case 7:
							{
								break;
							}
							// todo
							default:
								break;
							}
						}
					});
				}
				catch (...)
				{
					volatile int i = 0;
				}
				//lr1_processor pro(imp[k - 1]);
				//AllAst.push_back(pro.generate_ast(vec.begin(), vec.end(), [](auto in) {return *std::get<0>(in); }));
			}
		}

		volatile int i = 0;

		
		



		/*
		lr1 imp[4] = {
			lr1::create(*TerSymbol::Statement, {
		{*TerSymbol::Statement, *TerSymbol::DefineTerminal, *TerSymbol::Equal, *TerSymbol::Rex},
		{*TerSymbol::Statement, *TerSymbol::DefineTerminal, *TerSymbol::Equal, *TerSymbol::DefineTerminal},
			},
			{}
		),
			lr1::create(*TerSymbol::Statement, {
			{*TerSymbol::Statement, *TerSymbol::Terminal},
			{*TerSymbol::Statement, *TerSymbol::Statement, *TerSymbol::Terminal},
			},
			{}
		),

		lr1::create(
			*TerSymbol::Statement, {
			{ *TerSymbol::Statement, *TerSymbol::StartSymbol, *TerSymbol::Equal, *TerSymbol::NoTerminal },
			{ *TerSymbol::Statement, *TerSymbol::StartSymbol, *TerSymbol::Equal, *TerSymbol::Terminal },
			{ *TerSymbol::Statement, *TerSymbol::NoTerminal, *TerSymbol::Equal, *TerSymbol::TerList },
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
		),

		lr1::create(
			*TerSymbol::Statement, {
			{ *TerSymbol::TerList, *TerSymbol::RexTerminal },
			{ *TerSymbol::TerList, *TerSymbol::Terminal },
			{ *TerSymbol::TerList, *TerSymbol::TerList, *TerSymbol::TerList },
			{ *TerSymbol::Statement, *TerSymbol::Statement, *TerSymbol::Statement },
			{ *TerSymbol::Statement, *TerSymbol::LS_Brace, *TerSymbol::TerList, *TerSymbol::RS_Brace },
			{ *TerSymbol::Statement, *TerSymbol::LM_Brace, *TerSymbol::TerList, *TerSymbol::RM_Brace },
			},
			{}
		) };

		std::array<std::size_t, 4> SectionStart = { 0, 0, 0, 0 };
		std::array<std::size_t, 4> SectionEnd = { 0, 0, 0, 0 };
		for (size_t i = 1; i < 4; ++i)
			SectionStart[i] = SectionStart[i - 1] + Size[i - 1];
		for (size_t i = 0; i < 4; ++i)
			SectionEnd[i] = SectionStart[i] + Size[i];

		
		std::vector<lr1_processor::ast> AllAst;
		for (size_t i = 0; i < AllToken.size(); ++i)
		{
			for (size_t k = 4; k > 0; --k)
			{
				if (i >= SectionStart[k - 1])
				{
					auto& [vec, lin] = AllToken[i];
					lr1_processor pro(imp[k-1]);
					AllAst.push_back(pro.generate_ast(vec.begin(), vec.end(), [](auto in) {return *std::get<0>(in); }));
					break;
				}
			}
		}

		std::map<std::wstring_view, type> ter_to_index;
		std::map<type, std::wstring_view> index_to_ter;
		std::vector<std::tuple<std::wstring, type>> rexs;
		std::set<type> remove_index;
		std::set<type> unused_index;
		type terminal_start = 0;
		type noterminal_start = lr1::noterminal_start();

		for (size_t i = SectionStart[0]; i < SectionEnd[0]; ++i)
		{
			auto& AST = AllAst[i];
			assert(AST.sym == *TerSymbol::Statement);
			std::vector<lr1_processor::ast> Vect = std::get<0>(AST.index);
			assert(Vect.size() == 3);

			auto& Data = std::get<0>(AllToken[i]);
			auto& Token = std::get<1>(Data[std::get<1>(Vect[0].index)]);
			auto& Token2 = std::get<1>(Data[std::get<1>(Vect[2].index)]);
			auto Ite = ter_to_index.insert({ Token, terminal_start });
			if (Ite.second)
			{
				index_to_ter.insert({ Ite.first->second, Ite.first->first });
				++terminal_start;
			}
				
			rexs.push_back({ std::wstring{Token2}, Ite.first->second });
		}

		for (size_t i = SectionStart[1]; i < SectionEnd[1]; ++i)
		{
			auto& AST = AllAst[i];
			const ast* Cur = &AST;
			std::vector<std::array<std::vector<ast>::const_iterator, 2>> ast_buffer;
			while (Cur != nullptr)
			{
				if (std::holds_alternative<std::vector<ast>>(Cur->index))
				{
					auto& ite = std::get<std::vector<ast>>(Cur->index);
					ast_buffer.push_back({ ite.begin(), ite.end() });
				}
				else {
					size_t index = std::get<size_t>(Cur->index);
					auto token = std::get<0>(AllToken[i])[index];
					auto find = ter_to_index.find(std::get<1>(token));
					if (find != ter_to_index.end())
						remove_index.insert(find->second);
					else
						throw Error::SBNFError{ std::get<std::size_t>(AllToken[i]), L"Removed Terminal Is Not Defined", std::wstring{std::get<1>(token)} };
				}
				Cur = nullptr;
				while (!ast_buffer.empty())
				{
					auto& ite = *ast_buffer.rbegin();
					if (ite[0] != ite[1])
					{
						Cur = &*ite[0];
						++ite[0];
						break;
					}
					else {
						ast_buffer.pop_back();
					}
				}
			}
		}


		{
			std::wstring LastInput;
			for (size_t i = SectionStart[2]; i < SectionEnd[2]; ++i)
			{
				auto& AST = AllAst[i];

			}
		}
		
		volatile int i = 0;
		*/

		/*
		std::map<std::wstring_view, type> TerminalMapping;
		std::vector<std::tuple<std::wstring, type>> RegexToTerminal;

		{
			uint32_t TerminalIndex = 0;
			std::vector<std::wstring_view> Order(Unnamed.begin(), Unnamed.end());
			std::sort(Order.begin(), Order.end(), [](const std::wstring_view& l, const std::wstring_view& r) {
				if (l.size() > r.size())
					return true;
				else if (l.size() < r.size())
					return false;
				else {
					for (size_t i = 0; i < l.size(); ++i)
						if (l[i] < r[i])
							return true;
					return false;
				}
			});

			for (auto& ite : Order)
			{
				TerminalMapping.insert({ ite, TerminalIndex });
				std::wstring Rex;
				assert(ite.size() >= 3);
				auto s = ite.begin() + 1;
				auto e = ite.end() - 1;
				for (; s!=e; ++s)
				{
					Rex.push_back(L'\\');
					Rex.push_back(*s);
				}
				RegexToTerminal.push_back({ Rex ,TerminalIndex });
				++TerminalIndex;
			}

			for (auto& ite : Ter)
			{
				auto [ter, rex, index] = ite;
				auto find = TerminalMapping.insert({ ter, TerminalIndex });
				if (find.second)
					++TerminalIndex;
				RegexToTerminal.push_back({ std::wstring{rex}, find.first->second });
			}
		}

		std::set<type> remove_set;

		{
			for (auto& ite : Remove)
			{
				auto find = TerminalMapping.find(ite);
				if (find != TerminalMapping.end())
					remove_set.insert(find->second);
			}
		}

		static lr1 imp(*TerSymbol::Statement, {
			{*TerSymbol::Statement, *TerSymbol::NoTerminal, *TerSymbol::Equal, *TerSymbol::NullableTerList},
			{*TerSymbol::NullableTerList},
			{*TerSymbol::NullableTerList, *TerSymbol::NullableTerList, *TerSymbol::TerList},
			{*TerSymbol::TerList, *TerSymbol::NoTerminal},
			{*TerSymbol::TerList, *TerSymbol::Terminal},
			{*TerSymbol::TerList, *TerSymbol::TerList, *TerSymbol::Or, *TerSymbol::TerList},
			{*TerSymbol::TerList, *TerSymbol::LS_Brace, *TerSymbol::TerList, *TerSymbol::LS_Brace},
			{*TerSymbol::TerList, *TerSymbol::LM_Brace, *TerSymbol::TerList, *TerSymbol::LM_Brace},
			},
			{}
		);
		{
			for (auto& ite : Pro)
			{
				for (auto& ite2 : ite.second)
				{
					lr1_processor processor{ imp };
					lr1_processor::ast result = processor.generate_ast(std::get<0>(ite2).begin(), std::get<0>(ite2).end(), [](auto input) {return *std::get<1>(input); });
				}
				
			}
			
		}
		std::wstring Table;
		std::vector<std::wstring> TerminalRegex;
		std::map<std::wstring_view, std::tuple<uint32_t, bool>> index;
		*/
		//for(size_t )



		//std::vector<std::wstring> Terminal;
		//std::map<std::wstring_view>

		return std::nullopt;
	}

	std::optional<parser> LoadSBNFFile(const std::filesystem::path& Path, std::wstring& storage)
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