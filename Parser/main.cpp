#include "../Potato/syntax.h"
#include "../Potato/character_encoding.h"
#include <filesystem>
#include <fstream>
#include "../Potato/parser.h"

int main()
{
	auto p = std::filesystem::current_path();
	try {
		auto Re = Potato::LoadSBNFFile(LR"(msc.sbnf)");
	}
	catch (Potato::SBNFError error)
	{
		volatile int i = 0;
	}
}