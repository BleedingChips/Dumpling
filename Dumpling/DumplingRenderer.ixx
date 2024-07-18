module;


export module DumplingRenderer;

import std;
import PotatoMisc;
import PotatoPointer;
import DumplingForm;
import PotatoIR;
import DumplingPipeline;

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

	enum class RendererResourceType
	{
		FLOAT32,
		UNSIGNED_INT64,
		TEXTURE_RT,
		TEXTURE_DS,
		TEXTURE1D,
		TEXTURE1D_ARRAY,
		TEXTURE2D,
		TEXTURE2D_ARRAY,
		TEXTURE3D,
		TEXTURE_CUBE
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

	struct RendererFormWrapper
	{
		struct Wrapper
		{
			template<typename Type> void AddRef(Type* ptr) const { ptr->AddRendererFormWrapperRef(); }
			template<typename Type> void SubRef(Type* ptr) const { ptr->SubRendererFormWrapperRef(); }
		};

		using Ptr = Potato::Pointer::IntrusivePtr<RendererFormWrapper, Wrapper>;

		virtual bool TryFlush() { return false; }

	protected:

		virtual void AddRendererFormWrapperRef() const = 0;
		virtual void SubRendererFormWrapperRef() const = 0;
	};

	struct SubRenderer
	{
		struct Wrapper
		{
			template<typename Type> void AddRef(Type* ptr) const { ptr->AddSubRendererRef(); }
			template<typename Type> void SubRef(Type* ptr) const { ptr->SubSubRendererRef(); }
		};

		using Ptr = Potato::Pointer::IntrusivePtr<SubRenderer, Wrapper>;

		virtual PipelineRequester::Ptr GetPipelineRequester() const = 0;
		virtual Potato::IR::StructLayoutObject::Ptr GetParameters() const = 0;

	protected:

		virtual void AddSubRendererRef() const = 0;
		virtual void SubSubRendererRef() const = 0;
	};

	struct Renderer
	{
		struct Wrapper
		{
			template<typename Type> void AddRef(Type* ptr) const { ptr->AddRendererRef(); }
			template<typename Type> void SubRef(Type* ptr) const { ptr->SubRendererRef(); }
		};

		using Ptr = Potato::Pointer::IntrusivePtr<Renderer, Wrapper>;

		virtual bool Execute(PipelineRequester::Ptr requester, Pipeline const& pipeline);
		virtual Pass::Ptr RegisterPass(PassProperty pass_property);
		virtual bool UnregisterPass(Pass const&);
		virtual SubRenderer::Ptr PopRequest(Pass const&);

	protected:

		Pass::Ptr RegisterPass_AssumedLocked(PassProperty pass_property);

		virtual SubRenderer::Ptr CreateSubRenderer(PipelineRequester::Ptr requester, Potato::IR::StructLayoutObject::Ptr parameter) = 0; 
		virtual void AddRendererRef() const = 0;
		virtual void SubRendererRef() const = 0;

		std::shared_mutex pass_mutex;

		enum class Status
		{
			WAITING,
			RUNNING,
			DONE,
		};

		struct PassTuple
		{
			std::u8string_view pass_name;
			Pass::Ptr pass;
		};

		std::pmr::vector<PassTuple> passes;

		std::mutex request_mutex;

		struct PassRequest
		{
			Pass::Ptr pass;
			Pipeline::Ptr pipeline;
			PipelineRequester::Ptr requester;
			Potato::IR::StructLayoutObject::Ptr object;
		};

		std::pmr::vector<PassRequest> requests;
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

		virtual RendererFormWrapper::Ptr CreateFormWrapper(Form& form, Renderer& renderer, FormRenderTargetProperty property = {}, std::pmr::memory_resource* resource = std::pmr::get_default_resource()) = 0;
		static Ptr Create(std::pmr::memory_resource* resource = std::pmr::get_default_resource());

		virtual std::optional<AdapterDescription> EnumAdapter(std::size_t ite) const = 0;

		virtual Renderer::Ptr CreateRenderer(std::optional<std::size_t> adapter_count = std::nullopt, std::pmr::memory_resource* resource = std::pmr::get_default_resource()) = 0;

	protected:

		virtual void AddHardDeviceRef() const = 0;
		virtual void SubHardDeviceRef() const = 0;
	};


	



}