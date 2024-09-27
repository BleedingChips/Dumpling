module;

#include <cassert>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <intsafe.h>
#include <OCIdl.h>

#undef interface

module DumplingDx12Renderer;


namespace Dumpling
{
	bool Device::InitDebugLayer()
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

	bool FormWrapper::Present(std::size_t syn_interval)
	{
		auto re = swap_chain->Present(syn_interval, 0);
		if(SUCCEEDED(re))
		{
			std::lock_guard lg(logic_mutex);
			auto current = swap_chain->GetCurrentBackBufferIndex();
			assert(config.swap_buffer_count == 1 || ((current_index + 1) % config.swap_buffer_count) == current);
			current_index = current;
		}
		return SUCCEEDED(re);
	}

	bool FormWrapper::LogicPresent()
	{
		std::lock_guard lg(logic_mutex);
		auto tar = (logic_current_index + 1) % config.swap_buffer_count;
		if(tar == current_index)
		{
			return false;
		}else
		{
			logic_current_index = tar;
			return true;
		}
	}

	auto FormWrapper::GetDescription(D3D12_RESOURCE_STATES require_state) const -> Description
	{
		if(require_state == D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_RENDER_TARGET)
		{
			ResourcePtr resource;
			std::size_t index = 0;
			{
				std::shared_lock sl(logic_mutex);
				index = logic_current_index;
			}
			swap_chain->GetBuffer(index, __uuidof(decltype(resource)::InterfaceType), reinterpret_cast<void**>(resource.GetAddressOf()));
			return{
				resource,
				m_rtvHeap->GetCPUDescriptorHandleForHeapStart().ptr + offset * index,
				D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_PRESENT
			};
		}
		return {};
	}

	struct FormWrapperImp : public FormWrapper, public Potato::IR::MemoryResourceRecordIntrusiveInterface
	{
		FormWrapperImp(Potato::IR::MemoryResourceRecord record, SwapChainPtr swap_chain, DescriptorHeapPtr m_rtvHeap, Config config, std::size_t offset)
			: MemoryResourceRecordIntrusiveInterface(record), FormWrapper(std::move(swap_chain), std::move(m_rtvHeap), config, offset)
		{
			
		}
	protected:
		void AddRendererResourceRef() const override { MemoryResourceRecordIntrusiveInterface::AddRef(); }
		void SubRendererResourceRef() const override { MemoryResourceRecordIntrusiveInterface::SubRef(); }
	};

	FormWrapper::Ptr Device::CreateFormWrapper(Form& form, FormWrapper::Config fig, std::pmr::memory_resource* resource)
	{
		assert(factory);

		auto Hwnd = form.GetWnd();
		if(Hwnd != nullptr)
		{
			RECT rect;
			if(GetClientRect(Hwnd, &rect))
			{
				DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
				swapChainDesc.BufferCount = fig.swap_buffer_count;
				swapChainDesc.Width = rect.right - rect.left;
				swapChainDesc.Height = rect.bottom - rect.top;
				swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
				swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
				swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
				swapChainDesc.SampleDesc.Count = 1;

				ComPtr<IDXGISwapChain1> swapChain;
				auto re = factory->CreateSwapChainForHwnd(
					queue.Get(), Hwnd, &swapChainDesc, nullptr, nullptr,
					swapChain.GetAddressOf()
				);
				if(SUCCEEDED(re))
				{
					SwapChainPtr swap_chain;
					if(SUCCEEDED(swapChain.As(&swap_chain)))
					{
						DescriptorHeapPtr m_rtvHeap;
						D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
				        rtvHeapDesc.NumDescriptors = swapChainDesc.BufferCount;
				        rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
				        rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
				        auto re = device->CreateDescriptorHeap(&rtvHeapDesc,
							__uuidof(decltype(m_rtvHeap)::InterfaceType), reinterpret_cast<void**>(m_rtvHeap.GetAddressOf())
						);
						if(SUCCEEDED(re))
						{
							std::size_t m_rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
							D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle {m_rtvHeap->GetCPUDescriptorHandleForHeapStart()};

					        // Create a RTV and a command allocator for each frame.
					        for (UINT n = 0; n < swapChainDesc.BufferCount; n++)
					        {
								ResourcePtr resource;
					            swap_chain->GetBuffer(n, __uuidof(decltype(resource)::InterfaceType), reinterpret_cast<void**>(resource.GetAddressOf()));
					            device->CreateRenderTargetView(resource.Get(), nullptr, rtvHandle);
								rtvHandle.ptr += m_rtvDescriptorSize;
					        }

							return Potato::IR::MemoryResourceRecord::AllocateAndConstruct<FormWrapperImp>(
								resource,
								std::move(swap_chain),
								std::move(m_rtvHeap),
								fig,
								m_rtvDescriptorSize
							);
						}
					}
				}
			}
		}
		return {};
	}

