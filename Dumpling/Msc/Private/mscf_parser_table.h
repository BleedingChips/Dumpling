
#include "../../FrameWork/Public/path_system.h"
#include "../../PineApple/Public/Ebnf.h"
#include "../../PineApple/Public/CharEncode.h"
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
