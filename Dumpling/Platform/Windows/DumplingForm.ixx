module;

#include <Windows.h>
#include <wrl.h>

#undef max
#undef interface

export module DumplingForm;

import std;
import Potato;
import DumplingFormEvent;


export namespace Dumpling
{

	export struct Form;

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
			FormEventHook::Ptr event_hook;
			std::u8string_view title = u8"Dumpling Form";
		};

		HWND GetPlatformValue() const { return handle; }

		static Form Create(Config fig);

		operator bool() const { return handle != nullptr; }

		Form(Form&& form);
		Form() = default;
		Form(Form const&) = delete;
		Form& operator=(Form const&) = delete;
		Form& operator=(Form&&);
		~Form();
		bool DestroyForm();
		bool ShowForm(bool show);

		static std::optional<bool> PeekMessageEventOnce()
		{
			HRESULT(*function)(void* data, FormEvent& event) = nullptr;
			return PeekMessageEventOnce(function, nullptr);
		}

		static std::optional<bool> PeekMessageEventOnce(FormEvent::Respond(*function)(void* data, FormEvent& event), void* data);

		template<typename Func>
		static std::optional<bool> PeekMessageEventOnce(Func&& func) requires(std::is_invocable_r_v<FormEvent::Respond, Func, FormEvent&>)
		{
			return Form::PeekMessageEventOnce([](void* data, FormEvent& event)->FormEvent::Respond
			{
					return (*static_cast<Func*>(data))(event);
			}, &func);
		}

	protected:

		Form(HWND handle) : handle(handle) {}

		HWND handle = nullptr;
	};
}
