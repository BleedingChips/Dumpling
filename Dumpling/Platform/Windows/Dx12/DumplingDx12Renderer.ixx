module;

#include "d3d12.h"
#include "dxgi1_6.h"
#include "wrl.h"

#undef interface
#undef max

export module DumplingDx12Renderer;

import std;
import PotatoPointer;
import PotatoIR;
import DumplingWindowsForm;
import DumplingPipeline;
import DumplingDXGI;
export import DumplingRendererTypes;

export namespace Dumpling
{


	struct RendererResource
	{
		struct Wrapper
		{
			template<typename Type> void AddRef(Type* ptr) const { ptr->AddRendererResourceRef(); }
			template<typename Type> void SubRef(Type* ptr) const { ptr->SubRendererResourceRef(); }
		};

		using Ptr = Potato::Pointer::IntrusivePtr<RendererResource, Wrapper>;

	protected:

		virtual void AddRendererResourceRef() const = 0;
		virtual void SubRendererResourceRef() const = 0;
	};

	using Dumpling::Windows::ComPtr;

	using DevicePtr = ComPtr<ID3D12Device>;
	using CommandQueuePtr = ComPtr<ID3D12CommandQueue>;
	using CommandAllocatorPtr = ComPtr<ID3D12CommandAllocator>;
	using CommandListPtr = ComPtr<ID3D12CommandList>;
	using GraphicCommandListPtr = ComPtr<ID3D12GraphicsCommandList>;
	using FencePtr = ComPtr<ID3D12Fence>;

	export struct Renderer;

	struct FormWrapper : public RendererResource, public Potato::Pointer::DefaultIntrusiveInterface
	{

		struct Config
		{
			
		};

		using Ptr = Potato::Pointer::IntrusivePtr<FormWrapper>;

		RendererResource::Ptr GetAvailableRenderResource() { return this; }

	protected:

		FormWrapper(Potato::IR::MemoryResourceRecord record, SwapChainPtr swap_chain)
			: record(record), swap_chain(std::move(swap_chain)){}

		virtual void AddRendererFormWrapperRef() const { DefaultIntrusiveInterface::AddRef(); }
		virtual void SubRendererFormWrapperRef() const { DefaultIntrusiveInterface::SubRef(); }
		virtual void AddRendererResourceRef() const override { AddRendererFormWrapperRef(); }
		virtual void SubRendererResourceRef() const override { SubRendererFormWrapperRef(); }

		void Release() override;

		Potato::IR::MemoryResourceRecord record;
		SwapChainPtr swap_chain;

		friend struct Renderer;
	};

	struct PassRendererIdentity
	{
		std::size_t reference_allocator_index;
	};

	export struct PassRenderer;

	export struct Renderer : public Potato::IR::MemoryResourceRecordIntrusiveInterface
	{

		using Ptr = Potato::Pointer::IntrusivePtr<Renderer>;

		FormWrapper::Ptr CreateFormWrapper(HardDevice& hard_device, Form& form, FormWrapper::Config fig = {}, std::pmr::memory_resource* resource = std::pmr::get_default_resource());

		struct PassRenderer : public Potato::IR::MemoryResourceRecordIntrusiveInterface
		{
			PassRenderer(Potato::IR::MemoryResourceRecord record, Renderer::Ptr owner, GraphicCommandListPtr ptr, PassRendererIdentity identity)
				: MemoryResourceRecordIntrusiveInterface(record), owner(std::move(owner)), command_list(std::move(ptr)), identity(identity) {}

			using Ptr = Potato::Pointer::IntrusivePtr<PassRenderer>;

			Potato::IR::StructLayoutObject::Ptr GetParameters() const { return {}; }
			PipelineRequester::Ptr GetPipelineRequester() const { return {}; }
			ID3D12GraphicsCommandList* operator->() const { return command_list.Get(); }
			bool ClearRendererTarget(RendererResource& render_target, Color color = Color::black, std::size_t index = 0);
			~PassRenderer();

		public:

			GraphicCommandListPtr command_list;
			Renderer::Ptr owner;
			PassRendererIdentity identity;

			friend struct Renderer;
		};

		IUnknown* GetDevice() const { return direct_queue.Get(); }
		
		//CommandQueuePtr GetDirectCommandQueue() const { return direct_queue; }

		static Renderer::Ptr Create(std::optional<AdapterDescription> adapter = std::nullopt, std::pmr::memory_resource* resource = std::pmr::get_default_resource());

		std::optional<std::size_t> CommitedAndSwapContext();
		std::tuple<bool, std::size_t> TryFlushFrame(std::size_t require_frame);
		bool FlushWindows(FormWrapper&);
		std::size_t GetFrame() const { std::shared_lock sl(frame_mutex); return current_frame;  }

		PassRenderer::Ptr PopPassRenderer(Pass const& pass, std::pmr::memory_resource* resource = std::pmr::get_default_resource());
		bool ExecutePipeline(PipelineRequester::Ptr requester, Pipeline const& pipeline) { return pipeline_manager.ExecutePipeline(std::move(requester), pipeline); }
		Pass::Ptr RegisterPass(PassProperty pass_property){ return pipeline_manager.RegisterPass(std::move(pass_property)); }
		//bool UnregisterPass(Pass const& pass) {}

	protected:

		Renderer(Potato::IR::MemoryResourceRecord record, DevicePtr device, CommandQueuePtr direct_queue);

		void FinishPassRenderer(GraphicCommandListPtr ptr, PassRendererIdentity identity);

		PipelineManager pipeline_manager;

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

		friend struct PassRenderer;
	};
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
