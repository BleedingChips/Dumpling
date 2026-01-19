module;
#include <cassert>

export module DumplingRendererTypes;

import std;
import Potato;

export namespace Dumpling
{
	using StructLayout = Potato::IR::StructLayout;

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

	template<typename Type>
	concept HLSLConstBufferType16AlignRequire = requires(Type type)
	{
		typename Type::HLSLConstBufferRequire16Align;
	};

	template<typename Type> requires(alignof(Type) <= 16)
	struct HLSLConstBufferLayoutOverride
	{
		Potato::IR::Layout GetLayout() const {
			return {16, Potato::MemLayout::AlignTo(sizeof(Type), 16)};
		}

		Potato::IR::Layout GetLayoutAsMember() const
		{
			if constexpr(HLSLConstBufferType16AlignRequire<Type>)
				return { 16, sizeof(Type) };
			return Potato::IR::Layout::Get<Type>();
		}

	};

	template<typename Type, std::size_t Columns>
		requires(sizeof(Type) == sizeof(float) && Columns > 0 && Columns <= 4)
	struct HLSLCBVector
	{
		float Data[Columns];
		/*
		static constexpr Potato::IR::Layout HLSLConstBufferLayout() {
			return { Columns * sizeof(float),  Columns * sizeof(float) };
		}
		*/
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
		using HLSLConstBufferRequire16Align = void;
		
		/*
		static constexpr Potato::IR::Layout HLSLConstBufferLayout() { 
			return { sizeof(float) * 4,  Rows * sizeof(float) + 4 * sizeof(float) * (Columns - 1)};
		}
		*/
	};

	template<typename Type, std::size_t Rows>
		requires(sizeof(Type) == sizeof(float) && Rows > 0 && Rows <= 4)
	struct HLSLCBMatrixColumnsMajor<Type, Rows, 1>
	{
		float Data[Rows];
		using HLSLConstBufferRequire16Align = void;
		/*
		static constexpr Potato::IR::Layout HLSLConstBufferLayout() {
			return { sizeof(float) * 4,  Rows * sizeof(float) };
		}
		*/
	};

	template<template<std::size_t Columns> class Wrapper>
	StructLayout::Ptr Mapping1DLayout(std::size_t Columns)
	{
		switch (Columns)
		{
		case 1:
		{
			using Type = typename Wrapper<1>::Type;
			return StructLayout::GetStatic<Type, HLSLConstBufferLayoutOverride>();
		}
		case 2:
		{
			using Type = typename Wrapper<2>::Type;
			return StructLayout::GetStatic<Type, HLSLConstBufferLayoutOverride>();
		}
		case 3:
		{
			using Type = typename Wrapper<3>::Type;
			return StructLayout::GetStatic<Type, HLSLConstBufferLayoutOverride>();
		}
		case 4:
		{
			using Type = typename Wrapper<4>::Type;
			return StructLayout::GetStatic<Type, HLSLConstBufferLayoutOverride>();
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
	StructLayout::Ptr Mapping2DLayout(std::size_t Rows, std::size_t Columns)
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
	StructLayout::Ptr MappingVectorLayout(std::size_t Columns)
	{
		return Mapping1DLayout<MappingVectorWrapper<ElementT>::template Wrapper>(Columns);
	}

	template<typename ElementT>
	StructLayout::Ptr MappingMatrixLayout(std::size_t Rows, std::size_t Columns)
	{
		return Mapping2DLayout<MappingMatrixWrapper<ElementT>::template Wrapper>(Rows, Columns);
	}

	using Float4 = HLSLCBVector<float, 4>;
	using Float3 = HLSLCBVector<float, 3>;
	using Float2 = HLSLCBVector<float, 2>;

	constexpr std::optional<Potato::MemLayout::MermberLayout> HLSLConstBufferCombineMemberFunc(Potato::MemLayout::Layout& target_layout, Potato::MemLayout::Layout member, std::size_t array_count)
	{

		constexpr std::size_t max_align = sizeof(float) * 4;

		assert(member.align <= max_align);
		if (member.align > max_align)
			return std::nullopt;

		Potato::MemLayout::MermberLayout offset;

		if (array_count == 0)
		{
			offset.array_layout = { 0, member.size };
		}
		else {
			member.align = max_align;
			auto aligned_size = Potato::MemLayout::AlignTo(member.size, max_align);
			offset.array_layout = { array_count, aligned_size };
			auto each_element_count = (member.size / max_align) + 1;
			member.size = (aligned_size * (array_count - 1)) + member.size;
		}

		if (target_layout.align < member.align)
			target_layout.align = member.align;

		if (member.align == max_align)
		{
			target_layout.size = Potato::MemLayout::AlignTo(target_layout.size, member.align);
		}
		else {
			auto edge = (target_layout.size % max_align);
			if (edge + member.size > max_align)
			{
				target_layout.size = Potato::MemLayout::AlignTo(target_layout.size, member.align);
			}
		}

		offset.offset = target_layout.size;
		target_layout.size += member.size;
		return offset;
	}

	constexpr std::optional<Potato::MemLayout::MermberLayout> HLSLConstBufferCompleteLayoutFunc(Potato::MemLayout::Layout& target_layout, Potato::MemLayout::Layout member, std::size_t array_count)
	{

		constexpr std::size_t max_align = sizeof(float) * 4;

		assert(member.align <= max_align);
		if (member.align > max_align)
			return std::nullopt;

		if (target_layout.align < member.align)
			target_layout.align = member.align;

		Potato::MemLayout::MermberLayout offset;

		if (array_count == 0)
		{
			offset.array_layout = { 0, member.size };
		}
		else {
			member.align = max_align;
			auto aligned_size = Potato::MemLayout::AlignTo(member.size, max_align);
			offset.array_layout = { array_count, aligned_size };
			auto each_element_count = (member.size / max_align) + 1;
			member.size = (aligned_size * (array_count - 1)) + member.size;
		}

		if (member.align == max_align)
		{
			target_layout.size = Potato::MemLayout::AlignTo(target_layout.size, member.align);
		}
		else {
			auto edge = (target_layout.size % max_align);
			if (edge + member.size > max_align)
			{
				target_layout.size = Potato::MemLayout::AlignTo(target_layout.size, member.align);
			}
		}

		offset.offset = target_layout.size;
		target_layout.size += member.size;
		return offset;
	}

	constexpr std::optional<Potato::MemLayout::Layout> HLSLConstBufferCompleteLayoutFunc(Potato::MemLayout::Layout layout) {
		auto max_align = std::max(layout.align, std::size_t{ 16 });
		return Potato::MemLayout::Layout{max_align, Potato::MemLayout::AlignTo(layout.size, max_align)};
	}

	Potato::MemLayout::LayoutPolicyRef GetHLSLConstBufferPolicy() { return Potato::MemLayout::LayoutPolicyRef(HLSLConstBufferCombineMemberFunc, HLSLConstBufferCompleteLayoutFunc); }
}