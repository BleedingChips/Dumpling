#include "mscf.h"
#include "mscf_parser.h"
namespace Dumpling::Mscf
{
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
}