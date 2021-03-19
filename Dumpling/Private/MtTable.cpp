#include "../Public/MtTable.h"
#include "../Public/Mscf.h"
#include "Potato/Public/StrScanner.h"
#include "Potato/Public/FileSystem.h"
#include "Potato/Public/Ebnf.h"
#include <array>

namespace Dumpling::MscfParser
{
	using namespace Potato;
	
	Ebnf::Table const& MtEbnfInstance(){
		static Ebnf::Table WTF = []()->Ebnf::Table{
			auto P = FileSystem::GobalPathMapping()(U"$Source:/Content/mt.ebnf");
			auto Datas = FileSystem::LoadEntireFile(P);
			auto Code = StrEncode::DocumentWrapper(Datas.data(), Datas.size()).ToString<char32_t>();
			if(!Code.empty())
			{
				return Ebnf::CreateTable(Code);
			}
			return {};
		}();
		return WTF;
	};
	
	MscfContent MscfParser(std::u32string_view code)
	{
		auto& Table = MtEbnfInstance();
		try
		{
			auto His = Ebnf::Process(Table, code);
			MscfContent result;
			His.operator()([&](Ebnf::NTElement& E) -> std::any
			{
				switch (E.mask)
				{
				case 4:
				{
					ValueMask mask;
					Potato::AnyViewerTemplate<float, int64_t, std::u32string_view>{}(E[0].data, [&](auto&& ref)
					{
						mask = result.InsertValue(ref);
					});
					result.PushCommand(C_PushData{mask}, E.section);
					return mask;
				} break;
				case 5:
					return result.InsertValue(true);
					break;
				case 6:
					return result.InsertValue(false);
					break;
				case 7:
				{
					auto name = E[0].Consume<std::u32string_view>();
					auto TypeMask = result.FindActiveSymbolAtLast(name);
					size_t count = 0;
					for (auto& ite : E)
						if (ite.IsNoterminal())
							count += 1;
					result.PushCommand(C_ConverType{ TypeMask, name, count }, E.section);
				}break;
				case 22:
				{
					size_t count = 0;
					for(auto& ite : E)
						if(ite.IsNoterminal())
							count += 1;
					result.PushCommand(C_MarkAsArray{count}, E.section);
				}break;
				case 8:
				{
					ValueSymbol vs;
					vs.type = E[0].Consume<TypeProperty>();
					return std::tuple<ValueSymbol, std::u32string_view>{std::move(vs), E[1].Consume<std::u32string_view>()};
				}break;
				case 9:
				{
					auto [vs, name] = E[0].Consume<std::tuple<ValueSymbol, std::u32string_view>>();
					vs.arrays.push_back(E[1].Consume<int32_t>());
					return std::tuple<ValueSymbol, std::u32string_view>{std::move(vs), name};
				}break;
				case 10:
				{
					auto [vs, name] = E[0].Consume<std::tuple<ValueSymbol, std::u32string_view>>();
					vs.arrays.push_back(std::nullopt);
					return std::tuple<ValueSymbol, std::u32string_view>{std::move(vs), name};
				}break;
				case 20:
				{
					TypeProperty tp;
					tp.name = E[0].Consume<std::u32string_view>();
					tp.type = result.FindActiveSymbolAtLast(tp.name);
					if (E.production.size() > 1)
					{
						tp.sampler_name = E[2].Consume<std::u32string_view>();
						tp.sampler = result.FindActiveSymbolAtLast(tp.sampler_name);
					}
					return tp;
				}break;
				case 11:
				{
					auto [vs, name] = E[0].Consume<std::tuple<ValueSymbol, std::u32string_view>>();
					auto vm = result.InsertSymbol(name, std::move(vs), E.section);
					if(E.production.size() > 1)
						result.PushCommand(C_SetValue{ vm }, E.section);
					return vm;
				}break;
				case 23:
				{
					ValueSymbol Vp;
					Vp.type.name = E[0].Consume<std::u32string_view>();
					Vp.type.type = result.FindActiveSymbolAtLast(Vp.type.name);
					Vp.is_member = true;
					auto name2 = E[1].Consume<std::u32string_view>();
					auto mask = result.InsertSymbol(name2, std::move(Vp), E.section);
					return mask;
				}break;
				case 12:
				{
					TypeSymbol Tp;
					size_t count = 0;
					for(size_t i = 2; i < E.production.size(); ++i)
					{
					    auto& ref = E[i];
						if(ref.IsNoterminal())
							count +=1;
					}
					Tp.member = result.PopSymbolAsUnactive(count);
					return result.InsertSymbol(E[1].Consume<std::u32string_view>(), Tp, E.section);
				}break;
				case 13:
				{
					MateDataSymbol MDP;
					auto mask = result.InsertSymbol(E[0].Consume<std::u32string_view>(), MDP, E.section);
					result.PushCommand(C_SetValue{mask}, E.section);
					return mask;
				}break;
				case 14:
					{
						size_t index = 0;
						for(size_t i = 0; i < E.production.size(); ++i)
						{
							auto& ref = E[i];
							if(ref.IsNoterminal())
								++index;
						}
						return result.PopSymbolAsUnactive(index);
					}break;
				case 15:
					{
						auto mate = E[0].Consume<AreaMask>();
						size_t count = 0;
						for(size_t i = 2; i < E.production.size(); ++i)
						{
							if(E[i].IsNoterminal())
							{
								auto ref = E[i].Consume<size_t>();
								count += ref;
							}
						}
						
						result.FindActiveProperty(count, [&](Table::Property& P){
							AnyViewer(P.property, [&](ValueSymbol& VP){
								VP.mate_data.push_back(mate);
							});
						});
						return count;
					}break;
				case 16:
					{
						auto mate = E[0].Consume<AreaMask>();
						auto mask = E[1].Consume<SymbolMask>();
						result.FindProperty(mask, [&](Table::Property& P){
							AnyViewer(P.property, [&](ValueSymbol& VP){
								VP.mate_data.push_back(mate);
							});
						});
						return size_t(1);
					}break;
				case 17:
					{
						return size_t(0);
					}break;
				case 19:
					{
						auto P = E[0].Consume<size_t>();
						return size_t(P + 1);
					}break;
				case 18:
					{
						auto P = E[0].Consume<size_t>();
						auto P2 = E[1].Consume<size_t>();
						return size_t(P + P2);
					}break;
				case 21:
					{
					    auto P = E[2].Consume<size_t>();
						auto area = result.PopSymbolAsUnactive(P);
						PropertySymbol PP{area};
						return result.InsertSymbol(U"$.Property", PP, E.section);
					}break;
				case 30:
				    {
						ImportSymbol Ip;
						Ip.path = E[1].Consume<std::u32string_view>();
						return result.InsertSymbol(E[3].Consume<std::u32string_view>(), std::move(Ip), E.section);
				    }break;
				case 31:
				    {
						References rp;
						rp.reference_name = E[0].Consume<std::u32string_view>();
						rp.property_reference = result.FindActiveSymbolAtLast(rp.reference_name);
						return rp;
				    }break;
				case 32:
					{
						auto P = E[0].Consume<References>();
						P.sub_references.push_back(E[2].Consume<std::u32string_view>());
						return std::move(P);
					}break;
				case 33:
					{
						std::vector<References> all_reference;
						for(size_t i= 0; i < E.production.size(); ++i)
						{
							auto& ref = E[i];
							if (ref.IsNoterminal())
							{
								auto P = ref.Consume<References>();
								all_reference.push_back(std::move(P));
							}
						}
						return all_reference;
					}break;
				case 34:
					{
						CodeSymbol cp;
						cp.reference = E[2].Consume<std::vector<References>>();
						cp.code = E[3].Consume<std::u32string_view>();
						return result.InsertSymbol(E[1].Consume<std::u32string_view>(), std::move(cp), E.section);
					}break;
				case 36:
					{
						InoutParameter Ip;
						size_t shift = 0;
				        if(E.production.size() == 3)
						{
							shift += 1;
							if(E[0].shift.capture == U"out")
							    Ip.is_input = false;
							else
							    Ip.is_input = false;
						}
						Ip.type_name = E[0 + shift].Consume<std::u32string_view>();
						Ip.type = result.FindActiveSymbolAtLast(Ip.type_name);
						Ip.name = E[1+shift].Consume<std::u32string_view>();
						return Ip;
					}break;
				case 37:
				    {
				        std::vector<InoutParameter> pars;
						for(size_t i = 0; i < E.production.size(); ++i)
						{
							if(E[i].IsNoterminal())
								pars.push_back(E[i].Consume<InoutParameter>());
						}
						return pars;
				    }break;
				case 38:
				    {
						SnippetSymbol Sp;
						auto name = E[1].Consume<std::u32string_view>();
						Sp.reference = E[2].Consume<std::vector<References>>();
						Sp.parameters = E[3].Consume<std::vector<InoutParameter>>();
						Sp.code = E[4].Consume<std::u32string_view>();
						return result.InsertSymbol(name, std::move(Sp), E.section);
				    }break;
				case 39:
				{
					MaterialUsingSymbol MUP;
					MUP.reference_path = E[2].Consume<References>();
					return result.InsertSymbol(E[4].Consume<std::u32string_view>(), std::move(MUP), E.section);
				}break;
				case 40:
				{
					MaterialDefineSymbol MDP;
					MDP.define_target = E[1].Consume<std::u32string_view>();
					MDP.define_source = E[2].Consume<std::u32string_view>();
					return result.InsertSymbol({}, std::move(MDP), E.section);
				}break;
				case 42:
				{
					MaterialSymbol Mp;
					Mp.mate_data = E[0].Consume<AreaMask>();
					Mp.shading_mode = E[2].Consume<std::u32string_view>();
					auto name = E[4].Consume<std::u32string_view>();
					size_t statemenet_count = E[6].Consume<size_t>();
					Mp.propertys = result.PopSymbolAsUnactive(statemenet_count);
					return result.InsertSymbol(name, std::move(Mp), E.section);
				}break;
				case 1:
				{
				}break;
				default:
					assert(false);
					break;
				}
				return {};
			},
			[&](Ebnf::TElement& E)-> std::any
			{
				switch (E.mask)
				{
				case 0:
				{
					float Num = 0.0f;
					Potato::StrScanner::DirectProcess(E.capture, Num);
					return Num;
				}break;
				case 1:
				{
					int32_t Num = 0;
					Potato::StrScanner::DirectProcess(E.capture, Num);
					return Num;
				}break;
				case 2:
				{
					return std::u32string_view{ E.capture.begin() + 1, E.capture.end() -2 };
				}break;
				case 3:
				{
					return std::u32string_view{ E.capture.begin() + 2, E.capture.end() - 4 };
				}break;
				default:
				{
					return E.capture;
				}break;
				}
			}
			);
			return std::move(result);
		}catch(Potato::Exception::Ebnf::UnacceptableSyntax const& US)
		{
			volatile int i = 0;
		}
		catch (Potato::Exception::Ebnf::Interface&)
		{
			
		}
		return {};
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

	/*
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
	*/
	

	/*
	void FilterAndCheck(Content& content, Table& table)
	{
		std::map<std::u32string_view, Section> AllName;
	}
	*/

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
