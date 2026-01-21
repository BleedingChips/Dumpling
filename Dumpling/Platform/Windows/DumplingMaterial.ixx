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
	D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView(std::size_t buffer_size, Potato::MemLayout::ArrayLayout array_layout, ID3D12Resource& vertex_resource);
	D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView(Potato::IR::StructLayoutObject& object, ID3D12Resource& vertex_resource) { return GetVertexBufferView(object.GetBuffer().size(), object.GetArrayLayout(), vertex_resource); }
	
	struct MaterialState
	{
		Potato::IR::StructLayout::Ptr vs_layout;
	};
}