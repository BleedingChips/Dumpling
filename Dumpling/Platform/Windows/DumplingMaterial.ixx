module;
#include <d3d12.h>

export module DumplingMaterial;

import std;
import Potato;
import DumplingPlatform;
import DumplingRendererTypes;
import DumplingShader;

export namespace Dumpling
{
	std::optional<std::size_t> CreateInputDescription(Potato::IR::StructLayout const& vertex_layout, std::span<D3D12_INPUT_ELEMENT_DESC> desc, std::span<char8_t> temporary_str);
	D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView(std::size_t buffer_size, Potato::MemLayout::ArrayLayout array_layout, ID3D12Resource& vertex_resource, std::size_t offset = 0);
	D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView(Potato::IR::StructLayoutObject& object, ID3D12Resource& vertex_resource, std::size_t offset = 0) { return GetVertexBufferView(object.GetBuffer().size(), object.GetArrayLayout(), vertex_resource, offset); }
	D3D12_INDEX_BUFFER_VIEW GetIndexBufferView(ID3D12Resource& index_resource, std::size_t index_size, std::size_t offset = 0);

	enum class PrimitiveTolopogy
	{
		TRIANGLE = D3D12_PRIMITIVE_TOPOLOGY_TYPE::D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
	};

	struct MaterialState
	{
		Potato::IR::StructLayout::Ptr vs_layout;
		PrimitiveTolopogy tolopogy = PrimitiveTolopogy::TRIANGLE;
		ComPtr<ID3D10Blob> vs_shader;
		ComPtr<ID3D10Blob> ps_shader;
	};

	ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(ID3D12Device& device, ShaderSlot const& shader_slot);
	ComPtr<ID3D12RootSignature> CreateRootSignature(ID3D12Device& device, ShaderSlot const& shader_slot);
	ComPtr<ID3D12PipelineState> CreatePipelineState(ID3D12Device& device, ID3D12RootSignature& root_signature, MaterialState const& material_state);
}