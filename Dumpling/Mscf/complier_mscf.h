#pragma once
#include <d3dcompiler.h>
#include "../Gui/Dxgi/define_dxgi.h"
#include <Windows.h>
namespace Dumpling::Mscf
{
	struct MscfInput {
		std::string Code;
		std::unordered_map<std::string, std::tuple<std::string, std::variant<bool, std::string>>>
	};
}