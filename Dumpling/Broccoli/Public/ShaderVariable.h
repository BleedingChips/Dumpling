#pragma once
#include <cassert>
#include <cstdint>
#include <type_traits>
#include "../../../../Potato/Potato/Public/tmp.h"

namespace Broccoli::ShaderVariable
{	
	/*
	template<typename Type>
	struct Alignas
	{
		constexpr size_t value = (
			sizeof(Type) <= 8 ? 8 : (
				sizeof(Type) >= 16 ? 16 : 32
				));

		Alignas()
		{ 
			static_assert(alignof(Type) > value, "alignas no receive");
		}
	};
	*/

	constexpr std::size_t MaxSpace = sizeof(float) * 4;

	
	template<typename Type, Potato::Tmp::const_string str>
	struct Define
	{
		using StorageType = Type;
		static constexpr auto name = str;
	};

	namespace Implement
	{

		template<size_t N>
		constexpr bool SameNameForEach(Potato::Tmp::const_string<N> require_str)
		{
			return false;
		}

		template<size_t N, size_t N3, size_t ...N2>
		constexpr bool SameNameForEach(Potato::Tmp::const_string<N> require_str, Potato::Tmp::const_string<N3> require_str2, Potato::Tmp::const_string<N2> ...other_str)
		{
			return (require_str == require_str2) || SameNameForEach(require_str, other_str...);
		}

		constexpr bool SameName()
		{
			return false;
		}

		template<size_t N, size_t ...N1>
		constexpr bool SameName(Potato::Tmp::const_string<N> require_str, Potato::Tmp::const_string<N1> ... other_str)
		{
			return SameNameForEach(require_str, other_str...) || SameName(other_str...);
		}
		
		template<typename Type>
		struct IsAcceptableDefine { static constexpr bool value = false; };

		template<typename Type, Potato::Tmp::const_string str>
		struct IsAcceptableDefine<Define<Type, str>> { static constexpr bool value = true; };

		template<typename Type>
		static constexpr auto IsAcceptableDefineV = IsAcceptableDefine<Type>::value;

		struct VariableAlignasAndSizeTuple
		{
			std::size_t require_size;
			std::size_t require_alignas;
			std::size_t require_last_size;
			constexpr VariableAlignasAndSizeTuple(std::size_t size, std::size_t alig, std::size_t last_size) : require_size(size), require_alignas(alig), require_last_size(last_size) {}
		};

		constexpr VariableAlignasAndSizeTuple CalculateVariableAlignasAndSizeTuple(VariableAlignasAndSizeTuple const& last, VariableAlignasAndSizeTuple const& append)
		{
			const size_t append_size = std::max(append.require_size, sizeof(float));
			const size_t append_alignas = std::max(append.require_alignas, alignof(float));
			const size_t surplus = (last.require_size % MaxSpace);
			const size_t needed = (surplus == 0 ? 0 : MaxSpace - surplus);
			return {
				needed >= append_size ? last.require_size + append_size : last.require_size + needed + append.require_size,
				std::max(last.require_alignas, append_alignas),
				needed >= append_size ? last.require_size : last.require_size + needed
			};
		}
		
		template<VariableAlignasAndSizeTuple last_tuple, typename ...Type>
		struct VariableSizeAndAlignasDetect
		{
			static constexpr VariableAlignasAndSizeTuple SizeAlignesTuple() { return {
				((last_tuple.require_size % last_tuple.require_alignas) == 0) ?
					last_tuple.require_size : last_tuple.require_size + last_tuple.require_alignas - (last_tuple.require_size % last_tuple.require_alignas),
				last_tuple.require_alignas,
				last_tuple.require_last_size
			}; }
		};

		template<VariableAlignasAndSizeTuple last_tuple, typename Type, typename ...OtherType>
		struct VariableSizeAndAlignasDetect<last_tuple, Type, OtherType...>
		{
			static constexpr VariableAlignasAndSizeTuple tuple = CalculateVariableAlignasAndSizeTuple(last_tuple, { sizeof(typename Type::StorageType), alignof(typename Type::StorageType), 0 });
			using AppendType = VariableSizeAndAlignasDetect<tuple, OtherType...>;
			
			static typename Type::StorageType& GetImplement(std::byte* buffer,  Potato::Tmp::const_string_holder<Type::name>)
			{
				return *reinterpret_cast<typename Type::StorageType*>(buffer + tuple.require_last_size);
			}

