module;

#include <cassert>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <intsafe.h>
#include <OCIdl.h>
#include <d3d12sdklayers.h>
#include <Windows.h>

#undef interface
#undef max

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
			D3D12GetDebugInterface(__uuidof(ID3D12Debug), debug_layout.GetPointerVoidAdress());
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
			ComPtr<ID3D12Resource> resource;
			std::size_t index = 0;
			{
				std::shared_lock sl(logic_mutex);
				index = logic_current_index;
			}
			swap_chain->GetBuffer(index, __uuidof(ID3D12Resource), resource.GetPointerVoidAdress());
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
		FormWrapperImp(Potato::IR::MemoryResourceRecord record, ComPtr<IDXGISwapChain3> swap_chain, ComPtr<ID3D12DescriptorHeap> m_rtvHeap, Config config, std::size_t offset)
			: MemoryResourceRecordIntrusiveInterface(record), FormWrapper(std::move(swap_chain), std::move(m_rtvHeap), config, offset)
		{
			
		}
	protected:
		void AddRendererResourceRef() const override { MemoryResourceRecordIntrusiveInterface::AddRef(); }
		void SubRendererResourceRef() const override { MemoryResourceRecordIntrusiveInterface::SubRef(); }
	};

	FormWrapper::Ptr Device::CreateFormWrapper(Form const& form, FrameRenderer& render, FormWrapper::Config fig, std::pmr::memory_resource* resource)
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

				ComPtr<IDXGISwapChain1> new_swap_chain;
				auto re = factory->CreateSwapChainForHwnd(
					render.command_queue.GetPointer(), form.GetPlatformValue(), &swapChainDesc, nullptr, nullptr,
					new_swap_chain.GetPointerAdress()
				);
				if(SUCCEEDED(re))
				{
					ComPtr<IDXGISwapChain3> swap_chain;

					if (SUCCEEDED(new_swap_chain->QueryInterface(
						__uuidof(decltype(swap_chain)::Type),
						swap_chain.GetPointerVoidAdress())
					))
					{
						ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
						D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
						rtvHeapDesc.NumDescriptors = swapChainDesc.BufferCount;
						rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
						rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
						auto re = device->CreateDescriptorHeap(&rtvHeapDesc,
							__uuidof(decltype(m_rtvHeap)::Type), m_rtvHeap.GetPointerVoidAdress()
						);
						if (SUCCEEDED(re))
						{
							std::size_t m_rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
							D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle{ m_rtvHeap->GetCPUDescriptorHandleForHeapStart() };

							// Create a RTV and a command allocator for each frame.
							for (UINT n = 0; n < swapChainDesc.BufferCount; n++)
							{
								ComPtr<ID3D12Resource> resource;
								swap_chain->GetBuffer(n, __uuidof(decltype(resource)::Type), resource.GetPointerVoidAdress());
								device->CreateRenderTargetView(resource.GetPointer(), nullptr, rtvHandle);
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

	bool FrameRenderer::PopPassRenderer(PassRenderer& output, PassRequest const& request)
	{
		assert(*this);
		if (output)
			return false;
		ComPtr<ID3D12CommandAllocator> target_allocator;
		if (!target_allocator && !command_allocator_current_frame.empty())
		{
			target_allocator = std::move(command_allocator_current_frame.back());
			command_allocator_current_frame.pop_back();
		}
		if (!target_allocator)
		{
			auto re = device->CreateCommandAllocator(
				D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT,
				__uuidof(decltype(target_allocator)::Type), target_allocator.GetPointerVoidAdress()
			);
			assert(re);
		}
		if (!target_allocator)
			return false;
		ComPtr<ID3D12GraphicsCommandList> target_command_list;
		if (!target_command_list && !idle_command_list.empty())
		{
			target_command_list = std::move(idle_command_list.back());
			idle_command_list.pop_back();
		}
		if (!target_command_list)
		{
			auto re = device->CreateCommandList(
				0, D3D12_COMMAND_LIST_TYPE_DIRECT, target_allocator.GetPointer(), nullptr,
				__uuidof(decltype(target_command_list)::Type), target_command_list.GetPointerVoidAdress()
			);
			assert(re);
		}
		if (!target_command_list)
		{
			command_allocator_current_frame.emplace_back(std::move(target_allocator));
			return false;
		}
		output.command = std::move(target_command_list);
		output.frame = current_frame;
		output.allocator = std::move(target_allocator);
		output.order = request.order;
	}

	FrameRenderer::~FrameRenderer()
	{
	}

	bool FrameRenderer::FinishPassRenderer(PassRenderer& output, PassGraphics& out_graphics)
	{
		assert(*this);
		if (!output)
			return false;
		output.PreFinishRender();
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
				finished_command_list.emplace_back(
					output.order.value_or(std::numeric_limits<std::size_t>::max()),
					std::move(output.command)
				);
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
				std::sort(
					finished_command_list.begin(),
					finished_command_list.end(),
					[](CommandList const& i1, CommandList const& i2) {
						return i1.order < i2.order;
					}
				);

				std::pmr::vector<ID3D12CommandList*> command_list;
				command_list.reserve(finished_command_list.size());

				for (auto& ite : finished_command_list)
				{
					command_list.push_back(ite.command_list.Get());
				}

				queue->ExecuteCommandLists(command_list.size(), command_list.data());
				auto re = queue->Signal(fence.Get(), current_frame);
				assert(SUCCEEDED(re));
				old_frame = current_frame;
				++current_frame;
			}
			
			for(auto ite : finished_command_list)
			{
				free_command_list.emplace_back(std::move(ite.command_list));
			}
			finished_command_list.clear();
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
			assert(cur != UINT64_MAX);
			if (cur == UINT64_MAX)
				return false;
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
		FrameRendererImp(Potato::IR::MemoryResourceRecord record, Dx12DevicePtr device, Dx12CommandQueuePtr ptr, Dx12FencePtr fence)
			: MemoryResourceRecordIntrusiveInterface(record), FrameRenderer(std::move(device), std::move(ptr), std::move(fence), record.GetMemoryResource())
		{

		}
		virtual void AddFrameRendererRef() const override { MemoryResourceRecordIntrusiveInterface::AddRef(); }
		virtual void SubFrameRendererRef() const override { MemoryResourceRecordIntrusiveInterface::SubRef(); }
	};

	FrameRenderer::Ptr Device::CreateFrameRenderer(std::pmr::memory_resource* resource)
	{
		Dx12CommandQueuePtr command_queue;
		D3D12_COMMAND_QUEUE_DESC desc{
			D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT,
			D3D12_COMMAND_QUEUE_PRIORITY::D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
			D3D12_COMMAND_QUEUE_FLAGS::D3D12_COMMAND_QUEUE_FLAG_NONE,
		0
		};

		auto result = device->CreateCommandQueue(
			&desc, __uuidof(decltype(command_queue)::InterfaceType), reinterpret_cast<void**>(command_queue.GetAddressOf())
		);
		if (SUCCEEDED(result))
		{
			Dx12FencePtr fence;
			auto re = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, __uuidof(decltype(fence)::InterfaceType), reinterpret_cast<void**>(fence.GetAddressOf()));
			if (SUCCEEDED(re))
			{
				return Potato::IR::MemoryResourceRecord::AllocateAndConstruct<FrameRendererImp>(resource, device, std::move(command_queue), std::move(fence));
			}
		}
		return {};
	}
		

	struct DeviceImp : public Device, public Potato::IR::MemoryResourceRecordIntrusiveInterface
	{
		DeviceImp(Potato::IR::MemoryResourceRecord record, Dx12FactoryPtr factory, Dx12DevicePtr device)
			: MemoryResourceRecordIntrusiveInterface(record), Device(std::move(factory), std::move(device))
		{}
	protected:
		virtual void AddDeviceRef() const override { MemoryResourceRecordIntrusiveInterface::AddRef(); }
		virtual void SubDeviceRef() const override { MemoryResourceRecordIntrusiveInterface::SubRef(); }
	};

	auto Device::Create(std::pmr::memory_resource* resource)-> Ptr
	{
		Dx12FactoryPtr rptr;
		UINT Flags = 0;
		Flags |= DXGI_CREATE_FACTORY_DEBUG;
		HRESULT result = CreateDXGIFactory2(Flags, __uuidof(decltype(rptr)::InterfaceType), reinterpret_cast<void**>(rptr.GetAddressOf()));
		if(SUCCEEDED(result))
		{
			Dx12DevicePtr dev_ptr;
			result = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_1, __uuidof(decltype(dev_ptr)::InterfaceType), reinterpret_cast<void**>(dev_ptr.GetAddressOf()));
			if(SUCCEEDED(result))
			{
				return Potato::IR::MemoryResourceRecord::AllocateAndConstruct<DeviceImp>(
					resource, std::move(rptr), std::move(dev_ptr)
				);
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
