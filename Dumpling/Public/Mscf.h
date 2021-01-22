#pragma once
#include <string>
#include <array>
#include <string>
#include <map>
#include <set>
#include <array>

#include "Potato/Public/Ebnf.h"

namespace Dumpling::Mscf
{
	using Section = Potato::Ebnf::Section;

	namespace Error
	{

		struct Interface
		{
			
		};
		
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

	struct Mscf
	{
		
	};

	Mscf Translate(std::u32string const& code);

}
