module;

#include <cassert>
#include <d3d12.h>

#include <dxgi1_6.h>
#include <intsafe.h>
#include <OCIdl.h>
#include <OCIdl.h>
#include <OCIdl.h>
#include <OCIdl.h>
#include <OCIdl.h>

#undef interface

module DumplingDx12Renderer;


namespace Dumpling::Dx12
{
	/*
	auto HardDevice::Create(std::pmr::memory_resource* resource) -> Dumpling::HardDevice::Ptr
	{
		auto re = Potato::IR::MemoryResourceRecord::Allocate<HardDevice>(resource);
		if(re)
		{
			Dx12FactoryPtr rptr;
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

	Dumpling::Renderer::Ptr HardDevice::CreateRenderer(std::optional<std::size_t> adapter_count, std::pmr::memory_resource* resource)
	{
		if(factory)
		{
			auto record = Potato::IR::MemoryResourceRecord::Allocate<Renderer>(resource);
			if(record)
			{
				ComPtr<IDXGIAdapter> adapter;
				if(adapter_count.has_value())
				{
					auto re = factory->EnumAdapters(*adapter_count, adapter.GetAddressOf());
					if(!SUCCEEDED(re))
					{
						record.Deallocate();
						return {};
					}
				}
				Dx12DevicePtr dev_ptr;
				auto  re = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_1, __uuidof(decltype(dev_ptr)::InterfaceType), reinterpret_cast<void**>(dev_ptr.GetAddressOf()));
				if(SUCCEEDED(re))
				{
					Dx12CommandQueuePtr command_queue;
					D3D12_COMMAND_QUEUE_DESC desc{
						D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT,
						D3D12_COMMAND_QUEUE_PRIORITY::D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
						D3D12_COMMAND_QUEUE_FLAGS::D3D12_COMMAND_QUEUE_FLAG_NONE,
					0
					};

					auto re = dev_ptr->CreateCommandQueue(
						&desc, __uuidof(decltype(command_queue)::InterfaceType), reinterpret_cast<void**>(command_queue.GetAddressOf())
					);

					if(SUCCEEDED(re))
					{
						return new (record.Get()) Renderer{record, factory, std::move(dev_ptr), std::move(command_queue)};
					}
				}
				record.Deallocate();
			}
		}
		return {};
	}

	Dx12SwapChainPtr Renderer::CreateSwapChain(FormRenderTargetProperty property, HWND hwnd)
	{
		Dx12SwapChainPtr result;
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
		swapChainDesc.BufferCount = 2;
		swapChainDesc.Width = 1024;
		swapChainDesc.Height = 768;
		swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.SampleDesc.Count = 1;

		auto re2 = factory->CreateSwapChainForHwnd(
			direct_queue.Get(), hwnd, &swapChainDesc, nullptr, nullptr,
			result.GetAddressOf()
		);
		return result;
	}

	void Renderer::Release()
	{
		auto re = record;
		this->~Renderer();
		re.Deallocate();
	}
	*/
	/*
	CommandQueue::Ptr Renderer::GetCommandQueue(std::thread::id thread_id)
	{
		std::lock_guard lg(mutex);
		for (auto& ite : commands)
		{
			if (ite.thread_id == thread_id)
			{
				return ite.queue;
			}
		}
		{
			

			ComPtr<ID3D12CommandQueue> que_ptr2;

			auto re2 = device->CreateCommandQueue(
				&desc, __uuidof(decltype(que_ptr2)::InterfaceType), reinterpret_cast<void**>(que_ptr2.GetAddressOf())
			);

			//que_ptr->ExecuteCommandLists();

			if (SUCCEEDED(re))
			{
				//return { std::move(que_ptr) };
			}
		}
		return {};
	}
	*/

	/*
	Dumpling::FormRenderTarget::Ptr Renderer::CreateFormRenderTarget(std::optional<RendererSocket> socket, FormRenderTargetProperty property, std::pmr::memory_resource* resource)
	{
		auto re = Potato::IR::MemoryResourceRecord::Allocate<FormRenderTarget>(resource);

		if(re)
		{
			RendererSocket cur_socket;
			if(socket.has_value())
			{
				cur_socket = *socket;
			}
			return new(re.Get()) FormRenderTarget{re, cur_socket, this, property };
		}
		return {};
	}

	void FormRenderTarget::Release()
	{
		auto re = record;
		this->~FormRenderTarget();
		re.Deallocate();
	}

	void FormRenderTarget::OnFormCreated(Form& interface)
	{
		Windows::Win32Form* real_form = dynamic_cast<Windows::Win32Form*>(&interface);
		if(real_form != nullptr)
		{
			assert(!swap_chain && renderer);
			swap_chain = renderer->CreateSwapChain(property, real_form->GetWnd());
		}
	}
	*/

	/*
	ComPtr<IDXGIFactory2> rptr;
	UINT Flags = 0;
	if (EnableDebug)
	{
		Flags |= DXGI_CREATE_FACTORY_DEBUG;
	}

	HRESULT re = CreateDXGIFactory2(Flags, __uuidof(decltype(rptr)::InterfaceType), reinterpret_cast<void**>(rptr.GetAddressOf()));
	if (SUCCEEDED(re))
	{
		return HardwareDevice{ std::move(rptr) };
	}
	return {};
	*/





