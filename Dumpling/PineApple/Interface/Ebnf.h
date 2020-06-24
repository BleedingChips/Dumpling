#pragma once
#include "Lr0.h"
#include "Nfa.h"

namespace PineApple::Ebnf
{

	using Symbol = Lr0::Symbol;

	struct Table
	{
		std::u32string symbol_table;
		std::vector<std::tuple<std::size_t, std::size_t>> symbol_map;
		size_t ter_count;
		Nfa::Table nfa_table;
		Lr0::Table lr0_table;
		std::u32string_view FindSymbolString(size_t input, bool IsTerminal) const noexcept;
		std::optional<size_t> FindSymbolState(std::u32string_view sym) const noexcept { bool is_terminal; return FindSymbolState(sym, is_terminal); }
		std::optional<size_t> FindSymbolState(std::u32string_view sym, bool& Isterminal) const noexcept;
	};

	Table CreateTable(std::u32string_view Code);

	struct Step
	{
		size_t state;
		std::u32string_view string;
		bool is_terminal;
		union {
			struct {
				size_t mask;
				size_t production_count;
			}reduce;
			struct {
				std::u32string_view capture;
				Nfa::Location loc;
			}shift;
		};
		bool IsTerminal() const noexcept { return is_terminal; }
		bool IsNoterminal() const noexcept { return !IsTerminal(); }
	};

	struct Element : Step
	{
		std::tuple<size_t, std::u32string_view, std::any>* datas = nullptr;
		std::tuple<size_t, std::u32string_view, std::any>& operator[](size_t index) { return datas[index]; }
		decltype(auto) GetRawData(size_t index) { return std::get<2>((*this)[index]); }
		template<typename Type>
		decltype(auto) GetData(size_t index) { return std::any_cast<Type>(std::get<2>((*this)[index])); }
		size_t GetAcception(size_t index) { return std::get<0>((*this)[index]); }
		std::u32string_view GetString(size_t index) { return std::get<1>((*this)[index]); }
		Element(Step const& ref) : Step(ref) {}
	};

	struct History
	{
		std::vector<Step> steps;
		std::any operator()(std::any(*Function)(void*, Element&), void* FUnctionBody) const;
		template<typename RespondFunction>
		std::any operator()(RespondFunction&& Func) const{
			auto FunctionImp = [](void* FunctionBody, Element& data) -> std::any {
				return  std::forward<RespondFunction>(*static_cast<std::remove_reference_t<RespondFunction>*>(FunctionBody))(data);
			};
			return operator()(FunctionImp, static_cast<void*>(&Func));
		}
	};

	History Process(Table const& Tab, std::u32string_view Code);
	inline std::any Process(History const& His, std::any(*Function)(void*, Element&), void* FUnctionBody) { return His(Function, FUnctionBody); }
	template<typename RespondFunction>
	std::any Process(History const& ref, RespondFunction&& Func) { return ref(std::forward<RespondFunction>(Func));}
	template<typename RequireType, typename RespondFunction>
	RequireType ProcessWrapper(History const& ref, RespondFunction&& Func) { return std::any_cast<RequireType>(ref(std::forward<RespondFunction>(Func))); }


	namespace Error
	{

		struct ExceptionStep
		{
			std::u32string Name;
			bool IsTerminal = false;
			size_t production_mask = Lr0::ProductionInput::default_mask();
			size_t production_count = 0;
			std::u32string capture;
			Nfa::Location loc;
		};

		struct MissingStartSymbol {};

		struct UndefinedTerminal {
			std::u32string token;
			Nfa::Location loc;
		};

		struct UndefinedNoterminal {
			std::u32string token;
		};

		struct UnsetDefaultProductionHead {};

		struct RedefinedStartSymbol {
			Nfa::Location loc;
		};

		struct UnaccableToken {
			std::u32string token;
			Nfa::Location loc;
		};

		struct UnaccableSyntax {
			std::u32string type;
			std::u32string data;
			Nfa::Location loction;
			std::vector<ExceptionStep> exception_step;
		};

		/*
		struct ErrorMessage {
			std::u32string message;
			Nfa::Location loc;
		};
		*/
	}
}

namespace PineApple::StrFormat
{
	template<> struct Formatter<Ebnf::Table>
	{
		std::u32string operator()(std::u32string_view, Ebnf::Table const& ref);
	};
}