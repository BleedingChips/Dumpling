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

	Lexical::nfa n = Lexical::nfa::create_from_rex(UR"(/\*[.\n]*?\*/|//.*?\n)", 1);

	auto p = std::filesystem::current_path();
	p += "/msc.sbnf";
	std::ifstream file(p);
	if (file.is_open())
	{
		size_t file_size = std::filesystem::file_size(p);
		std::byte* data = new std::byte[file_size];
		file.read(reinterpret_cast<char*>(data), file_size);
		auto [Type, size] = Encoding::translate_binary_to_bomtype(data, file_size);
		std::byte* ite = data;
		ite += size;
		std::u32string Code;
		switch (Type)
		{
		case Encoding::BomType::UTF8:
		case Encoding::BomType::None:{
			Encoding::string_encoding<char> se(reinterpret_cast<char*>(ite), file_size - size);
			Code = se.to_string<char32_t>();
		}	break;
		default: assert(false);
			break;
		}
		auto Ref = Parser::sbnf::create(Code);
	}
	else assert(false);
	




	//std::wofstream tem_f(L"Result.txt");



	//tem_f << Result;

	volatile int irr = 0;
}