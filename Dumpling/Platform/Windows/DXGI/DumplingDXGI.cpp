module;

#include <cassert>
#include <d3d12.h>
#include <d3dcommon.h>
#include <dxgi1_6.h>
#include <intsafe.h>

#undef interface

module DumplingDXGI;

import DumplingDx12Renderer;


namespace Dumpling
{
	/*
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

	bool HardDevice::InitDebugLayout()
	{
		static std::mutex debug_mutex;
		static Windows::ComPtr<ID3D12Debug> debug_layout;
		std::lock_guard lg(debug_mutex);
		if(!debug_layout)
		{
			D3D12GetDebugInterface(IID_PPV_ARGS(debug_layout.GetAddressOf()));
			if(debug_layout)
				debug_layout->EnableDebugLayer();
		}
		return debug_layout;
	}
	*/
}
