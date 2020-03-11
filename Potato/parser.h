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

	struct lr1
	{
		using storage_t = uint32_t;
		static constexpr storage_t flag = 0x8000'0000;


		static inline bool is_terminal(storage_t symbol) noexcept { return symbol < flag; }
		static inline uint32_t terminal_eof() { return static_cast<uint32_t>(flag - 1); }
		uint32_t noterminal_start() { return (0xffff'ffff); }

		struct table
		{
			std::map<storage_t, storage_t> m_shift;
			std::map<storage_t, storage_t> m_reduce;
		};

		static lr1 create_table(
			uint32_t start_symbol,
			std::vector<std::vector<storage_t>> production,
			std::vector<std::tuple<std::vector<std::variant<storage_t, std::pair<storage_t, storage_t>>>, Associativity>> priority
		);

		lr1(
			uint32_t start_symbol,
			std::vector<std::vector<storage_t>> production,
			std::vector<std::tuple<std::vector<std::variant<storage_t, std::pair<storage_t, storage_t>>>, Associativity>> priority
		) : lr1(create_table(start_symbol, std::move(production), std::move(priority))) {}

		lr1(
			std::vector<std::tuple<storage_t, storage_t>> production,
			std::vector<table> table
		) : m_production(std::move(production)), m_table(std::move(table)) {}

		lr1(lr1&&) = default;

	private:

		std::vector<std::tuple<uint32_t, uint32_t>> m_production;
		std::vector<table> m_table;
	};




	struct parser 
	{
		parser(
			std::wstring table,
			std::map<std::wstring_view, uint32_t> string_to_symbol,
			std::map<uint32_t, std::wstring_view> symbol_to_string,
			const std::vector<std::tuple<std::wstring, uint32_t>>& token_rex,
			std::set<uint32_t> remove_set,
			std::set<uint32_t> temp_non_terminal,
			Implement::LR1_implement lr1_imp
		);
	private:
		std::wstring table;
		std::map<std::wstring_view, uint32_t> string_to_symbol;
		std::map<uint32_t, std::wstring_view> symbol_to_string;
		std::map<std::wstring, uint32_t> production_mapping;
		std::vector<std::tuple<std::wregex, uint32_t>> token_rex;
		std::set<uint32_t> remove_set;
		std::set<uint32_t> temp_non_terminal;
		Implement::LR1_implement lr1_imp;
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

	std::optional<parser> LoadSBNFFile(const std::filesystem::path& Path);
	std::optional<parser> LoadSBNFCode(const std::wstring& Code);


}