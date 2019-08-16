#include "define_dxgi.h"
#include <assert.h>
#include <comdef.h>
namespace Dumpling::Dxgi
{
	std::tuple<FactoryPtr, HRESULT> CreateFactory()
	{
		FactoryPtr result;
		HRESULT re = CreateDXGIFactory(__uuidof(Factory), result(void_t{}));
		return { std::move(result), re };
	}

	std::vector<AdapterPtr> EnumAdapter(Factory* ptr)
	{
		assert(ptr);
		uint32_t index = 0;
		AdapterPtr tem;
		std::vector<AdapterPtr> result;
		while (ptr->EnumAdapters1(index++, tem()) != DXGI_ERROR_NOT_FOUND)
			result.push_back(std::move(tem));
		return std::move(result);
	}

	std::vector<OutputPtr> EnumOutput(Adapter* ptr)
	{
		assert(ptr != nullptr);
		uint32_t index = 0;
		OutputPtr tem;
		std::vector<OutputPtr> result;
		while (ptr->EnumOutputs(index++, tem()) != DXGI_ERROR_NOT_FOUND)
			result.push_back(std::move(tem));
		return std::move(result);
	}

	SwapChainDesc CreateDefaultSwapChainDesc(DXGI_FORMAT pixel_format, uint32_t width, uint32_t height, uint32_t buffer_count)
	{
		return SwapChainDesc{ width, height, pixel_format, false, DXGI_SAMPLE_DESC{1, 0},  DXGI_USAGE_RENDER_TARGET_OUTPUT, buffer_count, 
			DXGI_SCALING_STRETCH, DXGI_SWAP_EFFECT_FLIP_DISCARD, DXGI_ALPHA_MODE_UNSPECIFIED, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH };
	}

	std::tuple<SwapChainPtr, HRESULT> CreateSwapChain(Factory* factory, IUnknown* device, HWND hwnd, const SwapChainDesc& desc, const DXGI_SWAP_CHAIN_FULLSCREEN_DESC* fullscreen_desc, IDXGIOutput* output)
	{
		assert(factory != nullptr && device != nullptr);
		SwapChainPtr tem;
		HRESULT re = factory->CreateSwapChainForHwnd(device, hwnd, &desc, fullscreen_desc, output, tem());
		return {std::move(tem), re};
	}
}