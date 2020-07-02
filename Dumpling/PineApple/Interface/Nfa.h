#pragma once
#include "Range.h"
#include <string_view>
#include <variant>
#include <optional>
#include <set>
#include "StrFormat.h"
namespace PineApple::Nfa
{
	struct Table
	{
		using RangeSet = Range::Set<char32_t>;
		enum class EdgeType : size_t
		{
			Acception,
			Comsume,
		};
		std::vector<RangeSet::Range> ComsumeEdge;
		std::vector<std::tuple<size_t, size_t, size_t, size_t>> Edges;
		std::vector<std::tuple<size_t, size_t>> Nodes;
	};

	Table CreateTableFromRex(std::u32string_view const* input, size_t const* state, size_t const* mask, size_t input_length);
	Table CreateTableFromRex(std::u32string_view const* input, size_t input_length);
	Table CreateTableFromRexReversal(std::u32string_view const* input, size_t input_length);
	Table CreateTableFromRexReversal(std::u32string_view const* input, size_t const* state, size_t const* mask, size_t input_length);

	struct MarchElement
	{
		size_t acception;
		size_t mask;
		std::u32string_view capture;
		std::u32string_view last_string;
	};

	std::optional<MarchElement> Consume(Table const& Ref, std::u32string_view String);
	std::vector<MarchElement> Process(Table const& Ref, std::u32string_view String);

	struct Location {
		size_t start_index = 0;
		size_t line = 0;
		size_t total_index = 0;
		size_t length = 0;
	};

	struct DocumenetMarchElement
	{
		MarchElement march;
		Location location;
	};

	std::optional<DocumenetMarchElement> DecumentComsume(Table const& Ref, std::u32string_view String, Location& Loc);
	std::vector<DocumenetMarchElement> DecumentProcess(Table const& Ref, std::u32string_view String);

	namespace Error
	{
		struct UnaccaptableRexgex {
			std::u32string Regex;
			size_t AccepetableState;
			size_t AccepetableMask;
			size_t Index;
		};

		struct UnaccaptableString {
			std::u32string TotalString;
			Location Loc;
		};
	}
}

namespace PineApple::StrFormat
{
	template<> struct Formatter<Nfa::Table>
	{
		std::u32string operator()(std::u32string_view, Nfa::Table const&);
	};
}