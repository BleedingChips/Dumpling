#pragma once
#include <string>
#include <vector>
#include <regex>
#include <tuple>
#include <optional>
#include <map>
#include <filesystem>
#include <variant>
#include <set>
#include "syntax.h"

namespace Potato
{
	struct parser_sbnf
	{
		using storage_t = lr1::storage_t;

		parser_sbnf(
			std::wstring table,
			std::map<std::tuple<std::size_t, std::size_t>, storage_t> symbol_str_to_index,
			std::map<storage_t, std::tuple<std::size_t, std::size_t>> symbol_index_to_str,
			std::vector<std::tuple<std::wstring, storage_t>> terminal_rex,
			std::set<storage_t> unused_terminal,
			std::set<storage_t> temporary_noterminal,
			std::map<storage_t, storage_t> production_mapping,
			lr1 lr1imp
		);

		parser_sbnf(parser_sbnf&&) = default;
		std::vector<storage_t> serialization();
		static parser_sbnf unserialization(const storage_t* data, size_t length);

		struct travel
		{
			storage_t sym;
			std::wstring_view sym_str;
			union {
				std::wstring_view terminal_data;
				struct {
					size_t no_terminal_production_index;
					size_t no_terminal_production_count;
				};
			};
		};

		template<typename RespondFunction>
		void analyze(const wchar_t* code, size_t length, RespondFunction&& Function)
		{
			token_generator tokens(*this, {}, {});
			std::tuple<const wchar_t*, size_t> Ite{code, length};
			size_t temporary_buffer = 0;
			lr1_process(lr1imp, tokens, [&](lr1_processor::travel input)  {
				if (input.is_terminal())
				{
					assert(tokens.last_tokens.has_value());
					travel tra;
					tra.sym = input.symbol;
					tra.terminal_data = *tokens.last_tokens;
					std::forward<RespondFunction&&>(Function)(tra);
				}
				else {
					// todo
				}
			}, Ite);
		}

	private:

		struct token_generator
		{
			parser_sbnf& ref;
			std::optional<std::wstring_view> last_tokens;
			std::optional<std::wstring_view> cur_tokens;
			std::optional<lr1::storage_t> operator()(std::tuple<const wchar_t*, size_t>& ite);
		};

		void build_rex();

		std::wstring table;
		std::map<std::tuple<std::size_t, std::size_t>, storage_t> symbol_str_to_index;
		std::map<storage_t, std::tuple<std::size_t, std::size_t>> symbol_index_to_str;
		std::vector<std::tuple<std::wstring, storage_t>> terminal_rex;
		std::set<storage_t> unused_terminal;
		std::set<storage_t> temporary_noterminal;
		std::vector<std::tuple<std::wregex, storage_t>> terminal_rex_imp;
		std::map<storage_t, storage_t> production_mapping;
		lr1 lr1imp;
 	};

	namespace Error
	{
		struct SBNFError
		{
			size_t line;
			std::wstring error_message;
			std::wstring analyzed_string;
		};
	}

	std::optional<parser_sbnf> LoadSBNFFile(const std::filesystem::path& Path, std::wstring& storage);
	parser_sbnf LoadSBNFCode(const std::wstring& Code);
}