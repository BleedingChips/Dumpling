#include "../Public/mscf.h"
#include "mscf_parser_table.h"
#include "../../PineApple/Public/CharEncode.h"
#include "mscf_define.h"
#include <array>

float StringToFloat(std::u32string_view Input)
{
	auto str = CharEncode::Wrapper(Input).To<char>();
	float Result;
	sscanf_s(str.c_str(), "%f", &Result);
	return Result;
}

int64_t StringToInt(std::u32string_view Input)
{
	auto str = CharEncode::Wrapper(Input).To<char>();
	int64_t Result;
	sscanf_s(str.c_str(), "%I64i", &Result);
	return Result;
}

namespace Dumpling::Mscf
{
	
	using namespace PineApple::Symbol;
	using String = std::u32string;
	using StringView = std::u32string_view;

	void CheckRedefine(Table const& table, std::vector<Mask> const& masks)
	{
		std::map<StringView, Section> NameSet;
		for(auto ite : masks)
		{
			assert(ite);
			auto result = table.Find<ValueProperty>(ite);
			assert(result.Exist());
			auto re = NameSet.insert({result.name, result.section});
			if(!re.second)
				throw Error::RedefineProperty{String(re.first->first), result.section , re.first->second };
		}
	}

	mscf translate(String const& code)
	{
		auto& Mref = MscfEbnfInstance();
		try {
			auto History = Ebnf::Process(MscfEbnfInstance(), code);
			auto P = History.Expand();

			auto [Table, Command] = CreateDefaultContent();

			Ebnf::Process(History, [&](Ebnf::Element& E) -> std::any {
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
					case 1: { return {}; } break;
					case 2: { return {}; } break;
					case 3: { return {}; } break;
					case 4: {
						auto& Data = E[0];
						if (Data.TryGetData<int64_t>())
							Command.PushData(*Data.TryGetData<int64_t>(), Data.section);
						else if(Data.TryGetData<float>())
							Command.PushData(*Data.TryGetData<float>(), Data.section);
						else if(Data.TryGetData<StringView>())
							Command.PushData(*Data.TryGetData<StringView>(), Data.section);
						else
							assert(false);
						return {}; 
					} break;
					case 5:{
						Command.PushData(true, E.section);
					} break;
					case 6:{
						Command.PushData(false, E.section);
					} break;
					case 7:{
						auto P = E[0].GetData<StringView>();
						auto TypeMask = Table.FindActiveLast(P);
						if(!TypeMask)
							throw Error::UndefineType{String(P), E[0].section };
						size_t index = 0;
						for (auto& Ite : E)
						{
							if (Ite.IsNoterminal())
								++index;
						}
						Command.CoverToType(TypeMask, index, E.section);
					} break;
					case 22:{
						auto P = Table.FindActiveLast(U"__MateData");
						assert(P);
							size_t used = 0;
						for(auto& Ite : E)
						{
							if(Ite.IsNoterminal())
								++used;
						}
						Command.CoverToType(P, used, E.section);
					} break;
					case 8:{
						std::vector<int64_t> Result;
						Result.push_back(0);
						return std::move(Result);
					} break;
					case 9:{
						std::vector<int64_t> Last = E[0].MoveData<std::vector<int64_t>>();
						Last.push_back(E[2].GetData<int64_t>());
						return std::move(Last);
					} break;
					case 10:{
						return std::vector<int64_t>{};
					} break;
					case 20:
						{
							auto TypeName = E[0].GetData<StringView>();
							auto TypeMask = Table.FindActiveLast(TypeName);
							if (!TypeMask)
								throw Error::UndefineType{ String(TypeName), E[0].section };
							Mask SampleMask;
							if (E.reduce.production_count == 3)
							{
								auto ReaderName = E[2].GetData<StringView>();
								SampleMask = Table.FindActiveLast(ReaderName);
								if (!SampleMask)
									throw Error::UndefineType{ String(ReaderName), E[2].section };
							}
							return std::tuple<Mask, Mask>{TypeMask, SampleMask};
						} break;
					case 11: {
						auto [TypeMask, ReaderMask] = E[0].GetData<std::tuple<Mask, Mask>>();
						auto ValName = E[1].GetData<StringView>();
						auto ArrayCount = E[2].MoveData<std::vector<int64_t>>();
						if(ReaderMask)
						{
							auto Find = Table.Find<TextureProperty>(TypeMask);
							assert(Find.Exist());
							if(!Find)
								throw Error::RequireTypeDonotSupportSample{ String(Find.name), E[0].section };
						}
						ValueProperty Pro{ TypeMask,ReaderMask, std::move(ArrayCount), {} };
						auto ValueMask = Table.Insert(ValName, std::move(Pro), E.section);
						if(E.reduce.production_count == 5)
							Command.EqualData(ValueMask, E.section);
						return ValueMask;
					}break;
					case 12:{
							auto TypeName = E[1].GetData<StringView>();
							std::vector<Mask> AllProperty;
							for(size_t i = 3; i < E.reduce.production_count; ++i)
							{
								if(E[i].IsNoterminal())
									AllProperty.push_back(E[i].GetData<Mask>());
							}
							CheckRedefine(Table, AllProperty);
							Table.PopElementAsUnactive(AllProperty.size());
							auto TypeMask = Table.Insert(TypeName, {std::move(AllProperty)}, E.section);
							return TypeMask;
					} break;
					case 13:
						{
							auto MateType = Table.FindActiveLast(U"__MateData");
							assert(MateType);
							auto Name = E[0].GetData<StringView>();
							auto MateMask = Table.Insert(Name, ValueProperty{MateType, {}, {}, {}}, E.section);
							if(E.reduce.production_count == 3)
								Command.EqualData(MateMask, E.section);
							return MateMask;
						}break;
					case 14:
						{
							std::vector<Mask> AllMateData;
							for(size_t i = 0; i < E.reduce.production_count; ++i)
							{
								if(!E[i].IsTerminal())
									AllMateData.push_back(E[i].GetData<Mask>());
							}
							Table.PopElementAsUnactive(AllMateData.size());
							return std::move(AllMateData);
						}break;
					case 15:
						{
							std::vector<Mask> DefinedValue;
							std::vector<Mask> AppendMate = E[0].MoveData<std::vector<Mask>>();
							for(size_t i = 1; i < E.reduce.production_count; i+=1)
							{
								if(E[i].IsNoterminal())
								{
									std::vector<Mask> Value = E[i].MoveData<std::vector<Mask>>();
									for(auto Item : Value)
									{
										auto Fined = Table.Find<ValueProperty>(Item);
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
							auto Result = Table.Find<ValueProperty>(ValueMask);
							assert(Result);
							auto& ref = Result->mate_data;
							ref.insert(ref.end(), SelfMateData.rbegin(), SelfMateData.rend());
							return std::vector<Mask>({ValueMask});
						}break;
					case 17:
						{
							return std::tuple<std::vector<Mask>, size_t>({}, 0);
						} break;
					case 18:
						{
							auto [L1, count] = E[0].MoveData<std::tuple<std::vector<Mask>, size_t>>();
							auto L2 = E[1].MoveData<std::vector<Mask>>();
							count += L2.size();
							L1.insert(L1.end(), L2.begin(), L2.end());
							return std::tuple<std::vector<Mask>, size_t>{std::move(L1), count};
						}break;
					case 19:
						{
							auto [L1, count] = E[0].MoveData<std::tuple<std::vector<Mask>, size_t>>();
							++count;
							return std::tuple<std::vector<Mask>, size_t>{std::move(L1), count};
						} break;
					case 21:
						{
							auto [L1, count] = E[2].MoveData<std::tuple<std::vector<Mask>, size_t>>();
							for(auto Ite : L1)
							{
								auto Result = Table.Find<ValueProperty>(Ite);
								assert(Result);
								std::set<StringView> MateDataNameSet;
								auto& ref = Result->mate_data;
								ref.erase(std::remove_if(ref.begin(), ref.end(), [&](Mask mask) -> bool
								{
									auto MateData = Table.Find<ValueProperty>(mask);
									assert(MateData);
									auto Result = MateDataNameSet.insert(MateData.name);
									return !Result.second;
								}), ref.end());
							}
							auto pop_map = Table.PopAndReturnElementAsUnactive(count);
							CheckRedefine(Table, pop_map);
							volatile int i =0;
						} break;
					}
				}
				return {};
			});
			return {};
		}
		catch (Ebnf::Error::UnacceptableSyntax& US)
		{
			volatile int i = 0;
		}

		return {};
	}
	/*

	mscf translate(std::u32string const& code)
	{
		auto& Mref = MscfEbnfInstance();
		try {
			auto History = Ebnf::Process(MscfEbnfInstance(), code);
			auto P = History.Expand();

			SymbolTable table;
			DataStorage datas;

			Ebnf::Process(History, [&](Ebnf::Element& E) -> std::any {
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
					auto mask = E.reduce.mask;
					auto count = E.reduce.production_count;
					auto datas = E.datas;
					switch (mask)
					{
					case 1: return E[0].MoveData();
					case 2: return E[0].MoveData();
					case 3: return {};
					case 4: {
						auto & ref = E[0];
						if (auto p = ref.TryGetData<float>(); p != nullptr)
							commands.Push(Command::Data::Make(U"float", *p), E.loc);
						else if (auto p = ref.TryGetData<int32_t>(); p != nullptr)
							commands.Push(Command::Data::Make(U"int", *p), E.loc);
						else if (auto p = ref.TryGetData<uint32_t>(); p != nullptr)
							commands.Push(Command::Data::Make(U"uint", *p), E.loc);
						else if (auto p = ref.TryGetData<std::u32string_view>(); p != nullptr)
							commands.Push(Command::Data::Make(U"string", *p), E.loc);
						else
							assert(false);
						return {};
					} break;
					case 5:{
						commands.Push(Command::Data::Make(U"bool", true), E.loc);
						return {};
					} break;
					case 6: {
						commands.Push(Command::Data::Make(U"bool", false), E.loc);
						return {};
					} break;
					case 8 :{
						auto id = E[0].GetData<std::u32string_view>();
						size_t index = 0;
						for(size_t i = 1; i < E.reduce.production_count; ++i)
							if(E[i].IsNoterminal())
								++index;
						commands.Push(Command::MakeNoNameValue{index}, E.loc);
						return {};
					} break;
					case 19:{
						size_t index = 0;
						for (size_t i = 0; i < E.reduce.production_count; ++i)
							if (E[i].IsNoterminal())
								++index;
						commands.Push(Command::LinkNoNameValue{ index }, E.loc);
					} break;
					case 20:{ return ValueProperty{{}, true, 0}; } break;
					case 21:{ return ValueProperty{{}, true, E[1].GetData<size_t>()}; } break;
					case 22:{ return ValueProperty{{}, false}; }
					case 9:{
						std::array<std::u32string_view, 3> ids;
						ValueProperty ap;
						size_t index = 0;
						for (auto& ite : E)
						{
							if(ite.string == U"Id")
								ids[index++] = ite.shift.capture;
							else if(ite.string == U"<ArrayProperty>")
								ap = ite.GetData<ValueProperty>();
						}
						if (index == 2)
						{
							size_t index = commands.Push(Command::DefineValue{ids[0], ids[1], ap}, E.loc);
							return ids[1];
						}
						else {
							assert(index == 3);
							ap.read_format = ids[1];
							commands.Push(Command::DefineValue{ ids[0], ids[2], ap }, E.loc);
							return ids[2];
						}
					} break;
					case 10:{
						if (E.reduce.production_count == 3)
						{
							commands.Push(Command::EqualValue{E[0].GetData<std::u32string_view>(), 1}, E.loc);
						}
						else {
							assert(E.reduce.production_count == 1);
						}
						return E[0].MoveData();
					} break;
					case 11:{
						size_t index = 0;
						for (auto& ite : E)
						{
							if(E.IsNoterminal())
								index +=1;
						}
						commands.Push(Command::DefineType{E[0].GetData<std::u32string_view>()}, E.loc);
						return {};
					} break;
					case 12:{
						if (E.reduce.production_count == 3)
						{
							commands.Push(MakeMateValue{ E[0].GetData<std::u32string_view>(), 1 }, E.loc);
						}
						else {
							commands.Push(MakeMateValue{ E[0].GetData<std::u32string_view>(), 0 }, E.loc);
						}
						return E[0].GetData<std::u32string_view>();
					} break;
					case 13:{
						std::vector<std::u32string_view> all_mateData;
						for (auto& ite : E)
						{
							if(ite.IsNoterminal())
								all_mateData.push_back(ite.GetData<std::u32string_view>());
						}
						return std::move(all_mateData);
					} break;
					case 16:{
						return std::vector<std::u32string_view>{};
					} break;
					case 15:{
						auto EffectName = 
					} break;
					case 14:{
						auto all_mate_data = E[0].GetData<std::vector<std::u32string_view>>();

					} break;
					default: return {};
					}
				}
				return {};
			});
			return {};
		}
		catch (Ebnf::Error::UnacceptableSyntax& US)
		{
			volatile int i = 0;
		}
		
		return {};
	}
	*/

	void Translate(std::u32string_view InputCode)
	{

	}






	/*
	using namespace Potato::Parser;
	using namespace Dumpling::Dx;

	auto& ref = mscf_sbnf_instance();
	using storage_t = sbnf::storage_t;

	storage_t TyInt[4] = { *ref.find_symbol(U"'int'"), *ref.find_symbol(U"'int2'"), *ref.find_symbol(U"'int3'"), *ref.find_symbol(U"'int4'") };
	storage_t TyFloat[4] = { *ref.find_symbol(U"'float'"), *ref.find_symbol(U"'float2'"), *ref.find_symbol(U"'float3'"), *ref.find_symbol(U"'float4'") };
	storage_t TyTexture[2] = { *ref.find_symbol(U"'Texture2D'"), *ref.find_symbol(U"'Texture1D'") };
	storage_t TyRWTexture[2] = { *ref.find_symbol(U"'RWTexture2D'"), *ref.find_symbol(U"'RWTexture1D'") };

	struct MscfHandler
	{
		//std::vector<std::variant<std::u32string, int, float>>
		void operator()(sbnf_processer::travel tra);
		std::vector<std::u32string> ID;
		std::vector<std::u32string> String;
		std::vector<std::variant<std::u32string, Int, Int2, Int3, Int4, Float, Float2, Float3, Float4>> Variable;
	};

	void MscfHandler::operator()(sbnf_processer::travel tra)
	{
		return;
		if (tra.is_terminal())
		{
			if (tra.sym_str == U"'ID'")
				ID.push_back(std::u32string(tra.token_data));
			else if (tra.sym_str == U"'String'")
				String.push_back(std::u32string(tra.token_data));
		}
		else {

		}
	}

	mscf translate(std::u32string const& code)
	{

		auto& mscf_sbnf = mscf_sbnf_instance();
		MscfHandler Handler;
		try {
			sbnf_processer{}(mscf_sbnf, code, Handler);
		}
		catch (sbnf::error const& Message)
		{
			__debugbreak();
		}
		
		return {};
	}
	*/
}