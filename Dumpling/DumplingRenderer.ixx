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

	struct RenderTargetSize
	{
		std::size_t width = 1;
		std::size_t height = 1;
		std::size_t length = 1;
	};

	struct FormRenderTargetProperty
	{
		std::size_t swap_chin_count = 2;
		std::optional<float> present;
	};

	struct RendererSocket
	{
		std::size_t uuid = 0;
		std::size_t uuid2 = 0;
	};

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

	struct Pipeline
	{
		struct Wrapper
		{
			template<typename Type> void AddRef(Type* ptr) const { ptr->AddPipelineRef(); }
			template<typename Type> void SubRef(Type* ptr) const { ptr->SubPipelineRef(); }
		};

		using Ptr = Potato::Pointer::IntrusivePtr<Pipeline, Wrapper>;

	protected:

		virtual void AddPipelineRef() const = 0;
		virtual void SubPipelineRef() const = 0;
	};

	struct SubRenderer
	{
		struct Wrapper
		{
			template<typename Type> void AddRef(Type* ptr) const { ptr->AddSubRendererRef(); }
			template<typename Type> void SubRef(Type* ptr) const { ptr->SubSubRendererRef(); }
		};

		using Ptr = Potato::Pointer::IntrusivePtr<SubRenderer, Wrapper>;

	protected:

		virtual void AddSubRendererRef() const = 0;
		virtual void SubSubRendererRef() const = 0;
	};

	struct Pass
	{
		struct Wrapper
		{
			template<typename Type> void AddRef(Type* ptr) const { ptr->AddPassRef(); }
			template<typename Type> void SubRef(Type* ptr) const { ptr->SubPassRef(); }
		};

		using Ptr = Potato::Pointer::IntrusivePtr<Pass, Wrapper>;

		void Execute(SubRenderer const&);

	protected:

		virtual void AddPassRef() const = 0;
		virtual void SubPassRef() const = 0;
	};

	struct Renderer
	{
		struct Wrapper
		{
			template<typename Type> void AddRef(Type* ptr) const { ptr->AddRendererRef(); }
			template<typename Type> void SubRef(Type* ptr) const { ptr->SubRendererRef(); }
		};

		using Ptr = Potato::Pointer::IntrusivePtr<Renderer, Wrapper>;

		virtual FormRenderer::Ptr CreateFormRenderer(std::optional<RendererSocket> socket = std::nullopt, FormRenderTargetProperty property = {}, std::pmr::memory_resource* resource = std::pmr::get_default_resource()) = 0;
		SubRenderer::Ptr GetSubRenderer();
		virtual void Execute(Pipeline const& pipeline);

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

		static Ptr Create(std::pmr::memory_resource* resource = std::pmr::get_default_resource());

		virtual std::optional<AdapterDescription> EnumAdapter(std::size_t ite) const = 0;

		virtual Renderer::Ptr CreateRenderer(std::optional<std::size_t> adapter_count = std::nullopt, std::pmr::memory_resource* resource = std::pmr::get_default_resource()) = 0;

	protected:

		virtual void AddHardDeviceRef() const = 0;
		virtual void SubHardDeviceRef() const = 0;
	};


	



}