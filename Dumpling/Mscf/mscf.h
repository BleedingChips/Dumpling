#pragma once
#include <string>
#include "../Msc/msc.h"
namespace Dumpling::Mscf
{
	struct mscf : Msc::mscf_interface
	{
	};

	mscf translate(std::u32string const& code);
}