#pragma once

#include "Potato/Public/HIR.h"
#include "Potato/Public/Ebnf.h"
#include "Potato/Public/Misc.h"

namespace Dumpling::Parser
{
	using Potato::Section;
	using namespace Potato::HIR;

	/*
	enum class BuildInType : uint8_t
	{
		Float = 0,
		Float2,
		Float3,
		Float4,
		Int,
		Int2,
		Int3,
		Int4,
		Uint,
		Uint2,
		Uint3,
		Uint4,
		Bool,
		Matrix,
		Tex1,
		Tex2,
		Tex3,
		Sampler,
		String,
	};

	struct BuildInTypeProperty
	{
		std::u32string_view name;
		bool is_sample_type;
		MemoryTag mode;
	};

	std::optional<BuildInTypeProperty> GetBuildInTypeProperty(BuildInType input);
	std::optional<BuildInType> GetBuildInType(std::u32string_view input);

	struct Table : Potato::Grammar::Table
	{
		ValueMask InsertValue(float Data){ 
			return  Potato::Grammar::Table::InsertValue({}, GetBuildInTypeProperty(BuildInType::Float)->name, {reinterpret_cast<std::byte const*>(&Data), sizeof(Data)});
		}
		ValueMask InsertValue(std::int32_t Data) {
			return  Potato::Grammar::Table::InsertValue({}, GetBuildInTypeProperty(BuildInType::Int3)->name, { reinterpret_cast<std::byte const*>(&Data), sizeof(Data) });
		}
		ValueMask InsertValue(std::int64_t Data){ return InsertValue(static_cast<std::int32_t>(Data)); }
		ValueMask InsertValue(std::u32string_view Data){
			return  Potato::Grammar::Table::InsertValue({}, GetBuildInTypeProperty(BuildInType::Float)->name, { reinterpret_cast<std::byte const*>(&Data), sizeof(Data) });
		}
		ValueMask InsertValue(bool Data) {
			uint32_t va = (Data ? 1 : 0);
			return  Potato::Grammar::Table::InsertValue({}, GetBuildInTypeProperty(BuildInType::Bool)->name, { reinterpret_cast<std::byte const*>(&va), sizeof(va) });
		}
	};
	*/

	//std::tuple<std::vector<uint32_t>, std::tuple<>>
}

namespace Dumpling::Exception::Parser
{
	struct Interface {};

	using BaseDefineInterface = Potato::Exception::DefineInterface<Interface>;
	using Section = Dumpling::Parser::Section;

	struct UndefineSymbol
	{
		using ExceptionInterface = BaseDefineInterface;
		std::u32string name;
		Section section;
	};
}