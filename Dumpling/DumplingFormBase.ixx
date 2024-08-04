module;


export module DumplingForm;

import std;
import PotatoMisc;
import PotatoPointer;
import PotatoTaskSystem;

export namespace Dumpling
{

	export namespace FormEvent
	{
		enum class Category
		{
			UNACCEPTABLE = 0,
			SYSTEM = 0x01,
			MODIFY = 0x02,
			INPUT = 0x04
		};

		struct System
		{
			enum class Message
			{
				QUIT
			};
			Message message;
		};

		struct Modify
		{
			enum class Message
			{
				DESTROY
			};
			Message message;
		};

		struct Input
		{
			/*
			enum class Message
			{
				DESTROY
			};
			Message message;
			*/
		};

		enum class Respond
		{
			PASS,
			CAPTURED,
		};

	}

	struct Form;

	struct FormEventCapture
	{
		struct Wrapper
		{
			template<typename Type> void AddRef(Type* ptr) const { ptr->AddFormEventCaptureRef(); }
			template<typename Type> void SubRef(Type* ptr) const { ptr->SubFormEventCaptureRef(); }
		};

		using Ptr = Potato::Pointer::IntrusivePtr<FormEventCapture, Wrapper>;
		virtual FormEvent::Category AcceptedCategory() const { return FormEvent::Category::UNACCEPTABLE; } 
		virtual FormEvent::Respond Receive(Form& interface, FormEvent::Modify event) { return FormEvent::Respond::PASS; }
		virtual FormEvent::Respond Receive(Form& interface, FormEvent::System event) { return FormEvent::Respond::PASS; }
		virtual FormEvent::Respond Receive(Form& interface, FormEvent::Input event) { return FormEvent::Respond::PASS; }
	protected:

		virtual void AddFormEventCaptureRef() const = 0;
		virtual void SubFormEventCaptureRef() const = 0;
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
			std::size_t identity_id = 0,
			std::pmr::memory_resource* resource = std::pmr::get_default_resource()
		);

		template<typename Func>
		static bool PeekMessageEventOnce(Func&& func)
			requires(std::is_invocable_r_v<void, Func, FormEvent::System>)
		{
			return Form::PeekMessageEventOnce(
				[](void* data, FormEvent::System event)
				{
					(*static_cast<Func*>(data))(event);
				},
				&func
			);
		}

		template<typename Func>
		static std::size_t PeekMessageEvent(Func&& func)
			requires(std::is_invocable_r_v<void, Func, FormEvent::System>)
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

		Form(std::size_t identity_id = 0, std::pmr::memory_resource* resource = std::pmr::get_default_resource())
			: captures(resource), identity_id(identity_id) {}

		static void PostFormQuitEvent();

		bool InsertCapture(FormEventCapture::Ptr capture, std::size_t priority = 0)
		{
			std::lock_guard lg(capture_mutex);
			return InsertCapture_AssumedLocked(std::move(capture), priority);
		}
		bool InsertCapture_AssumedLocked(FormEventCapture::Ptr capture, std::size_t priority = 0);

	protected:

		virtual FormEvent::Respond HandleEvent(FormEvent::Modify event);
		virtual FormEvent::Respond HandleEvent(FormEvent::System event);
		virtual FormEvent::Respond HandleEvent(FormEvent::Input event);

		static bool PeekMessageEventOnce(void(*func)(void*, FormEvent::System), void*);


		std::size_t identity_id = 0;

		std::shared_mutex capture_mutex;
		struct CaptureTuple
		{
			std::size_t priority;
			FormEvent::Category acceptable_category;
			FormEventCapture::Ptr capture;
		};
		std::pmr::vector<CaptureTuple> captures;

		virtual void AddFormRef() const = 0;
		virtual void SubFormRef() const = 0;
	};

}

constexpr Dumpling::FormEvent::Category operator& (Dumpling::FormEvent::Category c1, Dumpling::FormEvent::Category c2)
{
	return static_cast<Dumpling::FormEvent::Category>(
		static_cast<std::size_t>(c1) & static_cast<std::size_t>(c2)
	);
}

constexpr Dumpling::FormEvent::Category operator| (Dumpling::FormEvent::Category c1, Dumpling::FormEvent::Category c2)
{
	return static_cast<Dumpling::FormEvent::Category>(
		static_cast<std::size_t>(c1) | static_cast<std::size_t>(c2)
	);
}