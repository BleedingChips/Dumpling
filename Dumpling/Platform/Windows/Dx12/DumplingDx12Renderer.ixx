module;

#include "d3d12.h"
#include "dxgi1_6.h"
#include "wrl.h"
export module DumplingDx12Renderer;

import std;
import PotatoPointer;
import PotatoIR;
import DumplingFormInterface;
import DumplingWindowsForm;


/*
export namespace Dumpling::Dx12
{
	using Win32::ComPtr;

	using Adapter = ComPtr<IDXGIAdapter>;

	struct SwapChinSetting
	{
		std::size_t buffer_count = 2;
		DXGI_FORMAT format = DXGI_FORMAT::DXGI_FORMAT_B8G8R8A8_UNORM;
	};

	export struct HardwareDevice;

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
