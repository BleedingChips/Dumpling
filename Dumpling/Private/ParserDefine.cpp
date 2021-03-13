#include <array>
#include <tuple>
#include "../Public/ParserDefine.h"
namespace Dumpling::Parser
{
	
	decltype(auto) BuildInTypeMapping() {
		static constexpr std::array Instance{
			std::u32string_view(U"float"),
			U"float2",
			U"float3",
			U"float4",
			U"int",
			U"int2",
			U"int3",
			U"int4",
			U"uint",
			U"uint2",
			U"uint3",
			U"uint4",
			U"Texture1D",
			U"Texture2D",
			U"Texture3D",
			U"SamplerState",
			U"String",
			U"bool",
		};
		return Instance;
	}


	ParserSymbol::ParserSymbol()
	{
		struct Tuple
		{
			BuildInType type;
			std::u32string_view name;
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