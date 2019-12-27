#pragma once
#include "define_dx12.h"
#include "../Win32/form.h"
namespace Dumpling::Dx12
{
	struct FormStyle
	{
		Win32::FormStyle Win32Style;
	};

	const FormStyle& Default() noexcept;

	struct FormSetting {
		Win32::FormSetting Win32Setting;
		Dxgi::FormatPixel Pixel = Dxgi::FormatPixel::RGBA16_Float;
	};

	struct Form;
	using FormPtr = ComPtr<Form>;

	struct Form : Win32::Form
	{
		static FormPtr Create(CommandQueue& Queue, const FormSetting& Setting = FormSetting{}, const FormStyle& Style = Default());
		ResourcePtr CurrentBackBuffer() const noexcept { return m_AllBackBuffer[m_BackBufferIndex]; }
		void PresentAndSwap() noexcept;
		uint8_t CurrentBackBufferIndex() const noexcept { return m_BackBufferIndex; }
		std::tuple<ResourcePtr, HRESULT> GetBackBuffer(uint8_t index) noexcept;
	private:
		Form(CommandQueue& Queue, const FormSetting&, const FormStyle&);
		Dxgi::SwapChainPtr m_SwapChain;
		std::vector<ResourcePtr> m_AllBackBuffer;
		ComPtr<DescriptorHeap> m_Heap;
		uint8_t m_BackBufferIndex;
		uint8_t m_MaxBufferCount;
	};

	inline FormPtr CreateForm(CommandQueue& Queue, const FormSetting& Setting = FormSetting{}, const FormStyle& Style = Default()) {
		return Form::Create(Queue, Setting, Style);
	}
}