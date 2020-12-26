#pragma once
#include <string>
#include "msc.h"
#include <array>
#include <string>
#include <map>
#include <set>
#include <array>

#include "../../PineApple/Public/Ebnf.h"

namespace Dumpling::Mscf
{
	using Section = PineApple::Ebnf::Section;

	

	struct mscf : Msc::mscf_interface
	{

	};

	namespace Error
	{
		struct UndefineType
		{
			std::u32string type_name;
			Section section;
		};
		
		struct RequireTypeDonotSupportSample
		{
			std::u32string type_name;
			Section section;
		};

		struct RedefineProperty
		{
			std::u32string redefine_name;
			Section redefine_section;
			Section predefine_section;
		};

		struct UndefineImport
		{
			std::u32string name;
			Section section;
		};
		
	}

	mscf translate(std::u32string const& code);

}