			template<typename OType>
			static decltype(auto) GetImplement(std::byte* buffer, OType)
			{
				return AppendType::GetImplement(buffer, OType{});
			}

			static constexpr VariableAlignasAndSizeTuple SizeAlignesTuple() { return AppendType::SizeAlignesTuple(); }
		};
		
	}

	template<typename ...Type>
	struct Variable
	{
		static_assert(Potato::Tmp::bool_and<true, Implement::IsAcceptableDefineV<Type>...>::value, "Variable only require Define<your_type, value_name> as type parameters");
		static_assert(!Implement::SameName(Type::name...), "Variable only require unique value name");
		using help = Implement::VariableSizeAndAlignasDetect < Implement::VariableAlignasAndSizeTuple{ 0, alignof(float), 0}, Type... >;
		std::array<std::byte, help::SizeAlignesTuple().require_size> buffer;
		template<Potato::Tmp::const_string str>
		decltype(auto) Get(){ return  help::GetImplement(buffer.data(), Potato::Tmp::const_string_holder<str>{});}
	};

	/*
	template<typename Type, size_t N, typename = std::enable_if_t<(N>0)>>
	struct Variable
	{
		Variable() = default;
		Variable(Variable const& var) = default;
		Variable(Variable&& var) = default;
		Variable& operator=(Variable const&) = default;
		Variable& operator=(Variable &&) = default;
		template<typename ThisType>
		Variable(ThisType&& ThisType) : current_level_storage(std::forward<ThisType>()), next_level_storage(std::forward<ThisType>()) {}
		template<typename ThisType, typename ...OtherType>
		Variable(ThisType&& ThisTypePar, OtherType&& OtherType)
			: current_level_storage(std::forward<ThisType>(ThisTypePar)), next_level_storage(std::forward<OtherType>(OtherType)...){}
		template<typename ThisType, size_t Num, typename ...OtherType>
		Variable(std::enable_if_t<(Num>1), Variable<ThisType, Num>>&& ThisTypePar, OtherType&& OtherType)
			: current_level_storage(std::move(ThisTypePar.current_level_storage)), next_level_storage(std::move(ThisTypePar.next_level_storage), std::forward<OtherType>(OtherType)...) {}
		template<typename ThisType, size_t Num, typename ...OtherType>
		Variable(std::enable_if_t<(Num > 1), Variable<ThisType, Num>> const& ThisTypePar, OtherType&& OtherType)
			: current_level_storage(ThisTypePar.current_level_storage), next_level_storage(ThisTypePar.next_level_storage, std::forward<OtherType>(OtherType)...) {}
		Type& operator[](size_t index){ if(index == 0) return current_level_storage; else return next_level_storage(index - 1);}
	private:
		Type alignas(AlignasV<Type>) current_level_storage;
		Variable<Type, N - 1> next_level_storage;
	};

	template<typename Type>
	struct Variable<Type, 1>
	{
		Variable() = default;
		Variable(Variable const& var) = default;
		Variable(Variable && var) = default;
		Variable& operator=(Variable const&) = default;
		Variable& operator=(Variable&&) = default;
		template<typename ThisType>
		Variable(ThisType&& ThisTypePar) : current_level_storage(std::forward<ThisType>(ThisTypePar)){}
		template<typename ThisType, typename ...OtherType>
		Variable(Variable<ThisType, 1>&& ThisTypePar) : current_level_storage(std::move(ThisTypePar.current_level_storage)) {}
		template<typename ThisType, typename ...OtherType>
		Variable(Variable<ThisType, 1> const& ThisTypePar) : current_level_storage(ThisTypePar.current_level_storage) {}
		Type& operator[](size_t index) { assert(index == 0); return current_level_storage; }
	private:
		Type alignas(AlignasV<Type>) current_level_storage;
	};

	using Float = Variable<float, 1>;
	using Float2 = Variable<float, 2>;
	using Float3 = Variable<float, 3>;
	using Float4 = Variable<float, 4>;
	using Int = Variable<int32_t, 1>;
	using Int2 = Variable<int32_t, 2>;
	using Int3 = Variable<int32_t, 3>;
	using Int4 = Variable<int32_t, 4>;

	using Matrix = Variable<Float4, 4>;
	*/

	
}
