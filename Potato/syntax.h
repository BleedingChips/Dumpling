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
namespace Potato
{
	/*
	enum class EdgeType
	{
		Epsilon,
		FirstEpsilon,
		Comsume,
		//FirstEpsilon,
		LookAheadPositiveAssert,
		LookAheadNegativeAssert,
		LookBehindPositiveAssert,
		LookBehindNegativeAssert,
		CaptureStart,
		Storage,
	};

	struct nfa
	{
		

		struct node
		{
			std::optional<size_t> is_accept;
			std::vector<std::variant<Epsilon, FirstEpsilon, LookAheadPositiveAssert, Comsume>> shift_table;
		};
	};










	struct nfa
	{
		struct node
		{
			std::optional<size_t> is_accept;
			std::map<size_t, Tool::range_set<char32_t>> table_shift;
			std::set<size_t> table_null_shift;
		};
		static nfa create_from_rex(std::u32string_view const* input, size_t length);
		const node& operator[](size_t index) const { return total_node[index]; }
		operator bool() const noexcept { return !total_node.empty(); }
		size_t start_state() const noexcept { return 0; }
		std::set<size_t> search_null_state_set(size_t head) const;
		nfa(const nfa&) = default;
		nfa(nfa&&) = default;
		nfa& operator=(const nfa&) = default;
		nfa& operator=(nfa&&) = default;
		nfa() = default;
		node& operator[](size_t index) { return total_node[index]; }
	private:
		std::tuple<size_t, size_t> create_single_rex(std::u32string_view Rex);
		std::vector<node> total_node; 
	};


	struct dfa
	{
		static dfa create_from_rexs(std::u32string_view const* rexs, size_t length);
		struct node
		{
			std::optional<size_t> is_accept;
			std::map<size_t, Tool::range_set<char32_t>> shift;
		};
		const node& operator[](size_t index) const { return nodes[index]; }
		size_t size() const noexcept { return nodes.size(); }
		size_t start_symbol() const noexcept { return 0; }
		operator bool() const noexcept { return !nodes.empty(); }
	private:
		node& operator[](size_t index) { return nodes[index]; }
		std::vector<node> nodes;
	};

	struct dfa_processer
	{
		struct travel
		{
			size_t acception;
			std::u32string_view token;
		};
		template<typename Function> 
		void analyze(const dfa& dfa, std::u32string_view code, Function&& function)
		{
			auto Func = [](void* Data, travel tra) {
				static_cast<Function&&>(*Data)(tra);
			};
			analyze_imp(dfa, code, Func, &function);
		}
		static std::optional<travel> comsume_analyze(const dfa& table, std::u32string_view input);
	private:
		void analyze_imp(const dfa& dfa, std::u32string_view code, void(*Func)(void*, travel), void* Data);
	};
	*/



	struct lr1
	{
		using storage_t = uint32_t;
		static constexpr storage_t mask = 0x8000'0000;

