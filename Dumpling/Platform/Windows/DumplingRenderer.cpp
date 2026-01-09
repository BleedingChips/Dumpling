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

	ComPtr<ID3D12Resource> Device::CreateUploadResource(std::size_t buffer_size)
	{
		return {};
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

	bool FrameRenderer::PopPassRenderer(PassRenderer& output, PassRequest const& request)
	{
		assert(*this);
		if (output)
			return false;
		ComPtr<ID3D12CommandAllocator> target_allocator;
		if (!target_allocator && !current_frame_allocators.empty())
		{
			target_allocator = std::move(current_frame_allocators.back());
			current_frame_allocators.pop_back();
		}
		if (!target_allocator && !idle_allocator.empty())
		{
			target_allocator = std::move(idle_allocator.back());
			idle_allocator.pop_back();
		}
		if (!target_allocator)
		{
			auto re = device->CreateCommandAllocator(
				D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT,
				__uuidof(decltype(target_allocator)::Type), target_allocator.GetPointerVoidAdress()
			);
			assert(SUCCEEDED(re));
		}
		if (!target_allocator)
			return false;
		ComPtr<ID3D12GraphicsCommandList> target_command_list;
		if (!target_command_list && !idle_command_list.empty())
		{
			target_command_list = std::move(idle_command_list.back());
			idle_command_list.pop_back();
			target_command_list->Reset(target_allocator.GetPointer(), nullptr);
		}
		if (!target_command_list)
		{
			auto re = device->CreateCommandList(
				0, D3D12_COMMAND_LIST_TYPE_DIRECT, target_allocator.GetPointer(), nullptr,
				__uuidof(decltype(target_command_list)::Type), target_command_list.GetPointerVoidAdress()
			);
			assert(SUCCEEDED(re));
		}
		if (!target_command_list)
		{
			current_frame_allocators.emplace_back(std::move(target_allocator));
			return false;
		}
		output.command = std::move(target_command_list);
		output.frame = current_frame;
		output.allocator = std::move(target_allocator);
		output.order = request.order;
		return true;
	}

	FrameRenderer::~FrameRenderer()
	{
	}

	bool FrameRenderer::FinishPassRenderer(PassRenderer& output)
	{
		assert(*this);
		if (!output)
			return false;
		output.PreFinishRender();
		output.command->Close();
		current_frame_command_lists.emplace_back(
			output.order.value_or(std::numeric_limits<std::size_t>::max()),
			std::move(output.command)
		);
		current_frame_allocators.emplace_back(std::move(output.allocator));
		output.PosFinishRender();
		return true;
	}

	std::optional<std::size_t> FrameRenderer::CommitFrame()
	{
		std::sort(
			current_frame_command_lists.begin(),
			current_frame_command_lists.end(),
			[](OrderedCommandList const& i1, OrderedCommandList const& i2) {
				return i1.order < i2.order;
			}
		);

		template_buffer.clear();

		for (auto& ite : current_frame_command_lists)
		{
			template_buffer.push_back(ite.command_list.GetPointer());
		}

		command_queue->ExecuteCommandLists(template_buffer.size(), template_buffer.data());
		auto re = command_queue->Signal(fence.GetPointer(), current_frame);
		assert(SUCCEEDED(re));

		for (auto& ite : current_frame_command_lists)
		{
			last_frame_command_list.emplace_back(
				std::move(ite.command_list), current_frame
			);
		}

		for (auto& ite : current_frame_allocators)
		{
			last_frame_allocator.emplace_back(
				std::move(ite), current_frame
			);
		}

		current_frame_command_lists.clear();
		current_frame_allocators.clear();

		auto old_frame = current_frame;
		++current_frame;
		return old_frame;
	}

	std::uint64_t FrameRenderer::FlushFrame()
	{
		auto current_frame = fence->GetCompletedValue();
		bool need_remove = false;
		for (auto& [ite, frame] : last_frame_allocator)
		{
			if (frame <= current_frame)
			{
				idle_allocator.emplace_back(std::move(ite));
				ite.Reset();
				need_remove = true;
			}
			else
				break;
		}

		if (need_remove)
		{
			last_frame_allocator.erase(
				std::remove_if(
					last_frame_allocator.begin(),
					last_frame_allocator.end(),
					[](auto& ite) {
						return !std::get<0>(ite);
					}
				),
				last_frame_allocator.end()
			);
			need_remove = false;
		}

		for (auto& [ite, frame] : last_frame_command_list)
		{
			if (frame <= current_frame)
			{
				idle_command_list.emplace_back(std::move(ite));
				ite.Reset();
				need_remove = true;
			}
			else
				break;
		}

		if (need_remove)
		{
			last_frame_command_list.erase(
				std::remove_if(
					last_frame_command_list.begin(),
					last_frame_command_list.end(),
					[](auto& ite) {
						return !std::get<0>(ite);
					}
				),
				last_frame_command_list.end()
			);
			need_remove = false;
		}
		return current_frame;
	}

	bool FrameRenderer::Init(ComPtr<ID3D12Device> in_devive)
	{
		if (*this || !in_devive)
			return false;

		ComPtr<ID3D12CommandQueue> command_queue;
		D3D12_COMMAND_QUEUE_DESC desc{
			D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT,
			D3D12_COMMAND_QUEUE_PRIORITY::D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
			D3D12_COMMAND_QUEUE_FLAGS::D3D12_COMMAND_QUEUE_FLAG_NONE,
		0
		};

		auto result = in_devive->CreateCommandQueue(
			&desc, __uuidof(decltype(command_queue)::Type), command_queue.GetPointerVoidAdress()
		);
		if (SUCCEEDED(result))
		{
			ComPtr<ID3D12Fence> fence;
			auto re = in_devive->CreateFence(0, D3D12_FENCE_FLAG_NONE, __uuidof(decltype(fence)::Type), fence.GetPointerVoidAdress());
			if (SUCCEEDED(re))
			{
				this->device = std::move(in_devive);
				this->command_queue = std::move(command_queue);
				this->fence = std::move(fence);
				return true;
			}
		}
		return {};
	}

	bool ResourceStreamer::Init(ComPtr<ID3D12Device> in_device)
	{
		if (*this || !in_device)
			return false;

		ComPtr<ID3D12CommandQueue> command_queue;
		D3D12_COMMAND_QUEUE_DESC desc{
			D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_COPY,
			D3D12_COMMAND_QUEUE_PRIORITY::D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
			D3D12_COMMAND_QUEUE_FLAGS::D3D12_COMMAND_QUEUE_FLAG_NONE,
		0
		};

		auto result = in_device->CreateCommandQueue(
			&desc, __uuidof(decltype(command_queue)::Type), command_queue.GetPointerVoidAdress()
		);
		if (SUCCEEDED(result))
		{
			ComPtr<ID3D12Fence> fence;
			auto re = in_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, __uuidof(decltype(fence)::Type), fence.GetPointerVoidAdress());
			if (SUCCEEDED(re))
			{
				this->device = std::move(in_device);
				this->command_queue = std::move(command_queue);
				this->fence = std::move(fence);
				return true;
			}
		}
		return {};
	}

	bool ResourceStreamer::PopRequester(StreamerRequest& request)
	{
		if (request)
			return false;

		ComPtr<ID3D12CommandAllocator> current_allocator;
		if (!current_allocator && !idle_allocator.empty())
		{
			current_allocator = std::move(*idle_allocator.rbegin());
			idle_allocator.pop_back();
		}

		if (!current_allocator)
		{
			auto re = device->CreateCommandAllocator(
				D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_COPY,
				__uuidof(decltype(current_allocator)::Type), current_allocator.GetPointerVoidAdress()
			);
			assert(SUCCEEDED(re));
		}

		if (!current_allocator)
			return false;


		ComPtr<ID3D12CommandList> current_command;

		if (!current_command && !idle_command_list.empty())
		{
			current_command = std::move(*idle_command_list.rbegin());
			idle_command_list.pop_back();
		}

		if (!current_command)
		{
			auto re = device->CreateCommandList(
				0, D3D12_COMMAND_LIST_TYPE_DIRECT, current_allocator.GetPointer(), nullptr,
				__uuidof(decltype(current_command)::Type), current_command.GetPointerVoidAdress()
			);
		}

		if (!current_command)
		{
			idle_allocator.emplace_back(std::move(current_allocator));
			return false;
		}

		request.commands = std::move(current_command);
		request.allocator = std::move(current_allocator);
		return true;
	}

	bool ResourceStreamer::TryFlushTo(std::uint64_t fence_value)
	{
		auto current_version = fence->GetCompletedValue();
		if (current_version != last_flush_version)
		{
			last_flush_version = current_version;
			
			bool change = false;

			for (auto& [version, allocator] : waitting_allocator)
			{
				if (version <= current_version)
				{
					idle_allocator.push_back(std::move(allocator));
					change = true;
					break;
				}
			}

			if (change)
			{
				waitting_allocator.erase(
					std::remove_if(waitting_allocator.begin(), waitting_allocator.end(), [](std::tuple<std::uint64_t, ComPtr<ID3D12CommandAllocator>>& ite) {
						return !std::get<1>(ite);
						}),
					waitting_allocator.end()
				);
			}

		}
		return current_version;
	}

	bool Device::InitFrameRenderer(FrameRenderer& target_frame_renderer)
	{
		return target_frame_renderer.Init(device);
	}

	bool Device::InitResourceStreamer(ResourceStreamer& target_resource_streamer)
	{
		return target_resource_streamer.Init(device);
	}

	bool Device::Init(Config config)
	{
		if (*this)
			return false;
		ComPtr<IDXGIFactory3> new_factory;
		UINT Flags = 0;
		Flags |= DXGI_CREATE_FACTORY_DEBUG;
		HRESULT result = CreateDXGIFactory2(Flags, __uuidof(decltype(new_factory)::Type), new_factory.GetPointerVoidAdress());
		if (SUCCEEDED(result))
		{
			ComPtr<ID3D12Device> dev_ptr;
			result = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_1, __uuidof(decltype(dev_ptr)::Type), dev_ptr.GetPointerVoidAdress());
			if (SUCCEEDED(result))
			{
				factory = std::move(new_factory);
				device = std::move(dev_ptr);
				return true;
			}
		}
		return false;
	}

	FormWrapper::Ptr Device::CreateFormWrapper(Form const& form, FrameRenderer& render, FormWrapper::Config fig, std::pmr::memory_resource* resource)
	{
		assert(factory);

		if (form)
		{
			RECT rect;
			if (GetClientRect(form.GetPlatformValue(), &rect))
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
				if (SUCCEEDED(re))
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
						ref.reference_resource.GetPointer(),
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
						ite.reference_resource.GetPointer(),
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
						ref.reference_resource.GetPointer(),
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
						ite.reference_resource.GetPointer(),
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

	void PassRenderer::PosFinishRender()
	{
		Reset();
	}

	void PassRenderer::Reset()
	{
		command.Reset();
		allocator.Reset();
		order.reset();
		frame = std::numeric_limits<std::size_t>::max();
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
