#pragma once
#include "range_set.h"
#include <string_view>
#include <variant>
#include <optional>
#include <set>
#include "parser_format.h"
namespace Potato::Parser::NFA
{
	struct Table
	{
		using RangeSet = Tool::range_set<char32_t>;
		enum class EdgeType : size_t
		{
			Acception,
			Comsume,
		};
		std::vector<RangeSet> ComsumeEdge;
		std::vector<std::tuple<size_t, size_t, size_t>> Edges;
		std::vector<std::tuple<size_t, size_t>> Nodes;
	};

	Table CreateTable(std::u32string_view const* input, size_t const* state, size_t input_length);
	Table CreateTable(std::u32string_view const* input, size_t input_length);
	Table CreateTableReversal(std::u32string_view const* input, size_t input_length);
	Table CreateTableReversal(std::u32string_view const* input, size_t const* state, size_t input_length);

	struct MarchElement
	{
		size_t acception;
		std::u32string_view capture;
		std::u32string_view last_string;
	};

	std::optional<MarchElement> Consume(Table const& Ref, std::u32string_view String);
	std::vector<MarchElement> Process(Table const& Ref, std::u32string_view String);

	struct Location {
		size_t start_index = 0;
		size_t line = 0;
		size_t total_index = 0;
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
		struct UnAccaptableString {
			std::u32string LastString;
		};
	}
}

namespace Potato::Parser::Format
{
	template<> struct Formatter<NFA::Table>
	{
		std::u32string operator()(std::u32string_view, NFA::Table const&);
	};
}