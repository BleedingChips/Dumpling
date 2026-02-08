module;

#include <cassert>
#include "Windows.h"
#include "d3d12shader.h"
#undef max
#undef FindResource

module DumplingDx12Shader;

import DumplingDxStructLayout;


namespace Dumpling::Dx12
{
	using Potato::IR::StructLayout;
	using namespace Dumpling::Dx;

	/*
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
	*/

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

	std::pmr::vector<ShaderSharedResource::ResourceDescriptor>* ShaderSharedResource::GetDescriptorTable(ShaderResourceType type)
	{
		switch (type)
		{
		case ShaderResourceType::TEXTURE:
		case ShaderResourceType::CONST_BUFFER:
			return &descriptor_table;
			break;
		case ShaderResourceType::SAMPLER:
			return &sampler_descriptor_table;
			break;
		default:
			assert(false);
			return nullptr;
		}
	}

	std::optional<std::size_t> ShaderSharedResource::FindResource(ShaderResourceType type, std::u8string_view name) const
	{
		std::pmr::vector<ResourceDescriptor> const* tar_descriptor = GetDescriptorTable(type);

		if (tar_descriptor != nullptr)
		{
			std::size_t index = 0;
			for (auto& ite : *tar_descriptor)
			{
				if (ite.type == ShaderResourceType::CONST_BUFFER)
				{
					auto ite_name = std::u8string_view{
						name_buffer.data() + ite.name.Begin(),
						name_buffer.data() + ite.name.End()
					};

					if (ite_name == name)
					{
						return index;
					}
				}
				index++;
			}
		}

		return std::nullopt;
	}

	ShaderSharedResource::ResourceDescriptor const* ShaderSharedResource::GetResourceDescriptor(ShaderResourceType type, std::size_t index)
	{
		std::pmr::vector<ResourceDescriptor> const* tar_descriptor = GetDescriptorTable(type);

		if (tar_descriptor != nullptr)
		{
			if (tar_descriptor->size() > index)
			{
				auto tar = tar_descriptor->data() + index;
				if (tar->type == type)
				{
					return tar;
				}
			}
		}
		return nullptr;
	}

