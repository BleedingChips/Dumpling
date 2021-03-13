#pragma once
#include <cassert>
#include <cstdint>
#include <type_traits>
#include "Potato/Public/TMP.h"

namespace Broccoli::Shader
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

	
	template<typename Type, Potato::ConstString str>
	struct Define
	{
		using StorageType = Type;
		static constexpr auto name = str;
	};

	namespace Implement
	{

		template<size_t N>
		constexpr bool SameNameForEach(Potato::ConstString<N> require_str)
		{
			return false;
		}

		template<size_t N, size_t N3, size_t ...N2>
		constexpr bool SameNameForEach(Potato::ConstString<N> require_str, Potato::ConstString<N3> require_str2, Potato::ConstString<N2> ...other_str)
		{
			return (require_str == require_str2) || SameNameForEach(require_str, other_str...);
		}

		constexpr bool SameName()
		{
			return false;
		}

		template<size_t N, size_t ...N1>
		constexpr bool SameName(Potato::ConstString<N> require_str, Potato::ConstString<N1> ... other_str)
		{
			return SameNameForEach(require_str, other_str...) || SameName(other_str...);
		}
		
		template<typename Type>
		struct IsAcceptableDefine { static constexpr bool value = false; };

		template<typename Type, Potato::ConstString str>
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
			static constexpr VariableAlignasAndSizeTuple SizeAlignesTuple() {return last_tuple; }
		};

		template<VariableAlignasAndSizeTuple last_tuple, typename Type, typename ...OtherType>
		struct VariableSizeAndAlignasDetect<last_tuple, Type, OtherType...>
		{
			static constexpr VariableAlignasAndSizeTuple tuple = CalculateVariableAlignasAndSizeTuple(last_tuple, { sizeof(typename Type::StorageType), alignof(typename Type::StorageType), 0 });
			using AppendType = VariableSizeAndAlignasDetect<tuple, OtherType...>;
			
			static typename Type::StorageType& GetImplement(std::byte* buffer,  Potato::ConstStringHolder<Type::name>)
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
	struct Buffer
	{
		//static_assert(Potato::Tmp::bool_and<true, Implement::IsAcceptableDefineV<Type>...>::value, "Variable only require Define<your_type, value_name> as type parameters");
		//static_assert(!Implement::SameName(Type::name...), "Variable only require unique value name");
		using help = Implement::VariableSizeAndAlignasDetect < Implement::VariableAlignasAndSizeTuple{ 0, alignof(float), 0}, Type... >;
		std::array<std::byte, help::SizeAlignesTuple().require_size> buffer;
		template<Potato::ConstString str>
		decltype(auto) Get(){ return  help::GetImplement(buffer.data(), Potato::ConstStringHolder<str>{});}
	};

	template<typename Type, size_t N>
	struct BaseVariable
	{
		static_assert(alignof(Type) >= alignof(float));
		std::enable_if_t<N >= 1, Type&> X() noexcept { return storage[0]; }
		std::enable_if_t<N >= 2, Type&> Y() noexcept { return storage[1]; }
		std::enable_if_t<N >= 3, Type&> Z() noexcept { return storage[2]; }
		std::enable_if_t<N >= 4, Type&> W() noexcept { return storage[3]; }
		std::enable_if_t<N >= 1, Type const&> X() const noexcept { return storage[0]; }
		std::enable_if_t<N >= 2, Type const&> Y() const noexcept { return storage[1]; }
		std::enable_if_t<N >= 3, Type const&> Z() const noexcept { return storage[2]; }
		std::enable_if_t<N >= 4, Type const&> W() const noexcept { return storage[3]; }
		std::enable_if_t<N >= 1, Type&> R() noexcept { return storage[0]; }
		std::enable_if_t<N >= 2, Type&> G() noexcept { return storage[1]; }
		std::enable_if_t<N >= 3, Type&> B() noexcept { return storage[2]; }
		std::enable_if_t<N >= 4, Type&> A() noexcept { return storage[3]; }
		std::enable_if_t<N >= 1, Type const&> R() const noexcept { return storage[0]; }
		std::enable_if_t<N >= 2, Type const&> G() const noexcept { return storage[1]; }
		std::enable_if_t<N >= 3, Type const&> B() const noexcept { return storage[2]; }
		std::enable_if_t<N >= 4, Type const&> A() const noexcept { return storage[3]; }
		
		constexpr BaseVariable(Type const & p)
		{
			for(auto & ite : storage)
				ite = p;
		}
		
		template<typename RequireType>
		constexpr operator BaseVariable<RequireType, N>() const noexcept
		{
			BaseVariable<RequireType, N> result;
			for(size_t i = 0; i < N; ++i)
				result.storage[i] = static_cast<RequireType>(storage[i]);
		}
		constexpr Type& operator[](size_t index) noexcept { static_assert(index < N); return storage[index]; }
		constexpr Type const& operator[](size_t index) const noexcept { static_assert(index < N); return storage[index]; }
		constexpr BaseVariable(BaseVariable const& BV) = default;
		constexpr BaseVariable& operator=(BaseVariable const& BV) = default;
		constexpr BaseVariable(std::enable_if_t<N >= 2, BaseVariable<Type, 1>> const& BV) : BaseVariable(BV.X()){}
	private:
		std::array<Type, N> storage;
	};

	using Float = BaseVariable<float, 1>;
	using Float2 = BaseVariable<float, 2>;
	using Float3 = BaseVariable<float, 3>;
	using Float4 = BaseVariable<float, 4>;

	using Int = BaseVariable<int32_t, 1>;
	using Int2 = BaseVariable<int32_t, 2>;
	using Int3 = BaseVariable<int32_t, 3>;
	using Int4 = BaseVariable<int32_t, 4>;

	using Uint = BaseVariable<uint32_t, 1>;
	using Uint2 = BaseVariable<uint32_t, 2>;
	using Uint3 = BaseVariable<uint32_t, 3>;
	using Uint4 = BaseVariable<uint32_t, 4>;

	using Materix = BaseVariable<Float4, 4>;

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
