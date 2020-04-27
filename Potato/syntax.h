#pragma once
#include <vector>
#include <string>
#include <functional>
#include <array>
#include <optional>
#include <regex>
#include <variant>
#include <set>
#include <map>
#include <deque>
#include <assert.h>
#include <variant>
#include "range_set.h"
namespace Potato::Syntax
{

	struct lr1_storage
	{
		using storage_t = size_t;
		static constexpr storage_t mask = 0x8000'0000;

		static inline bool is_terminal(storage_t symbol) noexcept { return symbol < mask; }
		static constexpr inline storage_t eof_symbol() { return static_cast<storage_t>(mask - 1); }
		static constexpr inline storage_t noterminal_start() { return mask; }
		static constexpr inline storage_t start_symbol() { return (0xffff'ffff); }
		static constexpr storage_t no_function_enum() { return std::numeric_limits<storage_t>::max(); }

		std::vector<std::tuple<storage_t, storage_t, storage_t>> productions;
		std::vector<std::tuple<storage_t, storage_t>> reduce_shift_table;
		std::vector<std::tuple<storage_t, storage_t, storage_t>> nodes;
	};



	struct lr1
	{
		using storage_t = lr1_storage::storage_t;
		static constexpr storage_t mask = lr1_storage::mask;
		static inline bool is_terminal(storage_t symbol) noexcept { return lr1_storage::is_terminal(symbol); }
		static constexpr inline storage_t eof_symbol() { return lr1_storage::eof_symbol(); }
		static constexpr inline storage_t noterminal_start() { return mask; }
		static constexpr inline storage_t start_symbol() { return lr1_storage::start_symbol(); }
		static constexpr storage_t no_function_enum() { return lr1_storage::no_function_enum(); }

		struct unacceptable_production_error : std::exception
		{
			storage_t production_index;
			char const* what() const noexcept override;
			unacceptable_production_error(storage_t index) : production_index(index) {}
			unacceptable_production_error(const unacceptable_production_error&) = default;
		};

		struct missing_noterminal_define_error : std::exception
		{
			missing_noterminal_define_error(storage_t noterminal_symbol) : noterminal_symbol(noterminal_symbol) {}
			missing_noterminal_define_error(const missing_noterminal_define_error& in) = default;
			storage_t noterminal_symbol;
			char const* what() const noexcept override;
		};

		struct production_redefine_error : std::exception
		{
			production_redefine_error(const production_redefine_error& in) = default;
			production_redefine_error(storage_t old_production_index, storage_t redefine_production_index) : old_production_index(old_production_index), redefine_production_index(redefine_production_index) {}
			storage_t old_production_index;
			storage_t redefine_production_index;
			char const* what() const noexcept override;
		};

		struct reduce_conflict_error : std::exception
		{
			storage_t token;
			storage_t possible_production_1;
			storage_t possible_production_2;
			reduce_conflict_error(storage_t token, storage_t possible_production_1, storage_t possible_production_2) :
				token(token), possible_production_1(possible_production_1), possible_production_2(possible_production_2) {}
			reduce_conflict_error(const reduce_conflict_error&) = default;
			char const* what() const noexcept override;
		};

		struct operator_conflict_error : std::exception
		{
			storage_t token;
			storage_t conflig_token;
			char const* what() const noexcept override;
			operator_conflict_error(storage_t token, storage_t conflig_token) : token(token), conflig_token(conflig_token) {}
			operator_conflict_error(const operator_conflict_error&) = default;
		};

		struct ope_priority
		{
			ope_priority(std::initializer_list<storage_t> sym) : ope_priority(std::move(sym), true) {}
			ope_priority(std::vector<storage_t> sym) : ope_priority(std::move(sym), true) {}
			ope_priority(std::vector<storage_t> sym, bool lp) : sym(std::move(sym)), left_priority(lp) {}
			ope_priority(storage_t sym) : ope_priority(std::vector<storage_t>{ sym }, true) {}
			ope_priority(storage_t sym, bool lp) : ope_priority(std::vector<storage_t>{ sym }, lp) {}
			std::vector<storage_t> sym;
			bool left_priority;
		};

		struct production_input
		{
			std::vector<storage_t> production;
			storage_t function_state;
			std::vector<storage_t> remove_forward;
			production_input(std::vector<storage_t> input, std::vector<storage_t> remove, storage_t function_enmu)
				: production(std::move(input)), function_state(function_enmu), remove_forward(std::move(remove)) {}
			
			production_input(std::vector<storage_t> input) : production_input(std::move(input), {}, no_function_enum()) {}
			production_input(std::vector<storage_t> input, storage_t funtion_enum) : production_input(std::move(input), {}, funtion_enum) {}
			production_input(std::vector<storage_t> input, std::vector<storage_t> remove) : production_input(std::move(input), std::move(remove), no_function_enum()) {}
			production_input(const production_input&) = default;
			production_input(production_input&&) = default;
			production_input& operator=(const production_input&) = default;
			production_input& operator=(production_input&&) = default;
		};

		static lr1 create(
			storage_t start_symbol,
			std::vector<production_input> production,
			std::vector<ope_priority> priority
		);

		struct table
		{
			std::map<storage_t, storage_t> m_shift;
			std::map<storage_t, storage_t> m_reduce;
		};

		operator lr1_storage() const;
		lr1_storage storage() const { return *this; }

		lr1(
			std::vector<std::tuple<storage_t, storage_t, storage_t>> production,
			std::vector<table> table
		) : m_production(std::move(production)), m_table(std::move(table)) {}

		lr1(lr1&&) = default;

		std::vector<std::tuple<storage_t, storage_t, storage_t>> m_production;
		std::vector<table> m_table;
	private:
		lr1() = default;
	};

	struct lr1_processor
	{
		using storage_t = lr1::storage_t;

		struct unacceptable_error : std::exception {
			storage_t token;
			storage_t last_symbol;
			char const* what() const noexcept override;
			unacceptable_error(storage_t token, storage_t last_symbol) : token(token), last_symbol(last_symbol) {}
			unacceptable_error(const unacceptable_error&) = default;
		};

		struct travel
		{
			storage_t symbol;
			union 
			{
				struct noterminal_t
				{
					std::size_t production_index;
					std::size_t production_count;
					storage_t const* symbol_array;
					std::size_t function_enum;
				}noterminal;

				struct terminal_t
				{
					std::size_t token_index;
				}terminal;			
			};
			bool is_terminal() const noexcept { return lr1::is_terminal(symbol); }
		};
		
		lr1_processor(const lr1_storage& table) : m_table_ref(table) { clear(); }

		// ForeachFunction : std::optional<terminal> (Ite), RespondFunction: void (lr1_processor::travel)

		template<typename TokenGenerator, typename RespondFunction>
		void analyze(TokenGenerator&& sym, RespondFunction&& Func)
		{
			auto TransFunc = [](void* Func, travel input) -> bool {
				std::forward<RespondFunction&&>(*reinterpret_cast<std::remove_reference_t<RespondFunction>*>(Func)).operator()(input);
				return true;
			};
			clear();
			while (true)
			{
				std::optional<storage_t> re = std::forward<TokenGenerator&&>(sym)();
				if (!try_reduce(re ? (*re) : lr1::eof_symbol(), TransFunc, &Func) || !re)
					break;
			}
		}

		template<typename TokenGenerator, typename RespondFunction>
		void controlable_analyze(TokenGenerator&& sym, RespondFunction&& Func)
		{
			auto TransFunc = [](void* Func, travel input) -> bool {
				return std::forward<RespondFunction&&>(*reinterpret_cast<std::remove_reference_t<RespondFunction>*>(Func)).operator()(input);
			};
			clear();
			while (true)
			{
				std::optional<storage_t> re = std::forward<TokenGenerator&&>(sym)();
				if (!try_reduce(re ? (*re) : lr1::eof_symbol(), TransFunc, &Func) || !re)
					break;
			}
		}

		void clear() { m_state_stack = { 0 }; m_input_buffer.clear(); }

	private:
		// for no terminal, index means production index, for terminal ,index means token stream index, count means elements in production
		bool try_reduce(storage_t symbol, bool (*Function)(void* Func, travel input), void* data);
		const lr1_storage& m_table_ref;
		size_t token_index = 0;
		std::vector<storage_t> m_state_stack;
		std::vector<std::tuple<storage_t, std::size_t>> m_input_buffer;
	};

	template<typename TokenGenerator, typename RespondFunction>
	void lr1_process(const lr1_storage& imp, TokenGenerator&& SF, RespondFunction&& Function)
	{
		lr1_processor pro(imp);
		pro.analyze(std::forward<TokenGenerator&&>(SF), std::forward<RespondFunction&&>(Function));
	}
};

