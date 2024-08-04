module;


export module DumplingFormEvent;

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

	export struct Form;

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





	struct CaptureManager
	{

		bool InsertCapture(FormEventCapture::Ptr capture, std::size_t priority = 0)
		{
			std::lock_guard lg(capture_mutex);
			return InsertCapture_AssumedLocked(std::move(capture), priority);
		}

		bool InsertCapture_AssumedLocked(FormEventCapture::Ptr capture, std::size_t priority = 0);

		FormEvent::Respond HandleEvent(Form& form, FormEvent::Modify event);
		FormEvent::Respond HandleEvent(Form& form, FormEvent::System event);
		FormEvent::Respond HandleEvent(Form& form, FormEvent::Input event);

		CaptureManager(std::pmr::memory_resource* resource = std::pmr::get_default_resource())
			: captures(resource) {}

	protected:

		

		//static bool PeekMessageEventOnce(void(*func)(void*, FormEvent::System), void*);


		std::size_t identity_id = 0;

		std::shared_mutex capture_mutex;
		struct CaptureTuple
		{
			std::size_t priority;
			FormEvent::Category acceptable_category;
			FormEventCapture::Ptr capture;
		};
		std::pmr::vector<CaptureTuple> captures;
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