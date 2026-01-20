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

			std::memcpy(str_ite.data(), view.name.data(), view.name.size());
			str_ite[view.name.size()] = u8'\0';

			target.SemanticName = reinterpret_cast<char*>(str_ite.data());
			str_ite = str_ite.subspan(view.name.size() + 1);
			target.SemanticIndex = 0;
			target.Format = Translate(*view.struct_layout);
			assert(target.Format != DXGI_FORMAT_UNKNOWN);
			target.InputSlot = 0;
			target.AlignedByteOffset = view.member_layout.offset;
			target.InputSlotClass = D3D12_INPUT_CLASSIFICATION::D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
			target.InstanceDataStepRate = 0;
		}
		return mmv.size();
	}
}
