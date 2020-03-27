#include "msc_parser.h"
#include "../../Potato/tool.h"
#include <optional>
#include <string>
namespace Dumpling
{
	/*
	using namespace Potato;
	std::optional<parser_sbnf> parser_instance;

	bool InitMscSBNFParser(const Potato::lr1::storage_t* binary, size_t length)
	{
		parser_instance.emplace(parser_sbnf::unserialization(binary, length));
	}

	struct MaterialShaderCodeImp : MaterialShaderCode
	{
		mutable Tool::atomic_reference_count m_ref;
		std::wstring table;
		virtual void add_ref() const noexcept { m_ref.add_ref(); }
		virtual void sub_ref() const noexcept { if (m_ref.sub_ref()) delete this; }
	};

	const Potato::parser_sbnf& Parser()
	{
		static Potato::parser_sbnf instance = Potato::parser_sbnf::unserialization(
			msc_parser_serialization, 
			sizeof(msc_parser_serialization) / sizeof(*msc_parser_serialization)
			);
		return instance;
	}
	*/
	/*
	material_shader_code material_shader_code::analyze(const std::wstring& code)
	{
		using type = Potato::lr1::storage_t;
		auto& par = Parser();

		//std::vector<std::tuple<type, std::arrsy>>
	}
	*/
}