module;

#include <cassert>
#include <Windows.h>

#undef max

#define DUMPLING_WM_GLOBAL_MESSAGE static_cast<UINT>(WM_USER + 100)

module DumplingWindowsForm;

import std;
import PotatoIR;
import PotatoEncode;


namespace Dumpling::Windows
{
	DWORD GetWSStyle(FormStyle style)
	{
		return WS_VISIBLE | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
	}

	wchar_t const* form_class_style_name = L"Dumpling_Default_GameStyle";

	std::optional<FormEvent> TranslateDumplingFormEvent(UINT msg, WPARAM wParam, LPARAM lParam);

	FormClassStyle::FormClassStyle()
	{
		const WNDCLASSEXW static_class = {
			sizeof(WNDCLASSEXW),
			CS_HREDRAW | CS_VREDRAW ,
			&Win32Form::DefaultWndProc, 0, 0, GetModuleHandle(0), NULL,NULL, 0, NULL, form_class_style_name, NULL };

		ATOM res = RegisterClassExW(&static_class);
		assert(res != 0);
	}

	FormClassStyle::~FormClassStyle()
	{
		UnregisterClassW(form_class_style_name, GetModuleHandle(0));
	}

	void Win32Form::PostQuitEvent()
	{
		::PostQuitMessage(0);
	}

	bool Win32Form::Init(FormProperty property, std::pmr::memory_resource* temp)
	{
		std::pmr::wstring str(temp);
		auto re = Potato::Encode::StrEncoder<char8_t, wchar_t>::RequireSpace(property.title);
		str.resize(re.TargetSpace + 1);
		std::span<wchar_t> wstr{str.data(), str.size()};
		Potato::Encode::StrEncoder<char8_t, wchar_t>::EncodeUnSafe(property.title, 
			wstr
		);
		static FormClassStyle class_style;

		HWND new_hwnd = CreateWindowExW(
			0,
			form_class_style_name,
			str.data(),
			GetWSStyle(property.style),
			100, 100, property.form_size.width, property.form_size.height,
			NULL,
			NULL,
			GetModuleHandle(0),
			static_cast<void*>(this)
		);
		if(new_hwnd != nullptr)
		{
			return true;
		}
		return false;
	}

	Form::Ptr Win32Form::Create(FormEventResponder::Ptr respond, std::size_t identity_id, std::pmr::memory_resource* resource)
	{
		auto re = Potato::IR::MemoryResourceRecord::Allocate<Win32Form>(resource);
		if(re)
		{
			auto ptr = new(re.Get()) Win32Form{ re, std::move(respond), identity_id };
			return ptr;
		}
		return {};
	}

	bool Win32Form::PeekMessageEvent(void(*func)(void*, Form*, FormEvent, FormEventRespond), void* data)
	{
		MSG msg;
		auto re = PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);
		if(re)
		{
			LRESULT result = S_OK;
			Win32Form* ptr = nullptr;
			if(msg.hwnd != nullptr)
			{
				ptr = reinterpret_cast<Win32Form*>(GetWindowLongPtrW(msg.hwnd, GWLP_USERDATA));
			}
			result = DispatchMessageW(&msg);
			if(func != nullptr)
			{
				auto event = TranslateDumplingFormEvent(msg.message, msg.lParam, msg.wParam);
				if(event)
				{
					func(data, ptr, *event, FormEventRespond::Default);
				}
			}
		}
		return re;
	}

	LRESULT CALLBACK Win32Form::DefaultWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		assert(hWnd != nullptr);
		switch (msg)
		{
		case WM_CREATE:
			{
				CREATESTRUCTA* Struct = reinterpret_cast<CREATESTRUCTA*>(lParam);
				assert(Struct != nullptr);
				Win32Form* inter = static_cast<Win32Form*>(Struct->lpCreateParams);
				assert(inter != nullptr);
				{
					std::lock_guard sl(inter->mutex);
					inter->hwnd = hWnd;
				}
				SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(inter));
				inter->AddFormRef();
				return inter->HandleEvent(hWnd, msg, wParam, lParam);
			}
		case WM_NCDESTROY:
		{
			LONG_PTR data = GetWindowLongPtr(hWnd, GWLP_USERDATA);
			Win32Form* ptr = reinterpret_cast<Win32Form*>(data);
			if (ptr != nullptr)
			{
				auto re = ptr->HandleEvent(hWnd, msg, wParam, lParam);
				SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(nullptr));
				ptr->SubFormRef();
				return re;
			}
			break;
		}
		default:
		{
			LONG_PTR data = GetWindowLongPtr(hWnd, GWLP_USERDATA);
			Win32Form* ptr = reinterpret_cast<Win32Form*>(data);
			if (ptr != nullptr)
			{
				return ptr->HandleEvent(hWnd, msg, wParam, lParam);
			}
			break;
		}
		}
		return DefWindowProcW(hWnd, msg, wParam, lParam);
	}

	void Win32Form::Release()
	{
		auto re = record;
		this->~Win32Form();
		re.Deallocate();
	}

	HRESULT Win32Form::HandleEvent(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		auto event = TranslateDumplingFormEvent(msg, wParam, lParam);
		if(event)
		{
			auto re = HandleResponder(*event);
		}
		return DefWindowProcW(hWnd, msg, wParam, lParam);
	}

	std::optional<FormEvent> TranslateDumplingFormEvent(UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch(msg)
		{
		case WM_NCDESTROY:
			return FormEvent{FormEventEnum::DESTROY};
		case WM_QUIT:
			return FormEvent{FormEventEnum::QUIT};
		}
		return std::nullopt;
	}
}