#include "mscf_define.h"
#include <array>
namespace Dumpling::Mscf
{
	using namespace PineApple::Symbol;

	void CreateInsideType(Table& table, Commands& Comm,
		std::u32string_view Name, Mask MemberType, std::u32string_view const* MemberName, size_t Count, Commands::DataType DefaultData
	)
	{
		TypeProperty TP;
		for (size_t i = 0; i < Count; ++i)
		{
			ValueProperty Vp{MemberType, {}, {}, {}};
			auto V1 = table.Insert(MemberName[i], std::move(Vp));
			TP.values.push_back(V1);
			Comm.PushData(DefaultData, {});
		}
		table.PopElementAsUnactive(Count);
		Mask Result = table.Insert(Name, std::move(TP));
		Comm.CoverToType(Result, Count, {});
		Comm.EqualData(Result, {});
	}

	std::tuple<Table, Commands> CreateContent()
	{
		Table table;
		Commands commands;

		static std::u32string_view MemberName[] = {U"x", U"y", U"z", U"w"};

		{
			static std::u32string_view DefineTypeName[] = { U"float2", U"float3", U"float4" };
			auto Base1 = table.Insert(U"float", TypeProperty{});
			for(size_t i = 0; i < std::size(DefineTypeName); ++i)
				CreateInsideType(table, commands, DefineTypeName[i], Base1, MemberName, i + 2, float(0));
		}

		{
			static std::u32string_view DefineTypeName[] = { U"int2", U"int3", U"int4" };
			auto Base1 = table.Insert(U"int", TypeProperty{});
			for (size_t i = 0; i < std::size(DefineTypeName); ++i)
				CreateInsideType(table, commands, DefineTypeName[i], Base1, MemberName, i + 2, int64_t(0));
		}

		{
			static std::u32string_view DefineTypeName[] = { U"uint2", U"uint3", U"uint4" };
			auto Base1 = table.Insert(U"uint", TypeProperty{});
			for (size_t i = 0; i < std::size(DefineTypeName); ++i)
				CreateInsideType(table, commands, DefineTypeName[i], Base1, MemberName, i + 2, int64_t(0));
		}

		{
			static std::tuple<TextureType, std::u32string_view> DefineType[] = {
				{TextureType::Tex1, U"Texture1D"}, {TextureType::Tex2, U"Texture2D"}, {TextureType::Tex3, U"Texture3D"}
			};
			for (size_t i = 0; i < std::size(DefineType); ++i)
			{
				auto Mask = table.Insert(std::get<1>(DefineType[i]), TextureProperty{std::get<0>(DefineType[i])});
				commands.PushData(std::u32string_view{}, {});
				commands.CoverToType(Mask, 1, {});
				commands.EqualData(Mask, {});
			}
		}

		{
			auto Mask = table.Insert(U"SamplerState", SamplerProperty{});
			commands.PushData(std::u32string_view{}, {});
			commands.CoverToType(Mask, 1, {});
			commands.EqualData(Mask, {});
		}

		{
			table.Insert(U"__MateData", MateDataProperty{});
			table.Insert(U"__UntypedList", UnTypedListProperty{});
		}

		return {std::move(table), std::move(commands)};
	}

	std::tuple<Table, Commands> CreateDefaultContent()
	{
		static std::tuple<Table, Commands> Content = CreateContent();
		return Content;
	}

	auto HlslStorageInfoLinker::Handle(StorageInfo cur, StorageInfo input) const ->HandleResult
	{
		auto old = cur;
		static constexpr size_t AlignSize = sizeof(float) * 4;
		cur.align = StorageInfoLinker::MaxAlign(cur, input);
		size_t rever_size = cur.size % AlignSize;
		if (input.size >= AlignSize || rever_size < input.size)
			cur.size += rever_size;
		cur.size += StorageInfoLinker::ReservedSize(cur, input);
		return { cur.align, cur.size - old.size };
	}

