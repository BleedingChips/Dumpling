#pragma once
#include "pre_define_dx12.h"
#include "descriptor_table_dx12.h"
#include <tuple>
namespace Dumpling::Dx12
{

	enum class ShaderType {
		VS = 0,
		DS,
		HS,
		GS,
		PS,
		Num,
	};

	struct ShaderReference {
		std::tuple<std::byte*, size_t> Shaders[static_cast<size_t>(ShaderType::Num)];
	};

	struct ShaderMapping {
		
	};

	struct RawMaterial
	{

	};
}