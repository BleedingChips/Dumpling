module;

#include "d3d12shader.h"
#include <cassert>

#undef interface
#undef max
#undef FindResource

export module DumplingDx12Shader;

import std;
import Potato;
import DumplingDx12Define;

export namespace Dumpling::Dx12
{
	struct ShaderSlotLocate
	{
		std::size_t context_id = std::numeric_limits<std::size_t>::max();
		std::size_t context_index = std::numeric_limits<std::size_t>::max();
		bool IsShaderDefine() const { return context_id == std::numeric_limits<std::size_t>::max(); }
		bool IsContextDefine() const { return !IsShaderDefine(); }
		operator bool() const { return context_index != std::numeric_limits<std::size_t>::max(); }
	};

	struct ShaderSharedResource
	{

		struct ResourceProperty
		{
			Potato::IR::StructLayout::Ptr layout;
			std::size_t bind_count = std::numeric_limits<std::size_t>::max();
			std::size_t descriptor_table_index = std::numeric_limits<std::size_t>::max();
		};

		struct ResourceDescriptor
		{
			ShaderResourceType type = ShaderResourceType::UNKNOW;
			Potato::Misc::IndexSpan<> name;
			ResourceProperty property;
			std::size_t resource_index = std::numeric_limits<std::size_t>::max();
		};

		std::size_t const_buffer_count = 0;
		std::size_t texture_count = 0;
		std::pmr::vector<char8_t> name_buffer;
		std::pmr::vector<ResourceDescriptor> descriptor_table;
		std::pmr::vector<ResourceDescriptor> sampler_descriptor_table;
		// return descriptor_index; property; resource_index
		std::optional<std::size_t> FindResource(ShaderResourceType type, std::u8string_view name) const;
		ResourceDescriptor const* GetResourceDescriptor(ShaderResourceType type, std::size_t index);
		std::optional<std::size_t> AddResource(ShaderResourceType type, std::u8string_view name,  ResourceProperty property);
	protected:
		std::pmr::vector<ResourceDescriptor>* GetDescriptorTable(ShaderResourceType type);
		std::pmr::vector<ResourceDescriptor> const* GetDescriptorTable(ShaderResourceType type) const
		{
			return const_cast<ShaderSharedResource*>(this)->GetDescriptorTable(type);
		}
	};


	struct ShaderSlot
	{
		struct Slot
		{
			ShaderType shader_type;
			ShaderResourceType resource_type;
			ShaderSlotLocate located;
			std::size_t register_index = std::numeric_limits<std::size_t>::max();
			std::size_t space_index = std::numeric_limits<std::size_t>::max();
		};

		std::pmr::vector<Slot> slots;
		void Reset() { slots.clear(); }
	};

	struct ShaderReflectionContext
	{
		Potato::TMP::FunctionRef<ShaderSlotLocate(std::u8string_view, ShaderResourceType, std::size_t array_count)> context_define_resource;
		Potato::TMP::FunctionRef<StructLayout::Ptr(std::u8string_view)> const_buffer_struct_layout_override;
		std::pmr::memory_resource* layout_resource = std::pmr::get_default_resource();
		std::pmr::memory_resource* temporary_resource = std::pmr::get_default_resource();
	};

	//std::optional<ShaderStatistics> GetShaderStatistics(ID3D12ShaderReflection& target_reflection);


	ShaderSlotLocate CreateLayoutFromCBuffer(
		ID3D12ShaderReflectionConstantBuffer& target_const_buffer,
		std::size_t array_count,
		ShaderSharedResource& shared_resource,
		ShaderReflectionContext const& context = {}
	);

	bool GetShaderSlot(
		ShaderType type,
		ID3D12ShaderReflection& reflection, 
		ShaderSlot& out_slot,
		ShaderSharedResource& shared_resource,
		ShaderReflectionContext const& context = {}
	);
}