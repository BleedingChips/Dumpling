module;


export module DumplingInterfaceForm;

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
		
	};

	struct FormTaskProperty
	{
		Potato::Task::Priority priority = Potato::Task::Priority::Normal;
		std::chrono::microseconds sleep_duration = std::chrono::microseconds{ 100 };
		std::u8string_view display_name = u8"Win32 Default Form Manager";
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
		virtual std::optional<FormEventRespond> Respond(Form& interface, FormEvent event) { return std::nullopt; }

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
		using Ptr = Potato::Pointer::IntrusivePtr<Form, Wrapper>;

		
		virtual ~Form() = default;

	protected:

		virtual void AddFormRef() const = 0;
		virtual void SubFormRef() const = 0;
	};


	struct FormManager
	{

		struct Wrapper
		{
			template<typename Type> void AddRef(Type* ptr) const { ptr->AddFormManagerRef(); }
			template<typename Type> void SubRef(Type* ptr) const { ptr->SubFormManagerRef(); }
		};

		virtual bool Commite(
			Potato::Task::TaskContext& context,
			std::thread::id thread_id,
			FormTaskProperty property = {}
		) { return false; }

		virtual Form::Ptr CreateForm(
			FormProperty property = {},
			FormEventResponder::Ptr responder = {},
			FormRenderTarget::Ptr renderer = {},
			std::pmr::memory_resource* resource = std::pmr::get_default_resource()
			) = 0;

		virtual void CloseMessageLoop() = 0;

		using Ptr = Potato::Pointer::IntrusivePtr<FormManager, Wrapper>;

	protected:

		virtual void AddFormManagerRef() const = 0;
		virtual void SubFormManagerRef() const = 0;
	};

}