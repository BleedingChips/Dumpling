#include "../Dumpling/PineApple/Interface/Ebnf.h"
#include "../Dumpling/FrameWork/Interface/path_system.h"
#include "../Dumpling/PineApple/Interface/CharEncode.h"
#include <chrono>
#include <filesystem>
#include <fstream>
#include <fstream>
#include <sstream>
#include <iostream>

using namespace PineApple::Ebnf;

std::u32string MscfCPPCode =
UR"(
#include "../../PineApple/Interface/Ebnf.h"
using namespace PineApple;
namespace Dumpling::Mscf
{{
	Ebnf::Table const& MscfEbnfInstance(){{
		static Ebnf::Table WTF {};
		return WTF;
	}};
}}
)";

std::string MscfDirectCode =
R"(
#include "../../FrameWork/Interface/path_system.h"
#include "../../PineApple/Interface/Ebnf.h"
#include "../../PineApple/Interface/CharEncode.h"
using namespace PineApple;
namespace Dumpling::Mscf
{
	Ebnf::Table const& MscfEbnfInstance(){
		
		static Ebnf::Table WTF = [](){
			auto SearchedPath = Path::UpSearch(U"Parser");
			assert(SearchedPath);
			auto TargetPath = Path::Search(U"Mscf.ebnf", *SearchedPath);
			assert(TargetPath);
			auto Binary = Path::LoadEntireFile(*TargetPath);
			assert(Binary);
			auto MscfCode = CharEncode::Wrapper<std::byte>(Binary->data(), Binary->size()).To<char32_t>();
			return Ebnf::CreateTable(MscfCode);
		}();
		return WTF;
	};
}
)";

using namespace PineApple;
using namespace Dumpling;

bool UsedDirectCode = false;

int main()
{

	auto SearchedPath2 = Path::UpSearch(U"Dumpling");
	assert(SearchedPath2);
	auto TargetPath2 = Path::Search(U"mscf_parser_table.h", *SearchedPath2);
	assert(TargetPath2);
	std::ofstream ff(*TargetPath2, std::ios::binary);
	assert(ff.is_open());
	auto [b, s] = CharEncode::BomToBinary(CharEncode::BomType::UTF8);
	ff.write(reinterpret_cast<char const*>(b), s);
	if (UsedDirectCode)
	{
		ff.write(MscfDirectCode.data(), MscfDirectCode.size());
	}
	else {
		auto SearchedPath = Path::UpSearch(U"Parser");
		assert(SearchedPath);
		auto TargetPath = Path::Search(U"Mscf.ebnf", *SearchedPath);
		assert(TargetPath);
		auto Binary = Path::LoadEntireFile(*TargetPath);
		assert(Binary);
		auto MscfCode = CharEncode::Wrapper<std::byte>(Binary->data(), Binary->size()).To<char32_t>();

		auto Table = Ebnf::CreateTable(MscfCode);

		auto Patt = StrFormat::CreatePatternRef(MscfCPPCode);
		auto string = CharEncode::Wrapper(StrFormat::Process(Patt, Table)).To<char>();
		ff.write(string.data(), string.size());
	}
	ff.close();
}