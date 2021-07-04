#include "../Public/BasicType.h"
namespace Dumpling
{
	void InserBaseTypeInfo(Potato::SymbolForm& sform, Potato::HIRForm& hform)
	{
		auto f = sform.InsertSymbol(U"float", TypeSymbol{ {Potato::TypeIndex::DefaultType<float>(), TypeIndex::StorageType::CUSTOM }, {}, {} });
		auto i = sform.InsertSymbol(U"int", TypeSymbol{ {Potato::TypeIndex::DefaultType<int32_t>(), TypeIndex::StorageType::CUSTOM }, {}, {} });
		auto ui = sform.InsertSymbol(U"uint", TypeSymbol{ {Potato::TypeIndex::DefaultType<uint32_t>(), TypeIndex::StorageType::CUSTOM }, {}, {} });
		auto bo = sform.InsertSymbol(U"bool", TypeSymbol{ {Potato::TypeIndex::DefaultType<bool>(), TypeIndex::StorageType::CUSTOM }, {}, {} });
		//auto str = sform.InsertSymbol(U"string", TypeSymbol{ {Potato::TypeIndex::DefaultType<float>(), TypeIndex::StorageType::CUSTOM }, {}, {} });
		static std::u32string_view member[] = { U"x", U"y", U"z", U"w" };

		struct BuildInDataElement
		{
			std::u32string_view name;
			TypeIndex::StorageType storage_type;
		};

		struct BuildInData
		{
			BuildInDataElement name[3];
			Potato::TypeIndex type;
			TypeIndex::StorageType type;
		};

		{
			BuildInData datas[] = {
				{{{U"float2", TypeIndex::StorageType::FLOAT2}, {U"float3", TypeIndex::StorageType::FLOAT3}, {U"float4", TypeIndex::StorageType::FLOAT4}}, Potato::TypeIndex::DefaultType<float>()},
				{{{U"int2", TypeIndex::StorageType::INT2}, {U"int3", TypeIndex::StorageType::INT3}, {U"int4", TypeIndex::StorageType::INT4}}, Potato::TypeIndex::DefaultType<int32_t>()},
				{{{U"uint2",TypeIndex::StorageType::UINT2}, {U"uint3", TypeIndex::StorageType::UINT3}, {U"uint4", TypeIndex::StorageType::UINT4}}, Potato::TypeIndex::DefaultType<uint32_t>() },
			};

			for (size_t i = 0; i < std::size(datas); ++i)
			{
				auto& ref = datas[i];
				TypeSymbol symbol;
				hform.MarkTypeDefineStart();
				for (size_t k = 0; k < std::size(ref.name); ++k)
				{
					auto name = ref.name[k];
					hform.InsertMember({ref.type, {}});
				}
				symbol.mask = {*hform.FinishTypeDefine({}), TypeIndex::StorageType::;
			}
		}
	}
}