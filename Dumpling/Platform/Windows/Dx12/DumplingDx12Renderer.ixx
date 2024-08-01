module;

#include "d3d12.h"
#include "dxgi1_6.h"
#include "wrl.h"

#undef interface

export module DumplingDx12Renderer;

import std;
import PotatoPointer;
import PotatoIR;
import DumplingForm;
import DumplingWindowsForm;
import DumplingPipeline;
import DumplingRenderer;
import DumplingDXGI;



export namespace Dumpling::Dx12
{

	using Dumpling::Windows::ComPtr;

	using DevicePtr = ComPtr<ID3D12Device>;
	using CommandQueuePtr = ComPtr<ID3D12CommandQueue>;
	using CommandAllocatorPtr = ComPtr<ID3D12CommandAllocator>;
	using CommandListPtr = ComPtr<ID3D12CommandList>;
	using GraphicCommandListPtr = ComPtr<ID3D12GraphicsCommandList>;
	using FencePtr = ComPtr<ID3D12Fence>;

	export struct Renderer;

	struct FormWrapper : public Dumpling::RendererFormWrapper, public Dumpling::RendererResource, public Potato::Pointer::DefaultIntrusiveInterface
	{
		using Ptr = Dumpling::RendererFormWrapper::Ptr;

		virtual Dumpling::RendererResource::Ptr GetAvailableRenderResource() override { return this; }

	protected:

		FormWrapper(Potato::IR::MemoryResourceRecord record, DXGI::SwapChainPtr swap_chain)
			: record(record), swap_chain(std::move(swap_chain)){}

		virtual void AddRendererFormWrapperRef() const override { DefaultIntrusiveInterface::AddRef(); }
		virtual void SubRendererFormWrapperRef() const override { DefaultIntrusiveInterface::SubRef(); }
		virtual void AddRendererResourceRef() const override { AddRendererFormWrapperRef(); }
		virtual void SubRendererResourceRef() const override { SubRendererFormWrapperRef(); }

		void Release() override;

		Potato::IR::MemoryResourceRecord record;
		DXGI::SwapChainPtr swap_chain;

		friend struct Renderer;
	};

	struct PassRendererIdentity
	{
		std::size_t reference_allocator_index;
	};

	export struct PassRenderer;

	export struct Renderer : public DXGI::DXGIRenderer, public Potato::Pointer::DefaultIntrusiveInterface
	{

		

		using Ptr = Potato::Pointer::IntrusivePtr<Renderer, Dumpling::Renderer::Wrapper>;

		IUnknown* GetDevice() const override { return direct_queue.Get(); }
		RendererFormWrapper::Ptr CreateFormWrapper(DXGI::SwapChainPtr swap_chain, std::pmr::memory_resource* resource) override;

		//CommandQueuePtr GetDirectCommandQueue() const { return direct_queue; }

		static Dumpling::Renderer::Ptr Create(IDXGIAdapter* target_adapter, std::pmr::memory_resource* resource = std::pmr::get_default_resource());

		std::optional<std::size_t> CommitedAndSwapContext() override;
		std::tuple<bool, std::size_t> TryFlushFrame(std::size_t require_frame) override;
		bool FlushWindows(RendererFormWrapper&) override;
		std::size_t GetFrame() const override { std::shared_lock sl(frame_mutex); return current_frame;  }

	protected:

		Renderer(Potato::IR::MemoryResourceRecord record, DevicePtr device, CommandQueuePtr direct_queue);

		Dumpling::PassRenderer::Ptr CreatePassRenderer(::Dumpling::PipelineRequester::Ptr requester, Potato::IR::StructLayoutObject::Ptr parameter, PassProperty property, std::pmr::memory_resource* resource) override;
		void FinishPassRenderer(GraphicCommandListPtr ptr, PassRendererIdentity identity);

		void AddRendererRef() const override { DefaultIntrusiveInterface::AddRef(); }
		void SubRendererRef() const override { DefaultIntrusiveInterface::SubRef(); }
		void Release() override;

		Potato::IR::MemoryResourceRecord record;
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
		mutable std::shared_mutex frame_mutex;
		std::size_t current_frame = 1;
		std::size_t last_flush_frame = 0;


		mutable std::shared_mutex command_mutex;
		std::pmr::vector<AllocatorTuple> allocators;

		struct CommandTuple
		{
			CommandListPtr list;
			PassRendererIdentity identity;
		};

		std::pmr::vector<CommandTuple> frame_command;
		std::size_t losing_command = 0;

		friend struct Dumpling::Renderer::Wrapper;
		friend struct PassRenderer;
	};
	

	export struct PassRenderer : public Dumpling::PassRenderer, public Potato::IR::MemoryResourceRecordIntrusiveInterface
	{
		PassRenderer(Potato::IR::MemoryResourceRecord record, Renderer::Ptr owner, GraphicCommandListPtr ptr, PassRendererIdentity identity)
			: MemoryResourceRecordIntrusiveInterface(record), owner(std::move(owner)), command_list(std::move(ptr)), identity(identity)
		{
			
		}

		Potato::IR::StructLayoutObject::Ptr GetParameters() const override{ return {}; }
		PipelineRequester::Ptr GetPipelineRequester() const override { return {}; }
		ID3D12GraphicsCommandList* operator->() const { return command_list.Get(); }
		bool ClearRendererTarget(RendererResource& render_target, Color color, std::size_t index = 0) override;
		~PassRenderer();

	protected:

		virtual void AddPassRendererRef() const override { MemoryResourceRecordIntrusiveInterface::AddRef(); }
		virtual void SubPassRendererRef() const override { MemoryResourceRecordIntrusiveInterface::SubRef(); }


	public:

		GraphicCommandListPtr command_list;
		Renderer::Ptr owner;
		PassRendererIdentity identity;

		friend struct Renderer;
	};




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

	

}
