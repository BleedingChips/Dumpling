module;

#include <cassert>
#include <Windows.h>

#undef max
#undef IGNORE
#undef interface

#define DUMPLING_WM_GLOBAL_MESSAGE static_cast<UINT>(WM_USER + 100)

module DumplingWindowsForm;

import std;
import PotatoIR;
import PotatoEncode;
import DumplingFormEvent;

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

	

	FormClassStyle::FormClassStyle()
	{
		HBRUSH back_ground_brush = ::CreateSolidBrush(BLACK_BRUSH);
		const WNDCLASSEXW static_class = {
			sizeof(WNDCLASSEXW),
			CS_HREDRAW | CS_VREDRAW,
			&Dumpling::Form::DefaultWndProc, 0, 0, GetModuleHandle(0), NULL,NULL, back_ground_brush, NULL, form_class_style_name, NULL };

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
	Dumpling::FormEvent::Category TranslateCategory(UINT msg)
	{
		switch(msg)
		{
		case WM_QUIT:
			return Dumpling::FormEvent::Category::SYSTEM;
		case WM_NCDESTROY:
			return Dumpling::FormEvent::Category::MODIFY;
		default:
			return Dumpling::FormEvent::Category::UNACCEPTABLE;
		}
	}

	bool IsMessageMarkAsSkip(UINT msg, HRESULT result)
	{
		return result == E_NOTIMPL;
	}


	HRESULT TranslateResult(UINT msg, FormEvent::Respond respond)
	{
		switch(respond)
		{
		case FormEvent::Respond::CAPTURED:
			return S_OK;
		case FormEvent::Respond::PASS:
			return E_NOTIMPL;
		default:
			assert(false);
			return E_NOTIMPL;
		}
	}

	FormEvent::System TranslateSystemMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch(msg)
		{
		case WM_QUIT:
			return FormEvent::System{Dumpling::FormEvent::System::Message::QUIT};
		}
		assert(false);
		return {};
	}

	FormEvent::Modify TranslateModifyMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch(msg)
		{
		case WM_NCDESTROY:
			return FormEvent::Modify{Dumpling::FormEvent::Modify::Message::DESTROY};
		}
		assert(false);
		return {};
	}

	FormEvent::Input TranslateInputMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		assert(false);
		return {};
	}

	HRESULT FormEventCapture::ReceiveRaw(Form& interface, FormEvent::Category category, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		return MarkMessageSkip(msg);
	}

	bool Form::InsertCapture(FormEventCapture::Ptr capture, std::size_t priority)
	{
		std::lock_guard lg(capture_mutex);
		if(capture)
		{
			auto ite = std::find_if(captures.begin(), captures.end(), [=](CaptureTuple& tuple)
			{
				return tuple.priority < priority;
			});
			captures.insert(ite, {priority, capture->GetAcceptedCategory(), capture});
			return true;
		}
		return false;
	}

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

			RECT adject_rect {0, 0, property.form_size.width, property.form_size.height };

			AdjustWindowRect(
				&adject_rect,
				GetWSStyle(property.style),
				FALSE
			);

			HWND new_hwnd = CreateWindowExW(
				0,
				form_class_style_name,
				str.data(),
				GetWSStyle(property.style),
				100, 100, adject_rect.right - adject_rect.left, adject_rect.bottom - adject_rect.top,
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
				auto Category = TranslateCategory(msg.message);
				if(Category == FormEvent::Category::SYSTEM)
				{
					auto sys_event = TranslateSystemMessage(msg.hwnd, msg.message, msg.wParam, msg.lParam);
					func(data, sys_event);
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

	struct FormHelper
	{
		template<FormEvent::Category category, typename Type>
		static HRESULT DeliverMessage(Form& form, Type message, std::span<Form::CaptureTuple> captures, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
		{
			for(auto& Ite : captures)
			{
				if(Ite.acceptable_category == FormEvent::Category::RAW)
				{
					HRESULT re = Ite.capture->ReceiveRaw(form, category, hWnd, msg, wParam, lParam);
					if(!IsMessageMarkAsSkip(msg, re))
					{
						return re;
					}
				}
				else if((Ite.acceptable_category & category) != FormEvent::Category::UNACCEPTABLE)
				{
					HRESULT re = TranslateResult(msg, Ite.capture->Receive(form, message));
					if(!IsMessageMarkAsSkip(msg, re))
					{
						return re;
					}
				}
			}
			return MarkMessageSkip(msg);
		}
	};

	

	HRESULT Form::HandleEvent(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		using namespace FormEvent;

		auto category = TranslateCategory(msg);
		HRESULT re = MarkMessageSkip(msg);
		{
			std::shared_lock sl(capture_mutex);
			switch(category)
			{
			case FormEvent::Category::SYSTEM:
				{
					auto sys_event = TranslateSystemMessage(hwnd, msg, wParam, lParam);
					re = FormHelper::DeliverMessage<FormEvent::Category::SYSTEM>(*this, std::move(sys_event), std::span(captures), hWnd, msg, wParam, lParam);
				}
				break;
			case FormEvent::Category::MODIFY:
				{
					auto mod_event = TranslateModifyMessage(hwnd, msg, wParam, lParam);
					re = FormHelper::DeliverMessage<FormEvent::Category::MODIFY>(*this, std::move(mod_event), std::span(captures), hWnd, msg, wParam, lParam);
				}
				break;
			case FormEvent::Category::INPUT:
				{
					auto inp_event = TranslateInputMessage(hwnd, msg, wParam, lParam);
					re = FormHelper::DeliverMessage<FormEvent::Category::INPUT>(*this, std::move(inp_event), std::span(captures), hWnd, msg, wParam, lParam);
				}
				break;
			default:
				{
					for(auto& ite : captures)
					{
						if(ite.acceptable_category == FormEvent::Category::RAW)
						{
							auto re = ite.capture->ReceiveRaw(*this, category, hwnd, msg, wParam, lParam);
							if(!IsMessageMarkAsSkip(msg, re))
							{
								break;
							}
						}
					}
				}
				break;
			}
		}
		if(!IsMessageMarkAsSkip(msg, re))
		{
			return re;
		}
		return DefWindowProcW(hWnd, msg, wParam, lParam);
	}
}