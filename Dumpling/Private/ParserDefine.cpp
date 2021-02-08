#include "ParserDefine.h"
namespace Dumpling::Parser
{
	
	ParserSymbol::ParserSymbol()
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
			{BuildInType::Sampler, U"SamplerState"},
			{BuildInType::String, U"String"},
			{BuildInType::Bool, U"bool"},
		};

		for(auto& ite : AllList)
			Insert(ite.name, ite.type, {});
	}
	
	ValueMask ParserValue::InsertValue(ParserSymbol const& table, float Input)
	{
		auto Mask = table.FindActiveSymbolAtLast(U"float");
		assert(Mask);
		return Value::InsertValue(Mask, Input);
	}

	ValueMask ParserValue::InsertValue(ParserSymbol const& table, int32_t Input)
	{
		auto Mask = table.FindActiveSymbolAtLast(U"int");
		assert(Mask);
		return Value::InsertValue(Mask, Input);
	}

	ValueMask ParserValue::InsertValue(ParserSymbol const& table, std::u32string_view Input)
	{
		auto Mask = table.FindActiveSymbolAtLast(U"String");
		assert(Mask);
		return Value::InsertValue(Mask, Input);
	}

	ValueMask ParserValue::InsertValue(ParserSymbol const& table, bool Input)
	{
		auto Mask = table.FindActiveSymbolAtLast(U"bool");
		assert(Mask);
		return Value::InsertValue(Mask, Input);
	}

	

	
}