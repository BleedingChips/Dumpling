#include "../Potato/syntax.h"
#include "../Potato/character_encoding.h"
#include <filesystem>
#include <fstream>
#include "../Potato/parser.h"

int main()
{
	using namespace Potato;


	auto p = std::filesystem::current_path();
	std::wstring storage;
	auto Re = Potato::LoadSBNFFile(LR"(msc.sbnf)", storage);
}