	bool FrameRenderer::PopPassRenderer(PassRenderer& output)
	{
		std::lock_guard lg(renderer_mutex);
		return PopPassRenderer_AssumedLocked(output);
	}

	FrameRenderer::~FrameRenderer()
	{
		{
			std::lock_guard lg(renderer_mutex);
			for(auto& ite : need_commited_command)
			{
				ite->Release();
			}
			need_commited_command.clear();
		}
	}

	bool FrameRenderer::PopPassRenderer_AssumedLocked(PassRenderer& output)
	{
		if(!output.command)
		{
			std::size_t index = 0;
			for(;index < total_allocator.size(); ++index)
			{
				auto& ref = total_allocator[index];
				if(ref.state == State::Idle || ref.state == State::Waiting)
				{
					break;
				}
			}
			if(index == total_allocator.size())
			{
				CommandAllocatorPtr new_allocator;
				auto re = device->CreateCommandAllocator(
					D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT, 
					__uuidof(decltype(new_allocator)::InterfaceType), reinterpret_cast<void**>(new_allocator.GetAddressOf())
				);
				if(SUCCEEDED(re))
				{
					total_allocator.emplace_back(std::move(new_allocator), State::Idle, 0);
				}else
					return false;
			}
			auto& ref = total_allocator[index];
			GraphicCommandListPtr target_command_list;
			if(!free_command_list.empty())
			{
				target_command_list = std::move(free_command_list.back());
				free_command_list.pop_back();
				auto re = target_command_list->Reset(ref.allocator.Get(), nullptr);
				if(!SUCCEEDED(re))
				{
					return false;
				}
				
			}else
			{
				auto re = device->CreateCommandList(
					0, D3D12_COMMAND_LIST_TYPE_DIRECT, ref.allocator.Get(), nullptr,
					__uuidof(decltype(target_command_list)::InterfaceType), reinterpret_cast<void**>(target_command_list.GetAddressOf())
				);
				if(!SUCCEEDED(re))
				{
					return false;
				}
			}
			ref.state = State::Using;
			ref.frame = current_frame;
			output.command = std::move(target_command_list);
			output.frame = current_frame;
			output.reference_allocator_index = index;
			++running_count;
			return true;
		}
		return false;
	}

	bool FrameRenderer::FinishPassRenderer(PassRenderer& output)
	{
		std::lock_guard lg(renderer_mutex);
		return FinishPassRenderer_AssumedLocked(output);
	}

	bool FrameRenderer::FinishPassRenderer_AssumedLocked(PassRenderer& output)
	{
		if(output.command && output.frame == current_frame)
		{
			auto re = output.command->Close();
			if(SUCCEEDED(re))
			{
				auto ptr = output.command.Get();
				ptr->AddRef();
				output.command.Reset();
				need_commited_command.emplace_back(ptr);
				assert(total_allocator.size() > output.reference_allocator_index);
				auto& ref = total_allocator[output.reference_allocator_index];
				assert(ref.state == State::Using && ref.frame == current_frame);
				ref.state = State::Waiting;
				--running_count;
				return true;
			}
		}
		return false;
	}

