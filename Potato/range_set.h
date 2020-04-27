#pragma once
#include <vector>
#include <assert.h>
#include <optional>
#include <limits>
namespace Potato::Tool
{

	enum class RangeLocation
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
	struct range_set
	{
		struct range
		{

			static bool small(const Storage& i1, const Storage& i2) { return Less{}(i1, i2); }
			static bool small_equal(const Storage& i1, const Storage& i2) { return !big(i1, i2); }
			static bool big(const Storage& i1, const Storage& i2) { return small(i2, i1); }
			static bool big_equal(const Storage& i1, const Storage& i2) { return !small(i1, i2); }
			


			Storage left;
			Storage right;
			bool operator<(const range& input) { return Less{}(left, input.left); }
			std::tuple<RangeLocation, std::optional<range>> union_set(const range& input) const
			{
				if (small(right, input.left)) // right < input.left
					return { RangeLocation::Left, std::nullopt };
				else if (big(left, input.right)) // left > input.right
					return { RangeLocation::Right, std::nullopt };
				else {
					if (small(left, input.left)) // left < input.left
					{
						if(big(right, input.right)) // right > input.right
							return { RangeLocation::Include, range{left, right} };
						else if(small(right, input.right)) // right < input.right
							return { RangeLocation::LeftIntersect, range{left, input.right} };
						else // right == input.right
							return { RangeLocation::IncludeRightEqual, range{left, right} };
					}
					else if (big(left, input.left)) // left > input.left
					{
						if (small(right, input.right)) // right < input.right;
							return { RangeLocation::BeInclude, range{input.left, input.right} };
						else if(big(right, input.right))// right > input.right
							return { RangeLocation::RightIntersect, range{input.left, right} };
						else // right == input.right
							return { RangeLocation::BeIncludeRightEqual, range{input.left, input.right} };
					}
					else { // left == input.left
						if (small(right, input.right)) // right < input.right
							return { RangeLocation::BeIncludeLeftEqual, range{left, input.right} };
						else if(big(right, input.right))// right > input.right
							return { RangeLocation::IncludeLeftEqual, range{left, right} };
						else // right == input.right
							return { RangeLocation::Equal, range{left, input.right} };
					}
				}
			}
			auto operator|(const range& include) const { return union_set(include); }

			std::tuple<RangeLocation, std::optional<range>> intersection_set(const range& input) const
			{
				if (small_equal(right, input.left)) // right <= input.left
					return { RangeLocation::Left, std::nullopt };
				else if (big_equal(left, input.right)) // left >= input.right
					return { RangeLocation::Right, std::nullopt };
				else {
					if (small(left, input.left)) // left < input.left
					{
						if (big(right, input.right)) // right > input.right
							return { RangeLocation::Include, range{input.left, input.right} };
						else if(small(right, input.right))// right < input.right
							return { RangeLocation::LeftIntersect, range{input.left, right} };
						else // =
							return { RangeLocation::IncludeRightEqual, range{input.left, input.right} };
					}
					else if(big(left, input.left)){ // left > input.left
						if (small(right, input.right)) // right <= input.right;
							return { RangeLocation::BeInclude, range{left, right} };
						else if(big(right, input.right))// right > input.right
							return { RangeLocation::RightIntersect, range{left, input.right} };
						else // =
							return { RangeLocation::BeIncludeRightEqual, range{left, right} };
					}
					else { // left  == input.left
						if (small(right, input.right)) // right < input.right
							return { RangeLocation::BeIncludeLeftEqual, range{left, right} };
						else if(big(right, input.right))// right > input.right
							return { RangeLocation::IncludeLeftEqual, range{left, input.right} };
						else // right == input.right
							return { RangeLocation::Equal, range{left, input.right} };
					}
				}
			}

			auto operator&(const range& include) const { return intersection_set(include); }
		};

		range_set(const range_set&) = default;
		range_set(std::initializer_list<range> ul) : m_set(ul) {}
		range_set(range_set&&) = default;
		range_set& operator=(const range_set&) = default;
		range_set& operator=(range_set&&) = default;
		range_set() = default;
		range_set(const range& input) : m_set({ input }) { assert(Less{}(input.left, input.right)); }
		range_set(const Storage& input) : m_set({ range{input, input + 1}}) {}
		size_t size() const noexcept { return m_set.size(); }
		bool empty() const noexcept { return m_set.empty(); }
		range operator[](size_t index) const noexcept { return m_set[index]; }

		auto begin() const noexcept { return m_set.begin(); }
		auto end() const noexcept { return m_set.end(); }

