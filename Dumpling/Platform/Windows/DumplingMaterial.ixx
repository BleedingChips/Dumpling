module;
#include <d3d12.h>

export module DumplingMaterial;

import std;
import Potato;
import DumplingPlatform;
import DumplingRendererTypes;

export namespace Dumpling
{
	std::optional<std::size_t> CreateInputDescription(Potato::IR::StructLayout const& vertex_layout, std::span<D3D12_INPUT_ELEMENT_DESC> desc, std::span<char8_t> temporary_str);


	struct MaterialState
	{
		Potato::IR::StructLayout::Ptr vs_layout;
	};
}