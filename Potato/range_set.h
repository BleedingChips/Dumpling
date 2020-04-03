#pragma once
#include <vector>
#include <assert.h>
namespace Potato::Tool
{

	template<typename Storage, typename Less = std::less<Storage>, template<typename Type> class Allocator = std::allocator>
	struct range_set
	{
		struct range
		{
			Storage left;
			Storage right;
			bool operator<(const range& input) { return Less{}(left, input.left); }
			std::optional<range> operator&(const range& input)
			{
				if (Less{}(left, input.right) && Less {}(input.left, right))
				{
					return range{
						(Less{}(left, input.left) ? input.left : left),
						(Less{}(right, input.right) ? right : input.right),
					};
				}
				else
					return std::nullopt;
			}
			std::optional<range> operator|(const range& input)
			{
				if (!Less{}(input.right, left) || !Less {}(right, input.left))
				{
					return range{
						(Less{}(left, input.left) ? left : input.left),
						(Less{}(right, input.right) ? input.right : right),
					};
				}
				else
					return std::nullopt;
			}
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

		range_set operator|(const range_set&) const;
		











		bool operator==(const range_set& i) { return m_set == i.m_set; }

		range_set& operator+=(const range&);
		range_set& operator+=(const range_set& input) {
			for (auto& ite : input.m_set)
				*this += ite;
			return *this;
		}
		range_set& operator+=(const Storage& input) { return *this += range{input, input + 1}; }
		
		range_set operator+(const range_set& input) const { range_set result = *this; result += input; return result; }
		range_set operator+(const range& input) const { range_set result = *this; result += input; return result; }
		range_set operator+(const Storage& input) const { range_set result = *this; result += input; return result; }
		
		range_set& operator-=(const range& input) const;
		range_set& operator-=(const range_set& input) const {
			for (auto& ite : input.m_set)
				*this -= ite;
			return *this;
		}
		range_set& operator-=(const Storage& input) const { *this -= range{ input, input+1 };}

		range_set operator-(const range_set& input) const { range_set result = *this; result -= input; return result; }
		range_set operator-(const range& input) const { range_set result = *this; result -= input; return result; }
		range_set operator-(const Storage& input) const { range_set result = *this; result -= input; return result; }
		
		range_set operator&(const range_set&) const;
		range_set supplementary(const range_set&) const;
		range_set supplementary(Storage Left =  std::numeric_limits<Storage>::min(), Storage Right = std::numeric_limits<Storage>::max()) const;
		
	private:
		std::vector<range, Allocator<range>> m_set;
	};

