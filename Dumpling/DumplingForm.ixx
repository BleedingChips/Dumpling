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
		DESTROY,
		QUIT,

		MAX_UNDEFINED,
	};

	struct FormEvent
	{
		FormEventEnum message;
	};

	enum class FormEventRespond
	{
		Default,
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
		virtual FormEventRespond Respond(Form& interface, FormEvent event) { return FormEventRespond::Default; }

	protected:

		virtual void AddFormEventResponderRef() const = 0;
		virtual void SubFormEventResponderRef() const = 0;
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
			std::size_t identity_id = 0,
			std::pmr::memory_resource* resource = std::pmr::get_default_resource()
		);

		template<typename Func>
		static bool PeekMessageEventOnce(Func&& func)
			requires(std::is_invocable_r_v<void, Func, Form*, FormEvent, FormEventRespond>)
		{
			return Form::PeekMessageEventOnce(
				[](void* data, Form* form, FormEvent event, FormEventRespond respond)
				{
					(*static_cast<Func*>(data))(form, event, respond);
				},
				&func
			);
		}

		template<typename Func>
		static std::size_t PeekMessageEvent(Func&& func)
			requires(std::is_invocable_r_v<void, Func, Form*, FormEvent, FormEventRespond>)
		{
			std::size_t count = 0;
			while(PeekMessageEventOnce(std::forward<Func>(func)))
			{
				count += 1;
			}
			return count;
		}
		
		virtual ~Form() = default;

		std::size_t GetIdentityID() const { return identity_id; }

		Form(FormEventResponder::Ptr responder, std::size_t identity_id)
			: responder(std::move(responder)), identity_id(identity_id) {}

		static void PostFormQuitEvent();

	protected:

		virtual FormEventRespond HandleResponder(FormEvent event);

		static bool PeekMessageEventOnce(void(*func)(void*, Form*, FormEvent, FormEventRespond), void*);

		

		FormEventResponder::Ptr responder;
		std::size_t identity_id = 0;

		virtual void AddFormRef() const = 0;
		virtual void SubFormRef() const = 0;
	};

}