	std::optional<std::size_t> FrameRenderer::CommitFrame()
	{
		std::lock_guard lg(renderer_mutex);
		if(running_count == 0)
		{
			for(auto& ite : total_allocator)
			{
				assert(ite.state != State::Using);
				if(ite.state == State::Waiting)
				{
					assert(ite.frame == current_frame);
					ite.state = State::Done;
				}
			}
			std::size_t old_frame;
			{
				std::lock_guard lg(frame_mutex);
				queue->ExecuteCommandLists(need_commited_command.size(), need_commited_command.data());
				auto re = queue->Signal(fence.Get(), current_frame);
				assert(SUCCEEDED(re));
				old_frame = current_frame;
				++current_frame;
			}
			
			for(auto ite : need_commited_command)
			{
				GraphicCommandListPtr temp;
				auto re = ite->QueryInterface(
					__uuidof(decltype(temp)::InterfaceType), reinterpret_cast<void**>(temp.GetAddressOf())
				);
				assert(SUCCEEDED(re));
				free_command_list.push_back(std::move(temp));
				ite->Release();
			}
			need_commited_command.clear();
			return old_frame;
		}
		return std::nullopt;
	}

	void FrameRenderer::ResetAllocator_AssumedLocked(std::size_t frame)
	{
		if (frame != last_flush_frame)
		{
			for (auto& ite : total_allocator)
			{
				if (ite.state == State::Done && ite.frame <= frame)
				{
					ite.state = State::Idle;
					ite.allocator->Reset();
				}
			}
			last_flush_frame = frame;
		}
	}

	std::size_t FrameRenderer::TryFlushFrame()
	{
		auto cur = fence->GetCompletedValue();
		{
			std::lock_guard lg(renderer_mutex);
			ResetAllocator_AssumedLocked(cur);
		}
		return cur;
	}

	bool FrameRenderer::FlushToLastFrame(std::optional<std::chrono::steady_clock::duration> time_duration)
	{
		while(true)
		{
			auto cur = fence->GetCompletedValue();
			{
				std::lock_guard lg(renderer_mutex);
				ResetAllocator_AssumedLocked(cur);
				if(cur + 1 >= current_frame)
				{
					return true;
				}
			}
			if(time_duration.has_value())
			{
				std::this_thread::sleep_for(*time_duration);
			}else
			{
				std::this_thread::yield();
			}
		}
		return true;
	}

	bool FrameRenderer::FlushToCurrentFrame(std::optional<std::chrono::steady_clock::duration> time_duration)
	{
		while (true)
		{
			auto cur = fence->GetCompletedValue();
			{
				std::lock_guard lg(renderer_mutex);
				ResetAllocator_AssumedLocked(cur);
				if (cur == current_frame)
				{
					return true;
				}
			}
			if (time_duration.has_value())
			{
				std::this_thread::sleep_for(*time_duration);
			}
			else
			{
				std::this_thread::yield();
			}
		}
		return true;
	}

	struct FrameRendererImp : public FrameRenderer, public Potato::IR::MemoryResourceRecordIntrusiveInterface
	{
		FrameRendererImp(Potato::IR::MemoryResourceRecord record, DevicePtr device, CommandQueuePtr ptr, FencePtr fence)
			: MemoryResourceRecordIntrusiveInterface(record), FrameRenderer(std::move(device), std::move(ptr), std::move(fence), record.GetMemoryResource())
		{

		}
		virtual void AddFrameRendererRef() const override { MemoryResourceRecordIntrusiveInterface::AddRef(); }
		virtual void SubFrameRendererRef() const override { MemoryResourceRecordIntrusiveInterface::SubRef(); }
	};

