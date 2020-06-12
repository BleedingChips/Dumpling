#pragma once
#include <assert.h>
#include <vector>
#include <map>
#include <set>
#include <any>
#include <functional>
namespace Potato::Parser::Lr0
{
	using SymbolStorageT = int32_t;

	constexpr struct TerminalT {} terminal;
	constexpr struct NoTerminalT {} noterminal;

	struct Symbol
	{
		SymbolStorageT value;
		constexpr Symbol() : Symbol(Symbol::EndOfFile()){}
		constexpr Symbol(size_t input, TerminalT) : value(static_cast<SymbolStorageT>(input) + 1){
			assert(input < static_cast<uint64_t>(std::numeric_limits<SymbolStorageT>::max()) - 2);
		}
		constexpr Symbol(size_t input, NoTerminalT) : value(-static_cast<SymbolStorageT>(input) - 1) {
			assert(input < static_cast<uint64_t>(std::numeric_limits<SymbolStorageT>::max()) - 2);
		}
		constexpr bool operator<(Symbol const& input) const noexcept { return value < input.value; }
		constexpr bool operator== (Symbol const& input) const noexcept { return value == input.value; }
		constexpr bool IsTerminal() const noexcept { assert(value != 0); return value > 0; }
		constexpr bool IsNoTerminal() const noexcept { assert(value != 0); return value < 0; }
		constexpr static Symbol EndOfFile() noexcept { return Symbol(std::numeric_limits<SymbolStorageT>::max()); }
		constexpr static Symbol StartSymbol() noexcept { return Symbol(std::numeric_limits<SymbolStorageT>::min()); }
		constexpr bool IsEndOfFile() const noexcept { return *this == EndOfFile(); }
		constexpr bool IsStartSymbol() const noexcept { return *this == StartSymbol(); }
		constexpr SymbolStorageT Value() const noexcept { return value; }
	private:
		constexpr Symbol(SymbolStorageT input) : value(input) {}
	};

	struct Table {

		struct Production
		{
			Symbol value;
			size_t production_count;
			size_t mask;
			std::set<Symbol> cancle_reduce;
		};

		struct Reduce
		{
			size_t ProductionIndex;
		};

		struct Shift
		{
			Symbol RequireSymbol;
			size_t ShiftState;
		};

		struct Node
		{
			size_t reduce_adress;
			size_t reduce_count;
			size_t shift_adress;
			size_t shift_count;
		};

		std::vector<Production> productions;
		std::vector<Reduce> Reduces;
		std::vector<Shift> Shifts;
		std::vector<Node> nodes;
	};

	struct Processer
	{
		struct Step
		{
			Symbol value;
			union
			{
				struct {
					size_t production_index;
					size_t production_count;
					size_t mask;
				}reduce;

				struct {
					size_t token_index;
				}terminal;
			};
			constexpr bool IsTerminal() const noexcept { return value.IsTerminal(); }
			constexpr bool IsNoTerminal() const noexcept { return value.IsNoTerminal(); }
			constexpr bool IsEndOfFile() const noexcept { return value.IsEndOfFile(); }
			constexpr bool IsStartSymbol() const noexcept { return value.IsStartSymbol(); }
		};

		struct History
		{
			std::vector<Step> steps;
		};

		struct Element {
			Symbol symbol;
			std::any data;
		};

		History operator()(Table const& Table, Symbol const* TokenArray, size_t TokenLength);
		std::any operator()(Table const& Table, History const& history, std::any(*Function)(void*, Step const&, Element*), void* FunctionBody);
		template<typename RespondFunction>
		std::any operator()(Table const& Table, History const& history, RespondFunction&& Func) {
			auto FunctionImp = [](void* FunctionBody, Step const& tra, Element* data) -> std::any {
				return  std::forward<RespondFunction>(*static_cast<std::remove_reference_t<RespondFunction>*>(FunctionBody))(tra, data);
			};
			return operator()(Table, history, FunctionImp, &Func);
		}
	};

	enum class  Associativity
	{
		Left,
		Right,
	};

	struct OpePriority
	{
		//OpePriority(std::initializer_list<Symbol> sym) : OpePriority(std::move(sym), Associativity::Left) {}
		OpePriority(std::vector<Symbol> sym) : OpePriority(std::move(sym), Associativity::Left) {}
		OpePriority(std::vector<Symbol> sym, Associativity lp) : sym(std::move(sym)), left_priority(lp) {}
		//OpePriority(Symbol sym) : OpePriority(std::vector<Symbol>{ sym }, true) {}
		//OpePriority(Symbol sym, bool lp) : OpePriority(std::vector<Symbol>{ sym }, lp) {}
		std::vector<Symbol> sym;
		Associativity left_priority;
	};

	struct ProductionInput
	{

		constexpr static size_t default_mask() { return std::numeric_limits<size_t>::max(); }

		std::vector<Symbol> production;
		std::set<Symbol> remove;
		size_t function_mask;
		ProductionInput(std::vector<Symbol> input) : ProductionInput(std::move(input), default_mask()) {}
		ProductionInput(std::vector<Symbol> input, size_t funtion_enum) : production(std::move(input)), function_mask(funtion_enum) {}
		ProductionInput(std::vector<Symbol> input, std::set<Symbol> remove, size_t funtion_enum) : production(std::move(input)), function_mask(funtion_enum), remove(std::move(remove)){}
		ProductionInput(const ProductionInput&) = default;
		ProductionInput(ProductionInput&&) = default;
		ProductionInput& operator=(const ProductionInput&) = default;
		ProductionInput& operator=(ProductionInput&&) = default;
	};

	Table CreateTable(Symbol start_symbol, std::vector<ProductionInput> const& production, std::vector<OpePriority> const& priority);

	namespace Error
	{
		struct noterminal_production_undefine {
			Symbol value;
		};

		struct operator_priority_conflict {
			Symbol target_symbol;
			Symbol conflicted_symbol;
		};

		struct production_redefine
		{
			std::vector<Symbol> productions;
			size_t production_index_1;
			size_t respond_mask_1;
			size_t production_index_2;
			size_t respond_mask_2;
		};

		struct unacceptable_symbol {
			size_t index;
			Symbol symbol;
			Processer::History backup_step;
		};
	}
}