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

	export struct Renderer;

	struct FormWrapper : public Dumpling::RendererFormWrapper, public Dumpling::RendererResource, public Potato::Pointer::DefaultIntrusiveInterface
	{
		using Ptr = Dumpling::RendererFormWrapper::Ptr;

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

	

	export struct Renderer : public DXGI::DXGIRenderer, public Potato::Pointer::DefaultIntrusiveInterface
	{

		using Ptr = Potato::Pointer::IntrusivePtr<Renderer, Dumpling::Renderer::Wrapper>;

		IUnknown* GetDevice() const override { return direct_queue.Get(); }
		RendererFormWrapper::Ptr CreateFormWrapper(DXGI::SwapChainPtr swap_chain, std::pmr::memory_resource* resource) override;

		//CommandQueuePtr GetDirectCommandQueue() const { return direct_queue; }

		static Dumpling::Renderer::Ptr Create(IDXGIAdapter* target_adapter, std::pmr::memory_resource* resource = std::pmr::get_default_resource());

		void FlushFrame();

	protected:

		Renderer(Potato::IR::MemoryResourceRecord record, DevicePtr device, CommandQueuePtr direct_queue)
			: record(record), device(std::move(device)), direct_queue(std::move(direct_queue)) {}

		Dumpling::PassRenderer::Ptr CreatePassRenderer(::Dumpling::PipelineRequester::Ptr requester, Potato::IR::StructLayoutObject::Ptr parameter, PassProperty property, std::pmr::memory_resource* resource) override;

		void AddRendererRef() const override { DefaultIntrusiveInterface::AddRef(); }
		void SubRendererRef() const override { DefaultIntrusiveInterface::SubRef(); }
		void Release() override;

		
		Potato::IR::MemoryResourceRecord record;
		DevicePtr device;
		CommandQueuePtr direct_queue;

		enum class Status
		{
			Using,
			Idle
		};

		struct AllocatorTuple
		{
			Status status;
			CommandAllocatorPtr allocator;
			std::size_t max_frame_number;
		};

		std::mutex command_mutex;
		std::pmr::vector<AllocatorTuple> allocators;

		struct CommandTuple
		{
			CommandListPtr list;
			std::size_t reference_id;
		};

		std::pmr::vector<CommandTuple> command;

		friend struct Dumpling::Renderer::Wrapper;
	};
	

	struct PassRenderer : public Dumpling::PassRenderer, public Potato::Pointer::DefaultControllerViewerInterface
	{
		PassRenderer(Potato::IR::MemoryResourceRecord record)
			: record(record)
		{
			
		}

		Potato::IR::StructLayoutObject::Ptr GetParameters() const override{ return {}; }
		PipelineRequester::Ptr GetPipelineRequester() const override { return {}; }
		ID3D12CommandList* operator->() const { return command_list.Get(); }

	protected:

		virtual void AddPassRendererRef() const override { DefaultControllerViewerInterface::AddViewerRef();}
		virtual void SubPassRendererRef() const override { DefaultControllerViewerInterface::SubViewerRef();}

		virtual void ViewerRelease() override;
		virtual void ControllerRelease() override;


	public:

		Potato::IR::MemoryResourceRecord record;
		CommandListPtr command_list;
		Renderer::Ptr owner;
		std::size_t fast_command_list_reference;

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
