#include "define_dxgi.h"
#include <assert.h>
#include <comdef.h>
#undef max
namespace Dumpling::Dxgi
{
	uint8_t HardwareRenderers::AdapterCount() const noexcept { 
		assert(m_AllAdapter.size() <= std::numeric_limits<uint8_t>::max());
		return static_cast<uint8_t>(m_AllAdapter.size()); 
	}
	Dxgi::Adapter* HardwareRenderers::GetAdapter(uint8_t index) const noexcept {
		assert(index < AdapterCount());
		return m_AllAdapter[index];
	}

	std::vector<OutputPtr> HardwareRenderers::EnumOutput(uint8_t adapter_index) const noexcept {
		auto Adapter = GetAdapter(adapter_index);
		uint32_t index = 0;
		OutputPtr tem;
		std::vector<OutputPtr> result;
		while (Adapter->EnumOutputs(index++, tem()) != DXGI_ERROR_NOT_FOUND)
			result.push_back(std::move(tem));
		return std::move(result);
	}

	HardwareRenderers::HardwareRenderers() {
		HRESULT re = CreateDXGIFactory(__uuidof(Factory), m_Factory(VoidT{}));
		assert(SUCCEEDED(re));
		uint32_t index = 0;
		AdapterPtr tem;
		while (m_Factory->EnumAdapters1(index++, tem()) != DXGI_ERROR_NOT_FOUND)
			m_AllAdapter.push_back(std::move(tem));
	}

	HardwareRenderers& HardwareRenderers::Instance() {
		static HardwareRenderers instance;
		return instance;
	}

	SwapChainDesc CreateDefaultSwapChainDesc(DXGI_FORMAT pixel_format, uint32_t width, uint32_t height, uint32_t buffer_count)
	{
		return SwapChainDesc{ width, height, pixel_format, false, DXGI_SAMPLE_DESC{1, 0},  DXGI_USAGE_RENDER_TARGET_OUTPUT, buffer_count, 
			DXGI_SCALING_STRETCH, DXGI_SWAP_EFFECT_FLIP_DISCARD, DXGI_ALPHA_MODE_UNSPECIFIED, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH };
	}

	std::tuple<SwapChainPtr, HRESULT> HardwareRenderers::CreateSwapChain(IUnknown* device, HWND hwnd, const SwapChainDesc& desc, const DXGI_SWAP_CHAIN_FULLSCREEN_DESC* fullscreen_desc, IDXGIOutput* output)
	{
		assert(m_Factory);
		assert(device != nullptr);
		SwapChainPtr tem;
		HRESULT re = m_Factory->CreateSwapChainForHwnd(device, hwnd, &desc, fullscreen_desc, output, tem());
		return {std::move(tem), re};
	}
}