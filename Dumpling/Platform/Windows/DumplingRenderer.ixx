module;

#include "wrl.h"
#include "d3d12.h"
#include "dxgi1_6.h"
#include <cassert>

#undef interface
#undef max

export module DumplingRenderer;

import std;
import Potato;
import DumplingForm;
import DumplingPipeline;
import DumplingRendererTypes;
import DumplingDX12;

export namespace Dumpling
{
	export struct Renderer;

	struct RendererResource
	{
		struct Wrapper
		{
			template<typename Type> void AddRef(Type* ptr) const { ptr->AddRendererResourceRef(); }
			template<typename Type> void SubRef(Type* ptr) const { ptr->SubRendererResourceRef(); }
		};

		using Ptr = Potato::Pointer::IntrusivePtr<RendererResource, Wrapper>;

		struct Description
		{
			Dx12ResourcePtr resource_ptr;
			D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle;
			D3D12_RESOURCE_STATES default_state;
		};

		virtual Description GetDescription(D3D12_RESOURCE_STATES require_state) const = 0;

	protected:

		virtual void AddRendererResourceRef() const = 0;
		virtual void SubRendererResourceRef() const = 0;
	};

	struct FormWrapper : public RendererResource
	{

		struct Config
		{
			std::size_t swap_buffer_count = 2;
		};

		using Ptr = Potato::Pointer::IntrusivePtr<FormWrapper, RendererResource::Wrapper>;

		RendererResource::Ptr GetAvailableRenderResource() { return this; }

		Description GetDescription(D3D12_RESOURCE_STATES require_state) const override;
		bool Present(std::size_t syn_interval = 1);
		bool LogicPresent();

	protected:

		FormWrapper(Dx12SwapChainPtr swap_chain, Dx12DescriptorHeapPtr m_rtvHeap, Config config, std::size_t offset)
			: swap_chain(std::move(swap_chain)), m_rtvHeap(std::move(m_rtvHeap)), config(config), offset(offset)
		{
			current_index = this->swap_chain->GetCurrentBackBufferIndex();
			logic_current_index = current_index;
		}

		Dx12SwapChainPtr swap_chain;
		Dx12DescriptorHeapPtr m_rtvHeap;
		const std::size_t offset;
		const Config config;
		mutable std::shared_mutex logic_mutex;
		std::size_t current_index = 0;
		std::size_t logic_current_index= 0;

		friend struct Renderer;
	};

	export struct FrameRenderer;
	export struct PassRenderer;

	struct RenderTargetSet
	{

		static constexpr std::size_t max_render_target_count = 8;

		void Clear();
		std::optional<std::size_t> AddRenderTarget(RendererResource const& resource);
		bool SetDepthStencil(RendererResource const& resource);
		RenderTargetSet() = default;
		RenderTargetSet(RenderTargetSet const&) = default;
		RenderTargetSet(RenderTargetSet&& set);

	protected:

		struct ResourceRecord
		{
			Dx12ResourcePtr reference_resource;
			D3D12_CPU_DESCRIPTOR_HANDLE handle;
			D3D12_RESOURCE_STATES default_state = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COMMON;
		};

		std::array<ResourceRecord, max_render_target_count + 1> target_data;
		std::array<D3D12_CPU_DESCRIPTOR_HANDLE, max_render_target_count + 1> target;
		std::size_t render_target_count = 0;
		bool has_depth_stencil = false;

		friend struct PassRenderer;
	};

	struct PassRenderer
	{
		PassRenderer() = default;
		ID3D12GraphicsCommandList* operator->() const { return command.Get(); }
		~PassRenderer()
		{
			assert(!command);
		}

		Dx12GraphicCommandListPtr::InterfaceType* GetCommandList() { return command.Get(); }


		void SetRenderTargets(RenderTargetSet const& render_targets);
		bool ClearRendererTarget(std::size_t index, Color color = Color::black);
		//bool ClearDepthStencil(RendererTargetCarrier const& render_target, float depth, uint8_t stencil);

	protected:

		Dx12GraphicCommandListPtr command;
		std::size_t reference_allocator_index = std::numeric_limits<std::size_t>::max();
		std::size_t frame = 0;
		std::optional<std::size_t> order;

