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

	struct parser 
	{
		using storage = lr1::storage_t;

		parser(
			std::wstring table,
			std::map<std::wstring_view, storage> string_to_symbol,
			std::map<storage, std::wstring_view> symbol_to_string,
			std::vector<std::tuple<std::wstring, storage>> token_rex,
			std::set<storage> remove_set,
			std::set<storage> temp_non_terminal,
			lr1 lr1_imp
		);

		struct error {
			std::wstring message;
			std::wstring tokens;
			std::size_t line;
		};

	private:
		std::wstring table;
		std::map<std::wstring_view, storage> string_to_symbol;
		std::map<storage, std::wstring_view> symbol_to_string;
		std::map<std::wstring, storage> production_mapping;
		std::vector<std::tuple<std::wregex, storage>> token_rex;
		std::set<storage> remove_set;
		std::set<storage> temp_non_terminal;
		lr1 lr1_imp;
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

	std::optional<parser> LoadSBNFFile(const std::filesystem::path& Path, std::wstring& storage);
	std::optional<parser> LoadSBNFCode(const std::wstring& Code);


}