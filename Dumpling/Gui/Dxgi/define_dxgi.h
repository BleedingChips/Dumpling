#pragma once
#include "..//..//..//Potato/smart_pointer.h"
#include <tuple>
#include <vector>
#include <DirectXMath.h>
#include "..//Win32//aid.h"
#include <dxgi1_4.h>
#include <optional>
namespace Dumpling::Dxgi
{
	// Enum redefine **********************************************************************

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


	std::optional<size_t> CalculatePixelSize(DXGI_FORMAT);
	struct Texture2Size {
		Texture2Size(DXGI_FORMAT Format, size_t Width, size_t Height, size_t Mipmap = 0);
	};
	

	inline constexpr DXGI_FORMAT operator*(Dumpling::Dxgi::FormatPixel format) noexcept
	{
		return static_cast<DXGI_FORMAT>(format);
	}

	inline constexpr Dumpling::Dxgi::FormatPixel operator*(DXGI_FORMAT format) noexcept
	{
		return static_cast<Dumpling::Dxgi::FormatPixel>(format);
	}

	// Type Define **********************************************************************

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

		using float4x4 = DirectX::XMMATRIX;
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

	using Win32::ComPtr;
	template<typename T> using ComBase = Win32::ComBase<T>;
	using Win32::VoidT;

	using Factory = IDXGIFactory4;
	using FactoryPtr = ComPtr<Factory>;

	using Adapter = IDXGIAdapter1;
	using AdapterPtr = ComPtr<Adapter>;

	using Output = IDXGIOutput;
	using OutputPtr = ComPtr<Output>;

	using SwapChain = IDXGISwapChain1;
	using SwapChainDesc = DXGI_SWAP_CHAIN_DESC1;
	using SwapChainPtr = ComPtr<SwapChain>;

	// Base Function ********************************************************************************

	struct HardwareRenderers {
		static HardwareRenderers& Instance();
		uint8_t AdapterCount() const noexcept;
		Dxgi::Adapter* GetAdapter(uint8_t adapter_index) const noexcept;
		std::vector<OutputPtr> EnumOutput(uint8_t adapter_index) const noexcept;
		std::tuple<SwapChainPtr, HRESULT> CreateSwapChain(IUnknown* device, HWND hwnd, const SwapChainDesc& desc, const DXGI_SWAP_CHAIN_FULLSCREEN_DESC* = nullptr, IDXGIOutput* output = nullptr);
		std::optional<uint8_t> CalculateAdapter(LUID luid) const noexcept;
	private:
		HardwareRenderers();
		FactoryPtr m_Factory;
		std::vector<Dxgi::AdapterPtr> m_AllAdapter;
	};

	
	
}