	std::optional<std::size_t> ShaderSharedResource::AddResource(ShaderResourceType type, std::u8string_view name, ResourceProperty property)
	{
		assert(!FindResource(type, name).has_value());
		std::pmr::vector<ResourceDescriptor>* tar_descriptor = GetDescriptorTable(type);
		if (tar_descriptor != nullptr)
		{
			auto old_name_buffer = name_buffer.size();
			name_buffer.append_range(name);
			Potato::Misc::IndexSpan<> name_index = { old_name_buffer, name_buffer.size() };
			std::size_t resource_index = 0;
			switch (type)
			{
			case ShaderResourceType::CONST_BUFFER:
				resource_index = const_buffer_count++;
				break;
			case ShaderResourceType::TEXTURE:
				resource_index = texture_count++;
				break;
			case ShaderResourceType::SAMPLER:
				resource_index = sampler_descriptor_table.size();
				break;
			default:
				assert(false);
				return std::nullopt;
			}
			tar_descriptor->emplace_back(
				type,
				name_index,
				std::move(property),
				resource_index
			);
			return tar_descriptor->size() - 1;
		}
		return std::nullopt;
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


	ShaderSlotLocate CreateLayoutFromCBuffer(
		ID3D12ShaderReflectionConstantBuffer& target_const_buffer,
		std::size_t array_count,
		ShaderSharedResource& shared_resource,
		ShaderReflectionContext const& context
	)
	{
		D3D12_SHADER_BUFFER_DESC buffer_desc;
		target_const_buffer.GetDesc(&buffer_desc);
		std::u8string_view cbuffer_name{ reinterpret_cast<char8_t const*>(buffer_desc.Name) };

		auto finded_index = shared_resource.FindResource(ShaderResourceType::CONST_BUFFER, cbuffer_name);

		if (finded_index.has_value())
		{
			auto property = shared_resource.GetResourceDescriptor(ShaderResourceType::CONST_BUFFER, *finded_index);
			if (property == nullptr)
			{
				assert(property != nullptr);
				return {};
			}
			if (property->property.bind_count != array_count)
			{
				assert(property->property.bind_count == array_count);
				return {};
			}
			ShaderSlotLocate locate;
			locate.context_index = *finded_index;
			return locate;
		}

		if (context.context_define_resource)
		{
			auto locate = context.context_define_resource(
				cbuffer_name,
				ShaderResourceType::CONST_BUFFER,
				array_count
			);

			if (locate)
				return locate;
		}

		Potato::IR::StructLayout::Ptr cbuffer_layout;

		if (context.const_buffer_struct_layout_override)
		{
			cbuffer_layout = context.const_buffer_struct_layout_override(cbuffer_name);
		}

		if (!cbuffer_layout)
		{
			std::pmr::vector<Potato::IR::StructLayout::Member> members(context.temporary_resource);
			members.reserve(buffer_desc.Variables);
			for (std::size_t index = 0; index < buffer_desc.Variables; ++index)
			{
				auto ver = target_const_buffer.GetVariableByIndex(index);
				assert(ver != nullptr);
				auto [layout, array_count] = CreateLayoutFromVariable(*ver, context.const_buffer_struct_layout_override, context.layout_resource, context.temporary_resource);
				Potato::IR::StructLayout::Member current_member;
				current_member.struct_layout = std::move(layout);
				current_member.array_count = array_count;
				D3D12_SHADER_VARIABLE_DESC var_desc;
				bool re = SUCCEEDED(ver->GetDesc(&var_desc));
				assert(re);
				current_member.name = reinterpret_cast<char8_t const*>(var_desc.Name);
				members.push_back(current_member);
			}
			cbuffer_layout = StructLayout::CreateDynamic(
				reinterpret_cast<char8_t const*>(buffer_desc.Name),
				std::span(members.data(), members.size()), GetHLSLConstBufferPolicy(),
				context.layout_resource
			);
		}

		if (cbuffer_layout)
		{
			// for_debug
			{
				auto mvs = cbuffer_layout->GetMemberView();
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

			auto added_index = shared_resource.AddResource(
				ShaderResourceType::CONST_BUFFER,
				cbuffer_name,
				{std::move(cbuffer_layout), array_count }
			);

			if (!added_index.has_value())
			{
				assert(false);
				return {};
			}

			ShaderSlotLocate locate;
			locate.context_index = *added_index;
			return locate;
		}
		return {};
	}

	bool InserShaderSlot(
		ShaderType type,
		ShaderResourceType resource_type,
		D3D12_SHADER_INPUT_BIND_DESC bind_sesc,
		ShaderSlot& out_slot,
		ShaderSharedResource& shared_resource,
		ShaderReflectionContext const& context
	)
	{
		std::u8string_view name = { reinterpret_cast<char8_t const*>(bind_sesc.Name) };

		auto finded_index = shared_resource.FindResource(resource_type, name);

		if (finded_index.has_value())
		{
			auto pro = shared_resource.GetResourceDescriptor(resource_type, *finded_index);
			if (pro == nullptr)
			{
				assert(false);
				return false;
			}
			if (pro->property.bind_count != bind_sesc.BindCount)
			{
				assert(false);
				return false;
			}

			ShaderSlotLocate slot;
			slot.context_index = *finded_index;
			out_slot.slots.emplace_back(
				type,
				resource_type,
				slot,
				bind_sesc.BindPoint, bind_sesc.Space
			);

			return true;
		}

		if (context.context_define_resource)
		{
			auto locate = context.context_define_resource(
				name,
				resource_type,
				bind_sesc.BindCount
			);
			if (locate)
			{
				out_slot.slots.emplace_back(
					type,
					resource_type,
					locate,
					bind_sesc.BindPoint, bind_sesc.Space
				);
				return true;
			}
		}

		auto added_index = shared_resource.AddResource(
			resource_type,
			name,
			{
				{},
				bind_sesc.BindCount
			}
		);

		if (!added_index)
		{
			assert(false);
			return false;
		}

		ShaderSlotLocate locate;
		locate.context_index = *added_index;

		out_slot.slots.emplace_back(
			type,
			resource_type,
			locate,
			bind_sesc.BindPoint, bind_sesc.Space
		);

		return true;
	}

	bool GetShaderSlot(
		ShaderType type,
		ID3D12ShaderReflection& reflection,
		ShaderSlot& out_slot,
		ShaderSharedResource& shared_resource,
		ShaderReflectionContext const& context
		)
	{

		D3D12_SHADER_DESC desc;
		if (!SUCCEEDED(reflection.GetDesc(&desc)))
			return false;

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
					auto locate = CreateLayoutFromCBuffer(*buffer, bind_sesc.BindCount, shared_resource, context);
					if (!locate)
						return false;
					out_slot.slots.emplace_back(
						type,
						ShaderResourceType::CONST_BUFFER,
						locate,
						bind_sesc.BindPoint, bind_sesc.Space
					);
				}
				break;
			}
				
			case D3D_SHADER_INPUT_TYPE::D3D_SIT_TEXTURE:
			{
				if (!InserShaderSlot(
					type,
					ShaderResourceType::TEXTURE,
					bind_sesc,
					out_slot,
					shared_resource,
					context
				))
				{
					assert(false);
					return false;
				}
				break;
			}
				
			case D3D_SHADER_INPUT_TYPE::D3D_SIT_SAMPLER:
			{
				if (!InserShaderSlot(
					type,
					ShaderResourceType::SAMPLER,
					bind_sesc,
					out_slot,
					shared_resource,
					context
				))
				{
					assert(false);
					return false;
				}
				break;
			}
			default:
				assert(false);
			}
		}

		return true;
	}
}