	/*
	std::tuple<std::byte const*, size_t> DataStorage::Read(DataMask mask) const
	{
		return {datas.data() + mask.offset, mask.length};
	}

	DataMask DataStorage::Push(std::byte const* data, size_t length)
	{
		size_t offset = datas.size();
		datas.insert(datas.end(), data, data+length);
		return { offset, length };
	}

	auto HlslStorageInfoLinker::Handle(StorageInfo cur, StorageInfo input) const ->HandleResult
	{
		auto old = cur;
		static constexpr size_t AlignSize = sizeof(float) * 4;
		cur.align = StorageInfoLinker::MaxAlign(cur, input);
		size_t rever_size = cur.size % AlignSize;
		if (input.size >= AlignSize || rever_size < input.size)
			cur.size += rever_size;
		cur.size += StorageInfoLinker::ReservedSize(cur, input);
		return {cur.align, cur.size - old.size};
	}
	*/

	/*
	std::tuple<SymbolTable, DataStorage> CreateContent()
	{
		SymbolTable table;
		DataStorage ds;
		int32_t zero_i = 0;
		auto zero_i_data = ds.Push(reinterpret_cast<std::byte const*>(&zero_i), sizeof(zero_i));
		float zero_f = 0;
		auto zero_f_data = ds.Push(reinterpret_cast<std::byte const*>(&zero_f), sizeof(zero_f));
		{
			Type index{ U"int", { {sizeof(zero_i), alignof(zero_i)}, {}}, zero_i_data };
			table.InsertElement(std::move(index));
		}
	}
	*/

	/*

	template<typename DataT> ValueMask ToData(PineApple::Symbol::ValueBuffer& buffer, DataT const& data)
	{
		return buffer.Insert(reinterpret_cast<std::byte const*>(&data), sizeof(DataT));
	}

	void AddInsideMemberType(ValueBuffer& buffer, Table& table, std::u32string type_name, std::u32string member_type)
	{
		auto mask = table.FindType(member_type);
		assert(mask);
		auto mask = Table.InsertType(name[0], {});
		for (size_t i = 1; i < count; ++i)
		{
			std::vector<LRTable::Value> values;
			for (size_t k = 0; k <= i; ++k)
				values.push_back(LRTable::MakeValue(mask, member_name[k]));
			Table.InsertType(name[i], std::move(values));
		}
	}

	SymbolTable::SymbolTable()
	{
		table.InsertType(U"float", {}, TypeProperty{alignof(float), sizeof(float), ToData(buffer, float(0.0f))});
		table.InsertType(U"uint", {}, TypeProperty{ alignof(uint32_t), sizeof(uint32_t), ToData(buffer, uint32_t(0))});
		table.InsertType(U"int", {}, TypeProperty{ alignof(int32_t), sizeof(int32_t), ToData(buffer, int32_t(0))});
		table.InsertType(U"bool", {}, TypeProperty{ alignof(bool), sizeof(bool), ToData(buffer, bool(0))});
		table.InsertType(U"string", {}, TypeProperty{ alignof(std::u32string_view), sizeof(std::u32string_view), ToData(buffer, std::u32string_view())});
		static std::array<std::u32string_view, 4> member_name = { U"x", U"y", U"z", U"w" };
		for (size_t i = 1; i < 4; ++i)
		{
			std::vector<Value> values;
			for (size_t k = 0; k <= i; ++k)
				values.push_back(Value{U});
			Table.InsertType(name[i], std::move(values));
		}





		InsertType(U"float2", {Value{     }})
	}
	*/













	/*
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
			res.InsertType(U"bool", {});
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

	void ValueBuildCommand::PushData(LRTable const& table, uint32_t data)
	{
		auto datas = InsertData(ToData(data));
		auto ite = table.FindType(U"uint");
		assert(ite);
		commands.push_back(PushDataC{ ite, datas });
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

	void ValueBuildCommand::PushData(LRTable const& table, bool data)
	{
		auto datas = InsertData(ToData(data));
		auto ite = table.FindType(U"bool");
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

	void ValueBuildCommand::MakePushValue(size_t count)
	{
		commands.push_back(MakePushValueC{count});
	}
	void ValueBuildCommand::LinkPushValue(size_t count)
	{
		commands.push_back(LinkPushValueC{ count });
	}
	*/
}