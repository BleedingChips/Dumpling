#pragma once
#include "range_set.h"
#include <string_view>
#include <variant>
#include <optional>
#include <set>
namespace Potato::Lexical
{

	struct nfa_storage
	{
		using range_set = Tool::range_set<char32_t>;
		enum class EdgeType
		{
			Acception,
			Comsume,
		};
		std::vector<range_set> ComsumeEdge;
		std::vector<std::tuple<EdgeType, size_t, size_t>> Edges;
		std::vector<std::tuple<size_t, size_t>> Nodes;
	};

	struct nfa_processer
	{
		struct travel {
			size_t acception;
			std::u32string_view capture;
		};
		nfa_processer(nfa_storage const& ref, std::u32string_view LastCode) : ref(ref), LastCode(LastCode) {}
		nfa_processer(const nfa_processer&) = default;
		nfa_processer& operator=(nfa_processer const&) = default;
		std::optional<travel> operator()();
		operator bool() const noexcept { return !LastCode.empty(); }
		nfa_storage const& ref;
		std::u32string_view LastCode;
	};

	struct nfa_lexer
	{
		struct travel : nfa_processer::travel
		{
			size_t line =0;
			size_t index = 0;
			size_t total_index = 0;
			travel(nfa_processer::travel tra, size_t line, size_t index, size_t total_index) :nfa_processer::travel(tra), line(line), index(index), total_index(total_index){}
			travel() = default;
			travel& operator=(travel const&) = default;
		};
		travel stack() { return processer_stack; }
		nfa_lexer(std::u32string_view code) : last_code(code) {}
		nfa_lexer(nfa_storage const& ref, std::u32string_view code) : last_code(code) { reset_nfa(ref); }
		void reset_nfa(nfa_storage const& ref);
		operator bool() const noexcept { return !last_code.empty() && processer && *processer; }
		std::optional<size_t> operator ()() noexcept;
		size_t current_line() const noexcept { return line; }
		size_t current_index() const noexcept { return index; }
		std::u32string_view last() const noexcept { return last_code; }
	private:
		std::optional<nfa_processer> processer;
		travel processer_stack;
		std::u32string_view last_code;
		size_t line = 0;
		size_t index = 0;
		size_t total_index = 0;
	};

	struct nfa
	{
		struct unacceptable_rex_error : std::exception
		{
			std::u32string rex;
			size_t acception_state;
			size_t index;
			unacceptable_rex_error(std::u32string rex, size_t acception_state, size_t index) : rex(rex), acception_state(acception_state), index(index) {}
			char const* what() const noexcept override; 
		};
		using range_set = nfa_storage::range_set;

		struct epsilon { size_t state; };
		struct comsume { size_t state; range_set require; };
		struct acception { size_t state; size_t acception_state;  };

		using edge_t = std::variant<epsilon, comsume, acception>;

		static constexpr size_t no_accepted = std::numeric_limits<size_t>::max();

		struct node { std::vector<edge_t> edge; };

		nfa& append_rex(std::u32string_view rex, size_t accept_state);
		static nfa create_from_rexs(std::u32string_view const* input, size_t length = 0);
		static nfa create_from_rex(std::u32string_view Rex, size_t);
		const node& operator[](size_t index) const { return nodes[index]; }
		operator bool() const noexcept { return !nodes.empty(); }
		operator nfa_storage() const { return simplify(); }
		size_t start_state() const noexcept { return 0; }
		nfa(const nfa&) = default;
		nfa(nfa&&) = default;
		nfa& operator=(const nfa&) = default;
		nfa& operator=(nfa&&) = default;
		nfa() = default;
		node& operator[](size_t index) { return nodes[index]; }
		nfa_storage simplify() const;
		size_t back_construction(node&& n);
		size_t size() const noexcept { return nodes.size(); }
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
