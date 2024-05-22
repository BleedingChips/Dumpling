module;


export module DumplingForm;

import std;
import PotatoMisc;
import PotatoPointer;
import PotatoTaskSystem;

export namespace Dumpling
{
	enum class FormEventEnum
	{
		DESTORYED,
	};



	struct FormEvent
	{
		FormEventEnum message;
	};

	struct FormEventRespond
	{
		enum class Style
		{
			Ignore,
			Output
		}style = Style::Ignore;
		static FormEventRespond ignore;
	};

	enum class FormStyle
	{
		FixedSizeWindow,
	};

	struct FormSize
	{
		std::size_t width = 1024;
		std::size_t height = 768;
	};

	struct FormProperty
	{
		FormStyle style = FormStyle::FixedSizeWindow;
		FormSize form_size;
		std::u8string_view title = u8"Default Dumping Form";
	};

	struct Form;

	struct FormEventResponder
	{
		struct Wrapper
		{
			template<typename Type> void AddRef(Type* ptr) const { ptr->AddFormEventResponderRef(); }
			template<typename Type> void SubRef(Type* ptr) const { ptr->SubFormEventResponderRef(); }
		};

		using Ptr = Potato::Pointer::IntrusivePtr<FormEventResponder, Wrapper>;
		virtual FormEventRespond Respond(Form& interface, FormEvent event) { return FormEventRespond::ignore; }

	protected:

		virtual void AddFormEventResponderRef() const = 0;
		virtual void SubFormEventResponderRef() const = 0;
	};

	struct FormRenderTarget
	{
		struct Wrapper
		{
			template<typename Type> void AddRef(Type* ptr) const { ptr->AddFormRenderTargetRef(); }
			template<typename Type> void SubRef(Type* ptr) const { ptr->SubFormRenderTargetRef(); }
		};

		using Ptr = Potato::Pointer::IntrusivePtr<FormRenderTarget, Wrapper>;

		virtual void OnFormCreated(Form& interface) = 0;

	protected:

		virtual void AddFormRenderTargetRef() const = 0;
		virtual void SubFormRenderTargetRef() const = 0;
	};

	
	struct Form
	{
		struct Wrapper
		{
			template<typename Type> void AddRef(Type* ptr) const { ptr->AddFormRef(); }
			template<typename Type> void SubRef(Type* ptr) const { ptr->SubFormRef(); }
		};

		using Ptr = Potato::Pointer::IntrusivePtr<Form, Wrapper>;

		virtual bool Init(FormProperty property = {}, std::pmr::memory_resource* temp = std::pmr::get_default_resource()) = 0;

		static Ptr Create(
			FormEventResponder::Ptr respond = {},
			FormRenderTarget::Ptr render_target = {},
			std::pmr::memory_resource* resource = std::pmr::get_default_resource()
		);

		template<typename Func>
		static bool PeekMessageEventOnce(Func&& func)
			requires(std::is_invocable_r_v<FormEventRespond, Func, Form*, FormEvent>)
		{
			return Form::PeekMessageEventOnce(
				[](void* data, Form* form, FormEvent event)
				{
					return (*static_cast<Func*>(data))(form, event);
				},
				&func
			);
		}
		
		virtual ~Form() = default;

		Form(FormEventResponder::Ptr responder, FormRenderTarget::Ptr renderer_target)
			: responder(std::move(responder)), renderer_target(std::move(renderer_target)) {}

	protected:

		virtual FormEventRespond HandleResponder(FormEvent event);

		static bool PeekMessageEventOnce(FormEventRespond(*func)(void*, Form*, FormEvent), void*);

		bool OverrideTemporaryOutputFunction(
			FormEventRespond(*output_func)(void*, Form*, FormEvent),
			void* output_func_data,
			bool set
		);

		FormRenderTarget::Ptr renderer_target;
		FormEventResponder::Ptr responder;

		std::shared_mutex mutex;
		FormEventRespond(*output_event_responder)(void*, Form*, FormEvent) = nullptr;
		void* output_event_responder_data = nullptr;

		virtual void AddFormRef() const = 0;
		virtual void SubFormRef() const = 0;
	};

}