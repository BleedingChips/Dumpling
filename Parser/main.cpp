#include "../Potato/syntax.h"
#include "../Potato/character_encoding.h"
#include <filesystem>
#include <fstream>
#include "../Potato/parser.h"
#include "../Potato/lexical.h"
#include <fstream>
int main()
{
	using namespace Potato;

	std::u32string_view Rexs[] = {
		U"ab|c",
	};

	auto tem = dfa::create_from_rexs(Rexs, 1);


	auto p = std::filesystem::current_path();
	std::wstring storage;
	auto Re = Potato::LoadSBNFFile(LR"(msc.sbnf)", storage);
	

	auto Result = Re->serialization();
	auto re2 = Potato::parser_sbnf::unserialization(Result.data(), Result.size());

	std::filesystem::path TargetPath = std::filesystem::current_path();
	TargetPath.append(L".\\..\\Context\\MaterialCode\\Msc.sbnfb");
	TargetPath = std::filesystem::absolute(TargetPath);

	std::ofstream tem_f(TargetPath, std::ios::binary);

	if (tem_f.is_open())
	{
		tem_f.write(reinterpret_cast<const char*>(Result.data()), Result.size() * sizeof(lr1::storage_t));
		tem_f.close();
	}
	else
		assert(false);

	




	//std::wofstream tem_f(L"Result.txt");



	//tem_f << Result;

	volatile int irr = 0;
}