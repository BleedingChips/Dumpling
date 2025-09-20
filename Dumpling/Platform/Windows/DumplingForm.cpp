module;

#include <cassert>
#include <Windows.h>
#include <clocale>


#undef max
#undef min
#undef IGNORE
#undef interface

#define DUMPLING_WM_GLOBAL_MESSAGE static_cast<UINT>(WM_USER + 100)

module DumplingForm;

import std;


namespace Dumpling
{

	struct FixedFormStyle : public FormStyle
	{
		virtual void AddFormStyleRef() const override {}
		virtual void SubFormStyleRef() const override {}
		static wchar_t const* style_name;
		wchar_t const* PlatformStyleName() const override { return style_name; }
		FixedFormStyle()
		{
			HBRUSH back_ground_brush = ::CreateSolidBrush(BLACK_BRUSH);
			const WNDCLASSEXW static_class = {
				sizeof(WNDCLASSEXA),
				CS_HREDRAW | CS_VREDRAW,
				&FixedFormStyle::DefaultWndProc, 0, 0, GetModuleHandle(0), NULL,NULL, back_ground_brush, NULL, style_name, NULL};

			ATOM res = RegisterClassExW(&static_class);
			assert(res != 0);
		}
		~FixedFormStyle()
		{
			UnregisterClassW(style_name, GetModuleHandle(0));
		}
		DWORD PlatformWSStyle() const override { return WS_VISIBLE | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX; }
		static LRESULT CALLBACK DefaultWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	};

	wchar_t const* FixedFormStyle::style_name = L"dumpling_build_in_fixed";

	FormStyle::Ptr FormStyle::GetFixedStyle()
	{
		static FixedFormStyle instance;
		return FormStyle::Ptr{ &instance };
	}

	Form Form::Create(Config fig)
	{
		RECT adject_rect{ 0, 0,
				static_cast<LONG>(fig.rectangle.x_size),
				static_cast<LONG>(fig.rectangle.y_size)
		};

		AdjustWindowRect(
			&adject_rect,
			fig.style->PlatformWSStyle(),
			FALSE
		);

		std::array<wchar_t, 1024> title;

		auto info = Potato::Encode::StrEncoder<char8_t, wchar_t>{}.Encode(
			fig.title,
			std::span(title)
		);

		title[std::min(info.target_space, title.size() - 1)] = L'\0';

		HWND hwnd = CreateWindowExW(
			0,
			fig.style->PlatformStyleName(),
			title.data(),
			fig.style->PlatformWSStyle(),
			fig.rectangle.x_offset, fig.rectangle.y_offset, adject_rect.right - adject_rect.left, adject_rect.bottom - adject_rect.top,
			NULL,
			NULL,
			GetModuleHandle(0),
			fig.event_hook.GetPointer()
		);
		return Form{hwnd};
	}

	LRESULT CALLBACK FixedFormStyle::DefaultWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		assert(hWnd != nullptr);
		switch (msg)
		{
		case WM_CREATE:
			{
				CREATESTRUCTA* Struct = reinterpret_cast<CREATESTRUCTA*>(lParam);
				assert(Struct != nullptr);
				auto inter = static_cast<FormEventHook*>(Struct->lpCreateParams);
				if(inter != nullptr)
				{
					FormEventHook::Wrapper wrap;
					SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(inter));
					wrap.AddRef(inter);
					FormEvent event;
					event.hwnd = hWnd;
					event.message = msg;
					event.lParam = lParam;
					event.wParam = wParam;
					auto re = inter->Hook(event);
					if (event.IsRespondMarkAsHooked(re))
					{
						return re;
					}
				}
				break;
			}
		case WM_NCDESTROY:
			{
				LONG_PTR data = GetWindowLongPtr(hWnd, GWLP_USERDATA);
				auto ptr = reinterpret_cast<FormEventHook*>(data);
				if (ptr != nullptr)
				{
					SetWindowLongPtrA(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(nullptr));
					FormEventHook::Wrapper wrap;
					wrap.SubRef(ptr);
				}
				break;
			}
		default:
		{
			LONG_PTR data = GetWindowLongPtr(hWnd, GWLP_USERDATA);
			auto ptr = reinterpret_cast<FormEventHook*>(data);
			if (ptr != nullptr)
			{
				FormEvent event;
				event.hwnd = hWnd;
				event.message = msg;
				event.lParam = lParam;
				event.wParam = wParam;
				auto re = ptr->Hook(event);
				if (event.IsRespondMarkAsHooked(re))
				{
					return re;
				}
			}
			break;
		}
		}
		return DefWindowProcW(hWnd, msg, wParam, lParam);
	}

	std::optional<bool> Form::PeekMessageEventOnce(FormEvent::Respond(*function)(void* data, FormEvent& event), void* data)
	{
		FormEvent event;
		auto re = PeekMessage(&event, NULL, 0, 0, PM_REMOVE);
		if (re)
		{
			if (function != nullptr)
			{
				auto res = (*function)(data, event);
				if (event.IsRespondMarkAsHooked(res))
				{
					return re;
				}
			}

			if (event.message == WM_QUIT)
			{
				return std::nullopt;
			}

			DispatchMessageW(&event);

			return true;
		}
		return re;
	}

	Form::Form(Form&& form)
		: handle(form.handle)
	{
		form.handle = nullptr;
	}


	Form& Form::operator=(Form&& form)
	{
		handle = form.handle;
		form.handle = nullptr;
		return *this;
	}

	bool Form::DestroyForm()
	{
		auto re = DestroyWindow(handle);
		handle = nullptr;
		return re;
	}

	Form::~Form()
	{
		DestroyForm();
	}

	bool Form::ShowForm(bool show)
	{
		if (!show)
			return ::ShowWindow(handle, SW_HIDE);
		else
			return ::ShowWindow(handle, SW_SHOW);
	}
}