#include "../Potato/syntax.h"
#include "../Potato/character_encoding.h"
#include <filesystem>
#include <fstream>
#include "../Potato/parser.h"
#include "../Potato/regular_expression.h"
#include <fstream>
int main()
{
	using namespace Potato;

	std::u32string_view Rexs[] = {
		U"[ab]*?ab[ab]*"
	};

	auto tem = Rex::nfa::create_from_rex(Rexs[0], 0);

	//auto re = dfa_processer::comsume_analyze(tem, U"51cdsd");


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