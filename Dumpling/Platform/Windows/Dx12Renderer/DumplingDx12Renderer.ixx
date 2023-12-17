module;

#include "d3d12.h"
#include "dxgi1_6.h"
#include "wrl.h"
export module DumplingDx12Renderer;

import std;
import PotatoPointer;
import PotatoIR;
import DumplingWin32Form;



export namespace Dumpling::Dx12
{
	template<typename PtrT>
	using ComPtr = Microsoft::WRL::ComPtr<PtrT>;

	using AdapterPtr = ComPtr<IDXGIAdapter1>;

	export struct Renderer;

	struct Context : public Potato::Pointer::DefaultIntrusiveInterface
	{

		using Ptr = Potato::Pointer::IntrusivePtr<Context>;

		AdapterPtr EnumAdapter(std::size_t index);

		static auto Create(std::pmr::memory_resource* resource = std::pmr::get_default_resource()) -> Ptr;

	protected:

		Context(Potato::IR::MemoryResourceRecord record, ComPtr<IDXGIFactory7> factory);

		Potato::IR::MemoryResourceRecord record;
		ComPtr<IDXGIFactory7> dxgi_factory;

		virtual void Release() override;

		friend struct Renderer;
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
	};

	export struct Renderer : public Win32::Win32Renderer, public Potato::Pointer::DefaultControllerViewerInterface
	{
		using Ptr = Potato::Pointer::ControllerPtr<Renderer>;

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
