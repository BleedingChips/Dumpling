module;
#include <d3d12.h>
#undef max
export module DumplingDx12Material;

import std;
import Potato;
import DumplingDx12Define;
import DumplingDx12Shader;
import DumplingDx12ResourceStreamer;

export namespace Dumpling::Dx12
{
	std::optional<std::size_t> CreateInputDescription(Potato::IR::StructLayout const& vertex_layout, std::span<D3D12_INPUT_ELEMENT_DESC> desc, std::span<char8_t> temporary_str);
	D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView(std::size_t buffer_size, Potato::MemLayout::ArrayLayout array_layout, ID3D12Resource& vertex_resource, std::size_t offset = 0);
	D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView(Potato::IR::StructLayoutObject& object, ID3D12Resource& vertex_resource, std::size_t offset = 0) { return GetVertexBufferView(object.GetBuffer().size(), object.GetArrayLayout(), vertex_resource, offset); }
	D3D12_INDEX_BUFFER_VIEW GetIndexBufferView(ID3D12Resource& index_resource, std::size_t index_size, std::size_t offset = 0);

	enum class PrimitiveTolopogy
	{
		TRIANGLE = D3D12_PRIMITIVE_TOPOLOGY_TYPE::D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
	};

	struct DescriptorTableMapping
	{
		struct Mapping
		{
			D3D12_DESCRIPTOR_HEAP_TYPE type = D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES;
			std::size_t descriptor_table_identity = std::numeric_limits<std::size_t>::max();
		};
		std::pmr::vector<Mapping> mappings;
	};

	struct DescriptorTableDescription
	{
		struct DescriptorTable
		{
			D3D12_DESCRIPTOR_HEAP_TYPE type;
			ShaderSlotLocate locate;
		};
		std::pmr::vector<DescriptorTable> descriptor_table;
	};

	struct MaterialState
	{
		Potato::IR::StructLayout::Ptr vs_layout;
		PrimitiveTolopogy tolopogy = PrimitiveTolopogy::TRIANGLE;
		ComPtr<ID3D10Blob> vs_shader;
		ComPtr<ID3D10Blob> ps_shader;
	};

	struct ShaderDefineDescriptorTable
	{
		ComPtr<ID3D12DescriptorHeap> resource_heap;
		ComPtr<ID3D12DescriptorHeap> sampler_heap;
		bool CreateConstBufferView(ID3D12Device& device, ShaderSharedResource const& shared_resource, std::size_t resource_index, ID3D12Resource& resource, Potato::Misc::IndexSpan<> span);
	};

	std::optional<ShaderDefineDescriptorTable> CreateDescriptorHeap(ID3D12Device& device, ShaderSharedResource const& shared_resource);
	
	struct ContextDefinedDescriptorTable
	{
		D3D12_DESCRIPTOR_HEAP_TYPE type;
		std::size_t identity = std::numeric_limits<std::size_t>::max();
		std::size_t descriptor_table_offset = 0;
		operator bool() const { return identity != std::numeric_limits<std::size_t>::max(); }
	};


	ComPtr<ID3D12RootSignature> CreateRootSignature(
		ID3D12Device& device, 
		ShaderSlot const& shader_slot,
		DescriptorTableMapping& descriptor_table_mapping,
		Potato::TMP::FunctionRef<ContextDefinedDescriptorTable(ShaderSlotLocate)> context_defined_descriptor_mapping = {}
	);
	
	ComPtr<ID3D12PipelineState> CreatePipelineState(ID3D12Device& device, ID3D12RootSignature& root_signature, MaterialState const& material_state);


	struct ResourceIndexSpan
	{
		ComPtr<ID3D12Resource> resource;
		Potato::Misc::IndexSpan<> index_span;
	};

	struct ShaderSharedResourceInstance
	{
		std::pmr::vector<char8_t> name_buffers;
		struct ConstBuffer
		{
			Potato::Misc::IndexSpan<> name;
			std::size_t descriptor_index;
			ShaderSharedResource::ResourceProperty property;
			ResourceIndexSpan resource;
			Potato::IR::StructLayoutObject::Ptr const_buffer;
		};
		std::pmr::vector<ConstBuffer> const_buffers;
		struct Texture
		{
			Potato::Misc::IndexSpan<> name;
			std::size_t descriptor_index;
			ShaderSharedResource::ResourceProperty property;
			ResourceIndexSpan resource;
		};
		std::pmr::vector<Texture> textures;
		struct Sampler
		{
			Potato::Misc::IndexSpan<> name;
			std::size_t descriptor_index;
		};
		std::pmr::vector<Sampler> sampler;
		void Init(ShaderSharedResource const& shared_resource);
	};

	struct Vertex
	{
		void ResetVertexLayout(Potato::IR::StructLayout layout, D3D12_PRIMITIVE_TOPOLOGY_TYPE topology_type = D3D12_PRIMITIVE_TOPOLOGY_TYPE::D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
		std::size_t CalculateResource_size() const;
		//bool UploadResource(PassStreamer& streamer);
		//bool Draw(struct PassRenderer& renderer);
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