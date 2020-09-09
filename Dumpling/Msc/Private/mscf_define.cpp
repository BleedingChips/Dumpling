#include "mscf_define.h"
#include <array>
namespace Dumpling::Mscf
{

	void AddInsideType(LRTable& Table, std::u32string_view const* member_name, std::u32string_view const* name, size_t count)
	{
		auto mask = Table.InsertType(name[0], {});
		for (size_t i = 1; i < count; ++i)
		{
			std::vector<LRTable::Value> values;
			for (size_t k = 0; k <= i; ++k)
				values.push_back(LRTable::MakeValue(mask, member_name[k]));
			Table.InsertType(name[i], std::move(values));
		}
	}

	LRTable DefaultTable() {
		static LRTable instance = []() {
			LRTable res;

			static std::array<std::u32string_view, 4> member_name = {U"x", U"y", U"z", U"w"};
			static std::array<std::u32string_view, 4> float_name = { U"float", U"float2", U"float3", U"float4" };
			static std::array<std::u32string_view, 4> int_name = { U"int", U"int2", U"int3", U"int4" };
			static std::array<std::u32string_view, 4> uint_name = { U"uint", U"uint2", U"uint3", U"uint4" };
			
			AddInsideType(res, member_name.data(), float_name.data(), 4);
			AddInsideType(res, member_name.data(), int_name.data(), 4);
			AddInsideType(res, member_name.data(), uint_name.data(), 4);
			auto i_mask = res.FindType(U"int");
			auto str_mask = res.InsertType(U"str", {});
			res.InsertType(U"Sampler", { LRTable::MakeValue(str_mask, U"type"), LRTable::MakeValue(i_mask, U"lod_bisa") });
			res.InsertType(U"StructureBuffer", {});
			res.InsertType(U"RWStructureBuffer", {});
			res.InsertType(U"Texture1D", {});
			res.InsertType(U"Texture2D", {});
			res.InsertType(U"Texture3D", {});
			return res;
		}();
		return instance;
	};

	template<typename DataT> std::vector<std::byte> ToData(DataT const& data)
	{
		std::vector<std::byte> re;
		re.resize(sizeof(DataT));
		std::memcpy(re.data(), &data, sizeof(DataT));
		return re;
	}

	DataWrapper ValueBuildCommand::InsertData(std::vector<std::byte> const& data)
	{
		auto offset = datas.size();
		datas.insert(datas.end(), data.begin(), data.end());
		return {offset, data.size()};
	}

	void ValueBuildCommand::PushData(LRTable const& table, int32_t data)
	{
		auto datas = InsertData(ToData(data));
		auto ite = table.FindType(U"int");
		assert(ite);
		commands.push_back(PushDataC{ite, datas });
	}

	void ValueBuildCommand::PushData(LRTable const& table, float data)
	{
		auto datas = InsertData(ToData(data));
		auto ite = table.FindType(U"float");
		assert(ite);
		commands.push_back(PushDataC{ ite, datas });
	}

	void ValueBuildCommand::PushData(LRTable const& table, std::u32string_view data)
	{
		auto datas = InsertData(ToData(data));
		auto ite = table.FindType(U"str");
		assert(ite);
		commands.push_back(PushDataC{ ite, datas });
	}

	void ValueBuildCommand::PushDefaultValue(LRTable const& table, std::u32string_view type_name)
	{
		auto ite = table.FindType(type_name);
		assert(ite);
		commands.push_back(PushDefaultValueC{ ite });
	}

	void ValueBuildCommand::MakeDefaultValue(LRTable const& table, std::u32string_view type_name, size_t count)
	{
		auto ite = table.FindType(type_name);
		assert(ite);
		commands.push_back(MakeDefaultValueC{ ite });
	}

	void ValueBuildCommand::EqualValue(LRTable const& table, std::u32string_view type_name, size_t count)
	{
		auto ite = table.FindValue(type_name);
		assert(ite);
		commands.push_back(EqualValueC{ ite });
	}

	ValueBuildCommand DefaultCommand(LRTable const& Table)
	{
		ValueBuildCommand result;
		result.PushData(Table, int32_t(0));
		result.MakeDefaultValue(Table, U"int", 1);
		result.PushData(Table, float(0));
		result.PushData(Table, std::u32string_view{});
		result.MakeDefaultValue(Table, U"str", 1);

		static std::vector<std::u32string_view> InitStr = { U"Texture1D", U"Texture2D", U"Texture3D" };
		for (auto ite : InitStr)
		{
			result.PushData(Table, std::u32string_view{});
			result.MakeDefaultValue(Table, ite, 1);
		}
		return result;
	}
}