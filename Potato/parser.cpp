#include "parser.h"
#include <fstream>
#include "character_encoding.h"
#include <assert.h>
namespace
{
	

}



namespace Potato
{









	/*
	parser::parser(
		std::map<std::wstring, uint32_t> production_mapping,
		const std::vector<std::tuple<std::wstring, uint32_t>>& token_rex,
		std::set<uint32_t> remove_set,
		std::set<uint32_t> temp_non_terminal,
		Implement::LR1_implement imp
	)
		: production_mapping(std::move(production_mapping)), remove_set(std::move(remove_set)), temp_non_terminal(std::move(temp_non_terminal)), lr1_imp(std::move(lr1_imp))
	{
		auto flag = std::regex::optimize | std::regex::nosubs;
		for (auto& ite : token_rex)
		{
			auto& [str, index] = ite;
			this->token_rex.push_back({ std::wregex(str, flag), index });
		}
	}
	*/















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

	enum class TerSymbol : std::size_t
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
	};

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
		std::vector<std::tuple<std::wstring_view, std::wstring_view, std::size_t>>,
		std::set<std::wstring_view>,
		std::set<std::wstring_view>,
		std::map<std::wstring_view, std::vector<std::tuple<std::vector<std::tuple<std::wstring_view, TerSymbol>>, std::size_t>>>,
		std::vector<std::tuple<TerSymbol, std::wstring_view, std::size_t>>
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
			std::wregex {LR"(\|)", flag} // Or
		};

		auto ite = code.begin();
		auto code_end = code.end();
		size_t line_count = 1;
		size_t state = 0;

		size_t SymbolIndex = 0;

		std::vector<std::tuple<std::wstring_view, std::wstring_view, std::size_t>> ter_regex;
		std::set<std::wstring_view> remove_set;
		std::set<std::wstring_view> unnamed_set;
		std::map<std::wstring_view, std::vector<std::tuple<std::vector<std::tuple<std::wstring_view, TerSymbol>>, std::size_t>>> productions;
		std::vector<std::tuple<TerSymbol, std::wstring_view, std::size_t>> priority;
		std::wstring_view last_noterminal;
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
				switch (state)
				{
				case 0:
				{
					bool Currect = false;
					if (LineSymbol.size() == 1 && std::get<0>(LineSymbol[0]) == TerSymbol::Mask)
					{
						Currect = true;
						state += 1;
					}
					else if (LineSymbol.size() == 3)
					{
						if (std::get<0>(LineSymbol[0]) == TerSymbol::DefineTerminal && std::get<0>(LineSymbol[1]) == TerSymbol::Equal)
						{
							auto [Token, view] = LineSymbol[2];
							if (Token == TerSymbol::Rex || Token == TerSymbol::DefineTerminal)
							{
								Currect = true;
								std::wstring_view token = std::get<1>(LineSymbol[0]);
								std::wstring_view token_regex{ std::get<1>(LineSymbol[2])};
								ter_regex.push_back({token, token_regex, line_count});
							}
						}
					}
					if (!Currect)
						throw Error::SBNFError{ line_count, LR"(Error Synax)", {line_start, line_end} };
				}
				break;
				case 1:
				{
					if (LineSymbol.size() == 1 && std::get<0>(LineSymbol[0]) == TerSymbol::Mask)
						state += 1;
					else {
						for (auto& ite : LineSymbol)
						{
							auto [Symbol, str] = ite;
							if (Symbol == TerSymbol::Terminal)
								remove_set.insert(str);
							else
								throw Error::SBNFError{ line_count, LR"(Error Synax)", {line_start, line_end} };
						}
					}
				}
				break;
				case 2:
				{
					if (!LineSymbol.empty())
					{
						auto [sym, str] = LineSymbol[0];
						bool Currect = false;
						decltype(LineSymbol)::const_iterator start;
						if (LineSymbol.size() == 1 && std::get<0>(LineSymbol[0]) == TerSymbol::Mask)
						{
							state += 1;
							break;
						}
						else if (sym == TerSymbol::NoTerminal || sym == TerSymbol::StartSymbol)
						{
							last_noterminal = str;
							if (LineSymbol.size() >= 2 && std::get<0>(LineSymbol[1]) == TerSymbol::Equal)
							{
								start = LineSymbol.begin() + 2;
								Currect = true;
							}
						}
						else if (sym == TerSymbol::Equal)
						{
							if (!last_noterminal.empty())
							{
								start = LineSymbol.begin() + 1;
								Currect = true;
							}
						}
						if (Currect)
						{
							std::vector<std::tuple<std::wstring_view, TerSymbol>> Tem;
							for (; start != LineSymbol.end(); ++start)
							{
								auto [sym, str] = *start;
								if (sym == TerSymbol::RexTerminal)
									unnamed_set.insert(str);
								Tem.push_back({ str, sym });
								auto FindIte = productions.insert({ last_noterminal , {} });
								std::tuple<std::vector<std::tuple<std::wstring_view, TerSymbol>>, std::size_t> Res{ std::move(Tem), line_count };
								FindIte.first->second.push_back(std::move(Res));
							}
							
						}
						else {
							throw Error::SBNFError{ line_count, LR"(Error Synax)", {line_start, line_end} };
						}
					}
				}
				break;
				case 3:
				{
					for (auto& ite : LineSymbol)
					{
						auto [Symbol, str] = ite;
						priority.push_back({ Symbol, str, line_count });
					}
				}
				break;
				default:
					break;
				}
			}
			++line_count;
			ite = line_next;
		}
		return  { std::move(ter_regex), std::move(remove_set),  std::move(unnamed_set), std::move(productions), std::move(priority) };
	}
	

	std::optional<parser> LoadSBNFCode(const std::wstring& code)
	{
		auto [Ter, Remove, Unnamed, Pro, Priority] = DetectCodeToSymbol(code);
		std::map<std::wstring_view, uint32_t> TerminalMapping;
		std::vector<std::tuple<std::wstring, std::size_t>> RegexToTerminal;
		uint32_t TerminalIndex = 0;
		{
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
		}

		

		std::wstring Table;
		std::vector<std::wstring> TerminalRegex;
		std::map<std::wstring_view, std::tuple<uint32_t, bool>> index;
		//for(size_t )



		//std::vector<std::wstring> Terminal;
		//std::map<std::wstring_view>

		return std::nullopt;
	}

	std::optional<parser> LoadSBNFFile(const std::filesystem::path& Path)
	{
		using namespace Potato::Encoding;
		std::ifstream input(Path, std::ios::binary);
		if (input.is_open())
		{
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
				result = utf8_string.to_string<wchar_t>();
				break;
			}
			default:
				assert(false);
				break;
			}
			return LoadSBNFCode(result);
		}
		return std::nullopt;
	}
}