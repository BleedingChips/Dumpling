module;

#include <cassert>
#include <d3d12.h>
#include <dxgi1_6.h>

#undef interface

module DumplingDx12Renderer;


namespace Dumpling
{

	Renderer::Renderer(Potato::IR::MemoryResourceRecord record, DevicePtr in_device, CommandQueuePtr direct_queue)
		: MemoryResourceRecordIntrusiveInterface(record), device(std::move(in_device)), direct_queue(std::move(direct_queue))
	{
		assert(device);
		device->CreateFence(0, D3D12_FENCE_FLAG_NONE, __uuidof(decltype(current_fence)::InterfaceType), reinterpret_cast<void**>(current_fence.GetAddressOf()));
	}

	Renderer::Ptr Renderer::Create(std::optional<AdapterDescription> adapter, std::pmr::memory_resource* resource)
	{

		DevicePtr dev_ptr;
		auto  re = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_1, __uuidof(decltype(dev_ptr)::InterfaceType), reinterpret_cast<void**>(dev_ptr.GetAddressOf()));
		if(SUCCEEDED(re))
		{
			CommandQueuePtr command_queue;
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

				auto record = Potato::IR::MemoryResourceRecord::Allocate<Renderer>(resource);
				if(record)
				{
					return new (record.Get()) Renderer{record, std::move(dev_ptr), std::move(command_queue)};
				}
			}
		}
		return {};
	}

	bool Renderer::PopPassRenderer(PassRenderer& output_renderer, Pass const& pass)
	{
		std::lock_guard lg(pipeline_mutex);
		return PopPassRenderer_AssumedLocked(output_renderer, pass);
	}

	bool Renderer::PopPassRenderer_AssumedLocked(PassRenderer& output_renderer, Pass const& pass)
	{
		FinishPassRenderer_AssumedLocked(output_renderer);
		auto request = pipeline_manager.PopPassRequest(pass);
		if(request.has_value())
		{
			CommandAllocatorPtr cur_allocator;
			std::size_t allocator_index = 0;
			std::size_t cur_frame = current_frame;
			for(auto& ite : allocators)
			{
				if(ite.status == Status::IDLE || ite.status == Status::WAITING && ite.frame_number == cur_frame)
				{
					cur_allocator = ite.allocator;
				}else
				{
					++allocator_index;
				}
			}
			if(!cur_allocator)
			{
				auto re = device->CreateCommandAllocator(
					D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT, 
					__uuidof(decltype(cur_allocator)::InterfaceType), reinterpret_cast<void**>(cur_allocator.GetAddressOf())
				);
				if(SUCCEEDED(re))
				{
					allocators.emplace_back(
						Status::IDLE,
						cur_allocator,
						cur_frame
					);
				}else
				{
					pipeline_manager.PushPassRequest(std::move(*request));
					return false;
				}
			}

			GraphicCommandListPtr ptr;

			auto re = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT,
					cur_allocator.Get(), nullptr, __uuidof(decltype(ptr)::InterfaceType), reinterpret_cast<void**>(ptr.GetAddressOf())
				);

			if(SUCCEEDED(re))
			{
				output_renderer.command_list = ptr;
				output_renderer.identity = {allocator_index};
				allocators[allocator_index].status = Status::USING;
				allocators[allocator_index].frame_number = current_frame;
				++losing_command;
				return true;
			}
			pipeline_manager.PushPassRequest(std::move(*request));
			return false;
		}
		return false;
	}


	bool Renderer::FinishPassRenderer(PassRenderer& pass_renderer)
	{
		std::lock_guard lg(pipeline_mutex);
		return FinishPassRenderer_AssumedLocked(pass_renderer);
	}

	bool Renderer::FinishPassRenderer_AssumedLocked(PassRenderer& pass_renderer)
	{
		if(pass_renderer.command_list)
		{
			auto tem = std::move(pass_renderer.command_list);
			pass_renderer.identity = {};
			tem->Close();
			allocators[pass_renderer.identity.reference_allocator_index].status = Status::WAITING;
			frame_command.emplace_back(std::move(tem), pass_renderer.identity);
			assert(losing_command >= 1);
			--losing_command;
			return true;
		}
		return false;
	}

	std::optional<std::size_t> Renderer::CommitedAndSwapContext()
	{
		std::lock_guard lg(pipeline_mutex);
		if(losing_command != 0)
		{
			return std::nullopt;
		}
		for(auto& ite : allocators)
		{
			if(ite.frame_number == current_frame)
			{
				assert(ite.status != Status::USING);
				if(ite.status == Status::WAITING)
					ite.status = Status::Block;
			}
		}
		for(auto& ite : frame_command)
		{
			direct_queue->ExecuteCommandLists(1, ite.list.GetAddressOf());
		}
		direct_queue->Signal(current_fence.Get(), current_frame);
		frame_command.clear();
		auto tem_count = current_frame;
		current_frame += 1;
		return tem_count;
	}

	std::tuple<bool, std::size_t> Renderer::TryFlushFrame(std::size_t require_frame)
	{

		auto cur_value = current_fence->GetCompletedValue();

		std::lock_guard lg(pipeline_mutex);

		if(require_frame <= cur_value)
		{
			if(last_flush_frame_count != cur_value)
			{
				last_flush_frame_count = cur_value;
				for(auto& ite : allocators)
				{
					if(ite.frame_number <= cur_value && ite.status == Status::Block)
					{
						ite.allocator->Reset();
						ite.status = Status::IDLE;
					}
				}
			}
			return {true, cur_value};
		}

		return {false, cur_value};
	}

	bool Renderer::FlushWindows(FormWrapper& windows)
	{
		return true;
	}

	FormWrapper::Ptr Renderer::CreateFormWrapper(HardDevice& hard_device, Form& form, FormWrapper::Config fig, std::pmr::memory_resource* resource)
	{
		ComPtr<IDXGIFactory2> new_factor;
		hard_device.GetFactory()->QueryInterface(__uuidof(decltype(new_factor)::InterfaceType), reinterpret_cast<void**>(new_factor.GetAddressOf()));
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
			auto re = new_factor->CreateSwapChainForHwnd(
				direct_queue.Get(), form.GetWnd(), &swapChainDesc, nullptr, nullptr,
				swapChain.GetAddressOf()
			);
			if(SUCCEEDED(re))
			{
				auto record = Potato::IR::MemoryResourceRecord::Allocate<FormWrapper>(resource);
				if(record)
				{
					return new (record.Get()) FormWrapper{record, std::move(swapChain)};
				}
			}
		}
		return {};
	}

	bool PassRenderer::ClearRendererTarget(RendererResource& render_target, Color color, std::size_t index)
	{
		return true;
	}

	void FormWrapper::Release()
	{
		auto re = record;
		this->~FormWrapper();
		re.Deallocate();
	}


	/*
	void FormRenderer::OnFormCreated(Form& interface)
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
