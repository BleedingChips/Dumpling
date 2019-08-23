#pragma once
#include "pre_define_dxgi.h"
namespace Dumpling::Dxgi
{
	enum class FormatPixel
	{
		RGBA32_Float = DXGI_FORMAT_R32G32B32A32_FLOAT,
		RGB32_Float = DXGI_FORMAT_R32G32B32_FLOAT,
		RG32_Float = DXGI_FORMAT_R32G32_FLOAT,
		R32_Float = DXGI_FORMAT_R32_FLOAT,
		RGBA16_Float = DXGI_FORMAT_R16G16B16A16_FLOAT,
		RGBA16_Unorn = DXGI_FORMAT_R16G16B16A16_UNORM,
		RGBA8_Unorn = DXGI_FORMAT_R8G8B8A8_UNORM,
		R24G8_Typeless = DXGI_FORMAT_R24G8_TYPELESS,
		R24G8_Unorn_Typeless = DXGI_FORMAT_R24_UNORM_X8_TYPELESS,
		D24S8_Unorn_Uint = DXGI_FORMAT_D24_UNORM_S8_UINT,
		Unknown = DXGI_FORMAT_UNKNOWN,
	};

	inline constexpr DXGI_FORMAT operator*(Dumpling::Dxgi::FormatPixel format) noexcept
	{
		return static_cast<DXGI_FORMAT>(format);
	}

	inline constexpr Dumpling::Dxgi::FormatPixel operator*(DXGI_FORMAT format) noexcept
	{
		return static_cast<Dumpling::Dxgi::FormatPixel>(format);
	}

	


}

