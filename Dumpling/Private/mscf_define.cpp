#include "mscf_define.h"
#include "../Public/mscf.h"
#include <array>
#include "mscf_parser_table.h"

namespace Dumpling::Mscf
{
	using namespace PineApple::Symbol;

	Mask Commands::PushData(Table& table, bool value, Section section)
	{
		auto Mask = table.FindActiveLast(U"bool");
		assert(Mask);
		PushData(Mask, table, reinterpret_cast<std::byte const*>(&value), sizeof(decltype(value)), section);
		return Mask;
	}

	Mask Commands::PushData(Table& table, std::u32string_view value, Section section)
	{
		auto Mask = table.FindActiveLast(U"string");
		PushData(Mask, table, reinterpret_cast<std::byte const*>(&value), sizeof(decltype(value)), section);
		return Mask;
	}

	Mask Commands::PushData(Table& table, float value, Section section)
	{
		auto Mask = table.FindActiveLast(U"float");
		PushData(Mask, table, reinterpret_cast<std::byte const*>(&value), sizeof(decltype(value)), section);
		return Mask;
	}

	Mask Commands::PushData(Table& table, int32_t value, Section section)
	{
		auto Mask = table.FindActiveLast(U"int");
		PushData(Mask, table, reinterpret_cast<std::byte const*>(&value), sizeof(decltype(value)), section);
		return Mask;
	}

	/*
	void Commands::PushData(Mask mask, Table& table, std::byte const* data, size_t data_length, Section section)
	{
		assert(mask);
		auto result = table.Find<TypeProperty>(mask);
		assert(result);
		auto old_size = ConstDataTable.size();
		ConstDataTable.resize(ConstDataTable.size() + data_length, static_cast<std::byte>(0x00));
		std::memcpy(ConstDataTable.data() + old_size, data, data_length);
		AllCommands.push_back({ ValueScriptionC{mask, {old_size, data_length}}, section});
	}
	*/

	void CreateInsideType(Table& table, Commands& Comm,
		std::u32string_view Name, Mask MemberType, std::u32string_view const* MemberName, size_t Count
	)
	{
		TypeProperty TP;
		for (size_t i = 0; i < Count; ++i)
		{
			ValueProperty Vp{MemberType, {}, {}, {}};
			auto V1 = table.Insert(MemberName[i], std::move(Vp));
			TP.values.push_back(V1);
			//Comm.PushDefaultData(MemberType, {});
		}
		table.PopElementAsUnactive(Count);
		Mask Result = table.Insert(Name, std::move(TP));
		Comm.EqualValue(Result, Count, {});
	}

	std::tuple<Table, Commands> CreateContent()
	{
		Table table;
		Commands commands;

		static std::u32string_view MemberName[] = {U"x", U"y", U"z", U"w"};

		auto FloatT = table.Insert(U"float", TypeProperty{});
		commands.PushData(table, 0.0f, {});
		commands.EqualValue(FloatT, 1, {});

		auto IntT = table.Insert(U"int", TypeProperty{});
		commands.PushData(table, static_cast<int32_t>(0), {});
		commands.EqualValue(IntT, 1, {});

		auto BoolT = table.Insert(U"bool", TypeProperty{});
		commands.PushData(table, true, {});
		commands.EqualValue(BoolT, 1, {});
		
		auto StrT = table.Insert(U"str", TypeProperty{});
		commands.PushData(table, std::u32string_view{}, {});
		commands.EqualValue(BoolT, 1, {});
		
		{
			static std::u32string_view DefineTypeName[] = { U"float2", U"float3", U"float4" };
			for(size_t i = 0; i < std::size(DefineTypeName); ++i)
				CreateInsideType(table, commands, DefineTypeName[i], FloatT, MemberName, i + 2);
		}

		{
			static std::u32string_view DefineTypeName[] = { U"int2", U"int3", U"int4" };
			for (size_t i = 0; i < std::size(DefineTypeName); ++i)
				CreateInsideType(table, commands, DefineTypeName[i], IntT, MemberName, i + 2);
		}

		{
			static std::u32string_view MaterialMemberName[] = { U"v1", U"v2", U"v3", U"v4" };
			auto F4Type = table.FindActiveLast(U"float4");
			assert(F4Type);
			CreateInsideType(table, commands, U"matrix", F4Type, MaterialMemberName, 4);
		}

		{
			static std::tuple<TextureType, std::u32string_view> DefineType[] = {
				{TextureType::Tex1, U"Texture1D"}, {TextureType::Tex2, U"Texture2D"}, {TextureType::Tex3, U"Texture3D"}
			};
			
			for (size_t i = 0; i < std::size(DefineType); ++i)
				auto Mask = table.Insert(std::get<1>(DefineType[i]), TextureProperty{std::get<0>(DefineType[i])});
		}

		{
			auto Mask = table.Insert(U"SamplerState", SamplerProperty{});
		}

		return {std::move(table), std::move(commands)};
	}

