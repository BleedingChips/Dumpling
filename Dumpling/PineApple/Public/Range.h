#pragma once
#include <vector>
#include <assert.h>
#include <optional>
#include <limits>
#undef small
#undef max
namespace PineApple::Range
{

	enum class PositionalRelation
	{
		Left,
		LeftIntersect,
		BeInclude,
		BeIncludeLeftEqual,
		BeIncludeRightEqual,
		Include,
		IncludeLeftEqual,
		IncludeRightEqual,
		RightIntersect,
		Right,
		Equal,
	};

	template<typename Storage, typename Less = std::less<Storage>, template<typename Type> class Allocator = std::allocator>
	struct Set
	{
		struct Range
		{

			static bool small(const Storage& i1, const Storage& i2) { return Less{}(i1, i2); }
			static bool small_equal(const Storage& i1, const Storage& i2) { return !big(i1, i2); }
			static bool big(const Storage& i1, const Storage& i2) { return small(i2, i1); }
			static bool big_equal(const Storage& i1, const Storage& i2) { return !small(i1, i2); }
			
			Storage left;
			Storage right;
			bool operator<(const Range& input) { return Less{}(left, input.left); }
			std::tuple<PositionalRelation, std::optional<Range>> union_set(const Range& input) const
			{
				if (small(right, input.left)) // right < input.left
					return { PositionalRelation::Left, std::nullopt };
				else if (big(left, input.right)) // left > input.right
					return { PositionalRelation::Right, std::nullopt };
				else {
					if (small(left, input.left)) // left < input.left
					{
						if(big(right, input.right)) // right > input.right
							return { PositionalRelation::Include, Range{left, right} };
						else if(small(right, input.right)) // right < input.right
							return { PositionalRelation::LeftIntersect, Range{left, input.right} };
						else // right == input.right
							return { PositionalRelation::IncludeRightEqual, Range{left, right} };
					}
					else if (big(left, input.left)) // left > input.left
					{
						if (small(right, input.right)) // right < input.right;
							return { PositionalRelation::BeInclude, Range{input.left, input.right} };
						else if(big(right, input.right))// right > input.right
							return { PositionalRelation::RightIntersect, Range{input.left, right} };
						else // right == input.right
							return { PositionalRelation::BeIncludeRightEqual, Range{input.left, input.right} };
					}
					else { // left == input.left
						if (small(right, input.right)) // right < input.right
							return { PositionalRelation::BeIncludeLeftEqual, Range{left, input.right} };
						else if(big(right, input.right))// right > input.right
							return { PositionalRelation::IncludeLeftEqual, Range{left, right} };
						else // right == input.right
							return { PositionalRelation::Equal, Range{left, input.right} };
					}
				}
			}
			auto operator|(const Range& include) const { return union_set(include); }

			std::tuple<PositionalRelation, std::optional<Range>> intersection_set(const Range& input) const
			{
				if (small_equal(right, input.left)) // right <= input.left
					return { PositionalRelation::Left, std::nullopt };
				else if (big_equal(left, input.right)) // left >= input.right
					return { PositionalRelation::Right, std::nullopt };
				else {
					if (small(left, input.left)) // left < input.left
					{
						if (big(right, input.right)) // right > input.right
							return { PositionalRelation::Include, Range{input.left, input.right} };
						else if(small(right, input.right))// right < input.right
							return { PositionalRelation::LeftIntersect, Range{input.left, right} };
						else // =
							return { PositionalRelation::IncludeRightEqual, Range{input.left, input.right} };
					}
					else if(big(left, input.left)){ // left > input.left
						if (small(right, input.right)) // right <= input.right;
							return { PositionalRelation::BeInclude, Range{left, right} };
						else if(big(right, input.right))// right > input.right
							return { PositionalRelation::RightIntersect, Range{left, input.right} };
						else // =
							return { PositionalRelation::BeIncludeRightEqual, Range{left, right} };
					}
					else { // left  == input.left
						if (small(right, input.right)) // right < input.right
							return { PositionalRelation::BeIncludeLeftEqual, Range{left, right} };
						else if(big(right, input.right))// right > input.right
							return { PositionalRelation::IncludeLeftEqual, Range{left, input.right} };
						else // right == input.right
							return { PositionalRelation::Equal, Range{left, input.right} };
					}
				}
			}

