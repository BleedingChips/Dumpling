module;

#include <cassert>
#include <Windows.h>

#undef max
#undef IGNORE

#define DUMPLING_WM_GLOBAL_MESSAGE static_cast<UINT>(WM_USER + 100)

module DumplingWindowsForm;

import std;
import PotatoIR;
import PotatoEncode;


namespace
{
	struct FormClassStyle
	{
		FormClassStyle();
		~FormClassStyle();
	};

	DWORD GetWSStyle(Dumpling::FormStyle style)
	{
		return WS_VISIBLE | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
	}

	wchar_t const* form_class_style_name = L"Dumpling_Default_GameStyle";

	std::variant<std::nullopt_t, Dumpling::FormEvent::System, Dumpling::FormEvent::Modify, Dumpling::FormEvent::Input> TranslateDumplingFormEvent(UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch(msg)
		{
		case WM_QUIT:
			return Dumpling::FormEvent::System{Dumpling::FormEvent::System::Message::QUIT};
		case WM_NCDESTROY:
			return Dumpling::FormEvent::Modify{Dumpling::FormEvent::Modify::Message::DESTROY};
		}
		return std::nullopt;
	}

	FormClassStyle::FormClassStyle()
	{
		const WNDCLASSEXW static_class = {
			sizeof(WNDCLASSEXW),
			CS_HREDRAW | CS_VREDRAW ,
			&Dumpling::Form::DefaultWndProc, 0, 0, GetModuleHandle(0), NULL,NULL, 0, NULL, form_class_style_name, NULL };

		ATOM res = RegisterClassExW(&static_class);
		assert(res != 0);
	}

	FormClassStyle::~FormClassStyle()
	{
		UnregisterClassW(form_class_style_name, GetModuleHandle(0));
	}
}

namespace Dumpling
{
	void Form::PostQuitEvent()
	{
		::PostQuitMessage(0);
	}

	bool Form::Init(FormProperty property, std::pmr::memory_resource* temp)
	{
		std::lock_guard lg(mutex);
		if(hwnd == nullptr)
		{
			std::pmr::wstring str(temp);
			auto re = Potato::Encode::StrEncoder<char8_t, wchar_t>::RequireSpace(property.title);
			str.resize(re.TargetSpace + 1);
			std::span<wchar_t> wstr{ str.data(), str.size() };
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
			if (new_hwnd != nullptr)
			{
				return true;
			}
		}
		return false;
	}

	Form::Ptr Form::Create(std::size_t identity_id, std::pmr::memory_resource* resource)
	{
		auto re = Potato::IR::MemoryResourceRecord::Allocate<Form>(resource);
		if(re)
		{
			auto ptr = new(re.Get()) Form{ re, identity_id };
			return ptr;
		}
		return {};
	}

	bool Form::PeekMessageEvent(void(*func)(void*, FormEvent::System), void* data)
	{
		MSG msg;
		auto re = PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);
		if(re)
		{
			LRESULT result = S_OK;
			if(msg.hwnd != nullptr)
			{
				DispatchMessageW(&msg);
			}else
			{
				auto event = TranslateDumplingFormEvent(msg.message, msg.lParam, msg.wParam);
				if(std::holds_alternative<FormEvent::System>(event))
				{
					func(data, std::get<FormEvent::System>(event));
				}
			}
		}
		return re;
	}

	LRESULT CALLBACK Form::DefaultWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		assert(hWnd != nullptr);
		switch (msg)
		{
		case WM_CREATE:
			{
				CREATESTRUCTA* Struct = reinterpret_cast<CREATESTRUCTA*>(lParam);
				assert(Struct != nullptr);
				Form* inter = static_cast<Form*>(Struct->lpCreateParams);
				assert(inter != nullptr);
				inter->hwnd = hWnd;
				SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(inter));
				inter->AddRef();
				return inter->HandleEvent(hWnd, msg, wParam, lParam);
			}
		case WM_NCDESTROY:
		{
			LONG_PTR data = GetWindowLongPtr(hWnd, GWLP_USERDATA);
			Form* ptr = reinterpret_cast<Form*>(data);
			if (ptr != nullptr)
			{
				auto re = ptr->HandleEvent(hWnd, msg, wParam, lParam);
				SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(nullptr));
				ptr->SubRef();
				return re;
			}
			break;
		}
		default:
		{
			LONG_PTR data = GetWindowLongPtr(hWnd, GWLP_USERDATA);
			Form* ptr = reinterpret_cast<Form*>(data);
			if (ptr != nullptr)
			{
				return ptr->HandleEvent(hWnd, msg, wParam, lParam);
			}
			break;
		}
		}
		return DefWindowProcW(hWnd, msg, wParam, lParam);
	}

	void Form::Release()
	{
		auto re = record;
		this->~Form();
		re.Deallocate();
	}

	HRESULT Form::HandleEvent(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		using namespace FormEvent;
		auto event = TranslateDumplingFormEvent(msg, wParam, lParam);
		Respond respond = Respond::PASS;
		if(std::holds_alternative<System>(event))
		{
			respond = capture_manager.HandleEvent(*this, std::get<System>(event));
		}else if(std::holds_alternative<Modify>(event))
		{
			respond = capture_manager.HandleEvent(*this, std::get<Modify>(event));
		}else if(std::holds_alternative<Input>(event))
		{
			respond = capture_manager.HandleEvent(*this, std::get<Input>(event));
		}
		if(respond == Respond::PASS)
		{
			return DefWindowProcW(hWnd, msg, wParam, lParam);
		}else
		{
			return S_OK;
		}
	}
}