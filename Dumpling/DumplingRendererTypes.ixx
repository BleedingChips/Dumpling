module;
#include <cassert>

export module DumplingRendererTypes;

import std;
import Potato;

export namespace Dumpling
{

	struct Color
	{
		float R = 0.0f;
		float G = 0.0f;
		float B = 0.0f;
		float A = 1.0f;

		static Color red;
		static Color white;
		static Color blue;
		static Color black;
	};


	enum TexFormat
	{
		RGBA8
	};

	struct RenderTargetSize
	{
		std::size_t width = 1;
		std::size_t height = 1;
		std::size_t length = 1;
	};

	struct FormRenderTargetProperty
	{
		std::size_t swap_chin_count = 2;
		std::optional<float> present;
	};

	enum class RendererResourceType
	{
		FLOAT32,
		UNSIGNED_INT64,
		TEXTURE_RT,
		TEXTURE_DS,
		TEXTURE1D,
		TEXTURE1D_ARRAY,
		TEXTURE2D,
		TEXTURE2D_ARRAY,
		TEXTURE3D,
		TEXTURE_CUBE
	};

	template<typename Type, std::size_t Columns>
		requires(sizeof(Type) == sizeof(float) && Columns > 0 && Columns <= 4)
	struct HLSLCBVector
	{
		float Data[Columns];
		static constexpr Potato::IR::Layout HLSLConstBufferLayout() {
			return { Columns * sizeof(float),  Columns * sizeof(float) };
		}
		constexpr HLSLCBVector() {
			for (auto& ite : Data)
				ite = 0;
		}
		constexpr HLSLCBVector(Type value) {
			Data[0] = value;
			for (std::size_t i = 1; i < Columns; ++i)
			{
				Data[i] = 0;
			}
		}
		constexpr HLSLCBVector(Type value0, Type value1) requires(Columns >= 2) {
			Data[0] = value0;
			Data[1] = value1;
			for (std::size_t i = 2; i < Columns; ++i)
			{
				Data[i] = 0;
			}
		}
		constexpr HLSLCBVector(Type value0, Type value1, Type value2) requires(Columns >= 3) {
			Data[0] = value0;
			Data[1] = value1;
			Data[2] = value2;
			for (std::size_t i = 3; i < Columns; ++i)
			{
				Data[i] = 0;
			}
		}
		constexpr HLSLCBVector(Type value0, Type value1, Type value2, Type value3) requires(Columns >= 4) {
			Data[0] = value0;
			Data[1] = value1;
			Data[2] = value2;
			Data[3] = value3;
			for (std::size_t i = 4; i < Columns; ++i)
			{
				Data[i] = 0;
			}
		}
	};

	template<typename Type, std::size_t Rows, std::size_t Columns>
	requires(sizeof(Type) == sizeof(float) && Rows > 0 && Rows <= 4 && Columns > 0 && Columns <= 4)
	struct HLSLCBMatrixColumnsMajor
	{
		float Data[Columns - 1][Rows];
		float AppendData[Rows];
		static constexpr Potato::IR::Layout HLSLConstBufferLayout() { 
			return { sizeof(float) * 4,  Rows * sizeof(float) + 4 * sizeof(float) * (Columns - 1)};
		}
	};

	template<typename Type, std::size_t Rows>
		requires(sizeof(Type) == sizeof(float) && Rows > 0 && Rows <= 4)
	struct HLSLCBMatrixColumnsMajor<Type, Rows, 1>
	{
		float Data[Rows];
		static constexpr Potato::IR::Layout HLSLConstBufferLayout() {
			return { sizeof(float) * 4,  Rows * sizeof(float) };
		}
	};

	struct HLSLConstBufferLayout
	{
		Potato::IR::StructLayout::Ptr struct_layout;
		Potato::MemLayout::Layout memory_layout;
		explicit operator bool() const { return struct_layout; }
	};

	template<template<std::size_t Columns> class Wrapper>
	HLSLConstBufferLayout Mapping1DLayout(std::size_t Columns)
	{
		switch (Columns)
		{
		case 1:
		{
			using Type = typename Wrapper<1>::Type;
			return { Potato::IR::StructLayout::GetStatic<Type>(), GetHLSLConstBufferLayout<Type>() };
		}
		case 2:
		{
			using Type = typename Wrapper<2>::Type;
			return { Potato::IR::StructLayout::GetStatic<Type>(), GetHLSLConstBufferLayout<Type>() };
		}
		case 3:
		{
			using Type = typename Wrapper<3>::Type;
			return { Potato::IR::StructLayout::GetStatic<Type>(), GetHLSLConstBufferLayout<Type>() };
		}
		case 4:
		{
			using Type = typename Wrapper<4>::Type;
			return { Potato::IR::StructLayout::GetStatic<Type>(), GetHLSLConstBufferLayout<Type>() };
		}
		}
		assert(false);
		return {};
	}

	template<template<std::size_t, std::size_t> class SubWrapper, std::size_t Rows> struct Mapping2DTo1DWrapper
	{
		template<std::size_t Columns>
		struct Wrapper
		{
			using Type = typename SubWrapper<Rows, Columns>::Type;
		};
	};

	template<template<std::size_t Rows, std::size_t Columns> class Wrapper>
	HLSLConstBufferLayout Mapping2DLayout(std::size_t Rows, std::size_t Columns)
	{
		switch (Rows)
		{
		case 1:
			return Mapping1DLayout<Mapping2DTo1DWrapper<Wrapper, 1>::template Wrapper>(Columns);
		case 2:
			return Mapping1DLayout<Mapping2DTo1DWrapper<Wrapper, 2>::template Wrapper>(Columns);
		case 3:
			return Mapping1DLayout<Mapping2DTo1DWrapper<Wrapper, 3>::template Wrapper>(Columns);
		case 4:
			return Mapping1DLayout<Mapping2DTo1DWrapper<Wrapper, 4>::template Wrapper>(Columns);
		};
		assert(false);
		return {};
	}

	template<typename ElementT> struct MappingVectorWrapper
	{
		template<std::size_t Columns>
		struct Wrapper
		{
			using Type = HLSLCBVector<ElementT, Columns>;
		};
	};

	template<typename ElementT> struct MappingMatrixWrapper
	{
		template<std::size_t Rows, std::size_t Columns >
		struct Wrapper
		{
			using Type = HLSLCBMatrixColumnsMajor<ElementT, Rows, Columns>;
		};
	};

	template<typename ElementT>
	HLSLConstBufferLayout MappingVectorLayout(std::size_t Columns)
	{
		return Mapping1DLayout<MappingVectorWrapper<ElementT>::template Wrapper>(Columns);
	}

	template<typename ElementT>
	HLSLConstBufferLayout MappingMatrixLayout(std::size_t Rows, std::size_t Columns)
	{
		return Mapping2DLayout<MappingMatrixWrapper<ElementT>::template Wrapper>(Rows, Columns);
	}

	template<typename Type>
	concept HLSLConstBufferLayoutDefinedClass = requires(Type)
	{
		{ Type::HLSLConstBufferLayout() } -> std::same_as<Potato::IR::Layout>;
	};

	template<HLSLConstBufferLayoutDefinedClass Type>
	Potato::IR::Layout GetHLSLConstBufferLayout()
	{
		return Type::HLSLConstBufferLayout();
	}

	using Float4 = HLSLCBVector<float, 4>;
}