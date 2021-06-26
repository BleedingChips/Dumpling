#pragma once

#include "Potato/Public/HIR.h"
#include "Potato/Public/Ebnf.h"
#include "Potato/Public/Misc.h"

namespace Dumpling::Parser
{
	using namespace Potato;
	using namespace HIR;

	struct ValueTypeSymbol
	{
		enum class StorageType
		{
			HIRBuildIn,
			FLOAT2,
			FLOAT3,
			FLOAT4,
			INT2,
			INT3,
			INT4,
			UINT2,
			UINT3,
			UINT4,
			MATRIX,
			STRING,
			BOOL,
		};
		TypeTag tag;
		StorageType style;
	};

	struct TypeSymbol
	{
		TypeTag reg;
		std::vector<std::u32string_view> member;
		std::vector<std::tuple<std::u32string_view, FunctionTag>> functions;
	};

	void InserBaseTypeInfo(Form& form);

	struct MateDataSymbol
	{
		Register reg;
		SymbolTag type;
	};

	struct ValueSymbol
	{
		Register reg;
		SymbolTag type;
		std::vector<SymbolTag> mate_data;
	};

	struct PropertySymbol
	{
		std::vector<SymbolTag> values;
		std::vector<SymbolTag> mate_data;
	};

	struct ImportSymbol
	{
		std::u32string_view path_reference;
		size_t reference_used;
	};

	struct Reference
	{
		std::u32string_view path_reference;
		std::vector<std::u32string_view> sub_reference;
	};

	struct CodeSymbol
	{
		std::vector<Reference> refs;
		std::u32string_view code;
	};

	struct SnippetSymbol
	{
		std::vector<Reference> refs;
		std::u32string_view code;
		struct InoutElement
		{
			HIR::SymbolTag type;
			std::u32string_view name;
			bool is_input;
		};
		std::vector<InoutElement> inout_elements;
	};

	struct MaterialSymbol
	{
		std::vector<SymbolTag> values;
		std::vector<SymbolTag> mate_datas;
		std::vector<SymbolTag> reference_code;
		std::vector<std::tuple<std::u32string_view, SymbolTag>> snippets;
		std::vector<std::tuple<std::u32string_view, std::u32string_view>> defines;
	};

	struct MscfContent
	{
		std::vector<SymbolTag> codes;
		std::vector<SymbolTag> imports;
		std::vector<SymbolTag> propertys;
		std::vector<SymbolTag> snippets;
		std::vector<SymbolTag> materials;
	};








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
	using Potato::Section;

	struct UndefineSymbol
	{
		using ExceptionInterface = BaseDefineInterface;
		std::u32string name;
		Section section;
	};
}