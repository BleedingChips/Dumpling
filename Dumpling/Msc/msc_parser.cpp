#include "msc_parser.h"
#include "msc_parser_target.h"
namespace Dumpling
{
	const Potato::parser_sbnf& Parser()
	{
		static Potato::parser_sbnf instance = Potato::parser_sbnf::unserialization(
			msc_parser_serialization, 
			sizeof(msc_parser_serialization) / sizeof(*msc_parser_serialization)
			);
		return instance;
	}
	/*
	material_shader_code material_shader_code::analyze(const std::wstring& code)
	{
		using type = Potato::lr1::storage_t;
		auto& par = Parser();

		//std::vector<std::tuple<type, std::arrsy>>
	}
	*/
}