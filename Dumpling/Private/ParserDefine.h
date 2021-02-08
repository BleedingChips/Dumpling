#pragma once
#include "Potato/Public/Grammar.h"

namespace Dumpling::Parser
{
	using namespace Potato::Grammar;

	enum class BuildInType : uint8_t
	{
		FLOAT,
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

	namespace Exception
	{
		struct Interface {};
	}

	template<typename Type>
	auto MakeException(Type&& type) { return Potato::Misc::create_exception_tuple<Exception::Interface>(type); }

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
		ValueMask InsertValue(ParserSymbol const&, std::u32string_view);
		ValueMask InsertValue(ParserSymbol const&, bool);
	};
	
}