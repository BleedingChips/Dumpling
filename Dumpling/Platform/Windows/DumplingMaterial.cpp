module;
#include <d3d12.h>
#include <cassert>

module DumplingMaterial;


namespace Dumpling
{
	using Potato::IR::StructLayout;

	DXGI_FORMAT Translate(StructLayout const& layout)
	{
		if (layout == *GetHLSLConstBufferStructLayout<CBFloat2>())
			return DXGI_FORMAT_R32G32_FLOAT;
		if (layout == *GetHLSLConstBufferStructLayout<CBFloat3>())
			return DXGI_FORMAT_R32G32B32_FLOAT;
		if (layout == *GetHLSLConstBufferStructLayout<CBFloat4>())
			return DXGI_FORMAT_R32G32B32A32_FLOAT;
		return DXGI_FORMAT_UNKNOWN;
	}

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
			target.Format = Translate(*view.struct_layout);
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

	ComPtr<ID3D12RootSignature> CreateRootSignature(ID3D12Device& device, ShaderStatistics const& statics)
	{
		D3D12_ROOT_SIGNATURE_DESC desc;
		desc.NumParameters = 0;
		desc.NumStaticSamplers = 0;
		desc.pParameters = nullptr;
		desc.pStaticSamplers = nullptr;
		desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

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
			psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
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
