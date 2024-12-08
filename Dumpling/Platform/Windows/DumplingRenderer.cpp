module;

#include <cassert>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <intsafe.h>
#include <OCIdl.h>

#undef interface

module DumplingRenderer;


namespace Dumpling
{
	bool Device::InitDebugLayer()
	{
		static std::mutex debug_mutex;
		static ComPtr<ID3D12Debug> debug_layout;
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

	FormWrapper::Ptr Device::CreateFormWrapper(Form form, FormWrapper::Config fig, std::pmr::memory_resource* resource)
	{
		assert(factory);

		if(form)
		{
			RECT rect;
			if(GetClientRect(form.GetPlatformValue(), &rect))
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
					queue.Get(), form.GetPlatformValue(), &swapChainDesc, nullptr, nullptr,
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
			output.PreFinishRender();
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

	void RenderTargetSet::Clear()
	{
		for(auto& ite : target_data)
		{
			ite.reference_resource.Reset();
		}
		render_target_count = 0;
		has_depth_stencil = false;
	}

	RenderTargetSet::RenderTargetSet(RenderTargetSet&& set)
	{
		target_data = std::move(set.target_data);
		target = std::move(set.target);
		render_target_count = set.render_target_count;
		set.render_target_count = 0;
		has_depth_stencil = set.has_depth_stencil;
		set.has_depth_stencil = false;
	}

	std::optional<std::size_t> RenderTargetSet::AddRenderTarget(RendererResource const& resource)
	{
		if(render_target_count < max_render_target_count)
		{
			auto desc = resource.GetDescription(D3D12_RESOURCE_STATE_RENDER_TARGET);
			if(desc.resource_ptr)
			{
				auto& tar = target_data[render_target_count + 1];
				tar.reference_resource = std::move(desc.resource_ptr);
				tar.default_state = desc.default_state;
				tar.handle = desc.cpu_handle;
				target[render_target_count + 1] = tar.handle;
				++render_target_count;
				return render_target_count;
			}
		}
		return std::nullopt;
	}
	bool RenderTargetSet::SetDepthStencil(RendererResource const& resource)
	{
		auto desc = resource.GetDescription(D3D12_RESOURCE_STATE_DEPTH_WRITE);
		if(desc.resource_ptr)
		{
			auto& tar = target_data[0];
			tar.reference_resource = std::move(desc.resource_ptr);
			tar.default_state = desc.default_state;
			tar.handle = desc.cpu_handle;
			target[render_target_count + 1] = tar.handle;
			has_depth_stencil = true;
			return true;
		}
		return false;
	}

	void PassRenderer::SetRenderTargets(RenderTargetSet const& render_targets)
	{
		if(render_targets.render_target_count != 0 || render_targets.has_depth_stencil)
		{
			if(render_targets.has_depth_stencil)
			{
				auto& ref = render_targets.target_data[0];
				render_target_barriers[render_target_barriers_count] = D3D12_RESOURCE_BARRIER{
					D3D12_RESOURCE_BARRIER_TYPE::D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
					D3D12_RESOURCE_BARRIER_FLAGS::D3D12_RESOURCE_BARRIER_FLAG_NONE,
					D3D12_RESOURCE_TRANSITION_BARRIER{
						ref.reference_resource.Get(),
						D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
						ref.default_state,
						D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_DEPTH_WRITE
					}
				};
				++render_target_barriers_count;
			}

			auto rt_span = std::span(render_targets.target_data).subspan(1, render_targets.render_target_count);

			for(auto& ite : rt_span)
			{
				render_target_barriers[render_target_barriers_count] = D3D12_RESOURCE_BARRIER{
					D3D12_RESOURCE_BARRIER_TYPE::D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
					D3D12_RESOURCE_BARRIER_FLAGS::D3D12_RESOURCE_BARRIER_FLAG_NONE,
					D3D12_RESOURCE_TRANSITION_BARRIER{
						ite.reference_resource.Get(),
						D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
						ite.default_state,
						D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_RENDER_TARGET
					}
				};
				++render_target_barriers_count;
			}
			if(render_target_barriers_count != 0)
				command->ResourceBarrier(render_target_barriers_count, render_target_barriers.data());
			command->OMSetRenderTargets(
				render_targets.render_target_count,
				render_targets.target.data() + 1,
				FALSE,
				render_targets.has_depth_stencil ? render_targets.target.data() : nullptr
			);

			for(auto& ite : cache_render_target)
			{
				ite = D3D12_CPU_DESCRIPTOR_HANDLE{0};
			}

			render_target_barriers_count = 0;

			if(render_targets.has_depth_stencil)
			{
				auto& ref = render_targets.target_data[0];
				render_target_barriers[render_target_barriers_count] = D3D12_RESOURCE_BARRIER{
					D3D12_RESOURCE_BARRIER_TYPE::D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
					D3D12_RESOURCE_BARRIER_FLAGS::D3D12_RESOURCE_BARRIER_FLAG_NONE,
					D3D12_RESOURCE_TRANSITION_BARRIER{
						ref.reference_resource.Get(),
						D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
						D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_DEPTH_WRITE,
						ref.default_state
					}
				};
				++render_target_barriers_count;
				cache_render_target[0] = ref.handle;
			}

			std::size_t ite_index = 1;
			for(auto& ite : rt_span)
			{
				render_target_barriers[render_target_barriers_count] = D3D12_RESOURCE_BARRIER{
					D3D12_RESOURCE_BARRIER_TYPE::D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
					D3D12_RESOURCE_BARRIER_FLAGS::D3D12_RESOURCE_BARRIER_FLAG_NONE,
					D3D12_RESOURCE_TRANSITION_BARRIER{
						ite.reference_resource.Get(),
						D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
						D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_RENDER_TARGET,
						ite.default_state
					}
				};
				++render_target_barriers_count;
				cache_render_target[ite_index] = ite.handle;
				++ite_index;
			}
		}
	}

	void PassRenderer::PreFinishRender()
	{
		if(render_target_barriers_count != 0)
		{
			command->ResourceBarrier(render_target_barriers_count, render_target_barriers.data());
			render_target_barriers_count = 0;
			command->OMSetRenderTargets(
				0,
				nullptr,
				FALSE,
				nullptr
			);
		}
		for(auto& ite : cache_render_target)
		{
			ite = D3D12_CPU_DESCRIPTOR_HANDLE{0};
		}
	}

	bool PassRenderer::ClearRendererTarget(std::size_t index, Color color)
	{
		if(index < cache_render_target.size())
		{
			auto ref = cache_render_target[index];
			if(ref.ptr == 0)
			{
				std::array<float, 4> co =  {color.R, color.G, color.B, color.A};
				command->ClearRenderTargetView(
					cache_render_target[index + 1], co.data(), 0, nullptr);
				return true;
			}
		}
		return false;
	}
}
