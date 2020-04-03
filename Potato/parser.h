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
			std::vector<std::tuple<std::size_t, std::size_t>> sym_list,
			size_t ter_count,
			std::vector<std::tuple<std::wstring, storage_t>> terminal_rex,
			std::set<storage_t> unused_terminal,
			size_t temporary_prodution_start,
			lr1 lr1imp
		);
		parser_sbnf(const parser_sbnf&) = default;
		parser_sbnf(parser_sbnf&&) = default;
		parser_sbnf& operator=(const parser_sbnf&) = default;
		parser_sbnf& operator=(parser_sbnf&&) = default;
		std::vector<storage_t> serialization();
		static parser_sbnf unserialization(const storage_t* data, size_t length);

		struct travel
		{
			storage_t sym;
			std::wstring_view sym_str;
			union {
				struct {
					std::wstring_view ter_data;
					size_t line;
					size_t charactor_index;
				};
				struct {
					size_t noter_pro_index;
					size_t noter_pro_count;
				};
			};
		};

		template<typename RespondFunction>
		void analyze(const wchar_t* code, size_t length, RespondFunction&& Function) const
		{
			token_generator tokens(*this, {}, {});
			std::tuple<const wchar_t*, size_t> Ite{code, length};
			int64_t temporary_production_count = 0;
			lr1_process(lr1imp, tokens, [&](lr1_processor::travel input)  {
				auto result = translate(input, tokens, temporary_production_count);
				if (result)
					std::forward<RespondFunction&&>(Function)(*result);
			}, Ite);
		}

	private:

		struct token_generator
		{
			struct element
			{
				std::wstring_view token;
				size_t line = 0;
				size_t charactor_index = 0;
			};
			parser_sbnf& ref;
			std::optional<element> last_tokens;
			std::optional<element> cur_tokens;
			size_t current_line = 0;
			size_t charactor_index = 0;
			std::optional<lr1::storage_t> operator()(std::tuple<const wchar_t*, size_t>& ite);
		};

		std::optional<travel> translate(const lr1_processor::travel& input, const token_generator&, int64_t& pro_count) const;

		void build_rex();

		std::wstring table;
		std::vector<std::tuple<std::size_t, std::size_t>> sym_list;
		size_t ter_count;
		std::vector<std::tuple<std::wstring, storage_t>> terminal_rex;
		std::set<storage_t> unused_terminal;
		size_t temporary_prodution_start;
		std::vector<std::tuple<std::wregex, storage_t>> terminal_rex_imp;
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