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
import DumplingRenderer;




export namespace Dumpling::Dx12
{

	using Dumpling::Windows::ComPtr;

	struct HardDevice : public Dumpling::HardDevice, public Potato::Pointer::DefaultIntrusiveInterface
	{

		using Ptr = Potato::Pointer::IntrusivePtr<HardDevice, Dumpling::HardDevice::Wrapper>;

		static Dumpling::HardDevice::Ptr Create(std::pmr::memory_resource* resource);

	protected:

		HardDevice(Potato::IR::MemoryResourceRecord record, ComPtr<IDXGIFactory> factory)
			: record(record), factory(std::move(factory)) {}

		void AddHardDeviceRef() const override { DefaultIntrusiveInterface::AddRef(); }
		void SubHardDeviceRef() const override { DefaultIntrusiveInterface::SubRef(); }
		void Release() override;
		std::optional<AdapterDescription> EnumAdapter(std::size_t ite) const override;
		virtual Renderer::Ptr CreateRenderer(std::size_t adapter_count = 0, std::pmr::memory_resource* resource = std::pmr::get_default_resource());

		Potato::IR::MemoryResourceRecord record;
		ComPtr<IDXGIFactory> factory;
	};

	struct CommandQueue : public Dumpling::CommandQueue, public Potato::Pointer::DefaultIntrusiveInterface
	{
		using Ptr = Potato::Pointer::DefaultIntrusiveInterface<CommandQueue, Dumpling::CommandQueue::Wrapper>;
	protected:
		CommandQueue(Potato::IR::MemoryResourceRecord record, ComPtr<ID3D12CommandQueue> queue)
			: record(record), queue(std::move(queue)) {}
		Potato::IR::MemoryResourceRecord record;
		ComPtr<ID3D12CommandQueue> queue;
		void AddCommandQueueRef() const override { DefaultIntrusiveInterface::AddRef(); }
		void SubCommandQueueRef() const override { DefaultIntrusiveInterface::SubRef(); }
		void Release() override;
	};

	struct Renderer : public Dumpling::Renderer, public Potato::Pointer::DefaultIntrusiveInterface
	{

		FormRenderTarget::Ptr CreateFormRenderTarget(FormRenderTargetProperty property = {}, std::pmr::memory_resource* resource = std::pmr::get_default_resource());

	protected:

		Renderer(Potato::IR::MemoryResourceRecord record, HardDevice::Ptr hard_device, ComPtr<ID3D12Device> device)
			: record(record), hard_device(std::move(hard_device)), device(std::move(device)) {}

		virtual Dumpling::CommandQueue::Ptr GetThreadSafeCommandQueue(std::thread::id thread_id) override;

		virtual CommandQueue::Ptr GetCommandQueue(std::thread::id thread_id);

		void AddRendererRef() const override { DefaultIntrusiveInterface::AddRef(); }
		void SubRendererRef() const override { DefaultIntrusiveInterface::SubRef(); }
		void Release() override;

		HardDevice::Ptr hard_device;
		Potato::IR::MemoryResourceRecord record;
		ComPtr<ID3D12Device> device;

		struct ThreadCommandRef
		{
			std::thread::id thread_id;
			ComPtr<ID3D12CommandQueue> CurrentQueue;
		};

		std::shared_mutex mutex;
		std::pmr::vector<ThreadCommandRef> commands;

		friend struct HardDevice;
	};

	struct FormRenderTarget : public Dumpling::FormRenderTarget, public Potato::Pointer::DefaultIntrusiveInterface
	{
		
	protected:
		FormRenderTarget(Potato::IR::MemoryResourceRecord record, Renderer::Ptr renderer, HardDevice::Ptr device, FormRenderTargetProperty property)
			: record(record), renderer(std::move(renderer)), device(std::move(device)), property(property) {}

		virtual void AddFormRenderTargetRef() const override { DefaultIntrusiveInterface::AddRef(); }
		virtual void SubFormRenderTargetRef() const override { DefaultIntrusiveInterface::SubRef(); }
		void Release() override;
		virtual void OnFormCreated(FormInterface& interface) override;

		Potato::IR::MemoryResourceRecord record;
		Renderer::Ptr renderer;
		HardDevice::Ptr device;
		FormRenderTargetProperty property;
		ComPtr<IDXGISwapChain> swap_chain;

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
