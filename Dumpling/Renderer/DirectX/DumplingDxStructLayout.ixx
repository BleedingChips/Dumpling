module;

#include "dxgi.h"
#include <cassert>

#undef interface
#undef max

export module DumplingDxStructLayout;

import std;
import Potato;
import DumplingMathVector;
import DumplingDxDefine;

export namespace Dumpling::Dx
{

	template<typename Type>
	struct HLSLConstBufferAcceptableType;

	template<typename Type>
	concept IsHLSLConstBufferType16AlignRequire = requires(Type type)
	{
		typename HLSLConstBufferAcceptableType<Type>::HLSLConstBufferRequire16Align;
	};

	template<typename Type>
	concept IsHLSLConstBufferAcceptableType = requires(Type type)
	{
		typename HLSLConstBufferAcceptableType<Type>;
	};

	template<IsHLSLConstBufferAcceptableType Type> requires(alignof(Type) <= 16)
		struct HLSLConstBufferLayoutOverride
	{
		Potato::IR::Layout GetLayout() const {
			return { 16, Potato::MemLayout::AlignTo(sizeof(Type), 16) };
		}

		Potato::IR::Layout GetLayoutAsMember() const
		{
			if constexpr (IsHLSLConstBufferType16AlignRequire<Type>)
				return { 16, sizeof(Type) };
			return Potato::IR::Layout::Get<Type>();
		}
	};

	template<typename Type>
	decltype(auto) GetHLSLConstBufferStructLayout()
	{
		return StructLayout::GetStatic<Type, HLSLConstBufferLayoutOverride>();
	}

	template<> struct HLSLConstBufferAcceptableType<float> {};
	template<std::size_t Dimension> requires(Dimension > 0 && Dimension < 4) 
		struct HLSLConstBufferAcceptableType<Math::Vector<float, Dimension>> {};

	template<typename Type, std::size_t Rows, std::size_t Columns>
		requires(sizeof(Type) == sizeof(float) && Rows > 0 && Rows <= 4 && Columns > 0 && Columns <= 4)
	struct HLSLCBMatrixColumnsMajor
	{
		float Data[Columns - 1][Rows];
		float AppendData[Rows];
		using HLSLConstBufferRequire16Align = void;
	};

	template<typename Type, std::size_t Rows>
		requires(sizeof(Type) == sizeof(float) && Rows > 0 && Rows <= 4)
	struct HLSLCBMatrixColumnsMajor<Type, Rows, 1>
	{
		float Data[Rows];
		using HLSLConstBufferRequire16Align = void;
	};

	template<std::size_t Rows, std::size_t Columns>
		requires(Rows > 0 && Rows <= 4 && Columns > 0 && Columns <= 4)
	struct HLSLConstBufferAcceptableType<HLSLCBMatrixColumnsMajor<float, Rows, Columns>> {
		using HLSLConstBufferRequire16Align = void;
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
			using Type = Math::Vector<ElementT, Columns>;
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

	constexpr std::optional<Potato::MemLayout::MermberLayout> HLSLConstBufferCombineMemberFunc(Potato::MemLayout::Layout& target_layout, Potato::MemLayout::Layout member, std::size_t array_count)
	{

		constexpr std::size_t max_align = sizeof(float) * 4;

		assert(member.align <= max_align);
		if (member.align > max_align)
			return std::nullopt;

		Potato::MemLayout::MermberLayout offset;

		if (array_count == 0)
		{
			offset.array_layout = { 0, member.size, member.size };
		}
		else {
			member.align = max_align;
			auto aligned_size = Potato::MemLayout::AlignTo(member.size, max_align);
			offset.array_layout = { array_count, aligned_size, member.size };
			auto each_element_count = (member.size / max_align) + 1;
			member.size = (aligned_size * (array_count - 1)) + member.size;
		}

		if (target_layout.align < member.align)
			target_layout.align = member.align;

		if (member.align == max_align)
		{
			target_layout.size = Potato::MemLayout::AlignTo(target_layout.size, max_align);
		}
		else {
			auto edge = (target_layout.size % max_align);
			if (edge + member.size > max_align)
			{
				target_layout.size = Potato::MemLayout::AlignTo(target_layout.size, max_align);
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
		return Potato::MemLayout::Layout{ max_align, Potato::MemLayout::AlignTo(layout.size, 256) };
	}

	Potato::MemLayout::LayoutPolicyRef GetHLSLConstBufferPolicy() { return Potato::MemLayout::LayoutPolicyRef(HLSLConstBufferCombineMemberFunc, HLSLConstBufferCompleteLayoutFunc); }

	DXGI_FORMAT GetDXGIFormat(Potato::IR::StructLayout const& layout);
}