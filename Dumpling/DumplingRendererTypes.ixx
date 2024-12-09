module;


export module DumplingRendererTypes;

import std;
import Potato;
import DumplingPipeline;

export namespace Dumpling
{

	struct Color
	{
		float R = 0.0f;
		float G = 0.0f;
		float B = 0.0f;
		float A = 1.0f;

		static Color red;
		static Color white;
		static Color blue;
		static Color black;
	};


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

	/*
	struct RendererFormWrapper
	{
		struct Wrapper
		{
			template<typename Type> void AddRef(Type* ptr) const { ptr->AddRendererFormWrapperRef(); }
			template<typename Type> void SubRef(Type* ptr) const { ptr->SubRendererFormWrapperRef(); }
		};

		using Ptr = Potato::Pointer::IntrusivePtr<RendererFormWrapper, Wrapper>;

		virtual bool TryFlush() { return false; }
		virtual RendererResource::Ptr GetAvailableRenderResource() = 0;

	protected:

		virtual void AddRendererFormWrapperRef() const = 0;
		virtual void SubRendererFormWrapperRef() const = 0;
	};

	struct PassRenderer
	{
		struct Wrapper
		{
			template<typename Type> void AddRef(Type* ptr) const { ptr->AddPassRendererRef(); }
			template<typename Type> void SubRef(Type* ptr) const { ptr->SubPassRendererRef(); }
		};

		using Ptr = Potato::Pointer::IntrusivePtr<PassRenderer, Wrapper>;

		virtual PipelineRequester::Ptr GetPipelineRequester() const = 0;
		virtual Potato::IR::StructLayoutObject::Ptr GetParameters() const = 0;

		virtual bool ClearRendererTarget(RendererResource& render_target, Color color = Color::black, std::size_t index = 0) = 0;

	protected:

		virtual void AddPassRendererRef() const = 0;
		virtual void SubPassRendererRef() const = 0;
	};
	*/




	
	/*

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
		static bool InitDebugLayout();

	protected:

		virtual void AddHardDeviceRef() const = 0;
		virtual void SubHardDeviceRef() const = 0;
	};


	*/



}