#pragma once
#include "Interval.h"
#include <string_view>
#include <variant>
#include <optional>
#include <set>
#include "StrFormat.h"
namespace PineApple::Nfa
{
	struct Table
	{

		using Interval = ::PineApple::Interval<char32_t>;
		using Segment = typename Interval::Segment;

		enum class EdgeType : size_t
		{
			Acception,
			Comsume,
		};
		std::vector<Segment> comsume_edge;
		std::vector<std::tuple<size_t, size_t, size_t, size_t>> edges;
		std::vector<std::tuple<size_t, size_t>> nodes;
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

	struct SectionPoint
	{
		size_t total_index = 0;
		size_t line = 0;
		size_t line_index = 0;
		bool operator<(SectionPoint const& lp) const {return total_index < lp.total_index;}
	};

	using Section = ::PineApple::Segment<SectionPoint>;

	struct DocumenetMarchElement
	{
		MarchElement march;
		Section section;
	};

	std::optional<DocumenetMarchElement> DecumentComsume(Table const& Ref, std::u32string_view String, Section& Loc);
	std::vector<DocumenetMarchElement> DecumentProcess(Table const& Ref, std::u32string_view String);

	namespace Error
	{
		struct UnaccaptableRexgex {
			std::u32string regex;
			size_t accepetable_state;
			size_t accepetable_mask;
			size_t Index;
		};

		struct UnaccaptableString {
			std::u32string total_string;
			Section section;
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