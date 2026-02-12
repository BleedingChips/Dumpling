module;
#include <d3d12.h>
#include <cassert>
#undef max

module DumplingDx12Material;

import DumplingMath;
import DumplingDxStructLayout;


namespace Dumpling::Dx12
{
	using Potato::IR::StructLayout;
	using namespace Dumpling::Dx;
	

	std::optional<std::size_t> CreateInputDescription(Potato::IR::StructLayout const& vertex_layout, std::span<D3D12_INPUT_ELEMENT_DESC> desc, std::span<char8_t> temporary_str)
	{
		auto mmv = vertex_layout.GetMemberView();
		if (mmv.size() > desc.size())
			return std::nullopt;

		std::size_t str_require = 0;

		for (auto& view : mmv)
		{
			str_require += view.name.size() + 1;
		}

		if (str_require > temporary_str.size())
			return false;

		auto str_ite = temporary_str;

		for (std::size_t i = 0; i < mmv.size(); ++i)
		{
			auto& view = mmv[i];
			auto& target = desc[i];

			std::size_t semantic_index = 0;
			for (std::size_t i2 = 0; i2 < i; ++i2)
			{
				if (mmv[i2].name == view.name)
				{
					semantic_index += 1;
				}
			}

			std::memcpy(str_ite.data(), view.name.data(), view.name.size());
			str_ite[view.name.size()] = u8'\0';

			target.SemanticName = reinterpret_cast<char*>(str_ite.data());
			str_ite = str_ite.subspan(view.name.size() + 1);
			target.SemanticIndex = semantic_index;
			target.Format = GetDXGIFormat(*view.struct_layout);
			assert(target.Format != DXGI_FORMAT_UNKNOWN);
			target.InputSlot = 0;
			target.AlignedByteOffset = view.member_layout.offset;
			target.InputSlotClass = D3D12_INPUT_CLASSIFICATION::D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
			target.InstanceDataStepRate = 0;
		}
		return mmv.size();
	}

	D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView(std::size_t buffer_size, Potato::MemLayout::ArrayLayout array_layout, ID3D12Resource& vertex_resource, std::size_t offset)
	{
		D3D12_VERTEX_BUFFER_VIEW view;
		view.BufferLocation = vertex_resource.GetGPUVirtualAddress() + offset;
		view.SizeInBytes = buffer_size;
		view.StrideInBytes = array_layout.each_element_offset;
		return view;
	}

	D3D12_INDEX_BUFFER_VIEW GetIndexBufferView(ID3D12Resource& index_resource, std::size_t index_size, std::size_t offset)
	{
		D3D12_INDEX_BUFFER_VIEW view;
		view.BufferLocation = index_resource.GetGPUVirtualAddress() + offset;
		view.SizeInBytes = sizeof(std::uint32_t) * index_size;
		view.Format = DXGI_FORMAT_R32_UINT;
		return view;
	}

	std::optional<ShaderDefineDescriptorTable> CreateDescriptorHeap(ID3D12Device& device, ShaderSharedResource const& shared_resource)
	{
		ShaderDefineDescriptorTable result;

		if (shared_resource.descriptor_table.size() > 0)
		{
			D3D12_DESCRIPTOR_HEAP_DESC desc{
				D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
				static_cast<UINT>(shared_resource.descriptor_table.size()),
				D3D12_DESCRIPTOR_HEAP_FLAGS::D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
				1
			};

			auto re = device.CreateDescriptorHeap(
				&desc, __uuidof(decltype(result.resource_heap)::Type), result.resource_heap.GetPointerVoidAdress()
			);

			if (!SUCCEEDED(re))
			{
				return std::nullopt;
			}
		}

		if (shared_resource.sampler_descriptor_table.size() > 0)
		{
			D3D12_DESCRIPTOR_HEAP_DESC desc{
				D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
				static_cast<UINT>(shared_resource.sampler_descriptor_table.size()),
				D3D12_DESCRIPTOR_HEAP_FLAGS::D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
				1
			};

			auto re = device.CreateDescriptorHeap(
				&desc, __uuidof(decltype(result.sampler_heap)::Type), result.sampler_heap.GetPointerVoidAdress()
			);

			if (!SUCCEEDED(re))
			{
				return std::nullopt;
			}
		}

		return result;
	}