		range_set operator|(const range_set& input) const;
		range_set& operator|=(const range_set& input) { auto tem = std::move(*this); *this = (tem | input); return *this; }
		range_set operator&(const range_set&) const;
		range_set operator&=(const range_set& input) { auto tem = std::move(*this); *this = (tem & input); return *this; }
		range_set operator-(const range_set& input) const {
			auto cur = *this;
			cur -= input;
			return std::move(cur);
		};
		range_set& operator-=(const range_set& input) {
			auto input_c = input;
			auto re = intersection_cull(input_c);
			return *this;
		}
		range_set intersection_cull(range_set& input);
		std::vector<range, Allocator<range>>& storage() { return m_set; }
		bool intersection_find(const range& input) const;
	private:
		std::vector<range, Allocator<range>> m_set;
	};

	template<typename Storage, typename Less, template<typename Type> class Allocator>
	auto range_set<Storage, Less, Allocator>::intersection_find(const range& input) const-> bool
	{
		for (auto ite = m_set.begin(); ite != m_set.end();)
		{
			auto [lo, re] = *ite & input;
			switch (lo)
			{
			case RangeLocation::Left: ++ite; break;
			case RangeLocation::BeInclude:
			case RangeLocation::BeIncludeLeftEqual:
			case RangeLocation::BeIncludeRightEqual:
			case RangeLocation::LeftIntersect:
			case RangeLocation::Right:
			case RangeLocation::RightIntersect:
				return false;
			case RangeLocation::Include:
			case RangeLocation::IncludeLeftEqual:
			case RangeLocation::IncludeRightEqual:
			case RangeLocation::Equal:
				return true;
			default:
				break;
			}
		}
		return false;
	}

	template<typename Storage, typename Less, template<typename Type> class Allocator>
	auto range_set<Storage, Less, Allocator>::operator|(const range_set& input) const->range_set
	{
		range_set Temporary_result;
		auto& ref = Temporary_result.m_set;
		ref.reserve(m_set.size() + input.m_set.size());
		auto ite = m_set.begin();
		auto ite2 = input.m_set.begin();
		std::optional<range> result;
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
					case RangeLocation::Left: ref.push_back(*ite); ++ite; break;
					case RangeLocation::Right: ref.push_back(*ite2); ++ite2; break;
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
	auto range_set<Storage, Less, Allocator>::operator&(const range_set& input) const -> range_set
	{
		range_set result;
		auto& ref = result.m_set();
		auto i1 = m_set.begin();
		auto i2 = input.m_set.begin();
		while (i1 != m_set.end() && i2 != input.m_set.end())
		{
			auto [lo, re] = *i1 & *i2;
			switch (lo)
			{
			case RangeLocation::Left: 
				++i1; break;
			case RangeLocation::Right: ++i2; break;
			case RangeLocation::BeInclude: 
			case RangeLocation::LeftIntersect: 
				ref.push_back(*re); ++i1; break;
			case RangeLocation::RightIntersect:
			case RangeLocation::Include: 
				ref.push_back(*re); ++i2; break;
			case RangeLocation::Equal:
				ref.push_back(*re); ++i2; ++i1; break;
			default:
				break;
			}
		}
	}

	template<typename Storage, typename Less, template<typename Type> class Allocator>
	auto range_set<Storage, Less, Allocator>::intersection_cull(range_set& input) -> range_set
	{
		range_set result;
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
			case RangeLocation::Left: m_set.push_back(*i1); ++i1; break;
			case RangeLocation::Right: input.m_set.push_back(*i2); ++i2; break;
			case RangeLocation::BeInclude:
				input.m_set.push_back({i2->left, i1->left});
				ref.push_back(*re);
				i2->left = i1->right;
				++i1;
				break;
			case RangeLocation::BeIncludeLeftEqual:
				ref.push_back(*re);
				i2->left = i1->right;
				++i1;
				break;
			case RangeLocation::BeIncludeRightEqual:
				input.m_set.push_back({ i2->left, i1->left });
				ref.push_back(*re);
				++i2;
				++i1;
				break;
			case RangeLocation::LeftIntersect:
				m_set.push_back({ i1->left, i2->left });
				ref.push_back({ i2->left, i1->right }); 
				i2->left = i1->right;
				++i1;
				break;
			case RangeLocation::RightIntersect:
				input.m_set.push_back({i2->left, i1->left});
				ref.push_back(*re);
				i1->left = i2->right;
				++i2;
				break;
			case RangeLocation::Include:
				m_set.push_back({ i1->left, i2->left });
				ref.push_back(*re);
				i1->left = i2->right;
				++i2;
				break;
			case RangeLocation::IncludeLeftEqual:
				ref.push_back(*re);
				i1->left = i2->right;
				++i2;
				break;
			case RangeLocation::IncludeRightEqual:
				m_set.push_back({ i1->left, i2->left });
				ref.push_back(*re);
				++i1;
				++i2;
				break;
			case RangeLocation::Equal:
				ref.push_back(*i1); ++i2; ++i1; break;
			default: assert(false); break;
			}
		}
		m_set.insert(m_set.end(), i1, set1.end());
		input.m_set.insert(input.m_set.end(), i2, set2.end());
		return std::move(result);
	}

}
