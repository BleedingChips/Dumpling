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

export namespace Dumpling
{

	using Dumpling::Windows::ComPtr;

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
		bool Flush();

	protected:

		FormWrapper(SwapChainPtr swap_chain, DescriptorHeapPtr m_rtvHeap, std::size_t offset)
			: swap_chain(std::move(swap_chain)), m_rtvHeap(std::move(m_rtvHeap)), offset(offset) {}

		SwapChainPtr swap_chain;
		DescriptorHeapPtr m_rtvHeap;
		std::size_t offset;

		friend struct Renderer;
	};

	export struct FrameRenderer;

	struct PassRenderer
	{
		PassRenderer() = default;
		ID3D12GraphicsCommandList* operator->() const { return command.Get(); }
		bool ClearRendererTarget(RendererResource& render_target, Color color = Color::black, std::size_t index = 0);

		~PassRenderer()
		{
			assert(!command);
		}

	protected:

		GraphicCommandListPtr command;
		std::size_t reference_allocator_index = std::numeric_limits<std::size_t>::max();
		std::size_t frame = 0;

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
		std::size_t TryFlushFrame();

	protected:

		virtual ~FrameRenderer();

		bool PopPassRenderer_AssumedLocked(PassRenderer& output);
		bool FinishPassRenderer_AssumedLocked(PassRenderer& output);

		FrameRenderer(DevicePtr device, CommandQueuePtr queue, FencePtr fence, std::pmr::memory_resource* resource)
			: device(std::move(device)), queue(std::move(queue)), fence(std::move(fence)), total_allocator(resource), free_command_list(resource), need_commited_command(resource)
		{
			
		}

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
		std::size_t current_frame = 1;
		std::size_t last_flush_frame = 0;
		std::size_t running_count = 0;

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

		FormWrapper::Ptr CreateFormWrapper(Form& form, FormWrapper::Config fig = {}, std::pmr::memory_resource* resource = std::pmr::get_default_resource());
		FrameRenderer::Ptr CreateFrameRenderer(std::pmr::memory_resource* resource = std::pmr::get_default_resource());
		static bool InitDebugLayer();
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


	/*
	struct PassRendererIdentity
	{
		std::size_t reference_allocator_index;
	};

	

	struct FrameRenderer : public Potato::IR::MemoryResourceRecordIntrusiveInterface
	{
		using Ptr = Potato::Pointer::IntrusivePtr<PipelineRenderer>;
	protected:
		CommandQueuePtr direct_queue;
	};

	export struct Renderer : public Potato::IR::MemoryResourceRecordIntrusiveInterface
	{

		using Ptr = Potato::Pointer::IntrusivePtr<Renderer>;

		FormWrapper::Ptr CreateFormWrapper(HardDevice& hard_device, Form& form, FormWrapper::Config fig = {}, std::pmr::memory_resource* resource = std::pmr::get_default_resource());

		

		IUnknown* GetDevice() const { return direct_queue.Get(); }
		
		//CommandQueuePtr GetDirectCommandQueue() const { return direct_queue; }

		static Renderer::Ptr Create(std::optional<AdapterDescription> adapter = std::nullopt, std::pmr::memory_resource* resource = std::pmr::get_default_resource());

		std::optional<std::size_t> CommitedAndSwapContext();
		std::tuple<bool, std::size_t> TryFlushFrame(std::size_t require_frame);
		bool ForceFlush(std::size_t require_frame, std::chrono::steady_clock::duration waitting_duration = std::chrono::microseconds{10});

		bool FlushWindows(FormWrapper&);
		std::size_t GetFrame() const { std::shared_lock sl(pipeline_mutex); return current_frame;  }

		PipelineInstance::Ptr CreatePipelineInstance(Pipeline const& pipeline, std::pmr::memory_resource* resource = std::pmr::get_default_resource()) { return pipeline_manager.CreatPipelineInstance(pipeline, resource);}
		bool PopPassRenderer(PassRenderer& output_renderer, Pass const& pass);
		bool FinishPassRenderer(PassRenderer& output_renderer);
		bool ExecutePipeline(PipelineRequester::Ptr requester, PipelineInstance const& pipeline_instance) { return pipeline_manager.ExecutePipeline(std::move(requester), pipeline_instance); }
		Pass::Ptr RegisterPass(PassProperty pass_property){ return pipeline_manager.RegisterPass(std::move(pass_property)); }
		//bool UnregisterPass(Pass const& pass) {}

	protected:

		bool FinishPassRenderer_AssumedLocked(PassRenderer& output_renderer);
		bool PopPassRenderer_AssumedLocked(PassRenderer& output_renderer, Pass const& pass);

		Renderer(Potato::IR::MemoryResourceRecord record, DevicePtr device, CommandQueuePtr direct_queue);

		DevicePtr device;
		CommandQueuePtr direct_queue;

		enum class Status
		{
			IDLE,
			USING,
			WAITING,
			Block
		};

		struct AllocatorTuple
		{
			Status status;
			CommandAllocatorPtr allocator;
			std::size_t frame_number;
		};

		FencePtr current_fence;

		mutable std::shared_mutex pipeline_mutex;
		PipelineManager pipeline_manager;
		std::uint64_t current_frame = 1;
		std::uint64_t last_flush_frame_count = 0;

		std::pmr::vector<AllocatorTuple> allocators;

		struct CommandTuple
		{
			CommandListPtr list;
			PassRendererIdentity identity;
		};

		std::pmr::vector<CommandTuple> frame_command;
		std::size_t losing_command = 0;

		friend struct PassRenderer;
	};
	*/
}