			bool inside(Storage Input) const noexcept { return big_equal(Input, left) && small(Input, right); }

			auto operator&(const Range& include) const { return intersection_set(include); }
		};

		Set(const Set&) = default;
		Set(std::initializer_list<Range> ul) : m_set(ul) {}
		Set(Set&&) = default;
		Set& operator=(const Set&) = default;
		Set& operator=(Set&&) = default;
		Set() = default;
		Set(const Range& input) : m_set({ input }) { assert(Less{}(input.left, input.right)); }
		Set(const Storage& input) : m_set({ Range{input, input + 1}}) {}
		size_t size() const noexcept { return m_set.size(); }
		bool empty() const noexcept { return m_set.empty(); }
		Range operator[](size_t index) const noexcept { return m_set[index]; }

		auto begin() const noexcept { return m_set.begin(); }
		auto end() const noexcept { return m_set.end(); }

		Set operator|(const Set& input) const;
		Set& operator|=(const Set& input) { auto tem = std::move(*this); *this = (tem | input); return *this; }
		Set operator&(const Set&) const;
		Set operator&=(const Set& input) { auto tem = std::move(*this); *this = (tem & input); return *this; }
		Set operator-(const Set& input) const {
			auto cur = *this;
			cur -= input;
			return std::move(cur);
		};
		Set& operator-=(const Set& input) {
			auto input_c = input;
			auto re = intersection_cull(input_c);
			return *this;
		}
		Set intersection_cull(Set& input);
		std::vector<Range, Allocator<Range>>& storage() { return m_set; }
		std::vector<Range, Allocator<Range>> const& storage() const { return m_set; }
		bool intersection_find(const Range& input) const;
	private:
		std::vector<Range, Allocator<Range>> m_set;
	};

	template<typename Storage, typename Less, template<typename Type> class Allocator>
	auto Set<Storage, Less, Allocator>::intersection_find(const Range& input) const-> bool
	{
		for (auto ite = m_set.begin(); ite != m_set.end();)
		{
			auto [lo, re] = *ite & input;
			switch (lo)
			{
			case PositionalRelation::Left: ++ite; break;
			case PositionalRelation::BeInclude:
			case PositionalRelation::BeIncludeLeftEqual:
			case PositionalRelation::BeIncludeRightEqual:
			case PositionalRelation::LeftIntersect:
			case PositionalRelation::Right:
			case PositionalRelation::RightIntersect:
				return false;
			case PositionalRelation::Include:
			case PositionalRelation::IncludeLeftEqual:
			case PositionalRelation::IncludeRightEqual:
			case PositionalRelation::Equal:
				return true;
			default:
				break;
			}
		}
		return false;
	}

	template<typename Storage, typename Less, template<typename Type> class Allocator>
	auto Set<Storage, Less, Allocator>::operator|(const Set& input) const->Set
	{
		Set Temporary_result;
		auto& ref = Temporary_result.m_set;
		ref.reserve(m_set.size() + input.m_set.size());
		auto ite = m_set.begin();
		auto ite2 = input.m_set.begin();
		std::optional<Range> result;
		decltype(ite) search_target;
		bool LastDetect = false;
		while (ite != m_set.end() && ite2 != input.m_set.end())
		{
			if (result)
			{
				bool Change = false;
				auto [lo, r1] = *result | *ite;
				if (r1) {
					Change = true; ++ite;
					result = *r1;
				}
				auto [lo2, r2] = *result | *ite2;
				if (r2) {
					Change = true; ++ite2;
					result = *r2;
				}
				if (!Change)
				{
					ref.push_back(*result);
					result = std::nullopt;
				}
			}
			else {
				auto [lo, re] = *ite | *ite2;
				if (re)
				{
					result = *re;
					++ite; ++ite2;
				}
				else {
					switch (lo)
					{
					case PositionalRelation::Left: ref.push_back(*ite); ++ite; break;
					case PositionalRelation::Right: ref.push_back(*ite2); ++ite2; break;
					default: assert(false); break;
					}
				}
			}
		}
		if (result)
			ref.push_back(*result);
		ref.insert(ref.end(), ite, m_set.end());
		ref.insert(ref.end(), ite2, input.m_set.end());
		return Temporary_result;
	}

