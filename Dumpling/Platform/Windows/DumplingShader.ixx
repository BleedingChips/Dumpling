module;

#include "d3d12shader.h"
#include <cassert>

#undef interface
#undef max

export module DumplingShader;

import std;
import Potato;
import DumplingPlatform;
import DumplingRendererTypes;

export namespace Dumpling
{

	struct ShaderStatistics
	{
		std::size_t bound_resource_count = 0;
		std::size_t texture_count = 0;
		std::size_t sampler_count = 0;
		std::size_t const_buffer_count = 0;
	};

	enum class ShaderType
	{
		VS,
		PS,
	};

	struct ShaderSlot
	{

		enum class Type
		{
			VS_CONST_BUFFER,
			VS_TEXTURE,
			PS_CONST_BUFFER,
			PS_TEXTURE
		};

		struct Source
		{
			std::size_t source_type = std::numeric_limits<std::size_t>::max();
			std::size_t source_index = std::numeric_limits<std::size_t>::max();
		};

		struct ConstBuffer
		{
			Potato::IR::StructLayout::Ptr layout;
			Source source;
		};

		struct Slot
		{
			Type type;
			std::size_t slot_index;
			std::size_t resource_index;
			std::size_t space;
		};

		std::pmr::vector<ConstBuffer> const_buffer;
		std::pmr::vector<Slot> slots;
		ShaderStatistics total_statics;

		void Reset() { const_buffer.clear(); slots.clear(); total_statics = {}; }
	};

	struct ShaderReflectionConstBufferContext
	{
		ShaderSlot::Source default_source;
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