	/*
	void InitDebugLayer()
	{
		{
			ComPtr<ID3D12Debug> Debug;
			D3D12GetDebugInterface(__uuidof(decltype(Debug)::InterfaceType), reinterpret_cast<void**>(Debug.GetAddressOf()));
			if (Debug)
			{
				Debug->EnableDebugLayer();
			}
		}
	}

	void SwapChain::OnReInit(HWND hwnd, std::size_t size_x, std::size_t size_y)
	{
		if(factory && command_queue)
		{
			swap_chain.Reset();
			ComPtr<IDXGIFactory2> new_factor;
			factory->QueryInterface(__uuidof(decltype(new_factor)::InterfaceType), reinterpret_cast<void**>(new_factor.GetAddressOf()));
			if(new_factor)
			{

				DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
				swapChainDesc.BufferCount = Setting.buffer_count;
				swapChainDesc.Width = size_x;
				swapChainDesc.Height = size_y;
				swapChainDesc.Format = Setting.format;
				swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
				swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
				swapChainDesc.SampleDesc.Count = 1;

				ComPtr<IDXGISwapChain1> ptr;
				auto re = new_factor->CreateSwapChainForHwnd(command_queue.Get(), hwnd, &swapChainDesc, nullptr, nullptr, ptr.GetAddressOf());
				if(SUCCEEDED(re))
				{
					swap_chain = ptr;
				}
			}
		}
	}

	auto HardwareDevice::Create(bool EnableDebug)
		-> HardwareDevice
	{
		ComPtr<IDXGIFactory2> rptr;
		UINT Flags = 0;
		if(EnableDebug)
		{
			Flags |= DXGI_CREATE_FACTORY_DEBUG;
		}

		HRESULT re = CreateDXGIFactory2(Flags, __uuidof(decltype(rptr)::InterfaceType), reinterpret_cast<void**>(rptr.GetAddressOf()));
		if(SUCCEEDED(re))
		{
			return HardwareDevice{std::move(rptr)};
		}
		return {};
	}

	Adapter HardwareDevice::EnumAdapter(std::size_t index)
	{
		if(ptr)
		{
			ComPtr<IDXGIFactory1> fptr;
			auto re = ptr->QueryInterface(__uuidof(decltype(fptr)::InterfaceType), reinterpret_cast<void**>(fptr.GetAddressOf()));
			if(SUCCEEDED(re) && fptr)
			{
				ComPtr<IDXGIAdapter1> r_ptr;
				auto re = fptr->EnumAdapters1(index, r_ptr.GetAddressOf());
				if(SUCCEEDED(re))
				{
					return std::move(r_ptr);
				}
			}
		}
		return {};
	}

	SwapChain::Ptr HardwareDevice::CreateSwapChain(SwapChinSetting setting, Renderer renderer, std::pmr::memory_resource* resource)
	{
		if (resource != nullptr && ptr && renderer)
		{
			auto record = Potato::IR::MemoryResourceRecord::Allocate<SwapChain>(resource);
			if (record)
			{
				return new (record.Get()) SwapChain{ record, setting, ptr, renderer.ptr };
			}
		}
		return {};
	}

	Renderer SoftwareDevice::CreateRenderer()
	{
		if(ptr)
		{
			D3D12_COMMAND_QUEUE_DESC desc{
				D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT,
				D3D12_COMMAND_QUEUE_PRIORITY::D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
				D3D12_COMMAND_QUEUE_FLAGS::D3D12_COMMAND_QUEUE_FLAG_NONE,
				0
			};

			ComPtr<ID3D12CommandQueue> que_ptr;

			auto re = ptr->CreateCommandQueue(
				&desc, __uuidof(decltype(que_ptr)::InterfaceType), reinterpret_cast<void**>(que_ptr.GetAddressOf())
			);
			if(SUCCEEDED(re))
			{
				return {std::move(que_ptr)};
			}
		}
		return {};
	}

	SoftwareDevice SoftwareDevice::Create(Adapter adapter)
	{
		if(adapter)
		{

			auto feature_level = std::array{
				D3D_FEATURE_LEVEL_12_2,
				D3D_FEATURE_LEVEL_12_1,
				D3D_FEATURE_LEVEL_12_0
			};

			ComPtr<ID3D12Device> dev_ptr;

			for(auto ite : feature_level)
			{
				auto re = D3D12CreateDevice(adapter.Get(), ite, __uuidof(decltype(dev_ptr)::InterfaceType), reinterpret_cast<void**>(dev_ptr.GetAddressOf()));
				if(SUCCEEDED(re))
				{
					return {std::move(dev_ptr)};
				}
			}
		}
		return {};
	}

	void SwapChain::Release()
	{
		auto re = record;
		this->~SwapChain();
		re.Deallocate();
	}

	/*
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

	auto SwapChain::Create(CommandQueue::Ptr queue, Factory::Ptr context, std::pmr::memory_resource* resource) -> Ptr
	{
		if(queue && context)
		{
			auto record = Potato::IR::MemoryResourceRecord::Allocate<SwapChain>(resource);
			if(record)
			{
				Ptr ptr {
					new (record.Get()) SwapChain{}
				};
				ptr->record = record;
				ptr->context = std::move(context);
				ptr->queue = std::move(queue);
				return ptr;
			}
		}
		return {};
	}

	void SwapChain::OnInit(HWND hwnd)
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
		auto re = context->dxgi_factory->CreateSwapChainForHwnd(
			queue->command_queue.Get(), hwnd, &swapChainDesc, nullptr, nullptr,
			swapChain.GetAddressOf()
		);
	}

	void SwapChain::OnRelease(HWND)
	{
		
	}

	void SwapChain::OnUpdate()
	{
		
	}

	void SwapChain::ControllerRelease()
	{
		
	}

	void SwapChain::ViewerRelease()
	{
		auto re = record;
		this->~SwapChain();
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
			//dev_ptr->
		}
		return {};
	}

	void Device::Release()
	{
		auto re = record;
		this->~Device();
		re.Deallocate();
	}
	*/
}
