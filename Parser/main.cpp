#include "../Potato/character_encoding.h"
#include "../Potato/parser_ebnf.h"
#include "../Dumpling/FrameWork/path_system.h"
#include "../Potato/parser_format.h"
#include <chrono>
#include <filesystem>
#include <fstream>
#include <fstream>
#include <sstream>
#include <iostream>

using namespace Potato::Parser;
using namespace Potato::Encoding;
using namespace Dumpling::Path;

template<typename T>
std::string to_string(T&& t)
{
	std::stringstream ss;
	ss << std::hex << t;
	std::string re;
	ss >> re;
	return "0x" + re;
}

/*
std::string SbnfToString(Potato::Parser::sbnf const& ref, std::string_view Name, size_t TabCount = 0)
{
	std::string Tapstring;
	for (size_t i = 0; i < TabCount; ++i)
		Tapstring += U'\t';
	std::string Code;
	Code += Tapstring + "Potato::Parser::sbnf " + std::string(Name) + "{ \n";
	Code += Tapstring + "\tstd::u32string(U\"";
	for (auto ite : ref.table)
	{
		char Buffer[100];
		size_t used = sprintf_s(Buffer, 100, "\\x%lX", static_cast<uint32_t>(ite));
		Code += Buffer;
	}
	Code += "\"),\n";
	Code += Tapstring + "\t\t{";
	for (auto ite : ref.symbol_map)
	{
		auto [s, e] = ite;
		Code += "{" + to_string(s) + ", " + to_string(e) + "},";
	}
	Code += "},\n";
	Code += Tapstring + "\t\t" + to_string(ref.ter_count) + ",\n";
	Code += "\t\t" + to_string(ref.unused_terminal) + "," + '\n';
	Code += "\t\t" + to_string(ref.temporary_prodution_start) + "," + '\n';

	Code += "\t\tnfa_storage{\n";
	{
		Code += "\t\t\t{\n\t\t\t\t";
		for (auto& ite : ref.nfa_s.ComsumeEdge)
		{
			Code += "range_set({";
			for (auto& ite2 : ite)
			{
				Code += "{" + to_string(static_cast<uint32_t>(ite2.left)) + ", " + to_string(static_cast<uint32_t>(ite2.right)) + "},";
			}
			Code += "}),";
		}
		Code += "\n\t\t\t},\n";

		Code += "\n\t\t\t{\n\t\t\t\t";
		for (auto& ite : ref.nfa_s.Edges)
		{
			Code += "{";
			auto [type, i, k] = ite;
			switch (type)
			{
			case Potato::Lexical::nfa_storage::EdgeType::Acception:
				Code += "EdgeType::Acception, ";
				break;
			case Potato::Lexical::nfa_storage::EdgeType::Comsume:
				Code += "EdgeType::Comsume, ";
				break;
			default: assert(false);
			}
			Code += to_string(i) + ", " + to_string(k) + "},";
		}
		Code += "\n\t\t\t},\n";

		Code += "\t\t\t{\n\t\t\t\t";
		for (auto& ite : ref.nfa_s.Nodes)
		{
			Code += "{";
			auto [i, k] = ite;
			Code += to_string(i) + ", " + to_string(k) + "},";
		}
		Code += "\n\t\t\t}\n";
	}
	Code += "\t\t},\n";

	Code += "\t\tlr1_storage{\n";
	{
		Code += "\t\t\t{\n\t\t\t\t";
		for (auto& ite : ref.lr1_s.productions)
		{
			Code += "{";
			auto [i, k, l] = ite;
			Code += to_string(i) + ", " + to_string(k) + ", " + to_string(l) + "},";
		}
		Code += "\n\t\t\t}, \n";

		Code += "\t\t\t{\n\t\t\t\t";
		for (auto& ite : ref.lr1_s.reduce_shift_table)
		{
			Code += "{";
			auto [i, k] = ite;
			Code += to_string(i) + ", " + to_string(k) + "},";
		}
		Code += "\n\t\t\t},\n";

		Code += "\t\t\t{\n\t\t\t\t";
		for (auto& ite : ref.lr1_s.nodes)
		{
			Code += "{";
			auto [i, k, l] = ite;
			Code += to_string(i) + ", " + to_string(k) + ", " + to_string(l) + "},";
		}
		Code += "\n\t\t\t}\n";
	}
	Code += "\t\t}\n";

	Code += "\t};\n";
	return std::move(Code);
}
*/



