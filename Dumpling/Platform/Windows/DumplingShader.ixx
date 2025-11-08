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
	using ShaderReflection = ID3D12ShaderReflection;
	using ShaderReflectionPtr = PlatformPtr<ShaderReflection>;

	struct ShaderStatistics
	{
		std::size_t texture_count = 0;
		std::size_t sampler_count = 0;
		std::size_t const_buffer_count = 0;
	};

	std::optional<ShaderStatistics> GetShaderStatistics(ShaderReflection& target_reflection);

	Potato::IR::StructLayout::Ptr CreateLayoutFromCBuffer(
		ShaderReflection& target_reflection,
		std::size_t cbuffer_index,
		Potato::TMP::FunctionRef<Potato::IR::StructLayout::Ptr(std::u8string_view)> cbuffer_layout_override = {},
		Potato::TMP::FunctionRef<HLSLConstBufferLayout(std::u8string_view)> type_layout_override = {},
		std::pmr::memory_resource* layout_resource = std::pmr::get_default_resource(),
		std::pmr::memory_resource* temporary_resource = std::pmr::get_default_resource()
	);
}