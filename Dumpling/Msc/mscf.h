#pragma once
#include <string>
namespace Dumpling::Msc
{
	struct mscf {};
	mscf translate(std::u32string const& code);
}