	template<typename Storage, typename Less, template<typename Type> class Allocator>
	auto range_set<Storage, Less, Allocator>::operator|(const range_set& input) const -> range_set
	{
		if (!m_set.empty() && !input.m_set.empty())
		{
			std::vector<range> result;
			range target(*m_set.begin());
			auto ite = m_set.begin();
			auto ite2 = input.m_set.begin();
			while (ite != m_set.end() && ite2 != input.m_set.end())
			{
				auto try_and = target | *ite2;
				if (try_and)
				{
					target = *try_and;
					while (++ite != m_set.end())
					{
						try_and = target | *ite;
						if (try_and)
							target = *try_and;
						else{
							result.push_back(target);
							target = *ite;
							break;
						}
					}
					++ite2;
				}
				else if (Less{}(target.left, ite2->left))
				{

				}
			}
		}
		else {
			return {};
		}
		range_set result;
		
		while (ite != m_set.end() && ite2 != input.m_set.end())
		{
			auto try_and = *ite | *ite2;

		}



		if (!m_set.empty())
		{
			size_t start_index = 0;

			auto cur_ite = m_set.begin();
		
			while (input_ite != input.m_set.end())
			{
				auto try_and = *cur_ite | *input_ite;
				if (try_and)
				{
					*cur_ite = *try_and;
					for(auto ite = cur_ite; )
				}
			}

			auto ite2 = input.m_set.begin();
			while (ite2 != input.m_set.end())
			{

			}



			while (ite != m_set.end() && ite2 != input.m_set.end())
			{
				auto try_re = *ite | *ite2;
				if (try_re)
				{
					*ite = *try_re;
					for (auto ite2 = ite + 1; ite2 != m_set.end(); ++ite2)
					{
						auto try_re2 = *ite | ite2;
						if (try_re2)
						{
							*ite = try_re2;
							*ite2 = range{ 0,0 };
						}
						else {
							break;
						}
					}
				}
			}
		}
		else {
			m_set = input.m_set;
		}






		if (!m_set.empty())
		{
			auto RightLimite = m_set.begin();
			assert(Less{}(input.left, input.right));
			for (; RightLimite != m_set.end(); ++RightLimite)
				if (!Less{}(RightLimite->right, input.left))
					break;
			auto LeftLimiteR = m_set.rbegin();
			for (; LeftLimiteR != m_set.rend(); ++LeftLimiteR)
				if (Less{}(LeftLimiteR->left, input.right) || LeftLimiteR->left == input.right)
					break;
			auto LeftLimite = LeftLimiteR.base();
			if (LeftLimite != m_set.begin())
				--LeftLimite;
			else
				LeftLimite = m_set.end();
			if (RightLimite == LeftLimite)
			{
				assert(RightLimite != m_set.end() && LeftLimite != m_set.end());
				RightLimite->left = Less{}(RightLimite->left, input.left) ? RightLimite->left : input.left;
				RightLimite->right = Less{}(RightLimite->right, input.right) ? input.right : RightLimite->right;
			}
			else if (RightLimite < LeftLimite)
			{
				assert(RightLimite != m_set.end());
				if (LeftLimite != m_set.end())
				{
					range r;
					r.left = Less{}(RightLimite->left, input.left) ? RightLimite->left : input.left;
					r.right = Less{}(RightLimite->right, input.right) ? input.right : RightLimite->right;
					auto end = m_set.erase(RightLimite, LeftLimite + 1);
					m_set.insert(end, r);
				}
				else {
					if (RightLimite->left == input.right)
						RightLimite->left = input.left;
					else
						m_set.insert(m_set.begin(), input);
				}
			}
			else {
				assert(false);
			}
		}
		else {
			m_set.push_back(input);
		}
		return *this;
	}

	template<typename Storage, typename Less, template<typename Type> class Allocator>
	auto range_set<Storage, Less, Allocator>::operator-=(const range& input)->range_set&
	{
		assert(Less{}(Left, Right));
	}

	template<typename Storage, typename Less, template<typename Type> class Allocator>
	auto range_set<Storage, Less, Allocator>::supplementary(Storage Left, Storage Right) const -> range_set
	{
		assert(Less{}(Left, Right));
		range_set new_set;
		new_set.m_set.reserve(m_set.size() + 1);
		Storage IteLeft = Left;
		for (auto& ite : m_set)
		{
			if (Less{}(IteLeft, ite.left))
			{
				if (Less{}(ite.left, Right))
					new_set.m_set.push_back(range{ IteLeft, ite.left });
				else {
					new_set.m_set.push_back(range{ IteLeft, Right });
					return new_set;
				}
			}
			IteLeft = Less{}(IteLeft, ite.right) ? ite.right : IteLeft;
		}
		if (Less{}(IteLeft, Right) )
			new_set.m_set.push_back(range{ IteLeft, Right });
		return std::move(new_set);
	}

	template<typename Storage, typename Less, template<typename Type> class Allocator>
	auto range_set<Storage, Less, Allocator>::operator+(const range_set& input) const ->range_set
	{
		range_set result = *this;
		for (auto& ite : input.m_set)
			result.add_range(ite);
		return std::move(result);
	}

	template<typename Storage, typename Less, template<typename Type> class Allocator>
	auto range_set<Storage, Less, Allocator>::operator+=(const range_set& input)->range_set&
	{
		for (auto& ite : input.m_set)
			this->add_range(ite);
		return *this;
	}

}
