#pragma once
#include <vector>
#include <assert.h>
namespace Potato::Lr1
{

	using symbol_storage_t = int32_t;

	constexpr struct terminal_t {} terminal;
	constexpr struct noterminal_t {} noterminal;

	struct symbol
	{
		symbol_storage_t value;
		symbol(symbol_storage_t input) : value(input) {}
		symbol(size_t input, terminal_t) {
			assert(input < static_cast<uint64_t>(std::numeric_limits<symbol_storage_t>::max()) - 2);
			value = static_cast<symbol_storage_t>(input) + 1;
		}
		symbol(size_t input, noterminal_t) {
			assert(input < static_cast<uint64_t>(std::numeric_limits<symbol_storage_t>::max()) - 2);
			value = -static_cast<symbol_storage_t>(input) - 1;
		}
		bool operator<(symbol const& input) const noexcept { return value < input.value; }
		bool operator== (symbol const& input) const noexcept { return value == input.value; }
		bool is_terminal() const noexcept { return value >= 0; }
		bool is_noterminal() const noexcept { return value < 0; }
		static symbol eof() noexcept { return std::numeric_limits<symbol_storage_t>::max(); }
		static symbol start() noexcept { return std::numeric_limits<symbol_storage_t>::min(); }
		bool is_eof() const noexcept { return *this == eof(); }
		bool is_start() const noexcept { return *this == start(); }
		symbol_storage_t as_value() const noexcept { return value; }
	};

	enum class RespondType
	{
		Reduce,
		Shift,
	};

	struct table {
		
		struct production
		{
			symbol value;
			size_t production_count;
			size_t mask;
		};

		struct respond
		{
			RespondType type;
			symbol require_token;
			size_t state_or_index;
		};

		struct node
		{
			size_t responds_adress;
			size_t count;
		};

		std::vector<production> productions;
		std::vector<respond> responds;
		std::vector<node> nodes;
	};

	struct processer
	{
		struct step
		{
			RespondType type;
			symbol value;
			union
			{
				struct {
					size_t production_index;
					size_t production_count;
					size_t respond_function_mask;
				}reduce;

				struct {
					size_t token_index;
				}terminal;
			};
		};

		struct history
		{
			std::vector<step> steps;
		};

		history operator()(const table& table, symbol const* token_array, size_t index);
	};

	struct ope_priority
	{
		ope_priority(std::initializer_list<symbol> sym) : ope_priority(std::move(sym), true) {}
		ope_priority(std::vector<symbol> sym) : ope_priority(std::move(sym), true) {}
		ope_priority(std::vector<symbol> sym, bool lp) : sym(std::move(sym)), left_priority(lp) {}
		//ope_priority(symbol sym) : ope_priority(std::vector<symbol>{ sym }, true) {}
		//ope_priority(symbol sym, bool lp) : ope_priority(std::vector<symbol>{ sym }, lp) {}
		std::vector<symbol> sym;
		bool left_priority;
	};

	struct production_input
	{

		size_t default_mask() { return std::numeric_limits<size_t>::max(); }

		std::vector<symbol> production;
		size_t function_mask;
		std::vector<symbol> remove_forward;
		production_input(std::vector<symbol> input, std::vector<symbol> remove, size_t function_enmu)
			: production(std::move(input)), function_mask(function_enmu), remove_forward(std::move(remove)) {}

		production_input(std::vector<symbol> input) : production_input(std::move(input), {}, default_mask()) {}
		production_input(std::vector<symbol> input, size_t funtion_enum) : production_input(std::move(input), {}, funtion_enum) {}
		production_input(std::vector<symbol> input, std::vector<symbol> remove) : production_input(std::move(input), std::move(remove), default_mask()) {}
		production_input(const production_input&) = default;
		production_input(production_input&&) = default;
		production_input& operator=(const production_input&) = default;
		production_input& operator=(production_input&&) = default;
	};

	table create(
		symbol start_symbol,
		std::vector<production_input> production,
		std::vector<ope_priority> priority
	);

	namespace Error
	{
		struct noterminal_production_undefine {
			symbol value;
		};

		struct operator_priority_conflict {
			symbol target_symbol;
			symbol conflicted_symbol;
		};

		struct production_redefine
		{
			std::vector<symbol> productions;
			size_t production_index_1;
			size_t respond_mask_1;
			size_t production_index_2;
			size_t respond_mask_2;
		};
	}

}