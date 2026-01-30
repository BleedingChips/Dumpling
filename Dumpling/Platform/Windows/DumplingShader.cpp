module;

#include <cassert>
#include "Windows.h"
#include "d3d12shader.h"
#undef max

module DumplingShader;
import DumplingRendererTypes;


namespace Dumpling
{
	using Potato::IR::StructLayout;


	std::optional<ShaderStatistics> GetShaderStatistics(ID3D12ShaderReflection& target_reflection)
	{
		D3D12_SHADER_DESC desc;
		if (SUCCEEDED(target_reflection.GetDesc(&desc)))
		{
			ShaderStatistics statistics;
			statistics.bound_resource_count = desc.BoundResources;
			statistics.const_buffer_count = desc.ConstantBuffers;
			statistics.texture_count = desc.TextureLoadInstructions;
			return statistics;
		}
		return std::nullopt;
	}

	template<typename ElementT> struct MappingToScalarWrapper
	{
		StructLayout::Ptr operator()() {
			return StructLayout::GetStatic<ElementT, HLSLConstBufferLayoutOverride>();
		}
	};

	template<typename ElementT> struct MappingToVectorWrapper
	{
		StructLayout::Ptr operator()(std::size_t Columns) {
			return MappingVectorLayout<ElementT>(Columns);
		}
	};

	template<typename ElementT> struct MappingToMatrixWrapper
	{
		StructLayout::Ptr operator()(std::size_t Rows, std::size_t Columns) {
			return MappingMatrixLayout<ElementT>(Rows, Columns);
		}
	};


	template<template<class ElementT> class Wrapper, typename ...Parameters>
	StructLayout::Ptr MappingToVariable(D3D_SHADER_VARIABLE_TYPE type, Parameters&&... pars)
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

