#include "Potato/Public/Ebnf.h"
#include "Potato/Public/FileSystem.h"
#include "ParserTable.h"

namespace Dumpling::Mscf
{
	using namespace Potato;
	Ebnf::Table const& MscfEbnfInstance(){
		static Ebnf::Table WTF = []()->Ebnf::Table{
			auto P = FileSystem::GobalPathMapping()(U"$Source:/Content/Mscf.ebnf");
			auto Datas = FileSystem::LoadEntireFile(P);
			auto Code = StrEncode::DocumentWrapper(Datas.data(), Datas.size()).ToString<char32_t>();
			if(!Code.empty())
			{
				return Ebnf::CreateTable(Code);
			}
			return {};
		}();
		return WTF;
	};
}
