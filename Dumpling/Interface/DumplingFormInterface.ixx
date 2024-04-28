module;


export module DumplingFormInterface;

import std;
import PotatoMisc;
import PotatoPointer;
import PotatoTaskSystem;

export namespace Dumpling
{

	struct FormEvent
	{
		
	};

	struct FormEventRespond
	{
		
	};

	struct FormTaskProperty
	{
		Potato::Task::Priority priority = Potato::Task::Priority::Normal;
		std::chrono::microseconds sleep_duration = std::chrono::microseconds{ 100 };
	};

	struct FormInterface
	{
		struct Wrapper
		{
			template<typename Type> void AddRef(Type* ptr) const { ptr->AddFormInterfaceRef(); }
			template<typename Type> void SubRef(Type* ptr) const { ptr->SubFormInterfaceRef(); }
		};

		using Ptr = Potato::Pointer::IntrusivePtr<FormInterface, Wrapper>;

		virtual bool CommitedMessageLoop(Potato::Task::TaskContext& context, std::thread::id require_thread_id, FormTaskProperty property = {}) = 0;
		virtual ~FormInterface() = default;

	protected:

		virtual void AddFormInterfaceRef() const = 0;
		virtual void SubFormInterfaceRef() const = 0;
	};

	struct FormEventResponder
	{
		struct Wrapper
		{
			template<typename Type> void AddRef(Type* ptr) const { ptr->AddFormEventResponderRef(); }
			template<typename Type> void SubRef(Type* ptr) const { ptr->SubFormEventResponderRef(); }
		};

		using Ptr = Potato::Pointer::IntrusivePtr<FormEventResponder, Wrapper>;
		virtual std::optional<FormEventRespond> Respond(FormInterface const& interface, FormEvent event) { return std::nullopt; }

	protected:

		virtual void AddFormEventResponderRef() const {};
		virtual void SubFormEventResponderRef() const {};

	};

	struct FormRenderer
	{
		struct Wrapper
		{
			template<typename Type> void AddRef(Type* ptr) const { ptr->AddFormRendererRef(); }
			template<typename Type> void SubRef(Type* ptr) const { ptr->SubFormRendererRef(); }
		};

		using Ptr = Potato::Pointer::IntrusivePtr<FormRenderer, Wrapper>;

		virtual void OnFormCreated(FormInterface& target_form) = 0;

	protected:

		virtual void AddFormRendererRef() const {};
		virtual void SubFormRendererRef() const {};

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
		FormEventResponder::Ptr responder;
		FormRenderer::Ptr renderer;
		FormSize form_size;
		std::u8string_view title = u8"Default Dumping Form";
	};

	struct FormInitializer : protected Potato::Task::
	{
		
	};

	
	




	/*
	struct FormInterface
	{
		using Ptr = Potato::Pointer::IntrusivePtr<FormInterface, FormPointerWrapperT>;


		virtual void FormAddRef() const = 0;
		virtual void FormSubRef() const = 0;
	};
	*/




}