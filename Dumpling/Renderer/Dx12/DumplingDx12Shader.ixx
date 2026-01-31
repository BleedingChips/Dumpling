module;

#include "d3d12shader.h"
#include <cassert>

#undef interface
#undef max

export module DumplingDx12Shader;

import std;
import Potato;
import DumplingDx12Define;

export namespace Dumpling::Dx12
{

	struct ShaderStatistics
	{
		std::size_t bound_resource_count = 0;
		std::size_t texture_count = 0;
		std::size_t sampler_count = 0;
		std::size_t const_buffer_count = 0;
	};

	struct ShaderSlot
	{

		struct Source
		{
			std::size_t context_define_index = std::numeric_limits<std::size_t>::max();
			std::size_t context_define_sub_index = std::numeric_limits<std::size_t>::max();
			bool IsShaderDefine() const { return context_define_index == std::numeric_limits<std::size_t>::max(); }
			bool IsContextDefine() const { return !IsShaderDefine(); }
		};

		struct ConstBuffer
		{
			Potato::IR::StructLayout::Ptr layout;
			Source source;
		};

		struct Slot
		{
			ShaderType shader_type;
			ShaderResourceType resource_type;
			std::size_t resource_index;
			std::size_t register_index;
			std::size_t space_index;
		};

		std::pmr::vector<ConstBuffer> const_buffer;
		std::pmr::vector<Slot> slots;
		ShaderStatistics total_statics;

		void Reset() { const_buffer.clear(); slots.clear(); total_statics = {}; }
	};

	struct ShaderReflectionConstBufferContext
	{
		Potato::TMP::FunctionRef<StructLayout::Ptr(std::u8string_view)> type_layout_override;
		std::pmr::memory_resource* layout_resource = std::pmr::get_default_resource();
		std::pmr::memory_resource* temporary_resource = std::pmr::get_default_resource();
	};

	std::optional<ShaderStatistics> GetShaderStatistics(ID3D12ShaderReflection& target_reflection);

	ShaderSlot::ConstBuffer CreateLayoutFromCBuffer(
		ID3D12ShaderReflectionConstantBuffer& target_const_buffer,
		Potato::TMP::FunctionRef<ShaderSlot::ConstBuffer(std::u8string_view)> layout_override = {},
		ShaderReflectionConstBufferContext const& context = {}
	);

	bool GetShaderSlot(
		ShaderType type,
		ID3D12ShaderReflection& reflection, 
		ShaderSlot& out_slot,
		Potato::TMP::FunctionRef<ShaderSlot::ConstBuffer(std::u8string_view)> layout_override = {},
		ShaderReflectionConstBufferContext const& context = {}
	);
}