/*

export namespace Dumpling::Dx12
{

	




	/*
	

	using Adapter = ComPtr<IDXGIAdapter>;

	struct SwapChinSetting
	{
		std::size_t buffer_count = 2;
		DXGI_FORMAT format = DXGI_FORMAT::DXGI_FORMAT_B8G8R8A8_UNORM;
	};

	export struct HardwareDevice;

	struct RendererOutput : public Dumpling::FormRendererOutput
	{
		using Ptr = Potato::Pointer::IntrusivePtr<RendererOutput, Dumpling::FormRendererOutput::Wrapper>;


	};

	export struct SwapChain : public Win32::SwapChain, public Potato::Pointer::DefaultIntrusiveInterface
	{

		using Ptr = Potato::Pointer::IntrusivePtr<SwapChain, FormPointerWrapperT>;

	protected:

		SwapChain(Potato::IR::MemoryResourceRecord record, SwapChinSetting Setting, ComPtr<IDXGIFactory> factory, ComPtr<ID3D12CommandQueue> command_queue)
			: record(record), Setting(Setting), factory(std::move(factory)), command_queue(std::move(command_queue)) {}

		virtual ~SwapChain() = default;

		virtual void FormAddRef() const override { return DefaultIntrusiveInterface::AddRef(); }
		virtual void FormSubRef() const override { return DefaultIntrusiveInterface::SubRef(); }
		virtual void Release() override;
		virtual void OnReInit(HWND, std::size_t size_x, std::size_t size_y) override;
		virtual void OnRelease(HWND) override {}
		virtual void OnUpdate() override {}

		Potato::IR::MemoryResourceRecord record;
		SwapChinSetting Setting;
		ComPtr<IDXGIFactory> factory;
		ComPtr<ID3D12CommandQueue> command_queue;
		ComPtr<IDXGISwapChain> swap_chain;

		friend struct HardwareDevice;
		friend struct FormPointerWrapperT;
	};

	void InitDebugLayer();

	export struct Renderer;

	export struct HardwareDevice
	{
		static HardwareDevice Create(
			bool EnableDebug 
#ifndef NDEBUG
				= true
#else
				= false
#endif
		);

		operator bool() const { return ptr; }
		HardwareDevice() = default;
		~HardwareDevice() = default;
		HardwareDevice(HardwareDevice const&) = default;
		HardwareDevice(HardwareDevice&&) = default;
		HardwareDevice& operator=(HardwareDevice const&) = default;
		HardwareDevice& operator=(HardwareDevice&&) = default;

		Adapter EnumAdapter(std::size_t index = 0);

		SwapChain::Ptr CreateSwapChain(SwapChinSetting setting, Renderer renderer, std::pmr::memory_resource* resource = std::pmr::get_default_resource());

	protected:

		HardwareDevice(ComPtr<IDXGIFactory> ptr) : ptr(std::move(ptr)) {}

		ComPtr<IDXGIFactory> ptr;

		friend struct Renderer;
	};

	struct SoftwareDevice
	{
		static SoftwareDevice Create(Adapter adapter);

		operator bool() const { return ptr; }
		SoftwareDevice() = default;
		~SoftwareDevice() = default;
		SoftwareDevice(SoftwareDevice const&) = default;
		SoftwareDevice(SoftwareDevice&&) = default;
		SoftwareDevice& operator=(SoftwareDevice const&) = default;
		SoftwareDevice& operator=(SoftwareDevice&&) = default;

		Renderer CreateRenderer();

	protected:

		SoftwareDevice(ComPtr<ID3D12Device> ptr) : ptr(std::move(ptr)) {}

		ComPtr<ID3D12Device> ptr;
	};

	export struct Renderer
	{
		operator bool() const { return ptr; }
		Renderer() = default;
		~Renderer() = default;
		Renderer(Renderer const&) = default;
		Renderer(Renderer&&) = default;
		Renderer& operator=(Renderer const&) = default;
		Renderer& operator=(Renderer&&) = default;

	protected:

		Renderer(ComPtr<ID3D12CommandQueue> ptr) : ptr(ptr) {}

		ComPtr<ID3D12CommandQueue> ptr;

		friend struct SoftwareDevice;
		friend struct HardwareDevice;
	};
	*/

	

	/*
	struct Device
	{
		static 
	};
	*/


	/*
	struct Factory : public Potato::Pointer::DefaultIntrusiveInterface
	{

		using Ptr = Potato::Pointer::IntrusivePtr<Factory>;

		AdapterPtr EnumAdapter(std::size_t index);

		static auto Create(std::pmr::memory_resource* resource = std::pmr::get_default_resource()) -> Ptr;

	protected:

		Factory(Potato::IR::MemoryResourceRecord record, ComPtr<IDXGIFactory7> factory);

		Potato::IR::MemoryResourceRecord record;
		ComPtr<IDXGIFactory7> dxgi_factory;

		virtual void Release() override;

		friend struct SwapChain;
	};

	export struct Device;

	struct CommandQueue : public Potato::Pointer::DefaultIntrusiveInterface
	{
		using Ptr = Potato::Pointer::IntrusivePtr<CommandQueue>;

	protected:

		static auto Create(Potato::Pointer::IntrusivePtr<Device> dev, ComPtr<ID3D12CommandQueue> command_queue, std::pmr::memory_resource* resource = std::pmr::get_default_resource()) -> Ptr;

		CommandQueue() = default;

		virtual void Release() override;

		Potato::IR::MemoryResourceRecord record;
		Potato::Pointer::IntrusivePtr<Device> owner;
		ComPtr<ID3D12CommandQueue> command_queue;

		friend struct Device;
		friend struct SwapChain;
	};

	export struct SwapChain : public Win32::SwapChain, public Potato::Pointer::DefaultControllerViewerInterface
	{
		using Ptr = Potato::Pointer::ControllerPtr<SwapChain>;

		static auto Create(CommandQueue::Ptr queue, Context::Ptr context, std::pmr::memory_resource* = std::pmr::get_default_resource()) -> Ptr;

	protected:

		virtual void ControllerRelease() override;
		virtual void ViewerRelease() override;
		virtual void FormAddRef() const override{ return AddViewerRef(); }
		virtual void FormSubRef() const override { return SubViewerRef(); }
		virtual void OnInit(HWND) override;
		virtual void OnRelease(HWND) override;
		virtual void OnUpdate() override;

		Potato::IR::MemoryResourceRecord record;
		CommandQueue::Ptr queue;
		Context::Ptr context;
		ComPtr<IDXGISwapChain1> swap_chain;
	};

	export struct Device : public Potato::Pointer::DefaultIntrusiveInterface
	{
		using Ptr = Potato::Pointer::IntrusivePtr<Device>;

		static auto Create(AdapterPtr adapter, std::pmr::memory_resource* resource = std::pmr::get_default_resource()) -> Ptr;

		CommandQueue::Ptr CreateCommandQueue(std::pmr::memory_resource* resource = std::pmr::get_default_resource());

	protected:

		virtual void Release() override;

		Device(Potato::IR::MemoryResourceRecord record, ComPtr<ID3D12Device> device) : record(record), device(std::move(device)) {}

		Potato::IR::MemoryResourceRecord record;
		ComPtr<ID3D12Device> device;
	};
	*/



	/*
	struct HardwareRenderers : public Potato::Pointer::DefaultIntrusiveInterface
	{
		using AdapterPtr = Win32::ComPtr<IDXGIAdapter4>;

		std::vector<AdapterPtr>

	};


	struct Device : public Potato::Pointer::DefaultIntrusiveInterface
	{
		Win32::ComPtr<ID3D12Device> device;
	};
	*/

	

//}
