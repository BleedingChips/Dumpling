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
	
	enum class VariableType
	{
		Base,
		Texture,
		RWTexture,
		Struct,
		RWStruct,
	};

	enum class StorageType
	{
		Unorm,
		UUnorm,
		UInt,
		Int,
		Float,
		Custom,
	};

	struct VariableDesc
	{
		VariableType v_type;
		StorageType s_type;
		size_t channel;
		size_t size;
		size_t align;
		size_t array_size;
		size_t default_value_index;
	};

	struct ConstVariable
	{
		VariableDesc desc;
		union 
		{
			std::array<std::byte, sizeof(float) * 4> datas;
			std::u32string_view str;
		};
	};

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
	}

	mscf translate(std::u32string const& code);

}
