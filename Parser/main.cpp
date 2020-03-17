#include "../Potato/syntax.h"
#include "../Potato/character_encoding.h"
#include <filesystem>
#include <fstream>
#include "../Potato/parser.h"
#include <fstream>
int main()
{
	using namespace Potato;


	auto p = std::filesystem::current_path();
	std::wstring storage;
	auto Re = Potato::LoadSBNFFile(LR"(msc.sbnf)", storage);
	

	auto Result = Re->serialization();
	auto re2 = Potato::SBNF_parser::unserialization(Result.data(), Result.size());



	//std::wofstream tem_f(L"Result.txt");



	//tem_f << Result;

	volatile int i = 0;
}