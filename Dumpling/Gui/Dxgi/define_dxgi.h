#pragma once
#include <dxgi1_4.h>
#include "..//..//..//Potato/smart_pointer.h"
#include <tuple>
#include <vector>
#include <DirectXMath.h>
#pragma comment(lib, "dxgi.lib")
namespace Dumpling::Dxgi
{
	template<typename Type> using intrusive_ptr = Potato::Tool::intrusive_ptr<Type>;

	struct void_t {};

	struct ComWrapper
	{
		template<typename T> static void add_ref(T* com) { com->AddRef(); }
		template<typename T> static void sub_ref(T* com) { com->Release(); }

		template<typename SourceType>
		SourceType** operator ()(SourceType*& pi)
		{
			if (pi != nullptr)
			{
				sub_ref(pi);
				pi = nullptr;
			}
			return &pi;
		}

		template<typename SourceType>
		void** operator ()(SourceType*& pi, void_t)
		{
			return reinterpret_cast<void**>(this->operator()(pi));
		}
	};

	template<typename Type> using ComPtr = Potato::Tool::intrusive_ptr<Type, ComWrapper>;

	using Factory = IDXGIFactory4;
	using FactoryPtr = ComPtr<Factory>;
	using Adapter = IDXGIAdapter1;
	using AdapterPtr = ComPtr<Adapter>;
	using Output = IDXGIOutput;
	using OutputPtr = ComPtr<Output>;

	std::tuple<FactoryPtr, HRESULT> CreateFactory();
	std::vector<AdapterPtr> EnumAdapter(Factory*);
	std::vector<OutputPtr> EnumOutput(Adapter*);

	using SwapChain = IDXGISwapChain1;
	using SwapChainPtr = ComPtr<SwapChain>;
	using SwapChainDesc = DXGI_SWAP_CHAIN_DESC1;

	SwapChainDesc CreateDefaultSwapChainDesc(DXGI_FORMAT pixel_format, uint32_t width, uint32_t height, uint32_t buffer_count = 2);

	std::tuple<SwapChainPtr, HRESULT> CreateSwapChain(Factory* factory, IUnknown* device, HWND hwnd, const SwapChainDesc& desc, const DXGI_SWAP_CHAIN_FULLSCREEN_DESC* = nullptr, IDXGIOutput* output = nullptr);
}

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

	inline constexpr DXGI_FORMAT cast(FormatPixel format) noexcept { return static_cast<DXGI_FORMAT>(format); }

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
	struct FormatTranslate<float4> {
		static constexpr FormatPixel format = FormatPixel::RGBA32_Float;
		static constexpr DXGI_FORMAT format_dx = static_cast<DXGI_FORMAT>(format);
		constexpr operator FormatPixel() const noexcept { return format; }
		constexpr operator DXGI_FORMAT() const noexcept { return format_dx; }
	};

	template<>
	struct FormatTranslate<float3> {
		static constexpr FormatPixel format = FormatPixel::RGB32_Float;
		static constexpr DXGI_FORMAT format_dx = static_cast<DXGI_FORMAT>(format);
		constexpr operator FormatPixel() const noexcept { return format; }
		constexpr operator DXGI_FORMAT() const noexcept { return format_dx; }
	};

	template<>
	struct FormatTranslate<float2> {
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