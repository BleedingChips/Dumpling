module;

#include "d3d12.h"
#include "dxgi1_6.h"
#include "wrl.h"
#include <cassert>

#undef interface
#undef max

export module DumplingDx12Renderer;

import std;
import PotatoPointer;
import PotatoIR;
import DumplingWindowsForm;
import DumplingPipeline;
export import DumplingRendererTypes;

export namespace Dumpling::Dx12
{

	using Dumpling::Win32::ComPtr;

	using DevicePtr = ComPtr<ID3D12Device>;
	using CommandQueuePtr = ComPtr<ID3D12CommandQueue>;
	using CommandAllocatorPtr = ComPtr<ID3D12CommandAllocator>;
	using CommandListPtr = ComPtr<ID3D12CommandList>;
	using GraphicCommandListPtr = ComPtr<ID3D12GraphicsCommandList>;
	using FencePtr = ComPtr<ID3D12Fence>;
	using ResourcePtr = ComPtr<ID3D12Resource>;
	using DescriptorHeapPtr = ComPtr<ID3D12DescriptorHeap>;
	using FactoryPtr = ComPtr<IDXGIFactory2>;
	using SwapChainPtr = ComPtr<IDXGISwapChain3>;
	using DescriptorHeapPtr = ComPtr<ID3D12DescriptorHeap>;

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
			ResourcePtr resource_ptr;
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

		FormWrapper(SwapChainPtr swap_chain, DescriptorHeapPtr m_rtvHeap, Config config, std::size_t offset)
			: swap_chain(std::move(swap_chain)), m_rtvHeap(std::move(m_rtvHeap)), config(config), offset(offset)
		{
			current_index = this->swap_chain->GetCurrentBackBufferIndex();
			logic_current_index = current_index;
		}

		SwapChainPtr swap_chain;
		DescriptorHeapPtr m_rtvHeap;
		const std::size_t offset;
		const Config config;
		mutable std::shared_mutex logic_mutex;
		std::size_t current_index = 0;
		std::size_t logic_current_index= 0;

		friend struct Renderer;
	};

	export struct FrameRenderer;
	export struct PassRenderer;

	struct RendererTargetCarrier
	{

		static constexpr std::size_t max_render_target_count = 8;

		void Clear();
		std::optional<std::size_t> AddRenderTarget(RendererResource const& resource);
		bool SetDepthStencil(RendererResource const& resource);

	protected:

		struct ResourceRecord
		{
			ResourcePtr reference_resource;
			D3D12_CPU_DESCRIPTOR_HANDLE handle;
			D3D12_RESOURCE_STATES default_state = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COMMON;
		};

		std::array<ResourceRecord, max_render_target_count + 1> target_data;
		std::array<D3D12_CPU_DESCRIPTOR_HANDLE, max_render_target_count + 1> target;
		std::size_t render_target_count = 0;
		bool has_depth_stencil = false;

		friend struct PassRenderer;
	};

	export struct PassRenderer
	{
		PassRenderer() = default;
		ID3D12GraphicsCommandList* operator->() const { return command.Get(); }
		~PassRenderer()
		{
			assert(!command);
		}

		GraphicCommandListPtr::InterfaceType* GetCommandList() { return command.Get(); }


		void SetRenderTargets(RendererTargetCarrier const& render_targets);
		bool ClearRendererTarget(RendererTargetCarrier const& render_target, std::size_t index, Color color = Color::black);
		//bool ClearDepthStencil(RendererTargetCarrier const& render_target, float depth, uint8_t stencil);

	protected:

		GraphicCommandListPtr command;
		std::size_t reference_allocator_index = std::numeric_limits<std::size_t>::max();
		std::size_t frame = 0;

		std::array<D3D12_RESOURCE_BARRIER, (RendererTargetCarrier::max_render_target_count + 1) * 2> render_target_barriers;
		std::size_t render_target_barriers_count = 0;

		void PreFinishRender();

		friend struct FrameRenderer;
	};

	export struct FrameRenderer
	{
		struct Wrapper
		{
			void AddRef(FrameRenderer const* ptr) { ptr->AddFrameRendererRef(); }
			void SubRef(FrameRenderer const* ptr) { ptr->SubFrameRendererRef(); }
		};
		using Ptr = Potato::Pointer::IntrusivePtr<FrameRenderer, Wrapper>;

		bool PopPassRenderer(PassRenderer& output);
		bool FinishPassRenderer(PassRenderer& output);
		std::optional<std::size_t> CommitFrame();
		std::size_t GetCurrentFrame() const { std::shared_lock sl(frame_mutex); return current_frame; }
		std::size_t TryFlushFrame();
		bool FlushToLastFrame(std::optional<std::chrono::steady_clock::duration> time_duration = std::nullopt);
	protected:

		virtual ~FrameRenderer();

		bool PopPassRenderer_AssumedLocked(PassRenderer& output);
		bool FinishPassRenderer_AssumedLocked(PassRenderer& output);

		FrameRenderer(DevicePtr device, CommandQueuePtr queue, FencePtr fence, std::pmr::memory_resource* resource)
			: device(std::move(device)), queue(std::move(queue)), fence(std::move(fence)), total_allocator(resource), free_command_list(resource), need_commited_command(resource)
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
			CommandAllocatorPtr allocator;
			State state = State::Idle;
			std::size_t frame;
		};

		DevicePtr device;
		CommandQueuePtr queue;
		FencePtr fence;

		std::mutex renderer_mutex;
		std::pmr::vector<AllocateTuple> total_allocator;
		std::pmr::deque<GraphicCommandListPtr> free_command_list;
		std::pmr::vector<ID3D12CommandList*> need_commited_command;
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

		FormWrapper::Ptr CreateFormWrapper(Win32::Form& form, FormWrapper::Config fig = {}, std::pmr::memory_resource* resource = std::pmr::get_default_resource());
		FrameRenderer::Ptr CreateFrameRenderer(std::pmr::memory_resource* resource = std::pmr::get_default_resource());
		static bool InitDebugLayer();

		DevicePtr::InterfaceType* GetDevice() { return device.Get(); }

	protected:

		Device(FactoryPtr factory, DevicePtr device, CommandQueuePtr queue)
			:factory(std::move(factory)),  device(std::move(device)), queue(std::move(queue))
		{
			
		}

		FactoryPtr factory;
		DevicePtr device;
		CommandQueuePtr queue;

		virtual void AddDeviceRef() const = 0;
		virtual void SubDeviceRef() const = 0;
	};
}