		static inline bool is_terminal(storage_t symbol) noexcept { return symbol < mask; }
		static constexpr inline storage_t eof_symbol() { return static_cast<storage_t>(mask - 1); }
		static constexpr inline storage_t noterminal_start() { return mask; }
		static constexpr inline storage_t start_symbol() { return (0xffff'ffff); }

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

		struct UnacceableProduction {};

		struct production_input
		{
			std::vector<storage_t> production;
			storage_t function_state;
			std::vector<storage_t> remove_forward;
			production_input(std::vector<storage_t> input, storage_t function_enmu = 0, std::vector<storage_t> remove = {})
				: production(std::move(input)), function_state(function_enmu), remove_forward(std::move(remove)) {
				if (production.size() > 0 && production.size() < std::numeric_limits<uint32_t>::max() && !is_terminal(production[0]));
				else
					throw UnacceableProduction{};
			}
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

		enum class Error : size_t
		{
			ReduceConflig,
			HeadMissing,
			UnavailableSymbol,
			ProductionRedefine,
			OperatorPriorityConflig
		};

		struct reduce_conflict : std::logic_error
		{
			storage_t m_conflig_token;
			storage_t m_old_state_index;
			storage_t m_new_state_index;
			std::vector<std::tuple<storage_t, std::vector<storage_t>, std::set<storage_t>>> m_state;
			reduce_conflict(storage_t token, storage_t old_state_index, storage_t new_state_index, std::vector<std::tuple<storage_t, std::vector<storage_t>, std::set<storage_t>>>);
		};

		struct production_head_missing : std::logic_error
		{
			storage_t m_require_head;
			storage_t m_production_index;
			production_head_missing(storage_t head, storage_t index);
		};

		struct unavailable_symbol {};

		struct same_production : std::logic_error
		{
			storage_t m_old_production_index;
			storage_t m_new_production_index;
			std::vector<storage_t> m_production;
			same_production(storage_t old_index, storage_t new_index, std::vector<storage_t> production);
		};

		struct operator_level_conflict : std::logic_error
		{
			storage_t m_token;
			operator_level_conflict(storage_t token) : std::logic_error("operator level conflict"), m_token(token) {}
		};

		lr1(
			std::vector<std::tuple<storage_t, storage_t, storage_t>> production,
			std::vector<table> table
		) : m_production(std::move(production)), m_table(std::move(table)) {}

		lr1(lr1&&) = default;

		std::vector<storage_t> serialization();
		static lr1 unserialization(const storage_t*, size_t length);

		std::vector<std::tuple<storage_t, storage_t, storage_t>> m_production;
		std::vector<table> m_table;
	private:
		lr1() = default;
	};

	struct lr1_processor
	{
		using storage_t = lr1::storage_t;

		struct error_state {
			std::set<storage_t> m_shift;
			std::map<storage_t, storage_t> m_reduce;
		};

		struct unacceptable_error : std::logic_error, error_state {
			storage_t m_forward_token;
			unacceptable_error(storage_t forward_token, error_state lpes);
		};

		struct uncomplete_error : std::logic_error, error_state {
			uncomplete_error(error_state lpes);
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
		
		lr1_processor(const lr1& table) : m_table_ref(table) { clear(); }

		// ForeachFunction : std::optional<terminal> (Ite), RespondFunction: void (lr1_processor::travel)
		template<typename Ite, typename ForeachFunction, typename RespondFunction>
		void analyze(ForeachFunction&& sym, RespondFunction&& Func, Ite ite)
		{
			auto TransFunc = [](void* Func, travel input) {
				std::forward<RespondFunction&&>(*reinterpret_cast<std::remove_reference_t<RespondFunction>*>(Func)).operator()(input);
			};
			size_t index = 0;
			while (true)
			{
				std::optional<storage_t> re = std::forward<ForeachFunction&&>(sym)(ite);
				try_reduce( re ?  (*re) : lr1::eof_symbol(), index, TransFunc, &Func);
				if (!re.has_value())
					break;
				++index;
			}
		}

		template<typename Type, typename SymbolFunction, typename RespondFunction>
		void analyze(SymbolFunction&& SF, RespondFunction&& Function, Type begin, Type end)
		{
			this->analyze([&](Type& ite) -> std::optional<storage_t> {
				if (ite != end)
					return std::forward<SymbolFunction&&>(SF)(ite++);
				else
					return std::nullopt;
			}, std::forward<RespondFunction&&>(Function), begin);
		}

		void clear() { m_state_stack = { 0 }; m_input_buffer.clear(); }

	private:
		// for no terminal, index means production index, for terminal ,index means token stream index, count means elements in production
		void try_reduce(storage_t symbol, size_t index, void (*Function)(void* Func, travel input), void* data);
		const lr1& m_table_ref;
		std::vector<storage_t> m_state_stack;
		std::vector<std::tuple<storage_t, std::size_t>> m_input_buffer;
	};

	template<typename Ite, typename ForeachFunction, typename RespondFunction>
	void lr1_process(const lr1& imp, ForeachFunction&& SF, RespondFunction&& Function, Ite begin)
	{
		lr1_processor pro(imp);
		pro.analyze(std::forward<ForeachFunction&&>(SF), std::forward<RespondFunction&&>(Function), begin);
	}

	template<typename Type, typename SymbolFunction, typename RespondFunction>
	void lr1_process(const lr1& imp, SymbolFunction&& SF, RespondFunction&& Function, Type begin, Type end)
	{
		lr1_processor pro(imp);
		pro.analyze(std::forward<SymbolFunction&&>(SF), std::forward<RespondFunction&&>(Function), begin, end);
	}

	struct lr1_ast {
		lr1::storage_t sym;
		// to no terminal, storage the ast_list, to terminal storage index in token stream
		std::variant<std::vector<lr1_ast>, size_t> index;

		struct imp
		{
			std::vector<lr1_ast> ast_buffer;
			void operator()(lr1_processor::travel input);
			lr1_ast result();
		};

		template<typename Type, typename SymbolFunction>
		static lr1_ast analyze(const lr1& lr1_imp, Type begin, Type end, SymbolFunction&& SF)
		{
			lr1_processor processer(lr1_imp);
			imp i;
			processer.analyze(begin, end, std::forward<SymbolFunction&&>(SF), i);
			return i.result();
		}
	};
};