	std::tuple<Table, Commands> CreateDefaultContent()
	{
		static std::tuple<Table, Commands> Content = CreateContent();
		return Content;
	}

	auto HlslStorageInfoLinker::Handle(MemoryModel cur, MemoryModel input) const ->HandleResult
	{
		auto old = cur;
		static constexpr size_t AlignSize = sizeof(float) * 4;
		cur.align = MemoryModelMaker::MaxAlign(cur, input);
		size_t rever_size = cur.size % AlignSize;
		if (input.size >= AlignSize || rever_size < input.size)
			cur.size += rever_size;
		cur.size += MemoryModelMaker::ReservedSize(cur, input);
		return { cur.align, cur.size - old.size };
	}

	Content Parser(std::u32string_view code, Table& table, Commands& commands)
	{
		/*
		auto& Mref = MscfEbnfInstance();
		auto History = Ebnf::Process(MscfEbnfInstance(), code);
		auto P = History.Expand();

		using String = std::u32string;
		using StringView = std::u32string_view;

		auto Result = Ebnf::Process(History, [&](Ebnf::Element& E) -> std::any {
			if (E.IsTerminal())
			{
				switch (E.shift.mask)
				{
				case 0: {
					return StringToFloat(E.shift.capture);
				}
				case 1: return StringToInt(E.shift.capture);
				case 2: return E.shift.capture.substr(1, E.shift.capture.size() - 2);
				case 3: return E.shift.capture.substr(2, E.shift.capture.size() - 4);
				default: return E.shift.capture;
				}
			}
			else if (E.IsNoterminal())
			{
				switch (E.reduce.mask)
				{
				case 4: {
					auto& Data = E[0];
					if (Data.TryGetData<int64_t>())
						return commands.PushData(table, static_cast<int32_t>(Data.GetData<int64_t>()), Data.section);
					else if (Data.TryGetData<float>())
						return commands.PushData(table, Data.GetData<float>(), Data.section);
					else if (Data.TryGetData<StringView>())
						return commands.PushData(table, Data.GetData<StringView>(), Data.section);
					else
						assert(false);
					return {};
				} break;
				case 5: {
					return commands.PushData(table, true, E.section);
				} break;
				case 6: {
					return commands.PushData(table, false, E.section);
				} break;
				case 7: {
					auto P = E[0].GetData<StringView>();
					auto TypeMask = table.FindActiveLast(P);
					if (!TypeMask)
						throw Error::UndefineType{ String(P), E[0].section };
					size_t index = 0;
					for (auto& Ite : E)
					{
						if (Ite.IsNoterminal())
							++index;
					}
					commands.MakeValue(TypeMask, index, E.section);
					return TypeMask;
				} break;
				case 22: {
					auto P = table.FindActiveLast(U"__MateData");
					assert(P);
					std::vector<Mask> AllData;
					for (auto& Ite : E)
					{
						if (Ite.IsNoterminal())
							AllData.push_back(Ite.GetData<Mask>());
					}
					if(AllData.size() == 0)
						return commands.MakeList({}, 0, E.section);
					else
						return commands.MakeList(AllData[0], AllData.size(), E.section);
				} break;
				case 8: {
					std::vector<int64_t> Result;
					Result.push_back(0);
					return std::move(Result);
				} break;
				case 9: {
					std::vector<int64_t> Last = E[0].MoveData<std::vector<int64_t>>();
					Last.push_back(E[2].GetData<int64_t>());
					return std::move(Last);
				} break;
				case 10: {
					return std::vector<int64_t>{};
				} break;
				case 20:
				{
					auto TypeName = E[0].GetData<StringView>();
					auto TypeMask = table.FindActiveLast(TypeName);
					if (!TypeMask)
						throw Error::UndefineType{ String(TypeName), E[0].section };
					Mask SampleMask;
					if (E.reduce.production_count == 3)
					{
						auto ReaderName = E[2].GetData<StringView>();
						SampleMask = table.FindActiveLast(ReaderName);
						if (!SampleMask)
							throw Error::UndefineType{ String(ReaderName), E[2].section };
					}
					return std::tuple<Mask, Mask>{TypeMask, SampleMask};
				} break;
				case 11: {
					auto [TypeMask, ReaderMask] = E[0].GetData<std::tuple<Mask, Mask>>();
					auto ValName = E[1].GetData<StringView>();
					auto ArrayCount = E[2].MoveData<std::vector<int64_t>>();
					if (ReaderMask)
					{
						auto Find = table.Find<TextureProperty>(TypeMask);
						assert(Find.Exist());
						if (!Find)
							throw Error::RequireTypeDonotSupportSample{ String(Find.name), E[0].section };
					}
					ValueProperty Pro{ TypeMask,ReaderMask, std::move(ArrayCount), {} };
					auto ValueMask = table.Insert(ValName, std::move(Pro), E.section);
					if (E.reduce.production_count == 5)
						commands.EqualData(ValueMask, E.section);
					return ValueMask;
				}break;
				case 12: {
					auto TypeName = E[1].GetData<StringView>();
					std::vector<Mask> AllProperty;
					for (size_t i = 3; i < E.reduce.production_count; ++i)
					{
						if (E[i].IsNoterminal())
							AllProperty.push_back(E[i].GetData<Mask>());
					}
					table.PopElementAsUnactive(AllProperty.size());
					auto TypeMask = table.Insert(TypeName, { std::move(AllProperty) }, E.section);
					return TypeMask;
				} break;
				case 13:
				{
					auto MateType = table.FindActiveLast(U"__MateData");
					assert(MateType);
					auto Name = E[0].GetData<StringView>();
					auto MateMask = table.Insert(Name, ValueProperty{ MateType, {}, {}, {} }, E.section);
					if (E.reduce.production_count == 3)
						commands.EqualData(MateMask, E.section);
					return MateMask;
				}break;
				case 14:
				{
					std::vector<Mask> AllMateData;
					for (size_t i = 0; i < E.reduce.production_count; ++i)
					{
						if (!E[i].IsTerminal())
							AllMateData.push_back(E[i].GetData<Mask>());
					}
					table.PopElementAsUnactive(AllMateData.size());
					return std::move(AllMateData);
				}break;
				case 15:
				{
					std::vector<Mask> DefinedValue;
					std::vector<Mask> AppendMate = E[0].MoveData<std::vector<Mask>>();
					for (size_t i = 1; i < E.reduce.production_count; i += 1)
					{
						if (E[i].IsNoterminal())
						{
							std::vector<Mask> Value = E[i].MoveData<std::vector<Mask>>();
							for (auto Item : Value)
							{
								auto Fined = table.Find<ValueProperty>(Item);
								assert(Fined);
								auto& ref = Fined->mate_data;
								ref.insert(ref.begin(), AppendMate.rbegin(), AppendMate.rend());
							}
							DefinedValue.insert(DefinedValue.end(), Value.begin(), Value.end());
						}
					}
					return std::move(DefinedValue);
				} break;
				case 16:
				{
					std::vector<Mask> SelfMateData = E[0].MoveData<std::vector<Mask>>();
					Mask ValueMask = E[1].MoveData<Mask>();
					auto Result = table.Find<ValueProperty>(ValueMask);
					assert(Result);
					auto& ref = Result->mate_data;
					ref.insert(ref.end(), SelfMateData.rbegin(), SelfMateData.rend());
					return std::vector<Mask>({ ValueMask });
				}break;
				case 17:
				{
					return std::vector<Mask>();
				} break;
				case 18:
				{
					auto L1 = E[0].MoveData<std::vector<Mask>>();
					auto& Ref = E[1];
					auto L2 = E[1].MoveData<std::vector<Mask>>();
					L1.insert(L1.end(), L2.begin(), L2.end());
					return std::move(L1);
				}break;
				case 19:
				{
					auto& ref = E[0].GetData<std::vector<Mask>&>();
					auto type_define_mask = E[1].GetData<Mask>();
					ref.push_back(type_define_mask);
					return E[0].MoveRawData();
				} break;
				case 21:
				{
					auto L1 = E[2].MoveData<std::vector<Mask>>();
					for (auto Ite : L1)
					{
						auto Result = table.Find<ValueProperty>(Ite);
						if (Result)
						{
							std::set<StringView> MateDataNameSet;
							auto& ref = Result->mate_data;
							ref.erase(std::remove_if(ref.begin(), ref.end(), [&](Mask mask) -> bool
							{
								auto MateData = table.Find<ValueProperty>(mask);
								assert(MateData);
								auto Result = MateDataNameSet.insert(MateData.name);
								return !Result.second;
							}), ref.end());
						}
					}
					table.PopElementAsUnactive(L1.size());
					return std::move(L1);
				} break;
				case 30:
				{
					auto ImportID = E[3].MoveData<StringView>();
					auto ImportMask = table.FindActiveLast(ImportID);
					if (ImportMask)
					{
						auto redefine = table.FindRaw(ImportMask);
						assert(redefine);
						throw Error::RedefineProperty{ String(ImportID), E.section, redefine.section };
					}
					ImportMask = table.Insert(ImportID, ImportProperty{ E[1].MoveData<StringView>() }, E.section);
					return ImportMask;
				} break;
				case 31:
				{

					StringView RefName = E[0].GetData<StringView>();
					auto RefMask = table.FindActiveLast(RefName);
					if (!RefMask)
						throw Error::UndefineImport{ String(RefName), E.section };
					return ReferencesPath{ RefMask, {} };
				}break;
				case 32:
				{
					auto& P = E[0].GetData<ReferencesPath&>();
					auto IdName = E[1].GetData<StringView>();
					P.references.push_back(IdName);
					return E[0].MoveRawData();
					break;
				}
				case 33:
				{
					std::vector<ReferencesPath> all_paths;
					for (size_t i = 0; i < E.reduce.production_count; ++i)
					{
						if (E[i].IsNoterminal())
							all_paths.push_back(E[i].MoveData<ReferencesPath>());
					}
					return std::move(all_paths);
					break;
				}
				case 34:
				{
					auto code_name = E[1].GetData<StringView>();
					CodeProperty cp{ E[2].MoveData<std::vector<ReferencesPath>>(), E[3].GetData<StringView>() };
					auto code_mask = table.Insert(code_name, std::move(cp), E.section);
					return code_mask;
					break;
				}
				case 36:
				{
					StringView Typename;
					StringView IdName;
					bool InputOrOutput = true;
					if (E.reduce.production_count == 3)
					{
						Typename = E[1].GetData<StringView>();
						IdName = E[2].GetData<StringView>();
						InputOrOutput = E[0].GetData<StringView>() != U"in";
					}
					else if (E.reduce.production_count == 2)
					{
						Typename = E[0].GetData<StringView>();
						IdName = E[1].GetData<StringView>();
					}
					auto TypeMask = table.FindActiveLast(Typename);
					if (!TypeMask)
						throw Error::UndefineType{ String(Typename), E.section };
					return InoutParameter{ InputOrOutput, TypeMask, IdName, E.section };
					break;
				}
				case 37:
				{
					std::vector<InoutParameter> all_parameters;
					std::map<StringView, Section> parameter_name;
					for (size_t i = 0; i < E.reduce.production_count; ++i)
					{
						if (E[i].IsNoterminal())
						{
							auto par = E[i].MoveData<InoutParameter>();
							auto re = parameter_name.insert({ par.name, par.section });
							if (!re.second)
								throw Error::RedefineProperty{ String(par.name), par.section, re.first->second };
							all_parameters.push_back(par);
						}
					}
					return std::move(all_parameters);
					break;
				}
				case 38:
				{
					auto Name = E[1].GetData<StringView>();
					SnippetProperty Sp{ E[2].MoveData<std::vector<ReferencesPath>>(), E[4].GetData<StringView>(),
						E[3].MoveData<std::vector<InoutParameter>>() };
					auto snippet = table.Insert(Name, std::move(Sp), E.section);
					std::vector<Mask> Result;
					Result.push_back(snippet);
					return std::move(Result);
					break;
				}
				case 42:
				{
					auto Name = E[3].MoveData<StringView>();
					MaterialProperty pro{ E[1].GetData<StringView>(), E[4].MoveData<std::vector<Mask>>(), E[6].MoveData<std::vector<Mask>>() };
					table.PopElementAsUnactive(pro.snippets.size());
					table.PopElementAsUnactive(pro.property.size());
					return table.Insert(Name, std::move(pro), E.section);
					break;
				}
				case 46:
				{
					auto Name = E[4].MoveData<StringView>();
					MaterialProperty pro{ E[2].GetData<ReferencesPath>(), E[5].MoveData<std::vector<Mask>>(), E[7].MoveData<std::vector<Mask>>() };
					table.PopElementAsUnactive(pro.snippets.size());
					table.PopElementAsUnactive(pro.property.size());
					return table.Insert(Name, std::move(pro), E.section);
					break;
				}
				case 1:
				{
					Content content{ E[0].MoveData<std::vector<Mask>>(), {} };
					return std::move(content);
					break;
				}
				case 3:
				{
					return Content{};
					break;
				}
				case 43:
				{
					E[0].GetData<Content&>().statement.push_back(E[1].GetData<Mask>());
					return E[0].MoveRawData();
					break;
				}
				case 44:
				{
					table.PopElementAsUnactive(E[0].GetData<Content&>().statement.size());
					return E[0].MoveRawData();
				}
				default:
					assert(false);
					return {};
				}
				return {};
			}
			return {};
		});
		
		return std::move(std::any_cast<Content&>(Result));
		*/
		return {};
	}

	void RemoveSameMateData(std::vector<Mask>& output, Table& table)
	{
		std::set<std::u32string_view> AllName;
		output.erase(std::remove_if(output.begin(), output.end(), [&](Mask mask)->bool
		{
			auto Find = table.Find<ValueProperty>(mask);
			assert(Find.Exist());
			if(Find)
			{
				auto ite = AllName.insert(Find.name);
				return !ite.second;
			}
			return false;
		}), output.end());
	}

	void FilterAndCheck(Content& content, Table& table)
	{
		std::map<std::u32string_view, Section> AllName;
		/*
		for(auto& ite : content.propertys)
		{
			
		}
		*/
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
