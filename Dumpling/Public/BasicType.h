#pragma once
#include "Potato/Public/HIR.h"
#include "Potato/Public/Symbol.h"
namespace Dumpling
{
	struct TypeMask
	{
		enum class StorageType
		{
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
			CUSTOM,
		};
		Potato::TypeIndex index;
		StorageType style;
	};

	struct TypeSymbol
	{
		TypeMask mask;
		std::vector<std::u32string_view> member;
		std::vector<std::tuple<std::u32string_view, Potato::FunctionIndex>> functions;
	};

	struct MateDataSymbol
	{
		TypeMask mask;
		Potato::RegisterIndex reg;
	};

	struct ValueSymbol
	{
		TypeMask mask;
		Potato::RegisterIndex reg;
		std::vector<MateDataSymbol> mate_datas;
	};

	void InserBaseTypeInfo(Potato::SymbolForm& sform, Potato::HIRForm& hform);
}