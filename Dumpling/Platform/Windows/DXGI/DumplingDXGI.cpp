module;

#include <cassert>
#include <d3d12.h>
#include <d3dcommon.h>
#include <dxgi1_6.h>
#include <intsafe.h>

#undef interface

module DumplingDXGI;

import DumplingDx12Renderer;

namespace Dumpling::DXGI
{
	auto HardDevice::Create(std::pmr::memory_resource* resource) -> Dumpling::HardDevice::Ptr
	{
		auto re = Potato::IR::MemoryResourceRecord::Allocate<HardDevice>(resource);
		if(re)
		{
			FactoryPtr rptr;
			UINT Flags = 0;
			Flags |= DXGI_CREATE_FACTORY_DEBUG;
			HRESULT result = CreateDXGIFactory2(Flags, __uuidof(decltype(rptr)::InterfaceType), reinterpret_cast<void**>(rptr.GetAddressOf()));
			if(SUCCEEDED(result))
			{
				return new(re.Get()) HardDevice{re, std::move(rptr)};
			}else
			{
				re.Deallocate();
			}
		}
		return {};
	}

	void HardDevice::Release()
	{
		auto re = record;
		this->~HardDevice();
		re.Deallocate();
	}

	std::optional<AdapterDescription> HardDevice::EnumAdapter(std::size_t ite) const
	{
		return {};
	}

	RendererFormWrapper::Ptr HardDevice::CreateFormWrapper(Form& form, Renderer& renderer, FormRenderTargetProperty property, std::pmr::memory_resource* resource)
	{
		auto tar = dynamic_cast<Windows::Win32Form*>(&form);
		auto ren = dynamic_cast<DXGIRenderer*>(&renderer);
		if(tar != nullptr && ren != nullptr)
		{
			auto hwnd = tar->GetWnd();
			if(hwnd != nullptr)
			{
				ComPtr<IDXGIFactory2> new_factor;
				factory->QueryInterface(__uuidof(decltype(new_factor)::InterfaceType), reinterpret_cast<void**>(new_factor.GetAddressOf()));
				if(new_factor)
				{

					DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
					swapChainDesc.BufferCount = 2;
					swapChainDesc.Width = 1024;
					swapChainDesc.Height = 768;
					swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
					swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
					swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
					swapChainDesc.SampleDesc.Count = 1;

					ComPtr<IDXGISwapChain1> swapChain;
					auto re = factory->CreateSwapChainForHwnd(
						ren->GetDevice(), hwnd, &swapChainDesc, nullptr, nullptr,
						swapChain.GetAddressOf()
					);
					if(SUCCEEDED(re))
					{
						return ren->CreateFormWrapper(std::move(swapChain), resource);
					}
				}
			}
		}
		return {};
	}

	Dumpling::Renderer::Ptr HardDevice::CreateRenderer(std::optional<std::size_t> adapter_count, std::pmr::memory_resource* resource)
	{
		if(factory)
		{

			ComPtr<IDXGIAdapter> adapter;
			if(adapter_count.has_value())
			{
				auto re = factory->EnumAdapters(*adapter_count, adapter.GetAddressOf());
				if(!SUCCEEDED(re))
				{
					return {};
				}
			}
			return Dx12::Renderer::Create(adapter.Get(), resource);
		}
		return {};
	}
}
