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
#include "lexical.h"
#include "syntax.h"

namespace Potato::Parser
{
	struct sbnf
	{
		using storage_t = Syntax::lr1_storage::storage_t;

		struct error {
			std::u32string message;
			size_t line;
			size_t charactor_index;
		};

		std::u32string table;
		std::vector<std::tuple<std::size_t, std::size_t>> symbol_map;
		size_t ter_count;
		storage_t unused_terminal;
		size_t temporary_prodution_start;
		Lexical::nfa_storage nfa_s;
		Syntax::lr1_storage lr1_s;
		static sbnf create(std::u32string_view code);
 	};

	struct sbnf_processer
	{

		using storage_t = Syntax::lr1::storage_t;
		struct travel
		{
			Syntax::lr1_storage::storage_t sym;
			std::u32string_view sym_str;
			std::u32string_view token_data;
			union {
				struct terminal_t{

					size_t line;
					size_t charactor_index;
				}terminal;
				struct noterminal_t{
					size_t function_enum;
					size_t production_index;
					storage_t const* symbol_array;
					size_t production_element_count;
				}noterminal;
			};
			bool is_termina() const noexcept { return Syntax::lr1::is_terminal(sym); }
		};
		sbnf_processer(sbnf const& ref) : ref(ref) {}
		sbnf const& ref;
		template<typename Func> void analyze(std::u32string_view code, Func&& F) {
			auto Wrapper = [](void* data, travel input) {  (*reinterpret_cast<Func*>(F))(input); };
			analyze_imp(code, Wrapper, &F);
		}
	private:
		void analyze_imp(std::u32string_view, void(*Func)(void* data, travel), void* data);
	};

	/*

	std::optional<parser_sbnf> LoadSBNFFile(const std::filesystem::path& Path, std::wstring& storage);
	parser_sbnf LoadSBNFCode(std::u32string_view Code);

	*/
}