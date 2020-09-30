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

int32_t StringToInt(std::u32string_view Input)
{
	auto str = CharEncode::Wrapper(Input).To<char>();
	int32_t Result;
	sscanf_s(str.c_str(), "%i", &Result);
	return Result;
}


namespace Dumpling::Mscf
{

	using namespace PineApple::Symbol;

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