	bool ShaderDefineDescriptorTable::CreateConstBufferView(ID3D12Device& device, ShaderSharedResource const& shared_resource, std::size_t resource_index, ID3D12Resource& resource, Potato::Misc::IndexSpan<> span)
	{
		auto descriptor_offset = device.GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		if (resource_heap && shared_resource.descriptor_table.size() > resource_index)
		{
			auto& target_info = shared_resource.descriptor_table[resource_index];
			if (target_info.type != ShaderResourceType::CONST_BUFFER)
				return false;

			D3D12_CONSTANT_BUFFER_VIEW_DESC cbv_desc;
			cbv_desc.BufferLocation = resource.GetGPUVirtualAddress() + span.Begin();
			cbv_desc.SizeInBytes = span.Size();
			device.CreateConstantBufferView(
				&cbv_desc,
				D3D12_CPU_DESCRIPTOR_HANDLE{ resource_heap->GetCPUDescriptorHandleForHeapStart().ptr + target_info.resource_index * descriptor_offset }
			);
			return true;
			
		}
		return false;
	}

	ComPtr<ID3D12RootSignature> CreateRootSignature(
		ID3D12Device& device,
		ShaderSlot const& shader_slot,
		DescriptorTableMapping& descriptor_table_mapping,
		Potato::TMP::FunctionRef<ContextDefinedDescriptorTable(ShaderSlotLocate)> context_defined_descriptor_mapping
	)
	{
		struct ShaderResourceDescriptorInfo
		{
			ShaderResourceType resource_type;
			ContextDefinedDescriptorTable context_defined_table;
		};

		std::array<ShaderResourceDescriptorInfo, 128> shader_resource_descriptor_info;
		std::size_t shader_resource_descriptor_info_count = 0;

		auto total_resource_size = shader_slot.slots.size();

		if (total_resource_size >= shader_resource_descriptor_info.size())
		{
			assert(false);
			return {};
		}

		{
			auto heap_incread_size = device.GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			auto heap_sampler_incread_size = device.GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

			for (auto& ite : shader_slot.slots)
			{
				auto& tar = shader_resource_descriptor_info[shader_resource_descriptor_info_count];
				tar.resource_type = ite.resource_type;
				shader_resource_descriptor_info_count += 1;
				if (ite.located.IsContextDefine())
				{
					tar.context_defined_table = context_defined_descriptor_mapping(ite.located);
					assert(tar.context_defined_table);
					if (!tar.context_defined_table)
						return {};
				}
				else {
					switch (ite.resource_type)
					{
					case ShaderResourceType::CONST_BUFFER:
					case ShaderResourceType::TEXTURE:
					case ShaderResourceType::UNORDER_ACCED:
					{
						auto descriptor_offset = ite.located.context_index * heap_incread_size;
						tar.context_defined_table = { D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, std::numeric_limits<std::size_t>::max(), descriptor_offset };
						break;
					}
					case ShaderResourceType::SAMPLER:
					{
						auto descriptor_offset = ite.located.context_index * heap_sampler_incread_size;
						tar.context_defined_table = { D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, std::numeric_limits<std::size_t>::max(), descriptor_offset };
						break;
					}
					default:
						assert(false);
						return {};
					}
				}
			}
			
		}

		assert(shader_resource_descriptor_info_count == total_resource_size);

		struct ShaderSlotDescriptorInfo
		{
			D3D12_SHADER_VISIBILITY visibility;
			ContextDefinedDescriptorTable descriptor;
			std::size_t shader_resource_descriptor_info_index;
		};

		std::array<ShaderSlotDescriptorInfo, 128> shader_slot_descriptor_info;
		std::size_t shader_slot_descriptor_info_count = 0;
		bool need_input = true;
		std::size_t index = 0;
		for (auto& ite : shader_slot.slots)
		{
			auto& tar = shader_slot_descriptor_info[shader_slot_descriptor_info_count++];
			switch (ite.shader_type)
			{
			case ShaderType::VS:
				tar.visibility = D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_VERTEX;
				break;
			case ShaderType::PS:
				tar.visibility = D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_PIXEL;
				break;
			default:
				assert(false);
				return {};
			}
			tar.descriptor = shader_resource_descriptor_info[index].context_defined_table;
			tar.shader_resource_descriptor_info_index = index;
			++index;
		}

		std::sort(
			shader_slot_descriptor_info.begin(),
			shader_slot_descriptor_info.begin() + shader_slot_descriptor_info_count,
			[](ShaderSlotDescriptorInfo const& info1, ShaderSlotDescriptorInfo const& info2) {
				auto order = static_cast<std::size_t>(info1.visibility) <=> static_cast<std::size_t>(info2.visibility);
				if (order != std::strong_ordering::equal)
					return order == std::strong_ordering::less;
				order = static_cast<std::size_t>(info1.descriptor.type) <=> static_cast<std::size_t>(info2.descriptor.type);
				if (order != std::strong_ordering::equal)
					return order == std::strong_ordering::less;
				order = static_cast<std::size_t>(info1.descriptor.identity) <=> static_cast<std::size_t>(info2.descriptor.identity);
				if (order != std::strong_ordering::equal)
					return order == std::strong_ordering::less;
				order = static_cast<std::size_t>(info1.descriptor.descriptor_table_offset) <=> static_cast<std::size_t>(info2.descriptor.descriptor_table_offset);
				return order == std::strong_ordering::less;
			}
		);

		std::array<D3D12_ROOT_PARAMETER, 32> parameters;
		std::size_t parameters_index = 0;
		std::array<D3D12_DESCRIPTOR_RANGE, 256> ranges;
		Potato::Misc::IndexSpan<> ranges_span;

		D3D12_DESCRIPTOR_HEAP_TYPE last_type = D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES;
		D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_ALL;
		std::size_t heap_identity = std::numeric_limits<std::size_t>::max();

		for (auto& ite : std::span(shader_slot_descriptor_info).subspan(0, shader_slot_descriptor_info_count))
		{
			if (
				visibility != ite.visibility
				|| last_type != ite.descriptor.type
				|| heap_identity != ite.descriptor.identity
				)
			{
				if (last_type != D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES)
					++parameters_index;
				visibility = ite.visibility;
				last_type = ite.descriptor.type;
				heap_identity = ite.descriptor.identity;
				auto& cur = parameters[parameters_index];
				Potato::Misc::IndexSpan<> next_span = { ranges_span.End(), ranges_span.End() };
				ranges_span = next_span;
				cur.ShaderVisibility = visibility;
				cur.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
				cur.DescriptorTable = {
					1,
					ranges.data() + next_span.Begin()
				};
				descriptor_table_mapping.mappings.emplace_back(last_type, ite.descriptor.identity);
			}
			else {
				auto& cur = parameters[parameters_index];
				cur.DescriptorTable.NumDescriptorRanges += 1;
			}
			auto& tar_slot = shader_slot.slots[ite.shader_resource_descriptor_info_index];
			auto& tar_resource = shader_resource_descriptor_info[ite.shader_resource_descriptor_info_index];
			auto& tar_range = ranges[ranges_span.End()];
			switch (tar_resource.resource_type)
			{
			case ShaderResourceType::CONST_BUFFER:
				tar_range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
				break;
			case ShaderResourceType::SAMPLER:
				tar_range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
				break;
			case ShaderResourceType::TEXTURE:
				tar_range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
				break;
			case ShaderResourceType::UNORDER_ACCED:
				tar_range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
				break;
			default:
				assert(false);
			}
			tar_range.OffsetInDescriptorsFromTableStart = tar_resource.context_defined_table.descriptor_table_offset;
			tar_range.RegisterSpace = tar_slot.space_index;
			tar_range.BaseShaderRegister = tar_slot.register_index;
			tar_range.NumDescriptors = 1;
			ranges_span.BackwardEnd(1);
		}

		D3D12_ROOT_SIGNATURE_DESC desc;
		desc.NumParameters = parameters_index + 1;
		desc.NumStaticSamplers = 0;
		desc.pParameters = parameters.data();
		desc.pStaticSamplers = nullptr;

		if (need_input)
		{
			desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
		}

		ComPtr<ID3D10Blob> root_signature_code;
		ComPtr<ID3D10Blob> error_code;

		auto result = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, root_signature_code.GetPointerAdress(), error_code.GetPointerAdress());

