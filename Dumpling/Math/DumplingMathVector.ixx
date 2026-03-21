module;

export module DumplingMathVector;

import std;
import Potato;
import DumplingMathConcept;

namespace Dumpling::Math
{
}

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

		template<std::size_t other_dimension, typename ...AT>
			requires(sizeof...(AT) == other_dimension && (std::is_convertible_v<AT&&, std::size_t> && ... && true))
		Vector<Type, other_dimension> Slice(AT&& ...at) const
		{
			return Vector<Type, other_dimension>{
				this->operator[](static_cast<std::size_t>(at))...
			};
		}

		template<typename T>
			requires(std::is_constructible_v<Type, T&&>)
		constexpr Vector(T&& t) {
			std::array<Type, Dimension>::fill(std::forward<T>(t));
		}

		auto LengthSquare() const
		{
			return [this]<std::size_t ...i>(std::index_sequence<i...>) {
				return (((*this)[i] * (*this)[i]) + ...);
			}(std::make_index_sequence<Dimension>());
		}

		auto Length() const
			requires(std::is_convertible_v<decltype(LengthSquare()), double>)
		{
			return std::sqrt(LengthSquare());
		}

		template<typename OType>
			requires(Potato::TMP::IsAdditionEnable<Type, OType>)
		constexpr auto operator+(Vector<OType, Dimension> const& other) const
			-> Vector<decltype(std::declval<Type>() + std::declval<OType>()), Dimension>
		{
			return[&]<std::size_t ...i>(std::index_sequence<i...>) {
				return Vector{
					(this->operator[](i) + other[i])...
				};
			}(std::make_index_sequence<Dimension>());
		}

		template<typename OType>
			requires(Potato::TMP::IsAdditionEnable<Type, OType>)
		constexpr auto operator+(OType other) const
			-> Vector<decltype(std::declval<Type>() + std::declval<OType>()), Dimension>
		{
			return[&]<std::size_t ...i>(std::index_sequence<i...>) {
				return Vector{
					(this->operator[](i) + other)...
				};
			}(std::make_index_sequence<Dimension>());
		}

		template<typename OType>
			requires(Potato::TMP::IsSubtractionEnable<Type, OType>)
		constexpr auto operator-(Vector<OType, Dimension> const& other) const
			-> Vector<decltype(std::declval<Type>() - std::declval<OType>()), Dimension>
		{
			return[&]<std::size_t ...i>(std::index_sequence<i...>) {
				return Vector{
					(this->operator[](i) - other[i])...
				};
			}(std::make_index_sequence<Dimension>());
		}

		template<typename OType>
			requires(Potato::TMP::IsSubtractionEnable<Type, OType>)
		constexpr auto operator-(OType other) const
			-> Vector<decltype(std::declval<Type>() - std::declval<OType>()), Dimension>
		{
			return[&]<std::size_t ...i>(std::index_sequence<i...>) {
				return Vector{
					(this->operator[](i) - other)...
				};
			}(std::make_index_sequence<Dimension>());
		}

		template<typename OType>
			requires(Potato::TMP::IsMultiplicationEnable<Type, OType>)
		constexpr auto operator*(Vector<OType, Dimension> const& other) const
			-> Vector<decltype(std::declval<Type>() * std::declval<OType>()), Dimension>
		{
			return[&]<std::size_t ...i>(std::index_sequence<i...>) {
				return Vector{
					(this->operator[](i) * other[i])...
				};
			}(std::make_index_sequence<Dimension>());
		}

		template<typename OType>
			requires(Potato::TMP::IsMultiplicationEnable<Type, OType>)
		constexpr auto operator*(OType other) const
			-> Vector<decltype(std::declval<Type>() * std::declval<OType>()), Dimension>
		{
			return[&]<std::size_t ...i>(std::index_sequence<i...>) {
				return Vector{
					(this->operator[](i) * other)...
				};
			}(std::make_index_sequence<Dimension>());
		}

		template<typename OType>
			requires(Potato::TMP::IsDivisionEnable<Type, OType>)
		constexpr auto operator/(Vector<OType, Dimension> const& other) const
			-> Vector<decltype(std::declval<Type>() / std::declval<OType>()), Dimension>
		{
			return[&]<std::size_t ...i>(std::index_sequence<i...>) {
				return Vector{
					(this->operator[](i) / other[i])...
				};
			}(std::make_index_sequence<Dimension>());
		}

		template<typename OType>
			requires(Potato::TMP::IsDivisionEnable<Type, OType>)
		constexpr auto operator/(OType other) const
			-> Vector<decltype(std::declval<Type>()/ std::declval<OType>()), Dimension>
		{
			return[&]<std::size_t ...i>(std::index_sequence<i...>) {
				return Vector{
					(this->operator[](i) / other)...
				};
			}(std::make_index_sequence<Dimension>());
		}

		template<typename OType>
			requires(Potato::TMP::IsSelfAdditionEnable<Type, OType>)
		constexpr Vector& operator+=(Vector<OType, Dimension> const& other)
		{
			for (std::size_t i = 0; i < Dimension; ++i)
			{
				(*this)[i] += other[i];
			}
			return *this;
		}

		template<typename OType>
			requires(Potato::TMP::IsSelfAdditionEnable<Type, OType>)
		constexpr Vector& operator+=(OType other)
		{
			for (std::size_t i = 0; i < Dimension; ++i)
			{
				(*this)[i] += other;
			}
			return *this;
		}

		template<typename OType>
			requires(Potato::TMP::IsSelfSubtractionEnable<Type, OType>)
		constexpr Vector& operator-=(Vector<OType, Dimension> const& other)
		{
			for (std::size_t i = 0; i < Dimension; ++i)
			{
				(*this)[i] -= other[i];
			}
			return *this;
		}

		template<typename OType>
			requires(Potato::TMP::IsSelfSubtractionEnable<Type, OType>)
		constexpr Vector& operator-=(OType other)
		{
			for (std::size_t i = 0; i < Dimension; ++i)
			{
				(*this)[i] -= other;
			}
			return *this;
		}

		template<typename OType>
			requires(Potato::TMP::IsSelfMultiplicationEnable<Type, OType>)
		constexpr Vector& operator*=(Vector<OType, Dimension> const& other)
		{
			for (std::size_t i = 0; i < Dimension; ++i)
			{
				(*this)[i] *= other[i];
			}
			return *this;
		}

		template<typename OType>
			requires(Potato::TMP::IsSelfMultiplicationEnable<Type, OType>)
		constexpr Vector& operator*=(OType other)
		{
			for (std::size_t i = 0; i < Dimension; ++i)
			{
				(*this)[i] *= other;
			}
			return *this;
		}

		template<typename OType>
			requires(Potato::TMP::IsSelfDivisionEnable<Type, OType>)
		constexpr Vector& operator/=(Vector<OType, Dimension> const& other)
		{
			for (std::size_t i = 0; i < Dimension; ++i)
			{
				(*this)[i] /= other[i];
			}
			return *this;
		}

		template<typename OType>
			requires(Potato::TMP::IsSelfDivisionEnable<Type, OType>)
		constexpr Vector& operator/=(OType other)
		{
			for (std::size_t i = 0; i < Dimension; ++i)
			{
				(*this)[i] /= other;
			}
			return *this;
		}
	};

	template<typename Type>
	struct IsVector {
		static constexpr bool Value = false;
	};

	template<typename Type, std::size_t Dimension>
	struct IsVector<Vector<Type, Dimension>> {
		static constexpr bool Value = true;
	};

	template<typename Type>
	constexpr bool IsVectorV = IsVector<Type>::Value;

	template<typename Type, typename OType, std::size_t Dimension>
		requires(!IsVectorV<Type> && Potato::TMP::IsAdditionEnable<Type, OType>)
	constexpr auto operator+(Type value, Vector<OType, Dimension> const& other)
		-> Vector<decltype(std::declval<Type>() + std::declval<OType>()), Dimension>
	{
		return Vector<decltype(std::declval<Type>() + std::declval<OType>()), Dimension>{value} + other;
	}

	template<typename Type, typename OType, std::size_t Dimension>
		requires(!IsVectorV<Type> && Potato::TMP::IsSubtractionEnable<Type, OType>)
	constexpr auto operator-(Type value, Vector<OType, Dimension> const& other)
		-> Vector<decltype(std::declval<Type>() - std::declval<OType>()), Dimension>
	{
		return Vector<decltype(std::declval<Type>() - std::declval<OType>()), Dimension>{value} - other;
	}

	template<typename Type, typename OType, std::size_t Dimension>
		requires(!IsVectorV<Type> && Potato::TMP::IsMultiplicationEnable<Type, OType>)
	constexpr auto operator*(Type value, Vector<OType, Dimension> const& other)
		-> Vector<decltype(std::declval<Type>() * std::declval<OType>()), Dimension>
	{
		return Vector<decltype(std::declval<Type>() * std::declval<OType>()), Dimension>{value} * other;
	}

	template<typename Type, typename OType, std::size_t Dimension>
		requires(!IsVectorV<Type> && Potato::TMP::IsDivisionEnable<Type, OType>)
	constexpr auto operator/(Type value, Vector<OType, Dimension> const& other)
		-> Vector<decltype(std::declval<Type>() / std::declval<OType>()), Dimension>
	{
		return Vector<decltype(std::declval<Type>() / std::declval<OType>()), Dimension>{value} / other;
	}

	template <class First, class... Rest>
	requires(std::is_same_v<std::remove_cvref_t<First>, std::remove_cvref_t<Rest>> && ... && true)
	Vector(First, Rest...) -> Vector<std::remove_cvref_t<First>, 1 + sizeof...(Rest)>;

	using Float1 = Math::Vector<float, 1>;
	using Float2 = Math::Vector<float, 2>;
	using Float3 = Math::Vector<float, 3>;
	using Float4 = Math::Vector<float, 4>;

	using Double1 = Math::Vector<double, 1>;
	using Double2 = Math::Vector<double, 2>;
	using Double3 = Math::Vector<double, 3>;
	using Double4 = Math::Vector<double, 4>;
}

namespace std
{
	template<typename Type, std::size_t Dimension, typename CharT>
	struct formatter<Dumpling::Math::Vector<Type, Dimension>, CharT>
	{
		constexpr auto parse(std::basic_format_parse_context<CharT>& context)
		{
			return std::find(context.begin(), context.end(), static_cast<CharT>('}'));
		}
		template<typename FormatContext>
		constexpr auto format(Dumpling::Math::Vector<Type, Dimension> value, FormatContext& format_context) const
		{
			auto iterator = format_context.out();
			for (std::size_t i = 0; i < Dimension; ++i)
			{
				if constexpr (std::is_same_v<CharT, char>)
				{
					if (i + 1 == Dimension)
					{
						iterator = std::format_to(
							iterator,
							"{}", value[i]
						);
					}
					else {
						iterator = std::format_to(
							iterator,
							"{}, ", value[i]
						);
					}				
				}
				else if constexpr (std::is_same_v<CharT, wchar_t>)
				{
					if (i + 1 == Dimension)
					{
						iterator = std::format_to(
							iterator,
							L"{}", value[i]
						);
					}
					else {
						iterator = std::format_to(
							iterator,
							L"{}, ", value[i]
						);
					}
				}
			}
			return iterator;
		}
	};
}