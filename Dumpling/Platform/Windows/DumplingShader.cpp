module;

#include <cassert>
#include "Windows.h"
#include "d3d12shader.h"
#undef max

module DumplingShader;


namespace Dumpling
{
	using Potato::IR::StructLayout;


	std::optional<ShaderStatistics> GetShaderStatistics(ID3D12ShaderReflection& target_reflection)
	{
		D3D12_SHADER_DESC desc;
		if (SUCCEEDED(target_reflection.GetDesc(&desc)))
		{
			ShaderStatistics statistics;
			statistics.const_buffer_count = desc.ConstantBuffers;
			statistics.texture_count = desc.TextureLoadInstructions;
			return statistics;
		}
		return std::nullopt;
	}

	template<typename ElementT> struct MappingToScalarWrapper
	{
		HLSLConstBufferLayout operator()() {
			return { StructLayout::GetStatic<ElementT>(), {alignof(ElementT), sizeof(ElementT)} };
		}
	};

	template<typename ElementT> struct MappingToVectorWrapper
	{
		HLSLConstBufferLayout operator()(std::size_t Columns) {
			return MappingVectorLayout<ElementT>(Columns);
		}
	};

	template<typename ElementT> struct MappingToMatrixWrapper
	{
		HLSLConstBufferLayout operator()(std::size_t Rows, std::size_t Columns) {
			return MappingMatrixLayout<ElementT>(Rows, Columns);
		}
	};


	template<template<class ElementT> class Wrapper, typename ...Parameters>
	HLSLConstBufferLayout MappingToVariable(D3D_SHADER_VARIABLE_TYPE type, Parameters&&... pars)
	{
		switch (type)
		{
		case D3D_SHADER_VARIABLE_TYPE::D3D_SVT_FLOAT:
			return Wrapper<float>{}(std::forward<Parameters>(pars)...);
		case D3D_SHADER_VARIABLE_TYPE::D3D_SVT_INT:
			return Wrapper<std::int32_t>{}(std::forward<Parameters>(pars)...);
		case D3D_SHADER_VARIABLE_TYPE::D3D_SVT_UINT:
			return Wrapper<std::uint32_t>{}(std::forward<Parameters>(pars)...);
		default:
			assert(false);
			return {};
		}
	}

	std::tuple<HLSLConstBufferLayout, std::size_t> CreateLayoutFromReflectionType(
		ID3D12ShaderReflectionType& reflection_type,
		Potato::TMP::FunctionRef<HLSLConstBufferLayout(std::u8string_view)> type_layout_override,
		std::pmr::memory_resource* layout_resource,
		std::pmr::memory_resource* temporary_resource
	)
	{
		D3D12_SHADER_TYPE_DESC type_desc;
		if (SUCCEEDED(reflection_type.GetDesc(&type_desc)))
		{
			HLSLConstBufferLayout layout;

			switch (type_desc.Class)
			{
			case D3D_SHADER_VARIABLE_CLASS::D3D_SVC_SCALAR:
				layout = MappingToVariable<MappingToScalarWrapper>(type_desc.Type);
				break;
			case D3D_SHADER_VARIABLE_CLASS::D3D_SVC_VECTOR:
				layout = MappingToVariable<MappingToVectorWrapper>(type_desc.Type, type_desc.Columns);
				break;
			case D3D_SHADER_VARIABLE_CLASS::D3D10_SVC_MATRIX_COLUMNS:
				layout = MappingToVariable<MappingToMatrixWrapper>(type_desc.Type, type_desc.Rows, type_desc.Columns);
				break;
			case D3D_SHADER_VARIABLE_CLASS::D3D_SVC_STRUCT:
			{
				if (type_layout_override)
				{
					std::u8string_view type_name{ reinterpret_cast<char8_t const*>(type_desc.Name) };
					layout = type_layout_override(type_name);
				}
				if (!layout)
				{
					std::pmr::vector<StructLayout::Member> type_member{ temporary_resource };
					type_member.reserve(type_desc.Members);
					for (std::size_t i = 0; i < type_desc.Members; ++i)
					{
						auto type_var = reflection_type.GetMemberTypeByIndex(i);
						auto [type_layout, array_count] = CreateLayoutFromReflectionType(*type_var, type_layout_override, layout_resource, temporary_resource);
						assert(type_layout);
						Potato::IR::StructLayout::Member current_member;
						current_member.struct_layout = std::move(type_layout.struct_layout);
						current_member.overrided_memory_layout = type_layout.memory_layout;
						current_member.array_count = array_count;
						current_member.name = reinterpret_cast<char8_t const*>(type_desc.Name);
						type_member.push_back(current_member);
					}
					layout.struct_layout = StructLayout::CreateDynamic(
						reinterpret_cast<char8_t const*>(type_desc.Name),
						std::span(type_member.data(), type_member.size()),
						Potato::MemLayout::GetHLSLConstBufferPolicy(),
						layout_resource
					);
					assert(layout.struct_layout);
					layout.memory_layout = layout.struct_layout->GetLayout();
				}
				break;
			}
			}
			assert(layout);
			return { layout, type_desc.Elements };
		}
		return {};
	}

