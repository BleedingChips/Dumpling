#pragma once
#include "Potato/Public/Symbol.h"

namespace Dumpling::Parser
{
	using namespace Potato::Symbol;

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
	};

	namespace Exception
	{
		struct Interface {};
	}

	template<typename Type>
	auto MakeException(Type&& type) { return Potato::Misc::create_exception_tuple<Exception::Interface>(type); }

	struct ParserSymbolTable : Table
	{
		ParserSymbolTable();
		ParserSymbolTable(ParserSymbolTable&&) = default;
		ParserSymbolTable(ParserSymbolTable const&) = default;
	};
	
	struct ParserConstData : ConstDataTable
	{
		ConstDataTable::Mask Insert(ParserSymbolTable const&, float);
		ConstDataTable::Mask Insert(ParserSymbolTable const&, int32_t);
		ConstDataTable::Mask Insert(ParserSymbolTable const&, std::u32string_view);
	};
	
}