int main()
{

	auto P = Format::CreatePatternRef(U"sdasd 0x{-hex} sdasdas{{}}sadasdas");
	auto re = Format::Process(P, 123);

	std::u32string_view OI[] = {
		{UR"(\s)" },
		{U"\r\n|\n"},
		{UR"([a-zA-Z_][a-zA-Z_0-9]*)"},
		{UR"(:=)"},
		{UR"(%%%\s*?\n)"},
		{UR"('.*?[^\\]')"},
		{UR"(\<[_a-zA-Z][_a-zA-Z0-9]*\>)"},
	};

	auto PCC = NFA::CreateTableReversal(OI, 6);

	auto String = Format::Process(U"{}", PCC);



	std::ofstream WTF;
	





	auto Data = std::filesystem::current_path();

	auto ParserPath = UpSearch(U"Parser");
	assert(ParserPath);

	{
		auto MscfSbnf = Search(U"mscf.sbnf", *ParserPath);
		assert(MscfSbnf);

		auto MscfFile = LoadEntireFile(*MscfSbnf);
		assert(MscfFile);

		auto [BomType, Data, size] = FixBinaryWithBom(MscfFile->data(), MscfFile->size());

		std::u32string Code;
		switch (BomType)
		{
		case BomType::UTF8:
		case BomType::None: {
			Code = EncodingWrapper<char>(reinterpret_cast<char const*>(Data), size).To<char32_t>();
		}	break;
		case BomType::UTF16BE:
		case BomType::UTF16LE:
			Code = EncodingWrapper<char16_t>(reinterpret_cast<char16_t const*>(Data), size).To<char32_t>();
			break;
		case BomType::UTF32BE:
		case BomType::UTF32LE:
			Code = EncodingWrapper<char32_t>(reinterpret_cast<char32_t const*>(Data), size).To<char32_t>();
			break;
		default: assert(false);
			break;
		}

		/*try*/ {


			auto Time = std::chrono::system_clock::now();
			try {
				auto MscfSbnfInstance = Ebnf::CreateTable(Code);
			}
			catch (Ebnf::Error::ErrorMessage const& EM)
			{
				auto Time2 = std::chrono::system_clock::now();
				auto Dur = Time2 - Time;
				std::cout << Dur.count() << std::endl;
			}
			
			/*
			auto DumplingPath = UpSearch(U"Dumpling", *ParserPath);
			assert(DumplingPath);
			auto MscfRequirePath = Search(U"mscf_parser.h", *DumplingPath);
			assert(MscfRequirePath);
			MscfRequirePath->replace_extension(U".cpp");
			*/

			std::string TotalCode = R"(
#include "mscf_parser.h"
using namespace Potato::Parser;
using namespace Potato::Lexical;
using namespace Potato::Syntax;
using range_set = nfa_storage::range_set;
using range = range_set::range;
using EdgeType = nfa_storage::EdgeType;
sbnf const& mscf_sbnf_instance(){
	static )";

			//auto Code = SbnfToString(MscfSbnfInstance, "instance");

			//TotalCode += Code;
			TotalCode += R"(
	return instance;
}
)";
			//std::ofstream MscfOutputFile(*MscfRequirePath, std::ios::binary);

			//MscfOutputFile.write(TotalCode.data(), TotalCode.size());


		}
		//catch (...)
		{
			//throw;
		}
		/*
		catch (Potato::Parser::sbnf::error const& error)
		{
			auto Message = EncodingWrapper<char32_t>(error.message).To<wchar_t>();
			std::wcout << L"error :" << Message << L". in line :" << error.line << L", in Index :" << error.charactor_index << L"." << std::endl;
		}
		*/

		

	}
}