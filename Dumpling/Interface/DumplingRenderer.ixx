module;


export module DumplingRenderer;

import std;
import PotatoMisc;
import PotatoPointer;
import DumplingForm;

export namespace Dumpling
{

	enum TexFormat
	{
		RGBA8
	};


	struct FormRenderTargetProperty
	{
		std::size_t swap_chin_count = 2;
		TexFormat format = TexFormat::RGBA8;
	};

	struct CommandQueue
	{
		struct Wrapper
		{
			template<typename Type> void AddRef(Type* ptr) const { ptr->AddCommandQueueRef(); }
			template<typename Type> void SubRef(Type* ptr) const { ptr->SubCommandQueueRef(); }
		};

		using Ptr = Potato::Pointer::IntrusivePtr<CommandQueue, Wrapper>;

	protected:

		virtual void AddCommandQueueRef() const = 0;
		virtual void SubCommandQueueRef() const = 0;

	};

	struct Renderer
	{
		struct Wrapper
		{
			template<typename Type> void AddRef(Type* ptr) const { ptr->AddRendererRef(); }
			template<typename Type> void SubRef(Type* ptr) const { ptr->SubRendererRef(); }
		};

		using Ptr = Potato::Pointer::IntrusivePtr<Renderer, Wrapper>;

		virtual FormRenderTarget::Ptr CreateFormRenderTarget(FormRenderTargetProperty property = {}, std::pmr::memory_resource* resource = std::pmr::get_default_resource()) = 0;
		virtual CommandQueue::Ptr GetThreadSafeCommandQueue(std::thread::id thread_id = std::this_thread::get_id()) = 0;

	protected:

		virtual void AddRendererRef() const = 0;
		virtual void SubRendererRef() const = 0;
	};


	struct AdapterDescription
	{
		
	};


	struct HardDevice
	{
		struct Wrapper
		{
			template<typename Type> void AddRef(Type* ptr) const { ptr->AddHardDeviceRef(); }
			template<typename Type> void SubRef(Type* ptr) const { ptr->SubHardDeviceRef(); }
		};

		using Ptr = Potato::Pointer::IntrusivePtr<HardDevice, Wrapper>;

		virtual std::optional<AdapterDescription> EnumAdapter(std::size_t ite) const = 0;

		virtual Renderer::Ptr CreateRenderer(std::size_t adapter_count = 0, std::pmr::memory_resource* resource = std::pmr::get_default_resource()) = 0;

	protected:

		virtual void AddHardDeviceRef() const = 0;
		virtual void SubHardDeviceRef() const = 0;
	};


	



}