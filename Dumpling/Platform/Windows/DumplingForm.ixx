module;

#include <Windows.h>
#include <wrl.h>

#undef max
#undef interface

export module DumplingForm;

import std;
import PotatoPointer;
import PotatoIR;
import PotatoTaskSystem;
export import DumplingFormEvent;


export namespace Dumpling
{


	export struct Form;

	struct FormEventCapturePlatform
	{
		struct Wrapper
		{
			void AddRef(FormEventCapturePlatform const* ptr) const { ptr->AddFormEventCapturePlatformRef(); }
			void SubRef(FormEventCapturePlatform const* ptr) const { ptr->SubFormEventCapturePlatformRef(); }
		};

		using Ptr = Potato::Pointer::IntrusivePtr<FormEventCapturePlatform, Wrapper>;

		virtual HRESULT RespondEvent(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, std::optional<FormEvent> const& event) = 0;
		virtual ~FormEventCapturePlatform() = default;

	protected:

		virtual void AddFormEventCapturePlatformRef() const = 0;
		virtual void SubFormEventCapturePlatformRef() const = 0;
		
	};

	struct FormEventCapture : public FormEventCapturePlatform
	{
		struct Wrapper
		{
			void AddRef(FormEventCapture const* ptr) const { ptr->AddFormEventCaptureRef(); }
			void SubRef(FormEventCapture const* ptr) const { ptr->SubFormEventCaptureRef(); }
		};

		using Ptr = Potato::Pointer::IntrusivePtr<FormEventCapture, Wrapper>;

		
		virtual FormEvent::Respond RespondEvent(FormEvent event) = 0;

	protected:

		void AddFormEventCapturePlatformRef() const override final { AddFormEventCaptureRef(); }
		void SubFormEventCapturePlatformRef() const override final { SubFormEventCaptureRef(); }

		virtual void AddFormEventCaptureRef() const = 0;
		virtual void SubFormEventCaptureRef() const = 0;

		friend struct Form;

	private:

		HRESULT RespondEvent(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, std::optional<FormEvent> const& event) override final;
	};

	struct FormStyle
	{
		struct Wrapper
		{
			void AddRef(FormStyle const* ptr) const { ptr->AddFormStyleRef(); }
			void SubRef(FormStyle const* ptr) const { ptr->SubFormStyleRef(); }
		};

		using Ptr = Potato::Pointer::IntrusivePtr<FormStyle, Wrapper>;

		static Ptr GetFixedStyle();

	protected:

		virtual wchar_t const* PlatformStyleName() const = 0;
		virtual DWORD PlatformWSStyle() const = 0;

		virtual void AddFormStyleRef() const = 0;
		virtual void SubFormStyleRef() const = 0;

		friend struct Form;
	};

	export struct Form
	{

		struct Rectangle
		{
			std::size_t x_size = 1024;
			std::size_t y_size = 768;
			std::size_t x_offset = 100;
			std::size_t y_offset = 100;
		};

		struct Config
		{
			Rectangle rectangle;
			FormStyle::Ptr style = FormStyle::GetFixedStyle();
			FormEventCapturePlatform::Ptr event_capture;
			wchar_t const* title = L"Dumpling Form";
		};

		HWND GetPlatformValue() const { return handle; }

		static Form Create(Config fig);

		operator bool() const { return handle != nullptr; }

		static void PostQuitEvent();

		Form(Form&& form);
		Form() = default;
		Form(Form const&) = default;
		static bool PeekMessageEventOnce(FormEventCapturePlatform& event_capture);
		static std::size_t PeekMessageEvent(FormEventCapturePlatform& event_capture);


	protected:

		Form(HWND handle) : handle(handle) {}

		HWND handle = nullptr;

		
	};
}
