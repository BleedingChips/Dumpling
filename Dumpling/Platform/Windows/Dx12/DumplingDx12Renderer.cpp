module;

#include <cassert>
#include <d3d12.h>

#include <dxgi1_6.h>

module DumplingDx12Renderer;

namespace Dumpling::Dx12
{


	auto Context::Create(std::pmr::memory_resource* resource)
		-> Ptr
	{
		if(resource != nullptr)
		{
			ComPtr<IDXGIFactory7> rptr;

			UINT Flags = 0;

#ifndef NDEBUG
			Flags |= DXGI_CREATE_FACTORY_DEBUG;
#endif

			HRESULT re = CreateDXGIFactory2(Flags, __uuidof(decltype(rptr)::InterfaceType), reinterpret_cast<void**>(rptr.GetAddressOf()));
			if(SUCCEEDED(re))
			{
#ifndef NDEBUG
				{
					ComPtr<ID3D12Debug> Debug;
					D3D12GetDebugInterface(__uuidof(decltype(Debug)::InterfaceType), reinterpret_cast<void**>(Debug.GetAddressOf()));
					if (Debug)
					{
						Debug->EnableDebugLayer();
					}
				}
#endif
				auto record = Potato::IR::MemoryResourceRecord::Allocate<Context>(resource);
				if(record)
				{
					Context::Ptr cptr {
						new (record.Get()) Context {record, std::move(rptr)}
					};
					return cptr;
				}
			}
		}
		return {};
	}


	Context::Context(Potato::IR::MemoryResourceRecord record, ComPtr<IDXGIFactory7> factory)
		: record(record), dxgi_factory(std::move(factory))
	{
		assert(record && dxgi_factory);
	}

	void Context::Release()
	{
		auto res = record;
		this->~Context();
		res.Deallocate();
	}

	AdapterPtr Context::EnumAdapter(std::size_t index)
	{
		assert(dxgi_factory);
		AdapterPtr p_adapter = nullptr;
		auto re = dxgi_factory->EnumAdapters1(index, p_adapter.GetAddressOf());
		if(SUCCEEDED(re))
		{
			return p_adapter;
		}
		return {};
	}

	auto CommandQueue::Create(Potato::Pointer::IntrusivePtr<Device> dev, ComPtr<ID3D12CommandQueue> command_queue, std::pmr::memory_resource* resource) -> Ptr
	{
		if (dev && command_queue)
		{
			auto record = Potato::IR::MemoryResourceRecord::Allocate<CommandQueue>(resource);
			if (record)
			{
				Ptr ptr{ new (record.Get()) CommandQueue{} };
				ptr->record = record;
				ptr->owner = std::move(dev);
				ptr->command_queue = std::move(command_queue);
				return ptr;
			}
		}
		return {};
	}

	void CommandQueue::Release()
	{
		auto re = record;
		this->~CommandQueue();
		re.Deallocate();
	}

	auto RendererWrapper::Create(CommandQueue::Ptr queue, Context::Ptr context, std::pmr::memory_resource* resource) -> Ptr
	{
		if(queue && context)
		{
			auto record = Potato::IR::MemoryResourceRecord::Allocate<RendererWrapper>(resource);
			if(record)
			{
				Ptr ptr {
					new (record.Get()) RendererWrapper{}
				};
				ptr->record = record;
				ptr->context = std::move(context);
				ptr->queue = std::move(queue);
				return ptr;
			}
		}
		return {};
	}

	void RendererWrapper::OnInit(HWND hwnd)
	{
		DXGI_SWAP_CHAIN_DESC desc{
		};
	}

	void RendererWrapper::OnRelease(HWND)
	{
		
	}

	void RendererWrapper::OnUpdate()
	{
		
	}

	void RendererWrapper::ControllerRelease()
	{
		
	}

	void RendererWrapper::ViewerRelease()
	{
		auto re = record;
		this->~RendererWrapper();
		re.Deallocate();
	}

	CommandQueue::Ptr Device::CreateCommandQueue(std::pmr::memory_resource* resource)
	{
		if(resource != nullptr)
		{
			D3D12_COMMAND_QUEUE_DESC desc{
				D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT,
				D3D12_COMMAND_QUEUE_PRIORITY::D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
				D3D12_COMMAND_QUEUE_FLAGS::D3D12_COMMAND_QUEUE_FLAG_NONE,
				0
			};
			ComPtr<ID3D12CommandQueue> ptr;
			HRESULT Re = device->CreateCommandQueue(
				&desc, __uuidof(decltype(ptr)::InterfaceType), reinterpret_cast<void**>(ptr.GetAddressOf())
			);
			if(SUCCEEDED(Re))
			{
				return CommandQueue::Create(this, std::move(ptr), resource);
			}
		}
		return {};
	}

	auto Device::Create(AdapterPtr adapter, std::pmr::memory_resource* resource)
		-> Ptr
	{
		if(adapter && resource != nullptr)
		{
			ComPtr<ID3D12Device> dev_ptr;
			auto RE = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_1, __uuidof(decltype(dev_ptr)::InterfaceType), reinterpret_cast<void**>(dev_ptr.GetAddressOf()));
			if(SUCCEEDED(RE))
			{
				auto record = Potato::IR::MemoryResourceRecord::Allocate<Device>(resource);
				if(record)
				{
					auto ptr = new (record.Get()) Device{ record, std::move(dev_ptr) };
					Device::Ptr p { ptr };
					return p;
				}
			}
		}
		return {};
	}

	void Device::Release()
	{
		auto re = record;
		this->~Device();
		re.Deallocate();
	}
}
