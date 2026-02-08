module;

export module DumplingMathVector;

import std;
import Potato;
import DumplingMathConcept;

export namespace Dumpling::Math
{

	template<typename Type, std::size_t Dimension>
	requires(Dimension > 0)
	struct Vector : public std::array<Type, Dimension>
	{
		constexpr Vector() = default;
		constexpr Vector(Vector const&) = default;
		constexpr Vector(Vector &&) = default;
		constexpr Vector& operator=(Vector const&) = default;
		constexpr Vector& operator=(Vector &&) = default;
		template<typename T, typename ...OT>
		requires(1 + sizeof...(OT) <= Dimension)
		constexpr Vector(T&& t, OT&&... ot) : std::array<Type, Dimension>{ std::forward<T>(t), std::forward<OT>(ot)... } {};
		
		template<typename Type, std::size_t Dimension, typename OType>
			requires(ValueAcceptOperatorMul<Type, OType>)
		friend constexpr Vector<Type, Dimension>& operator*=(Vector<Type, Dimension>& in, OType&& type);

		template<typename Type, std::size_t Dimension, typename OType>
			requires(ValueAcceptOperatorMul<Type, OType>)
		friend constexpr Vector<Type, Dimension> operator*(Vector<Type, Dimension> const& in, OType&& type);

		template<typename Type, std::size_t Dimension, typename OType>
			requires(ValueAcceptOperatorAdd<Type, OType>)
		friend constexpr Vector<Type, Dimension>& operator+=(Vector<Type, Dimension>& in, OType&& type);

		template<typename Type, std::size_t Dimension, typename OType>
			requires(ValueAcceptOperatorAdd<Type, OType>)
		friend constexpr Vector<Type, Dimension> operator+(Vector<Type, Dimension> const& in, OType&& type);

		template<typename Type, std::size_t Dimension, typename OType>
			requires(ValueAcceptOperatorSub<Type, OType>)
		friend constexpr Vector<Type, Dimension>& operator-=(Vector<Type, Dimension>& in, OType&& type);

		template<typename Type, std::size_t Dimension, typename OType>
			requires(ValueAcceptOperatorSub<Type, OType>)
		friend constexpr Vector<Type, Dimension> operator-(Vector<Type, Dimension> const& in, OType&& type);

		template<typename Type, std::size_t Dimension, typename OType>
			requires(ValueAcceptOperatorDiv<Type, OType>)
		friend constexpr Vector<Type, Dimension>& operator/=(Vector<Type, Dimension>& in, OType&& type);

		template<typename Type, std::size_t Dimension, typename OType>
			requires(ValueAcceptOperatorDiv<Type, OType>)
		friend constexpr Vector<Type, Dimension> operator/(Vector<Type, Dimension> const& in, OType&& type);
	};

	template<typename Type, std::size_t Dimension, typename OType>
		requires(ValueAcceptOperatorMul<Type, OType>)
	constexpr Vector<Type, Dimension>& operator*=(Vector<Type, Dimension>& in, OType&& otype)
	{
		for (auto& ite : in)
			ite *= otype;
		return in;
	}

	template<typename Type, std::size_t Dimension, typename OType>
		requires(ValueAcceptOperatorMul<Type, OType>)
	constexpr Vector<Type, Dimension> operator*(Vector<Type, Dimension> const& in, OType&& otype)
	{
		Vector<Type, Dimension> new_one = in;
		return new_one *= otype;
	}

	template<typename Type, std::size_t Dimension, typename OType>
		requires(ValueAcceptOperatorAdd<Type, OType>)
	constexpr Vector<Type, Dimension>& operator+=(Vector<Type, Dimension>& in, OType&& otype)
	{
		for (auto& ite : in)
			ite += otype;
		return in;
	}

	template<typename Type, std::size_t Dimension, typename OType>
		requires(ValueAcceptOperatorAdd<Type, OType>)
	constexpr Vector<Type, Dimension> operator+(Vector<Type, Dimension> const& in, OType&& otype)
	{
		Vector<Type, Dimension> new_one = in;
		return new_one += otype;
	}

	template<typename Type, std::size_t Dimension, typename OType>
		requires(ValueAcceptOperatorSub<Type, OType>)
	constexpr Vector<Type, Dimension>& operator-=(Vector<Type, Dimension>& in, OType&& otype)
	{
		for (auto& ite : in)
			ite -= otype;
		return in;
	}

	template<typename Type, std::size_t Dimension, typename OType>
		requires(ValueAcceptOperatorSub<Type, OType>)
	constexpr Vector<Type, Dimension> operator-(Vector<Type, Dimension> const& in, OType&& otype)
	{
		Vector<Type, Dimension> new_one = in;
		return new_one -= otype;
	}

	template<typename Type, std::size_t Dimension, typename OType>
		requires(ValueAcceptOperatorDiv<Type, OType>)
	constexpr Vector<Type, Dimension>& operator/=(Vector<Type, Dimension>& in, OType&& otype)
	{
		for (auto& ite : in)
			ite /= otype;
		return in;
	}

	template<typename Type, std::size_t Dimension, typename OType>
		requires(ValueAcceptOperatorDiv<Type, OType>)
	constexpr Vector<Type, Dimension> operator/(Vector<Type, Dimension> const& in, OType&& otype)
	{
		Vector<Type, Dimension> new_one = in;
		return new_one /= otype;
	}

	template <class First, class... Rest>
	requires(Potato::TMP::IsRepeat<First, Rest...>::value)
	Vector(First, Rest...) -> Vector<First, 1 + sizeof...(Rest)>;

	using Float1 = Math::Vector<float, 1>;
	using Float2 = Math::Vector<float, 2>;
	using Float3 = Math::Vector<float, 3>;
	using Float4 = Math::Vector<float, 4>;
}