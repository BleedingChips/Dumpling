#pragma once
#include "enum_dxgi.h"
#include "..//..//..//Potato/smart_pointer.h"
#include <tuple>
#include <vector>
#include <DirectXMath.h>
#pragma comment(lib, "dxgi.lib")
namespace Dumpling::Dxgi
{


	std::tuple<FactoryPtr, HRESULT> CreateFactory();
	std::vector<AdapterPtr> EnumAdapter(Factory*);
	std::vector<OutputPtr> EnumOutput(Adapter*);

	SwapChainDesc CreateDefaultSwapChainDesc(DXGI_FORMAT pixel_format, uint32_t width, uint32_t height, uint32_t buffer_count = 2);

	std::tuple<SwapChainPtr, HRESULT> CreateSwapChain(Factory* factory, IUnknown* device, HWND hwnd, const SwapChainDesc& desc, const DXGI_SWAP_CHAIN_FULLSCREEN_DESC* = nullptr, IDXGIOutput* output = nullptr);

	namespace DataType {
		using float2 = DirectX::XMFLOAT2;
		using float3 = DirectX::XMFLOAT3;
		using float4 = DirectX::XMFLOAT4;

		using int32_2 = DirectX::XMINT2;
		using int32_3 = DirectX::XMINT3;
		using int32_4 = DirectX::XMINT4;
		using uint32_2 = DirectX::XMUINT2;
		using uint32_3 = DirectX::XMUINT3;
		using uint32_4 = DirectX::XMUINT4;
	}

	template<typename Type>
	struct FormatTranslate {};

	template<>
	struct FormatTranslate<DataType::float4> {
		static constexpr FormatPixel format = FormatPixel::RGBA32_Float;
		static constexpr DXGI_FORMAT format_dx = static_cast<DXGI_FORMAT>(format);
		constexpr operator FormatPixel() const noexcept { return format; }
		constexpr operator DXGI_FORMAT() const noexcept { return format_dx; }
	};

	template<>
	struct FormatTranslate<DataType::float3> {
		static constexpr FormatPixel format = FormatPixel::RGB32_Float;
		static constexpr DXGI_FORMAT format_dx = static_cast<DXGI_FORMAT>(format);
		constexpr operator FormatPixel() const noexcept { return format; }
		constexpr operator DXGI_FORMAT() const noexcept { return format_dx; }
	};

	template<>
	struct FormatTranslate<DataType::float2> {
		static constexpr FormatPixel format = FormatPixel::RG32_Float;
		static constexpr DXGI_FORMAT format_dx = static_cast<DXGI_FORMAT>(format);
		constexpr operator FormatPixel() const noexcept { return format; }
		constexpr operator DXGI_FORMAT() const noexcept { return format_dx; }
	};

	template<>
	struct FormatTranslate<float> {
		static constexpr FormatPixel format = FormatPixel::R32_Float;
		static constexpr DXGI_FORMAT format_dx = static_cast<DXGI_FORMAT>(format);
		constexpr operator FormatPixel() const noexcept { return format; }
		constexpr operator DXGI_FORMAT() const noexcept { return format_dx; }
	};
}

