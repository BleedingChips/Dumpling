module;

#include "dxgi.h"

module DumplingDxStructLayout;

import std;
import Potato;
import DumplingMathVector;

namespace Dumpling::Dx
{
	using Potato::IR::StructLayout;
	using namespace Dumpling::Math;

	DXGI_FORMAT GetDXGIFormat(StructLayout const& layout)
	{
		static std::array<std::type_index, 6> type_index_list = {
			typeid(float),
			typeid(Float1),
			typeid(Float2),
			typeid(Float3),
			typeid(Float4),
			typeid(std::uint32_t)
		};

		std::array<DXGI_FORMAT, 6> format_index = {
			DXGI_FORMAT_R32_FLOAT,
			DXGI_FORMAT_R32_FLOAT,
			DXGI_FORMAT_R32G32_FLOAT,
			DXGI_FORMAT_R32G32B32_FLOAT,
			DXGI_FORMAT_R32G32B32A32_FLOAT,
			DXGI_FORMAT_R32_UINT
		};

		auto index = layout.LocateNativeType(std::span(type_index_list));

		if (index.has_value())
			return format_index[*index];

		return DXGI_FORMAT_UNKNOWN;
	}
}