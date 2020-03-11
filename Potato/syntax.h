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

namespace Potato
{

	struct lr1
	{
		using storage_t = uint32_t;
		static constexpr storage_t mask = 0x8000'0000;

		static inline bool is_terminal(storage_t symbol) noexcept { return symbol < mask; }
		static inline storage_t terminal_eof() { return static_cast<storage_t>(mask - 1); }
		static inline storage_t noterminal_start() { return (0xffff'ffff); }

		struct ope_priority
		{
			ope_priority(std::vector<storage_t> sym, bool lp = true) : sym(std::move(sym)), left_priority(lp) {}
			std::vector<storage_t> sym;
			bool left_priority;
		};

		static lr1 create_table(
			uint32_t start_symbol,
			std::vector<std::vector<storage_t>> production,
			std::vector<ope_priority> priority
		);

		struct table
		{
			std::map<storage_t, storage_t> m_shift;
			std::map<storage_t, storage_t> m_reduce;
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
			storage_t start_symbol,
			std::vector<std::vector<storage_t>> production,
			std::vector<ope_priority> priority
		) : lr1(create_table(start_symbol, std::move(production), std::move(priority))) {}

		lr1(
			std::vector<std::tuple<storage_t, storage_t>> production,
			std::vector<table> table
		) : m_production(std::move(production)), m_table(std::move(table)) {}

		lr1(lr1&&) = default;

	private:

		lr1() = default;

		std::vector<std::tuple<storage_t, storage_t>> m_production;
		std::vector<table> m_table;
	};

	
	/*
	struct production_index
	{
		uint32_t m_production_index;
		uint32_t m_production_element_index;
		bool operator<(const production_index& pe) const
		{
			return m_production_index < pe.m_production_index || (m_production_index == pe.m_production_index && m_production_element_index < pe.m_production_element_index);
		}
		bool operator==(const production_index& pe) const
		{
			return m_production_index == pe.m_production_index && m_production_element_index == pe.m_production_element_index;
		}
	};

	struct LR1_implement
	{
		LR1_implement(uint32_t start_symbol, std::vector<std::vector<uint32_t>> production,
			std::vector<std::tuple<std::vector<std::variant<uint32_t, std::pair<uint32_t, uint32_t>>>, Associativity>> input);
		LR1_implement(const uint32_t* input, size_t length);
		size_t calculate_data_length() const noexcept;
		void output_data(uint32_t* output) const noexcept;
	private:
		std::vector<std::tuple<uint32_t, uint32_t>> m_production;
		std::vector<shift_reduce_description> m_table;
		friend struct lr1_processor;
	};

	struct accect {};

	struct lr1_process_error_state {
		std::set<uint32_t> m_shift;
		std::map<uint32_t, uint32_t> m_reduce;
	};

	struct lr1_process_unacceptable_error : std::logic_error, lr1_process_error_state {
		uint32_t m_forward_token;
		lr1_process_unacceptable_error(uint32_t forward_token, lr1_process_error_state lpes);
	};

	struct lr1_process_uncomplete_error : std::logic_error, lr1_process_error_state {
		lr1_process_uncomplete_error(lr1_process_error_state lpes);
	};

	struct lr1_processor
	{
		struct result
		{
			uint32_t reduce_symbol;
			uint32_t reduce_production_index;
			uint32_t element_used;
		};

		lr1_processor(const LR1_implement&);
		std::vector<result> receive(uint32_t symbol);
		auto finish_input() { return receive(terminal_eof()); }
	private:
		const LR1_implement& m_syntax;
		//std::deque<uint64_t> m_buffer;
		std::vector<uint32_t> m_state_stack;
		std::vector<uint32_t> m_input_buffer;
	};
	
	namespace Implement
	{

		

	}

	namespace Error
	{
		template<typename Noterminal, typename Terminal> struct prodution_state
		{
			uint32_t m_index;
			std::vector<std::variant<Noterminal, Terminal>> m_production;
			std::set<Terminal> m_forward;
			prodution_state(const std::tuple<uint32_t, std::vector<uint32_t>, std::set<uint32_t>>& state)
			{
				m_index = std::get<0>(state);
				for (auto& ite : std::get<1>(state))
				{
					if (Implement::is_terminal(ite))
						m_production.push_back(Implement::cast_terminal<Terminal>(ite));
					else
						m_production.push_back(Implement::cast_noterminal<Noterminal>(ite));
				}
				for (auto& ite : std::get<2>(state))
					m_forward.insert(Implement::cast_terminal<Terminal>(ite));
			}
		};

		template<typename Noterminal, typename Terminal>
		struct LR1_reduce_conflict_error : std::logic_error
		{
			Terminal m_token;
			uint32_t m_old_state_index;
			uint32_t m_new_state_index;
			std::vector<prodution_state<Noterminal, Terminal>> m_state;
			LR1_reduce_conflict_error(const Implement::lr1_reduce_conflict& lrc)
				: std::logic_error("reduce comflict"), m_token(Implement::cast_terminal<Terminal>(lrc.m_conflig_token)),
				m_old_state_index(lrc.m_old_state_index), m_new_state_index(lrc.m_new_state_index) {
				for (auto& ite : lrc.m_state)
					m_state.push_back(ite);
			}
		};

		template<typename NoTerminal>
		struct LR1_production_head_missing : std::logic_error
		{
			NoTerminal m_token;
			uint32_t m_index;
			LR1_production_head_missing(const Implement::lr1_production_head_missing& lphm)
				: std::logic_error("unable to find proction head"), m_token(Implement::cast_noterminal<NoTerminal>(lphm.m_require_head)),
				m_index(lphm.m_production_index) {}
		};

		template<typename Terminal>
		struct LR1_operator_level_conflict : std::logic_error
		{
			Terminal m_token;
			LR1_operator_level_conflict(Terminal token) : std::logic_error("operator level conflict"), m_token(token) {}
		};
	}
	*/

	/*
	example:

	enum class Terminal
	{
		Num,
		Add,
		Minus,
		Mulyiply,
		Divide,
		Question,
		Colon
	};

	enum class NoTerminal
	{
		Expr,
	};

	LR1<NoTerminal, Terminal> lr1(
	// Start Symbol
	NoTerminal::Expr,
	{
		// Production
		{NoTerminal::Expr, {Terminal::Num}},
		{NoTerminal::Expr, {NoTerminal::Expr, Terminal::Add, NoTerminal::Expr}},
		{NoTerminal::Expr, {NoTerminal::Expr, Terminal::Minus, NoTerminal::Expr}},
		{NoTerminal::Expr, {NoTerminal::Expr, Terminal::Divide, NoTerminal::Expr}},
		{NoTerminal::Expr, {NoTerminal::Expr, Terminal::Mulyiply, NoTerminal::Expr}},
		{NoTerminal::Expr, {NoTerminal::Expr, Terminal::Question, NoTerminal::Expr, Terminal::Colon, NoTerminal::Expr}},
	},
	{
		// Operator Priority
		{ {Terminal::Divide, Terminal::Mulyiply}, Associativity::Left},
		{ {Terminal::Add, Terminal::Minus}, Associativity::Left},
		{ {std::pair{Terminal::Question, Terminal::Colon} }, Associativity::Left},
	}
	);
	*/

/*
	// See the example in the source code
	template<typename NoTerminal, typename Terminal>
	struct LR1 : Implement::LR1_implement
	{
		static_assert(std::is_enum_v<NoTerminal> && std::is_enum_v<Terminal>, "LR1 only accept enum");

		using symbol = std::variant<NoTerminal, Terminal>;
		inline static uint32_t cast_symbol(std::variant<NoTerminal, Terminal> input) { return Implement::cast_symbol(input); }

		LR1(const uint32_t* input, size_t length) : Implement::LR1_implement(input, length) {}


		LR1(
			NoTerminal start_symbol,
			std::initializer_list<CFG_prodution<NoTerminal, Terminal>> production,
			std::initializer_list<operator_priority<Terminal>> priority = {}
		) try : Implement::LR1_implement(cast_symbol(start_symbol), std::move(translate(production)), std::move(translate(priority))) {}
		catch (const Implement::lr1_reduce_conflict& lrc)
		{
			Error::LR1_reduce_conflict_error<NoTerminal, Terminal> error(lrc);
			throw error;
		}
		catch (const Implement::lr1_production_head_missing& lphm)
		{
			Error::LR1_production_head_missing<NoTerminal> error(lphm);
			throw error;
		}

	private:

		static std::vector<std::vector<uint32_t>>
			translate(std::initializer_list<CFG_prodution<NoTerminal, Terminal>> production)
		{
			std::vector<std::vector<uint32_t>> production_tem;
			production_tem.reserve(production.size() + 1);
			for (auto& ite : production)
			{
				std::vector<uint32_t> temporary;
				temporary.reserve(ite.m_production.size() + 1);
				temporary.push_back(cast_symbol(ite.m_head_symbol));
				for (auto ite2 : ite.m_production)
					temporary.push_back(cast_symbol(ite2));
				production_tem.push_back(std::move(temporary));
			}
			return std::move(production_tem);
		}
		static auto
			translate(std::initializer_list<operator_priority<Terminal>> production)
		{
			std::vector<std::tuple<std::vector<std::variant<uint32_t, std::pair<uint32_t, uint32_t>>>, Associativity>> result;
			result.reserve(production.size());
			uint32_t index = 0;
			for (const operator_priority<Terminal>& ite : production)
			{
				std::vector<std::variant<uint32_t, std::pair<uint32_t, uint32_t>>> tem;
				tem.reserve(ite.m_operator.size());
				for (auto& ite2 : ite.m_operator)
				{
					if (std::holds_alternative<Terminal>(ite2))
						tem.push_back(cast_symbol(std::get<Terminal>(ite2)));
					else {
						auto& ref = std::get<std::pair<Terminal, Terminal>>(ite2);
						tem.push_back(std::pair{ cast_symbol(ref.first), cast_symbol(ref.second) });
					}
				}
				result.push_back({ std::move(tem), ite.m_associativity });
			}
			return std::move(result);
		}
	};

	namespace Error
	{
		template<typename Terminal>
		struct generate_ast_error_state
		{
			std::set<Terminal> m_shift;
			std::map<Terminal, uint32_t> m_reduce;
		};

		template<typename NoTerminal, typename Terminal>
		struct generate_ast_unacceptable_error : std::logic_error, generate_ast_error_state<Terminal>
		{
			using state_t = generate_ast_error_state<Terminal>;
			generate_ast_unacceptable_error(std::optional<std::tuple<Terminal, uint32_t>> forward_token, state_t state,
				std::vector<typename ast_node<NoTerminal, Terminal>::element> stack)
				: logic_error("unacceptable token"), m_forward_token(forward_token), state_t(std::move(state)),
				m_stack(std::move(stack)) {}
			std::optional<std::tuple<Terminal, uint64_t>> m_forward_token;
			std::vector<typename ast_node<NoTerminal, Terminal>::element> m_stack;
		};

		template<typename Terminal, typename DataType>
		struct generate_ast_uncomplete_error : std::logic_error, generate_ast_error_state<Terminal>
		{
			using state_t = generate_ast_error_state<Terminal>;
			generate_ast_uncomplete_error(state_t state)
				: logic_error("unacceptable eof"), state_t(std::move(state)) {}
		};

		template<typename Terminal>
		struct generate_ast_uncomplete_ast_error : std::logic_error, generate_ast_error_state<Terminal>
		{
			generate_ast_uncomplete_ast_error(generate_ast_error_state<Terminal> state)
				: logic_error("unacceptable eof symbol"), generate_ast_error_state<Terminal>(std::move(state)) {}
		};
	}

	namespace Implement
	{

		template<typename NoTerminal, typename Terminal>
		struct generate_ast_execute
		{
			using ast_node_t = ast_node<NoTerminal, Terminal>;
			using ast_node_terminal_t = ast_node_terminal<Terminal>;


			static Error::generate_ast_error_state<Terminal> cast_error_state(const lr1_process_error_state& lpe)
			{
				std::set<Terminal> shift;
				for (auto& ite : lpe.m_shift)
					if (Implement::is_terminal(ite) && ite != Implement::terminal_eof())
						shift.insert(Implement::cast_terminal<Terminal>(ite));
				std::map<Terminal, uint32_t> reduce;
				for (auto& ite : lpe.m_reduce)
					if (Implement::is_terminal(ite.first) && ite.first != Implement::terminal_eof())
						reduce.insert({ Implement::cast_terminal<Terminal>(ite.first), ite.second });
				return { std::move(shift), std::move(reduce) };
			}

			static void reduce_stack(std::vector<typename ast_node_t::element>& stack, const std::vector<typename Implement::lr1_processor::result>& input)
			{
				for (auto& ite : input)
				{
					if (ite.reduce_symbol != noterminal_start())
					{
						assert(ite.element_used <= stack.size());
						auto ite2 = stack.begin() + (stack.size() - ite.element_used);
						std::vector<typename ast_node_t::element> temporary(std::move_iterator(ite2), std::move_iterator(stack.end()));
						stack.erase(ite2, stack.end());
						stack.push_back(ast_node_t(Implement::cast_noterminal<NoTerminal>(ite.reduce_symbol), ite.reduce_production_index, std::move(temporary)));
					}
				}
			}

			template<typename Ite>
			auto operator()(const LR1<NoTerminal, Terminal>& syntax, Ite begin, Ite end) -> ast_node<NoTerminal, Terminal>
			{
				std::vector<typename ast_node_t::element> data_stack;
				uint32_t index = 0;
				Implement::lr1_processor lp(syntax);
				try {
					for (; begin != end; ++begin)
					{
						Terminal input = *begin;
						auto re = lp.receive(Implement::cast_symbol<NoTerminal, Terminal>(input));
						reduce_stack(data_stack, re);
						data_stack.push_back(ast_node_terminal_t{ input, index++ });
					}
					auto re = lp.finish_input();
					reduce_stack(data_stack, re);
					assert(data_stack.size() == 1);
					return std::move(data_stack[0]);
				}
				catch (const Implement::lr1_process_unacceptable_error& error)
				{
					if (begin != end)
						throw Error::generate_ast_unacceptable_error<NoTerminal, Terminal>{ { {*begin, index}}, cast_error_state(error), std::move(data_stack) };
					else
						throw Error::generate_ast_unacceptable_error<NoTerminal, Terminal>{ {}, cast_error_state(error), std::move(data_stack) };
				}
			}
		};
	}

	template<typename NoTerminal, typename Terminal, typename Ite>
	auto generate_ast(const LR1<NoTerminal, Terminal>& syntax, Ite begin, Ite end)
	{
		static_assert(std::is_same_v<std::remove_const_t<std::remove_reference_t<decltype(*begin)>>, Terminal>, "");
		return Implement::generate_ast_execute<NoTerminal, Terminal>{}(syntax, begin, end);
	}
	*/
};

