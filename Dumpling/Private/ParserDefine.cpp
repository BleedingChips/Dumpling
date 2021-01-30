#include "ParserDefine.h"
namespace Dumpling::Parser
{
	
	ParserSymbolTable::ParserSymbolTable()
	{
		struct Tuple
		{
			BuildInType type;
			std::u32string_view name;
		};
		
		static Tuple AllList[] = {
			{BuildInType::FLOAT, U"float"},
			{BuildInType::FLOAT2, U"float2"},
			{BuildInType::FLOAT3, U"float3"},
			{BuildInType::FLOAT4, U"float4"},
			{BuildInType::Int, U"int"},
			{BuildInType::Int2, U"int2"},
			{BuildInType::Int3, U"int3"},
			{BuildInType::Int4, U"int4"},
			{BuildInType::Uint, U"uint"},
			{BuildInType::Uint2, U"uint2"},
			{BuildInType::Uint3, U"uint3"},
			{BuildInType::Uint4, U"uint4"},
			{BuildInType::Tex1, U"Texture1D"},
			{BuildInType::Tex2, U"Texture2D"},
			{BuildInType::Tex3, U"Texture3D"},
			{BuildInType::Sampler, U"Sampler"},
			{BuildInType::String, U"String"},
		};

		for(auto& ite : AllList)
			Insert(ite.name, ite.type, {});
	}
	
	ConstDataTable::Mask ParserConstData::Insert(ParserSymbolTable const& table, float Input)
	{
		auto Mask = table.FindActiveLast(U"float");
		assert(Mask);
		return ConstDataTable::Insert(Mask, Input);
	}

	ConstDataTable::Mask ParserConstData::Insert(ParserSymbolTable const& table, int32_t Input)
	{
		auto Mask = table.FindActiveLast(U"int");
		assert(Mask);
		return ConstDataTable::Insert(Mask, Input);
	}

	ConstDataTable::Mask ParserConstData::Insert(ParserSymbolTable const& table, std::u32string_view Input)
	{
		auto Mask = table.FindActiveLast(U"String");
		assert(Mask);
		return ConstDataTable::Insert(Mask, Input);
	}

	

	
}