	template<typename Storage, typename Less, template<typename Type> class Allocator>
	auto Set<Storage, Less, Allocator>::operator&(const Set& input) const -> Set
	{
		Set result;
		auto& ref = result.m_set();
		auto i1 = m_set.begin();
		auto i2 = input.m_set.begin();
		while (i1 != m_set.end() && i2 != input.m_set.end())
		{
			auto [lo, re] = *i1 & *i2;
			switch (lo)
			{
			case PositionalRelation::Left: 
				++i1; break;
			case PositionalRelation::Right: ++i2; break;
			case PositionalRelation::BeInclude: 
			case PositionalRelation::LeftIntersect: 
				ref.push_back(*re); ++i1; break;
			case PositionalRelation::RightIntersect:
			case PositionalRelation::Include: 
				ref.push_back(*re); ++i2; break;
			case PositionalRelation::Equal:
				ref.push_back(*re); ++i2; ++i1; break;
			default:
				break;
			}
		}
	}

	template<typename Storage, typename Less, template<typename Type> class Allocator>
	auto Set<Storage, Less, Allocator>::intersection_cull(Set& input) -> Set
	{
		Set result;
		auto& ref = result.m_set;
		auto set1 = std::move(m_set);
		auto set2 = std::move(input.m_set);
		auto i1 = set1.begin();
		auto i2 = set2.begin();
		while (i1 != set1.end() && i2 != set2.end())
		{
			auto [lo, re] = *i1 & *i2;
			switch (lo)
			{
			case PositionalRelation::Left: m_set.push_back(*i1); ++i1; break;
			case PositionalRelation::Right: input.m_set.push_back(*i2); ++i2; break;
			case PositionalRelation::BeInclude:
				input.m_set.push_back({i2->left, i1->left});
				ref.push_back(*re);
				i2->left = i1->right;
				++i1;
				break;
			case PositionalRelation::BeIncludeLeftEqual:
				ref.push_back(*re);
				i2->left = i1->right;
				++i1;
				break;
			case PositionalRelation::BeIncludeRightEqual:
				input.m_set.push_back({ i2->left, i1->left });
				ref.push_back(*re);
				++i2;
				++i1;
				break;
			case PositionalRelation::LeftIntersect:
				m_set.push_back({ i1->left, i2->left });
				ref.push_back({ i2->left, i1->right }); 
				i2->left = i1->right;
				++i1;
				break;
			case PositionalRelation::RightIntersect:
				input.m_set.push_back({i2->left, i1->left});
				ref.push_back(*re);
				i1->left = i2->right;
				++i2;
				break;
			case PositionalRelation::Include:
				m_set.push_back({ i1->left, i2->left });
				ref.push_back(*re);
				i1->left = i2->right;
				++i2;
				break;
			case PositionalRelation::IncludeLeftEqual:
				ref.push_back(*re);
				i1->left = i2->right;
				++i2;
				break;
			case PositionalRelation::IncludeRightEqual:
				m_set.push_back({ i1->left, i2->left });
				ref.push_back(*re);
				++i1;
				++i2;
				break;
			case PositionalRelation::Equal:
				ref.push_back(*i1); ++i2; ++i1; break;
			default: assert(false); break;
			}
		}
		m_set.insert(m_set.end(), i1, set1.end());
		input.m_set.insert(input.m_set.end(), i2, set2.end());
		return std::move(result);
	}

}