	FrameRenderer::Ptr Device::CreateFrameRenderer(std::pmr::memory_resource* resource)
	{
		FencePtr fence;
		auto re = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, __uuidof(decltype(fence)::InterfaceType), reinterpret_cast<void**>(fence.GetAddressOf()));
		if(SUCCEEDED(re))
		{
			return Potato::IR::MemoryResourceRecord::AllocateAndConstruct<FrameRendererImp>(resource, device, queue, std::move(fence));
		}
		return {};
	}
		

	struct DeviceImp : public Device, public Potato::IR::MemoryResourceRecordIntrusiveInterface
	{
		DeviceImp(Potato::IR::MemoryResourceRecord record, FactoryPtr factory, DevicePtr device, CommandQueuePtr queue)
			: MemoryResourceRecordIntrusiveInterface(record), Device(std::move(factory), std::move(device), std::move(queue))
		{}
	protected:
		virtual void AddDeviceRef() const override { MemoryResourceRecordIntrusiveInterface::AddRef(); }
		virtual void SubDeviceRef() const override { MemoryResourceRecordIntrusiveInterface::SubRef(); }
	};

	auto Device::Create(std::pmr::memory_resource* resource)-> Ptr
	{
		FactoryPtr rptr;
		UINT Flags = 0;
		Flags |= DXGI_CREATE_FACTORY_DEBUG;
		HRESULT result = CreateDXGIFactory2(Flags, __uuidof(decltype(rptr)::InterfaceType), reinterpret_cast<void**>(rptr.GetAddressOf()));
		if(SUCCEEDED(result))
		{
			DevicePtr dev_ptr;
			result = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_1, __uuidof(decltype(dev_ptr)::InterfaceType), reinterpret_cast<void**>(dev_ptr.GetAddressOf()));
			if(SUCCEEDED(result))
			{
				CommandQueuePtr command_queue;
				D3D12_COMMAND_QUEUE_DESC desc{
					D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT,
					D3D12_COMMAND_QUEUE_PRIORITY::D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
					D3D12_COMMAND_QUEUE_FLAGS::D3D12_COMMAND_QUEUE_FLAG_NONE,
				0
				};

				result = dev_ptr->CreateCommandQueue(
					&desc, __uuidof(decltype(command_queue)::InterfaceType), reinterpret_cast<void**>(command_queue.GetAddressOf())
				);

				if(SUCCEEDED(result))
				{
					return Potato::IR::MemoryResourceRecord::AllocateAndConstruct<DeviceImp>(
						resource, std::move(rptr), std::move(dev_ptr), std::move(command_queue)
					);
				}
			}
		}
		return {};
	}

	bool PassRenderer::ClearRendererTarget(RendererResource& render_target, Color color, std::size_t index)
	{
		auto desc = render_target.GetDescription(D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_RENDER_TARGET);
		if(desc.resource_ptr)
		{
			if(desc.default_state != D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_RENDER_TARGET)
			{
				D3D12_RESOURCE_BARRIER barrier{
					D3D12_RESOURCE_BARRIER_TYPE::D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
					D3D12_RESOURCE_BARRIER_FLAGS::D3D12_RESOURCE_BARRIER_FLAG_NONE,
					D3D12_RESOURCE_TRANSITION_BARRIER{
						desc.resource_ptr.Get(),
						D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
						desc.default_state,
						D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_RENDER_TARGET
					}
				};
				command->ResourceBarrier(1, &barrier);
			}
			std::array<float, 4> co =  {color.R, color.G, color.B, color.A};
			command->ClearRenderTargetView(desc.cpu_handle, co.data(), 0, nullptr);
			if(desc.default_state != D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_RENDER_TARGET)
			{
				D3D12_RESOURCE_BARRIER barrier{
					D3D12_RESOURCE_BARRIER_TYPE::D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
					D3D12_RESOURCE_BARRIER_FLAGS::D3D12_RESOURCE_BARRIER_FLAG_NONE,
					D3D12_RESOURCE_TRANSITION_BARRIER{
						desc.resource_ptr.Get(),
						D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
						D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_RENDER_TARGET,
						desc.default_state
					}
				};
				command->ResourceBarrier(1, &barrier);
			}
		}
		return true;
	}








	/*
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

	bool Renderer::ForceFlush(std::size_t require_frame, std::chrono::steady_clock::duration waitting_duration = std::chrono::microseconds{10})
	{
		while(true)
		{
			auto [re, frame] = TryFlushFrame(require_frame);
			if(!re)
			{
				std::this_thread::sleep_for(waitting_duration);
			}else{
				return true;
			}
		}
	}

	bool Renderer::FlushWindows(FormWrapper& windows)
	{
		windows.swap_chain->Present(1, 0);
		return true;
	}

	FormWrapper::Ptr Renderer::CreateFormWrapper(HardDevice& hard_device, Form& form, FormWrapper::Config fig, std::pmr::memory_resource* resource)
	{
		
	}

	

	void FormWrapper::Release()
	{
		auto re = record;
		this->~FormWrapper();
		re.Deallocate();
	}
	*/


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
