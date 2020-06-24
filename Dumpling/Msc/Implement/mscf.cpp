#include "../Interface/mscf.h"
#include "mscf_parser_table.h"

std::u32string FloatToStr

namespace Dumpling::Mscf
{

	mscf translate(std::u32string const& code)
	{
		auto& Mref = MscfEbnfInstance();
		static size_t String = *Mref.FindSymbolState(U"String");
		static size_t Id = *Mref.FindSymbolState(U"Id");
		static size_t Code = *Mref.FindSymbolState(U"Code");
		static size_t FloatNumber = *Mref.FindSymbolState(U"FloatNumber");
		static size_t IntNumber = *Mref.FindSymbolState(U"IntNumber");

		auto History = Ebnf::Process(MscfEbnfInstance(), code);
		std::map<std::u32string, CustomType> custom_type;
		std::map<std::u32string, PropertySolt> solts;
		Ebnf::Process(History, [&](Ebnf::Element& E) -> std::any {
			if (E.IsTerminal())
			{
				switch (E.state)
				{
				case FloatNumber: 
				}
			}
			else if (E.IsNoterminal())
			{

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