#include "../Public/mscf.h"
#include "mscf_parser_table.h"
#include "../../PineApple/Public/CharEncode.h"
#include "../../PineApple/Public/Symbol.h"
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

		auto History = Ebnf::Process(MscfEbnfInstance(), code);
		std::map<std::u32string_view, VariableDesc> varables;

		auto table = DefaultTable();
		auto command = DefaultCommand(table);



		Ebnf::Process(History, [&](Ebnf::Element& E) -> std::any {
			if (E.IsTerminal())
			{
				switch (E.shift.mask)
				{
				case 0: { 
					return StringToFloat(E.shift.capture);
				}
				case 1: return StringToInt(E.shift.capture);
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
				case 4: return E[0].MoveData();
				case 5: return true;
				case 6: return false;
				case 7: return E[0].MoveData();
				case 8: {
					return {};
				} break;
				case 9: {
					std::vector<std::any> AllData;
					for (size_t i = 0; i < count; ++i)
					{

					}




					//auto RequireType = E[0].GetData<std::u32string_view>();
					//std::vector<std::any> AllData;
					/*
					for (size_t i = 0; i < count; ++i)
					{
						if(E.)
					}
					*/
				}
					/*
				case 10: return E.GetRawData(0);
					if (auto ptr = E.TryGetData<int>(0); ptr != nullptr)
					{
						ConstVariable re{ {VariableType::Base, StorageType::Int, 1, sizeof(int), alignof(int), 1, 0}};
						*reinterpret_cast<int*>(re.datas.data()) = *ptr;
						return re;
					}
					else if (auto ptr = E.TryGetData<float>(0); ptr != nullptr)
					{
						ConstVariable re{ {VariableType::Base, StorageType::Float, 1, sizeof(float), alignof(float), 1, 0} };
						*reinterpret_cast<float*>(re.datas.data()) = *ptr;
						return re;
					}
					else
						assert(false);
				} break;
				case 5: { 
					auto type = E.GetData<std::u32string_view>(0);
					auto TarDesc = DefaultInsideType().find(type)->second;
					assert(TarDesc.v_type == VariableType::Base);
					ConstVariable Result{TarDesc};
					switch (TarDesc.s_type)
					{
					case StorageType::Float: {
						auto datas = reinterpret_cast<float*>(Result.datas.data());
						size_t channel = 0;
						for (size_t i = 1; i < E.reduce.production_count; ++i)
						{
							if (auto pt = E.TryGetData<ConstVariable>(i); pt != nullptr)
							{
								if (pt->desc.v_type != VariableType::Base)
									assert(false);
								if (pt->desc.channel + channel > Result.desc.channel)
									assert(false);
								auto re = ConstVariableToFloat(*pt);
								for (size_t i = 0; i < pt->desc.channel; ++i)
									datas[channel++] = re[i];
							}
						}
					} break;
					case StorageType::Int: {
						auto datas = reinterpret_cast<int*>(TarDesc.channel);
						size_t channel = 0;
						for (size_t i = 1; i < E.reduce.production_count; ++i)
						{
							if (auto pt = E.TryGetData<ConstVariable>(i); pt != nullptr)
							{
								if (pt->desc.v_type != VariableType::Base)
									assert(false);
								if (pt->desc.channel + channel > Result.desc.channel)
									assert(false);
								auto re = ConstVariableToInt(*pt);
								for (size_t i = 0; i < pt->desc.channel; ++i)
									datas[channel++] = re[i];
							}
						}
					} break;
					default:
						break;
						
					}
					return Result;
					
				} break;
				*/
				}
			}
			return {};
		});
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