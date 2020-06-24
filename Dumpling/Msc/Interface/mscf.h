#pragma once
#include <string>
#include "msc.h"
#include <array>
#include <string>
#include <map>
#include <set>
namespace Dumpling::Mscf
{

	enum class BaseDataType
	{
		Int,
		Float,
		Texture,
		RWTexture,
		StructBuf,
		RWStructBuf
	};

	struct NamedValue
	{
		BaseDataType type;
		size_t channel;
		std::array<std::byte, sizeof(float) * 4> datas;
	};

	struct CustomType
	{
		size_t size;
		size_t aligned_size;
	};

	struct MateData
	{
		std::map<std::u32string, NamedValue> mate_datas;
		std::set<std::u32string> flags;
	};

	struct PropertySolt
	{
		NamedValue value;
		MateData matedatas;
	};

	struct mscf : Msc::mscf_interface
	{

	};

	mscf translate(std::u32string const& code);
}