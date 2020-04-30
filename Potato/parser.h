#pragma once
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

		struct unacceptable_token_error : std::exception {
			unacceptable_token_error(std::u32string string, size_t line, size_t index) : string(std::move(string)), line(line), index(index) {}
			unacceptable_token_error(const unacceptable_token_error&) = default;
			char const* what() const noexcept override;
			std::u32string string;
			size_t line;
			size_t index;
		};

		struct undefine_terminal_error : std::exception {
			undefine_terminal_error(std::u32string token, size_t line, size_t index) : string(std::move(string)), line(line), index(index) {}
			undefine_terminal_error(const undefine_terminal_error&) = default;
			char const* what() const noexcept override;
			std::u32string string;
			size_t line;
			size_t index;
		};

		struct miss_start_symbol : std::exception {
			miss_start_symbol() {}
			miss_start_symbol(const miss_start_symbol&) = default;
			char const* what() const noexcept override;
		};

		std::u32string table;
		std::vector<std::tuple<std::size_t, std::size_t>> symbol_map;
		size_t ter_count;
		storage_t unused_terminal;
		size_t temporary_prodution_start;
		Lexical::nfa_storage nfa_s;
		Syntax::lr1_storage lr1_s;
		static sbnf create(std::u32string_view code);
		std::u32string_view find_symbol(storage_t input) const noexcept;
		std::optional<storage_t> find_symbol(std::u32string_view sym) const noexcept;
 	};

	struct sbnf_processer
	{
		struct error : std::logic_error {
			using std::logic_error::logic_error;
			size_t line;
			size_t count;
		};

		using storage_t = Syntax::lr1::storage_t;
		struct travel
		{
			Syntax::lr1_storage::storage_t sym;
			std::u32string_view sym_str;
			std::u32string_view token_data;
			union {
				struct terminal_t{

					size_t line;
					size_t index;
				}terminal;
				struct noterminal_t{
					size_t function_enum;
					storage_t const* symbol_array;
					size_t array_count;
				}noterminal;
			};
			bool is_terminal() const noexcept { return Syntax::lr1::is_terminal(sym); }
		};
		sbnf_processer(sbnf const& ref) : ref(ref) {}
		sbnf const& ref;
		template<typename Func> void analyze(std::u32string_view code, Func&& F) {
			auto Wrapper = [](void* data, travel input) {  (*reinterpret_cast<Func*>(data))(input); };
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