		if (!SUCCEEDED(result))
			return {};

		ComPtr<ID3D12RootSignature> root_signature;

		auto re = device.CreateRootSignature(1,
			root_signature_code->GetBufferPointer(),
			root_signature_code->GetBufferSize(),
			__uuidof(decltype(root_signature)::Type),
			root_signature.GetPointerVoidAdress()
		);

		if (!SUCCEEDED(re))
			return {};

		return root_signature;
	}

	ComPtr<ID3D12PipelineState> CreatePipelineState(ID3D12Device& device, ID3D12RootSignature& root_signature, MaterialState const& material_state)
	{
		std::array<char8_t, 1024> temp_str;
		std::array<D3D12_INPUT_ELEMENT_DESC, 128> input_element;

		auto size = CreateInputDescription(*material_state.vs_layout, std::span(input_element), std::span(temp_str));

		if (!size.has_value())
			return {};

		// Describe and create the graphics pipeline state object (PSO).
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.InputLayout = { input_element.data(), static_cast<UINT32>(*size)};
		psoDesc.pRootSignature = &root_signature;
		psoDesc.VS = {
			material_state.vs_shader->GetBufferPointer(),
			material_state.vs_shader->GetBufferSize()
		};
		psoDesc.PS = {
			material_state.ps_shader->GetBufferPointer(),
			material_state.ps_shader->GetBufferSize()
		};
		{
			psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
			psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
			psoDesc.RasterizerState.FrontCounterClockwise = FALSE;
			psoDesc.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
			psoDesc.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
			psoDesc.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
			psoDesc.RasterizerState.DepthClipEnable = TRUE;
			psoDesc.RasterizerState.MultisampleEnable = FALSE;
			psoDesc.RasterizerState.AntialiasedLineEnable = FALSE;
			psoDesc.RasterizerState.ForcedSampleCount = 0;
			psoDesc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
		}
		{
			psoDesc.BlendState.AlphaToCoverageEnable = false;
			psoDesc.BlendState.IndependentBlendEnable = false;
			psoDesc.BlendState.RenderTarget[0] = {
				false,
				false,
				D3D12_BLEND_ONE,
				D3D12_BLEND_ZERO,
				D3D12_BLEND_OP_ADD,
				D3D12_BLEND_ONE,
				D3D12_BLEND_ZERO,
				D3D12_BLEND_OP_ADD,
				D3D12_LOGIC_OP_NOOP,
				D3D12_COLOR_WRITE_ENABLE_ALL
			};
		}
		

		psoDesc.DepthStencilState.DepthEnable = FALSE;
		psoDesc.DepthStencilState.StencilEnable = FALSE;
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.SampleDesc.Count = 1;

		ComPtr<ID3D12PipelineState> pipeline_state;

		auto re = device.CreateGraphicsPipelineState(
			&psoDesc,
			__uuidof(decltype(pipeline_state)::Type),
			pipeline_state.GetPointerVoidAdress()
		);

		if (!SUCCEEDED(re))
			return {};

		return pipeline_state;
	}
}
