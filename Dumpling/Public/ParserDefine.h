#pragma once

#include "Potato/Public/Grammar.h"
#include "Potato/Public/Misc.h"

namespace Dumpling::Parser
{
	using namespace Potato::Grammar;

	enum class BuildInType : uint8_t
	{
		FLOAT = 0,
		FLOAT2,
		FLOAT3,
		FLOAT4,
		Int,
		Int2,
		Int3,
		Int4,
		Uint,
		Uint2,
		Uint3,
		Uint4,
		Tex1,
		Tex2,
		Tex3,
		Sampler,
		String,
		Bool,
	};

	std::u32string_view Translate(BuildInType Input);
	BuildInType Translate(std::u32string_view Input);

	struct ParserSymbol : Symbol
	{
		ParserSymbol();
		ParserSymbol(ParserSymbol&&) = default;
		ParserSymbol(ParserSymbol const&) = default;
	};
	
	struct ParserValue : Value
	{
		ValueMask InsertValue(ParserSymbol const&, float);
		ValueMask InsertValue(ParserSymbol const&, int32_t);
		ValueMask InsertValue(ParserSymbol const& sym, int64_t val){ return InsertValue(sym, static_cast<int32_t>(val)); }
		ValueMask InsertValue(ParserSymbol const&, std::u32string_view);
		ValueMask InsertValue(ParserSymbol const&, bool);
	};
	
}

namespace Dumpling::Exception::Parser
{
	struct Interface {};

	using BaseDefineInterface = Potato::Exception::DefineInterface<Interface>;
}