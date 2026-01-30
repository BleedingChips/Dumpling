module;
#include <d3d12.h>
#undef max
export module DumplingMaterial;

import std;
import Potato;
import DumplingPlatform;
import DumplingRendererTypes;
import DumplingShader;
import DumplingResourceStreamer;
import DumplingRenderer;

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

	struct DescriptorTableDescription
	{
		struct DescriptorTable
		{
			D3D12_DESCRIPTOR_HEAP_TYPE type;
			ShaderSlot::SourceType source_type = ShaderSlot::SourceType::SHADER_DEFINE;
			std::size_t identity = std::numeric_limits<std::size_t>::max();
		};
		std::pmr::vector<DescriptorTable> descriptor_table;
		std::size_t shader_define_count = 0;
	};

	struct ShaderDescriptorTable
	{
		struct Info
		{
			ComPtr<ID3D12DescriptorHeap> heap;
			D3D12_DESCRIPTOR_HEAP_TYPE type;
			std::size_t count;
			std::size_t offset;
		};
		std::pmr::vector<Info> descriptor_heaps;
	};

	struct MaterialState
	{
		Potato::IR::StructLayout::Ptr vs_layout;
		PrimitiveTolopogy tolopogy = PrimitiveTolopogy::TRIANGLE;
		ComPtr<ID3D10Blob> vs_shader;
		ComPtr<ID3D10Blob> ps_shader;
	};

	ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(ID3D12Device& device, ShaderSlot const& shader_slot);
	
	struct ContextDefinedDescriptorTable
	{
		D3D12_DESCRIPTOR_HEAP_TYPE type;
		std::size_t identity = std::numeric_limits<std::size_t>::max();
		std::size_t descriptor_table_offset = 0;
	};


	ComPtr<ID3D12RootSignature> CreateRootSignature(
		ID3D12Device& device, 
		ShaderSlot const& shader_slot, 
		DescriptorTableDescription& description_table_description, 
		Potato::TMP::FunctionRef<ContextDefinedDescriptorTable(std::size_t identity)> context_defined_descriptor_mapping = {}
	);
	
	ComPtr<ID3D12PipelineState> CreatePipelineState(ID3D12Device& device, ID3D12RootSignature& root_signature, MaterialState const& material_state);

	struct Vertex
	{
		void ResetVertexLayout(Potato::IR::StructLayout layout, D3D12_PRIMITIVE_TOPOLOGY_TYPE topology_type = D3D12_PRIMITIVE_TOPOLOGY_TYPE::D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
		std::size_t CalculateResource_size() const;
		bool UploadResource(PassStreamer& streamer);
		bool Draw(PassRenderer& renderer);
	protected:
		Potato::IR::StructLayout::Ptr vertex_layout;
		Potato::IR::StructLayoutObject::Ptr vertex;
		std::pmr::vector<std::size_t> index_buffer;
		ComPtr<ID3D12Resource> vertex_resource;
		std::size_t vertex_resource_offset;
		ComPtr<ID3D12Resource> index_resource;
		std::size_t index_resource_offset;
	};



}