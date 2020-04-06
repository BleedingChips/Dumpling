#pragma once
#include <vector>
#include <assert.h>
namespace Potato::Tool
{

	enum class RangeLocation
	{
		Left,
		LeftIntersect,
		BeInclude,
		Include,
		RightIntersect,
		Right,
		Equal,
	};


	template<typename Storage, typename Less = std::less<Storage>, template<typename Type> class Allocator = std::allocator>
	struct range_set
	{
		struct range
		{
			Storage left;
			Storage right;
			bool operator<(const range& input) { return Less{}(left, input.left); }
			std::tuple<RangeLocation, std::optional<range>> union_set(const range& input)
			{
				if (Less{}(right, input.left)) // right < input.left
					return { RangeLocation::Left, std::nullopt };
				else if (Less{}(input.right, left)) // left > input.right
					return { RangeLocation::Right, std::nullopt };
				else {
					if (Less{}(left, input.left)) // left < input.left
					{
						if (!Less{}(input.right, right)) // right >= input.right
							return { RangeLocation::Include, range{left, input.right} };
						else // right < input.right
							return { RangeLocation::LeftIntersect, range{left, input.right} };
					}
					else if (left == input.left) // left  == input.left
					{
						if (Less{}(right, input.right)) // right < input.right
							return { RangeLocation::BeInclude, range{left, input.right} };
						else if(right == input.right) // right == input.right
							return { RangeLocation::Equal, range{left, input.right} };
						else // right > input.right
							return { RangeLocation::Include, range{left, right} };
					}
					else { // left > input.left
						if (!Less{}(input.right, right)) // right <= input.right;
							return { RangeLocation::BeInclude, range{input.left, right} };
						else // right > input.right
							return { RangeLocation::RightIntersect, range{input.left, input.righ} };
					}
				}
			}
			auto operator|(const range& include) { return union_set(include); }

			std::tuple<RangeLocation, std::optional<range>> intersection_set(const range& input)
			{
				if (Less{}(right, input.left)) // right < input.left
					return { RangeLocation::Left, std::nullopt };
				else if (Less{}(input.right, left)) // left > input.right
					return { RangeLocation::Right, std::nullopt };
				else {
					if (Less{}(left, input.left)) // left < input.left
					{
						if (!Less{}(input.right, right)) // right >= input.right
							return { RangeLocation::Include, range{input.left, input.right} };
						else // right < input.right
							return { RangeLocation::LeftIntersect, range{input.left, right} };
					}
					else if (left == input.left) // left  == input.left
					{
						if (Less{}(right, input.right)) // right < input.right
							return { RangeLocation::BeInclude, range{left, right} };
						else if (right == input.right) // right == input.right
							return { RangeLocation::Equal, range{left, input.right} };
						else // right > input.right
							return { RangeLocation::Include, range{left, input.right} };
					}
					else { // left > input.left
						if (!Less{}(input.right, right)) // right <= input.right;
							return { RangeLocation::BeInclude, range{left, right} };
						else // right > input.right
							return { RangeLocation::RightIntersect, range{left, input.right} };
					}
				}
			}

			auto operator*(const range& include) { return intersection_set(include); }
		};

		range_set(const range_set&) = default;
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
		range_set& operator|=(const range_set& input) { auto tem = std::move(*this); *this = tem | input; return *this; }
		range_set operator&(const range_set&) const;
		range_set operator&=(const range_set& input) { auto tem = std::move(*this); *this = tem & input; return *this; }
		range_set supplementary(const range& re = range{ std::numeric_limits<Storage>::min(), std::numeric_limits<Storage>::max() }) const;
		range_set operator-(const range_set& input) const;
		range_set& operator-=(const range_set& input) { auto tem = std::move(*this); *this = tem - input; return *this; }
		
	private:
		std::vector<range, Allocator<range>> m_set;
	};

	template<typename Storage, typename Less, template<typename Type> class Allocator>
	auto range_set<Storage, Less, Allocator>::operator|(const range_set& input) const->range_set
	{
		range_set result;
		auto& ref = result.m_set;
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
		ref.insert(ref.end(), ite, m_set.end());
		ref.insert(ref.end(), ite2, input.m_set.end());
		return result;
	}

	template<typename Storage, typename Less, template<typename Type> class Allocator>
	auto range_set<Storage, Less, Allocator>::operator&(const range_set& input) const -> range_set
	{
		range_set result;
		auto& ref = result.m_set();
		ref.reserve(std::max(m_set.size(), input.m_set.size()));
		auto i1 = m_set.begin();
		auto i2 = input.m_set.begin();
		while (i1 != m_set.end() && i2 != input.m_set.end())
		{
			auto [lo, re] = *i1 & *i2;
			switch (lo)
			{
			case RangeLocation::Left: 
				++i1; break;
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
	auto range_set<Storage, Less, Allocator>::operator-(const range_set& input) const -> range_set
	{
		range_set result;
		auto& ref = result.m_set();
		ref.reserve(std::max(m_set.size(), input.m_set.size()) + 1);
		auto i2 = input.m_set.begin();
		for (auto i1 : m_set)
		{
			std::optional<range> tem = i1;
			if (i2 != input.m_set.end())
			{
				do
				{
					auto [lo, re] = *tem & *i2;
					switch (lo)
					{
					case RangeLocation::Left:
						ref.push_back(*tem);
						++i1;
						tem = std::nullopt;
						break;
					case RangeLocation::LeftIntersect:
						ref.push_back(range{ tem->left, i2.left });
					case RangeLocation::Equal:
					case RangeLocation::BeInclude:
						++i1;
						tem = std::nullopt;
						break;
					case RangeLocation::Include:
						ref.push_back(tem->left, i2.left);
						tem = range{i2.right, tem->left};
						++i2;
						break;
					case RangeLocation::RightIntersect:
						tem = range{ i2.right, tem->left };
						++i2;
						break;
					default: assert(false); break;
					}
				} while (i2 != input.m_set.end() && tem);
			}
			if (tem)
				ref.push_back(*tem);
		}
		return result;
	}

	template<typename Storage, typename Less, template<typename Type> class Allocator>
	auto range_set<Storage, Less, Allocator>::supplementary(const range& input) const -> range_set
	{
		assert(Less{}(input.Left, input.Right));
		range_set result;
		auto& ref = result.m_set;
		ref.reserve(m_set.size() + 1);
		range last_state = input;
		for (auto& ite : m_set)
		{
			auto [lo, re] = last_state & ite;
			if (re)
			{
				switch (lo)
				{
				case RangeLocation::LeftIntersect:
					ref.push_back(range{ last_state.left, re->left });
					return result;
				case RangeLocation::Include:
					ref.push_back(range{ last_state.left, re->left });
					last_state = range{ re->right, last_state.right };
					break;
				case RangeLocation::RightIntersect:
					last_state = range{ re->right, last_state.right };
					break;
				case RangeLocation::BeInclude:
				case RangeLocation::Equal:
					assert(ref.empty());
					return result;
				default: assert(false);
				}
			}
		}
		ref.push_back(last_state);
		return std::move(result);
	}

}