		std::array<D3D12_RESOURCE_BARRIER, (RenderTargetSet::max_render_target_count + 1) * 2> render_target_barriers;
		std::array<D3D12_CPU_DESCRIPTOR_HANDLE, RenderTargetSet::max_render_target_count + 1> cache_render_target;
		std::size_t render_target_barriers_count = 0;

		void PreFinishRender();

		friend struct FrameRenderer;
	};

	struct FrameRenderer
	{
		struct Wrapper
		{
			void AddRef(FrameRenderer const* ptr) { ptr->AddFrameRendererRef(); }
			void SubRef(FrameRenderer const* ptr) { ptr->SubFrameRendererRef(); }
		};
		using Ptr = Potato::Pointer::IntrusivePtr<FrameRenderer, Wrapper>;

		bool PopPassRenderer(PassRenderer& output, PassRequest const& request);
		bool FinishPassRenderer(PassRenderer& output);
		std::optional<std::size_t> CommitFrame();
		std::size_t GetCurrentFrame() const { std::shared_lock sl(frame_mutex); return current_frame; }
		std::size_t TryFlushFrame();
		bool FlushToLastFrame(std::optional<std::chrono::steady_clock::duration> time_duration = std::nullopt);

	
	protected:

		virtual ~FrameRenderer();

		bool PopPassRenderer_AssumedLocked(PassRenderer& output, PassRequest const& request);
		bool FinishPassRenderer_AssumedLocked(PassRenderer& output);

		FrameRenderer(Dx12DevicePtr device, Dx12CommandQueuePtr queue, Dx12FencePtr fence, std::pmr::memory_resource* resource)
			: device(std::move(device)), queue(std::move(queue)), fence(std::move(fence)), total_allocator(resource), free_command_list(resource), finished_command_list(resource)
		{
			
		}

		void ResetAllocator_AssumedLocked(std::size_t frame);

		enum class State
		{
			Idle,
			Using,
			Waiting,
			Done,
		};

		struct AllocateTuple
		{
			Dx12CommandAllocatorPtr allocator;
			State state = State::Idle;
			std::size_t frame;
		};

		Dx12DevicePtr device;
		Dx12CommandQueuePtr queue;
		Dx12FencePtr fence;

		std::mutex renderer_mutex;
		std::pmr::vector<AllocateTuple> total_allocator;
		std::pmr::deque<Dx12GraphicCommandListPtr> free_command_list;

		struct CommandList
		{
			std::size_t order = std::numeric_limits<std::size_t>::max();
			Dx12GraphicCommandListPtr command_list;
		};

		std::pmr::vector<CommandList> finished_command_list;
		std::size_t last_flush_frame = 0;
		std::size_t running_count = 0;

		mutable std::shared_mutex frame_mutex;
		std::size_t current_frame = 0;

		//struct Allocator

		virtual void AddFrameRendererRef() const = 0;
		virtual void SubFrameRendererRef() const = 0;
	};

	struct Device
	{
		struct Wrapper
		{
			void AddRef(Device const* ptr) { ptr->AddDeviceRef(); }
			void SubRef(Device const* ptr) { ptr->SubDeviceRef(); }
		};
		using Ptr = Potato::Pointer::IntrusivePtr<Device, Wrapper>;

		static Ptr Create(std::pmr::memory_resource* resource = std::pmr::get_default_resource());

		FormWrapper::Ptr CreateFormWrapper(Form const& form, FormWrapper::Config fig = {}, std::pmr::memory_resource* resource = std::pmr::get_default_resource());
		FrameRenderer::Ptr CreateFrameRenderer(std::pmr::memory_resource* resource = std::pmr::get_default_resource());
		static bool InitDebugLayer();

		Dx12DevicePtr::InterfaceType* GetDevice() { return device.Get(); }

	protected:

		Device(Dx12FactoryPtr factory, Dx12DevicePtr device, Dx12CommandQueuePtr queue)
			:factory(std::move(factory)),  device(std::move(device)), queue(std::move(queue))
		{
			
		}

		Dx12FactoryPtr factory;
		Dx12DevicePtr device;
		Dx12CommandQueuePtr queue;

		virtual void AddDeviceRef() const = 0;
		virtual void SubDeviceRef() const = 0;
	};
}