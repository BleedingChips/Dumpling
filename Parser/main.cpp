#include "../Potato/character_encoding.h"
#include "../Potato/parser.h"
#include <filesystem>
#include <fstream>
#include <fstream>
#include <sstream>
#include <iostream>

template<typename T>
std::string to_string(T&& t)
{
	std::stringstream ss;
	ss << std::hex << t;
	std::string re;
	ss >> re;
	return "0x" + re;
}

int main()
{
	try {
		using namespace Potato;

		Lexical::nfa n = Lexical::nfa::create_from_rex(UR"('.*?[^\\]')", 1);
		auto storage = n.simplify();

		Lexical::nfa_processer np(storage, U"\'*\' \'+;");

		auto tra = np();

		auto p = std::filesystem::current_path();
		p += "/mscf.sbnf";
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
			case Encoding::BomType::None: {
				Encoding::string_encoding<char> se(reinterpret_cast<char*>(ite), file_size - size);
				Code = se.to_string<char32_t>();
			}	break;
			default: assert(false);
				break;
			}
			delete[](data);
			auto sbnf_instance = Parser::sbnf::create(Code);
			std::filesystem::path Path = std::filesystem::current_path();
			std::filesystem::path Target;

			for (size_t search_depth = 0; search_depth < 3 && Target.empty(); ++search_depth)
			{
				std::filesystem::recursive_directory_iterator Di(Path);
				for (auto& ite : Di)
				{
					if (ite.is_regular_file())
					{

						auto filename = ite.path().filename().generic_wstring();
						if (filename == L"mscf_parser.h")
						{
							Target = ite.path().parent_path() / (L"mscf_parser.cpp");
							break;
						}
					}
				}
				Path = Path.parent_path();
			}

			if (!Target.empty())
			{
				std::ofstream output_file(Target, std::ios::out);
				if (output_file.is_open())
				{
					output_file << R"(
#include "mscf_parser.h"
using namespace Potato::Parser;
using namespace Potato::Lexical;
using namespace Potato::Syntax;
using range_set = nfa_storage::range_set;
using range = range_set::range;
using EdgeType = nfa_storage::EdgeType;
sbnf const& mscf_sbnf_instance(){
	static sbnf instance{
)";
					output_file << "\t\tstd::u32string(U\"";
					for (auto ite : sbnf_instance.table)
					{
						char Buffer[100];
						size_t used = sprintf_s(Buffer, 100, "\\x%lX", static_cast<uint32_t>(ite));
						output_file << Buffer;
					}
					output_file << "\")," << std::endl;

					output_file << "\t\t{";
					for (auto ite : sbnf_instance.symbol_map)
					{
						auto [s, e] = ite;
						output_file << "{" << to_string(s) << ", " << to_string(e) << "},";
					}
					output_file << "}," << std::endl;;
					output_file << "\t\t" << to_string(sbnf_instance.ter_count) << "," << std::endl;
					output_file << "\t\t" << to_string(sbnf_instance.unused_terminal) << "," << std::endl;
					output_file << "\t\t" << to_string(sbnf_instance.temporary_prodution_start) << "," << std::endl;

					output_file << "\t\tnfa_storage{" << std::endl;
					{
						output_file << "\t\t\t{" << std::endl << "\t\t\t\t";
						for (auto& ite : sbnf_instance.nfa_s.ComsumeEdge)
						{
							output_file << "range_set({";
							for (auto& ite2 : ite)
							{
								output_file << "{" << to_string(ite2.left) << ", " << to_string(ite2.right) << "},";
							}
							output_file << "}),";
						}
						output_file << std::endl << "\t\t\t}," << std::endl;

						output_file << std::endl << "\t\t\t{" << std::endl << "\t\t\t\t";
						for (auto& ite : sbnf_instance.nfa_s.Edges)
						{
							output_file << "{";
							auto [type, i, k] = ite;
							switch (type)
							{
							case Potato::Lexical::nfa_storage::EdgeType::Acception:
								output_file << "EdgeType::Acception" << ", ";
								break;
							case Potato::Lexical::nfa_storage::EdgeType::Comsume:
								output_file << "EdgeType::Comsume" << ", ";
								break;
							default: assert(false);
							}
							output_file << to_string(i) << ", " << to_string(k) << "},";
						}
						output_file << std::endl << "\t\t\t}," << std::endl;

						output_file << "\t\t\t{" << std::endl << "\t\t\t\t";
						for (auto& ite : sbnf_instance.nfa_s.Nodes)
						{
							output_file << "{";
							auto [i, k] = ite;
							output_file << to_string(i) << ", " << to_string(k) << "},";
						}
						output_file << std::endl << "\t\t\t}" << std::endl;
					}
					output_file << "\t\t}," << std::endl;

					output_file << "\t\tlr1_storage{" << std::endl;
					{
						output_file << "\t\t\t{" << std::endl << "\t\t\t\t";
						for (auto& ite : sbnf_instance.lr1_s.productions)
						{
							output_file << "{";
							auto [i, k, l] = ite;
							output_file << to_string(i) << ", " << to_string(k) << ", " << to_string(l) << "},";
						}
						output_file << std::endl << "\t\t\t}," << std::endl;

						output_file << "\t\t\t{" << std::endl << "\t\t\t\t";
						for (auto& ite : sbnf_instance.lr1_s.reduce_shift_table)
						{
							output_file << "{";
							auto [i, k] = ite;
							output_file << to_string(i) << ", " << to_string(k) << "},";
						}
						output_file << std::endl << "\t\t\t}," << std::endl;

						output_file << "\t\t\t{" << std::endl << "\t\t\t\t";
						for (auto& ite : sbnf_instance.lr1_s.nodes)
						{
							output_file << "{";
							auto [i, k, l] = ite;
							output_file << to_string(i) << ", " << to_string(k) << ", " << to_string(l) << "},";
						}
						output_file << std::endl << "\t\t\t}" << std::endl;
					}
					output_file << "\t\t}" << std::endl;

					output_file << "\t};" << std::endl;
					output_file << R"(
	return instance;
}
)" << std::endl;
				}
				else assert(false);
			}
		}
		else assert(false);
	}
	catch (Potato::Parser::sbnf::error const& error)
	{
		Potato::Encoding::string_encoding<char32_t> se(error.message);
		auto string = se.to_string<char>();


		std::cout << "error :" << string << ". in line :" << error.line << ", in Index :" << error.charactor_index << "." << std::endl;
	}



	




	//std::wofstream tem_f(L"Result.txt");



	//tem_f << Result;

	volatile int irr = 0;
}