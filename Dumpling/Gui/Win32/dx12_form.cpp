#include "dx12_form.h"
namespace Dumpling::Dx12
{
	const FormStyle& Default() noexcept {
		static FormStyle Tem;
		return Tem;
	}

	Form::Form(CommandQueue& Queue, const FormSetting& Setting, const FormStyle& Style)
		: Win32::Form(Setting.Win32Setting, Style.Win32Style), m_BackBufferIndex(0)
	{
		auto& WS = Setting.Win32Setting;
		Dxgi::SwapChainDesc ChainDest{
			WS.Width, WS.Height, *Setting.Pixel, false, DXGI_SAMPLE_DESC{1, 0}, DXGI_USAGE_RENDER_TARGET_OUTPUT, 2,
			DXGI_SCALING_STRETCH, DXGI_SWAP_EFFECT_FLIP_DISCARD, DXGI_ALPHA_MODE_UNSPECIFIED, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH
		};
		m_SwapChain = Win32::ThrowIfFault(Dxgi::HardwareRenderers::Instance().CreateSwapChain(&Queue, GetHWnd(), ChainDest));
		m_MaxBufferCount = ChainDest.BufferCount;
		m_AllBackBuffer.resize(m_MaxBufferCount);
		for (uint8_t i = 0; i < m_MaxBufferCount; ++i)
		{
			HRESULT re;
			std::tie(m_AllBackBuffer[i], re) = GetBackBuffer(i);
			assert(SUCCEEDED(re));
		}
	}

	std::tuple<ResourcePtr, HRESULT> Form::GetBackBuffer(uint8_t index) noexcept
	{
		ResourcePtr res;
		HRESULT re = m_SwapChain->GetBuffer(index, __uuidof(Resource), res(VoidT{}));
		return { std::move(res), re };
	}

	void Form::PresentAndSwap() noexcept {
		DXGI_PRESENT_PARAMETERS Para{ 0, nullptr  , nullptr , nullptr };
		m_SwapChain->Present1(0, 0, &Para);
		m_BackBufferIndex = (m_BackBufferIndex + 1) % m_MaxBufferCount;
	}

	FormPtr Form::Create(CommandQueue& Queue, const FormSetting& Setting, const FormStyle& Style)
	{
		return new Form{ Queue, Setting, Style };
	}
}