	std::tuple<StructLayout::Ptr, std::size_t> CreateLayoutFromReflectionType(
		ID3D12ShaderReflectionType& reflection_type,
		Potato::TMP::FunctionRef<StructLayout::Ptr(std::u8string_view)> type_layout_override,
		std::pmr::memory_resource* layout_resource,
		std::pmr::memory_resource* temporary_resource
	)
	{
		D3D12_SHADER_TYPE_DESC type_desc;
		if (SUCCEEDED(reflection_type.GetDesc(&type_desc)))
		{
			StructLayout::Ptr layout;

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
						current_member.struct_layout = std::move(type_layout);
						current_member.array_count = array_count;
						current_member.name = reinterpret_cast<char8_t const*>(type_desc.Name);
						type_member.push_back(current_member);
					}
					layout = StructLayout::CreateDynamic(
						reinterpret_cast<char8_t const*>(type_desc.Name),
						std::span(type_member.data(), type_member.size()),
						GetHLSLConstBufferPolicy(),
						layout_resource
					);
					assert(layout);
				}
				break;
			}
			}
			assert(layout);
			return { layout, type_desc.Elements };
		}
		return {};
	}

	std::tuple<StructLayout::Ptr, std::size_t> CreateLayoutFromVariable(
		ID3D12ShaderReflectionVariable& variable,
		Potato::TMP::FunctionRef<StructLayout::Ptr(std::u8string_view)> type_layout_override,
		std::pmr::memory_resource* layout_resource,
		std::pmr::memory_resource* temporary_resource
	)
	{
		auto var_type = variable.GetType();
		assert(var_type != nullptr);
		return CreateLayoutFromReflectionType(*var_type, type_layout_override, layout_resource, temporary_resource);
	}


	ShaderSlot::ConstBuffer CreateLayoutFromCBuffer(
		ID3D12ShaderReflectionConstantBuffer& target_const_buffer,
		Potato::TMP::FunctionRef<ShaderSlot::ConstBuffer(std::u8string_view)> layout_override,
		ShaderReflectionConstBufferContext const& context
	)
	{
		D3D12_SHADER_BUFFER_DESC buffer_desc;
		target_const_buffer.GetDesc(&buffer_desc);
		std::u8string_view cbuffer_name{ reinterpret_cast<char8_t const*>(buffer_desc.Name) };
		if (layout_override)
		{
			auto cbuffer_source = layout_override(cbuffer_name);
			if (cbuffer_source.layout && cbuffer_source.layout->GetLayout().size >= buffer_desc.Size)
			{
				return cbuffer_source;
			}
		}

		std::pmr::vector<Potato::IR::StructLayout::Member> members(context.temporary_resource);
		members.reserve(buffer_desc.Variables);
		for (std::size_t index = 0; index < buffer_desc.Variables; ++index)
		{
			auto ver = target_const_buffer.GetVariableByIndex(index);
			assert(ver != nullptr);
			auto [layout, array_count] = CreateLayoutFromVariable(*ver, context.type_layout_override, context.layout_resource, context.temporary_resource);
			Potato::IR::StructLayout::Member current_member;
			current_member.struct_layout = std::move(layout);
			current_member.array_count = array_count;
			D3D12_SHADER_VARIABLE_DESC var_desc;
			bool re = SUCCEEDED(ver->GetDesc(&var_desc));
			assert(re);
			current_member.name = reinterpret_cast<char8_t const*>(var_desc.Name);
			members.push_back(current_member);
		}
		auto struct_layout = StructLayout::CreateDynamic(
			reinterpret_cast<char8_t const*>(buffer_desc.Name),
			std::span(members.data(), members.size()), GetHLSLConstBufferPolicy(),
			context.layout_resource
		);

		{
			auto mvs = struct_layout->GetMemberView();
			std::vector<D3D12_SHADER_VARIABLE_DESC> des;
			for (std::size_t index = 0; index < buffer_desc.Variables; ++index)
			{
				auto mv = mvs[index];
				auto ver = target_const_buffer.GetVariableByIndex(index);
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

		return { struct_layout, {} };
	}

	bool GetShaderSlot(
		ShaderType type, 
		ID3D12ShaderReflection& reflection, 
		ShaderSlot& out_slot,
		Potato::TMP::FunctionRef<ShaderSlot::ConstBuffer(std::u8string_view)> layout_override,
		ShaderReflectionConstBufferContext const& context
		)
	{
		std::array<ShaderSlot::Type, 2> slot_types;
		switch (type)
		{
		case ShaderType::PS:
			slot_types[0] = ShaderSlot::Type::PS_CONST_BUFFER;
			slot_types[1] = ShaderSlot::Type::PS_TEXTURE;
			break;
		default:
			slot_types[0] = ShaderSlot::Type::VS_CONST_BUFFER;
			slot_types[1] = ShaderSlot::Type::VS_TEXTURE;
			break;
		}

		D3D12_SHADER_DESC desc;
		if (!SUCCEEDED(reflection.GetDesc(&desc)))
			return false;

		std::size_t exist_index = std::numeric_limits<std::size_t>::max();

		auto reference_context = context;

		auto layout_exist_override = [&](std::u8string_view name) mutable -> ShaderSlot::ConstBuffer
			{
				std::size_t index = 0;

				for (auto& ite : out_slot.const_buffer)
				{
					if (name == ite.layout->GetName())
					{
						exist_index = index;
						return ite;
					}
					++index;
				}

				if (layout_override)
				{
					return layout_override(name);
				}

				return {};
			};

		for (std::size_t i = 0; i < desc.BoundResources; ++i)
		{
			D3D12_SHADER_INPUT_BIND_DESC bind_sesc;
			reflection.GetResourceBindingDesc(i, &bind_sesc);
			switch (bind_sesc.Type)
			{
			case D3D_SHADER_INPUT_TYPE::D3D_SIT_CBUFFER:
			{
				auto buffer = reflection.GetConstantBufferByName(bind_sesc.Name);
				if (buffer != nullptr)
				{
					exist_index = std::numeric_limits<std::size_t>::max();
					auto cbuffer_layout = CreateLayoutFromCBuffer(*buffer, layout_exist_override, context);
					if (!cbuffer_layout.layout)
						return false;
					if (exist_index == std::numeric_limits<std::size_t>::max())
					{
						auto current_buffer_index = out_slot.const_buffer.size();
						out_slot.const_buffer.emplace_back(std::move(cbuffer_layout));
						out_slot.slots.push_back({ slot_types[0], current_buffer_index, bind_sesc.BindPoint, bind_sesc.Space });
					}
					else {
						out_slot.slots.push_back({ slot_types[0], exist_index, bind_sesc.BindPoint, bind_sesc.Space });
					}
					out_slot.total_statics.bound_resource_count += 1;
					out_slot.total_statics.const_buffer_count += 1;
				}
				break;
			}
			}
		}

		return true;
	}
}
