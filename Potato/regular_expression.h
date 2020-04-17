#pragma once
#include "range_set.h"
#include <string_view>
#include <variant>
#include <set>
namespace Potato::Rex
{

	struct nfa
	{

		using range_set = Tool::range_set<char32_t>;

		struct epsilon { size_t state; };
		struct comsume { size_t state; range_set require; };

		static constexpr size_t no_accepted = std::numeric_limits<size_t>::max();

		struct node
		{
			size_t accept = no_accepted;
			std::vector<std::variant<epsilon, comsume>> edge;
		};

		static nfa create_from_rexs(std::u32string_view const* input, size_t length = 0);
		static nfa create_from_rex(std::u32string_view Rex, size_t);
		const node& operator[](size_t index) const { return nodes[index]; }
		operator bool() const noexcept { return !nodes.empty(); }
		size_t start_state() const noexcept { return 0; }
		std::set<size_t> search_null_state_set(size_t head) const;
		nfa(const nfa&) = default;
		nfa(nfa&&) = default;
		nfa& operator=(const nfa&) = default;
		nfa& operator=(nfa&&) = default;
		nfa() = default;
		node& operator[](size_t index) { return nodes[index]; }
		void link(nfa&& input);
		void simplify();
		size_t back_construction(node&& n);
	private:
		std::vector<node> nodes;
	};













	/*
	struct nfa
	{
		using storage_t = size_t;
		using range_set = Tool::range_set<char32_t>;
		constexpr storage_t no_accepted_state() { return std::numeric_limits<storage_t>::max(); }

		enum class EdgeType : uint8_t
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

		struct edge
		{
			EdgeType type;
			size_t index;
			storage_t shift_state;
		};

		struct node
		{
			storage_t is_acception;
			size_t edge_start;
			size_t edge_end;
		};

		
		std::vector<range_set> consume_edges;
		std::vector<edge> edges;
		std::vector<node> nodes;

		static nfa create_from_rex(std::u32string_view code, storage_t acception = 0);
	};

	
	nfa link(const nfa& i1, const nfa& i2);
	void simplify(nfa& input);
	*/
}
