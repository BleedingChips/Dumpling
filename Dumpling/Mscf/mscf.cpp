#include "mscf.h"
#include "mscf_parser.h"
namespace Dumpling::Mscf
{
	using namespace Potato::Parser;

	enum class StringType
	{
		ID, String, Path, Code
	};

	enum class Terminal : sbnf_processer::storage_t
	{
		IntNumber,
		FloatNumber,
		String,
		Code,
		Id,
		Int,
		Int2,
		Int3,
		Int4,
		Float,
		Float2,
		Float3,
		Float4,
	};

	sbnf_processer::storage_t operator*(Terminal input) { return static_cast<sbnf_processer::storage_t>(input); }

	std::map<Terminal, std::u32string> TerStrMap = {
		{Terminal::IntNumber, U"IntNumber"},
		{Terminal::FloatNumber, U"FloatNumber"},
		{Terminal::String, U"String"},
		{Terminal::Id, U"Id"},
		{Terminal::Int, U"'int'"},
		{Terminal::Int2, U"'int2'"},
		{Terminal::Int3, U"'int3'"},
		{Terminal::Int4, U"'int4'"},
		{Terminal::Float, U"'float'"},
		{Terminal::Float2, U"'float2'"},
		{Terminal::Float3, U"'float3'"},
		{Terminal::Float4, U"'float4'"},
	};

	std::map<Terminal, sbnf_processer::storage_t> TerminalSymbolMapping = [&]() {
		std::map<Terminal, sbnf_processer::storage_t> result;
		for (auto& ite : TerStrMap)
		{
			auto re = mscf_sbnf_instance().find_symbol(ite.second);
			assert(re);
			result.insert({ ite.first, *re });
		}
		return std::move(result);
	}();


	struct MscfHandler
	{
		//std::vector<std::variant<std::u32string, int, float>>
		void operator()(sbnf_processer::travel tra);
	};

	void MscfHandler::operator()(sbnf_processer::travel tra)
	{
		if (tra.is_terminal())
		{
			if (tra.sym == TerminalSymbolMapping[Terminal::IntNumber])
			{

			}
		}
		else {

		}
	}

	mscf translate(std::u32string const& code)
	{

		auto& mscf_sbnf = mscf_sbnf_instance();
		sbnf_processer sp(mscf_sbnf);
		MscfHandler Handler;

		sp.analyze(code, Handler);
		return {};
	}
}