	std::tuple<HLSLConstBufferLayout, std::size_t> CreateLayoutFromVariable(
		ID3D12ShaderReflectionVariable& variable,
		Potato::TMP::FunctionRef<HLSLConstBufferLayout(std::u8string_view)> type_layout_override,
		std::pmr::memory_resource* layout_resource,
		std::pmr::memory_resource* temporary_resource
	)
	{
		auto var_type = variable.GetType();
		assert(var_type != nullptr);
		return CreateLayoutFromReflectionType(*var_type, type_layout_override, layout_resource, temporary_resource);
	}


	Potato::IR::StructLayout::Ptr CreateLayoutFromCBuffer(
		ID3D12ShaderReflection& target_reflection,
		std::size_t cbuffer_index,
		Potato::TMP::FunctionRef<Potato::IR::StructLayout::Ptr(std::u8string_view)> cbuffer_layout_override,
		Potato::TMP::FunctionRef<HLSLConstBufferLayout(std::u8string_view)> type_layout_override,
		std::pmr::memory_resource* layout_resource,
		std::pmr::memory_resource* temporary_resource
	)
	{
		ID3D12ShaderReflectionConstantBuffer* const_buffer = target_reflection.GetConstantBufferByIndex(cbuffer_index);

		if (const_buffer != nullptr)
		{
			D3D12_SHADER_BUFFER_DESC buffer_desc;
			const_buffer->GetDesc(&buffer_desc);
			std::u8string_view cbuffer_name{ reinterpret_cast<char8_t const*>(buffer_desc.Name) };
			if (cbuffer_layout_override)
			{
				auto layout = cbuffer_layout_override(cbuffer_name);
				if (layout && layout->GetLayout().size >= buffer_desc.Size)
				{
					return layout;
				}
			}

			std::pmr::vector<Potato::IR::StructLayout::Member> members(temporary_resource);
			members.reserve(buffer_desc.Variables);
			for (std::size_t index = 0; index < buffer_desc.Variables; ++index)
			{
				auto ver = const_buffer->GetVariableByIndex(index);
				assert(ver != nullptr);
				auto [layout, array_count] = CreateLayoutFromVariable(*ver, type_layout_override, temporary_resource, temporary_resource);
				Potato::IR::StructLayout::Member current_member;
				current_member.struct_layout = std::move(layout.struct_layout);
				current_member.overrided_memory_layout = layout.memory_layout;
				current_member.array_count = array_count;
				D3D12_SHADER_VARIABLE_DESC var_desc;
				bool re = SUCCEEDED(ver->GetDesc(&var_desc));
				assert(re);
				current_member.name = reinterpret_cast<char8_t const*>(var_desc.Name);
				members.push_back(current_member);
			}
			auto struct_layout = StructLayout::CreateDynamic(
				reinterpret_cast<char8_t const*>(buffer_desc.Name),
				std::span(members.data(), members.size()), Potato::MemLayout::GetHLSLConstBufferPolicy(),
				layout_resource
			);

			{
				auto mvs = struct_layout->GetMemberView();
				std::vector<D3D12_SHADER_VARIABLE_DESC> des;
				for (std::size_t index = 0; index < buffer_desc.Variables; ++index)
				{
					auto mv = mvs[index];
					auto ver = const_buffer->GetVariableByIndex(index);
					assert(ver != nullptr);
					D3D12_SHADER_VARIABLE_DESC ver_desc;
					auto re = ver->GetDesc(&ver_desc);
					assert(SUCCEEDED(re));
					des.push_back(ver_desc);
					auto layout = mvs[index].member_layout;
					assert(layout.offset == ver_desc.StartOffset);
					assert((layout.array_layout.each_element_offset * std::max(layout.array_layout.count, std::size_t{ 1 })) >= ver_desc.Size);
				}
			}

			return struct_layout;
		}
		return {};
	}
}
