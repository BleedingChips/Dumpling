module;

#include <cassert>
#include <Windows.h>

#undef max

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

	std::optional<FormEvent> TranslateDumplingFormMessage(UINT msg, WPARAM wParam, LPARAM lParam);

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

	FormInit::Ptr FormInit::Create(FormStyle style, FormSize size, std::u8string_view title, std::pmr::memory_resource* resource)
	{
		auto layout = Potato::IR::Layout::Get<FormInit>();
		auto re = Potato::Encode::StrEncoder<char8_t, wchar_t>::RequireSpace(title);
		if(re)
		{
			auto layout_str = Potato::IR::Layout::GetArray<wchar_t>(re.TargetSpace + 1);
			auto offset = Potato::IR::InsertLayoutCPP(layout, layout_str);
			Potato::IR::FixLayoutCPP(layout);
			auto record = Potato::IR::MemoryResourceRecord::Allocate(resource, layout);
			if(record)
			{
				std::span<wchar_t> wstr{reinterpret_cast<wchar_t*>(record.GetByte() + offset), re.TargetSpace + 1};
				Potato::Encode::StrEncoder<char8_t, wchar_t>::EncodeUnSafe(title, 
					wstr
				);
				*wstr.rbegin() = L'\0';
				FormInit::Ptr form_init = new (record.Get()) FormInit(
					style, size, record, wstr
				);
				return std::move(form_init);
			}
		}
		return {};
	}

	void FormInit::Release()
	{
		auto re = record;
		this->~FormInit();
		re.Deallocate();
	}

	auto FormManager::CreateForm(
		FormProperty property,
		FormEventResponder::Ptr responder,
		FormRenderTarget::Ptr renderer,
		std::pmr::memory_resource* resource
	) -> Form::Ptr
	{
		if(resource != nullptr)
		{
			auto init = FormInit::Create(property.style, property.form_size, property.title, resource);
			if(init)
			{
				auto form_re = Potato::IR::MemoryResourceRecord::Allocate<Win32Form>(resource);
				if (form_re)
				{
					Form::Ptr ptr = new (form_re.Get())  Win32Form{form_re, std::move(responder), std::move(renderer)};
					std::lock_guard lg(mutex);
					init_requires.emplace_back(
						std::move(ptr),
						std::move(init)
					);
				}else
				{
					init->Release();
				}
			}
		}
		return {};
	}

	auto FormManager::CreateManager(std::pmr::memory_resource* resource) -> Ptr
	{
		auto re = Potato::IR::MemoryResourceRecord::Allocate<FormManager>(resource);
		if (re)
		{
			Ptr ptr = new (re.Get()) FormManager{ re };
			return ptr;
		}
		return {};
	}

	bool FormManager::Commite(
		Potato::Task::TaskContext& context,
		std::thread::id thread_id,
		FormTaskProperty property
	) {

		if (thread_id != std::thread::id{})
		{
			Potato::Task::TaskProperty new_property
			{
				property.display_name,
				{},
				property.priority,
				Potato::Task::Category::THREAD_TASK,
				0,
				thread_id
			};
			std::lock_guard lg(mutex);
			if(!current_thread_id.has_value() && context.CommitTask(this, new_property))
			{
				current_thread_id = thread_id;
				return true;
			}
		}
		return false;
	}


	void FormManager::Release()
	{
		auto re = record;
		this->~FormManager();
		re.Deallocate();
	}

	void FormManager::TaskTerminal(Potato::Task::TaskProperty property) noexcept
	{
		MSG msg;
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			//TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
		std::lock_guard lg(mutex);
		current_thread_id.reset();
		init_requires.clear();
	}


	void FormManager::TaskExecute(Potato::Task::ExecuteStatus& status)
	{
		while(true)
		{
			Form::Ptr ptr;
			FormInit::Ptr iptr;
			if (mutex.try_lock())
			{
				std::lock_guard lg(mutex, std::adopt_lock);
				if(!init_requires.empty())
				{
					std::tie(ptr, iptr) = std::move(*init_requires.rbegin());
					init_requires.pop_back();
				}
			}
			if(ptr)
			{
				static FormClassStyle class_style;

				HWND new_hwnd = CreateWindowExW(
					0,
					form_class_style_name,
					iptr->title.data(),
					GetWSStyle(iptr->style),
					100, 100, iptr->size.width, iptr->size.height,
					NULL,
					NULL,
					GetModuleHandle(0),
					ptr.GetPointer()
				);
				auto P = GetLastError();
				ptr.Reset();
				iptr.Reset();
			}else
			{
				break;
			}
		}
		MSG msg;
		bool need_quit = false;
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if(msg.message == WM_QUIT)
			{
				need_quit = true;
			}
			DispatchMessageW(&msg);
		}
		if (!need_quit)
		{
			status.context.CommitTask(this, status.task_property);
		}
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
				SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(inter));
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
				SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(nullptr));
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
		std::shared_lock sl(mutex);

		switch(msg)
		{
		case WM_CREATE:
			if(form_renderer)
			{
				form_renderer->OnFormCreated(*this);
			}
			break;
		}

		if(event_responder)
		{
			auto event = TranslateDumplingFormMessage(msg, wParam, lParam);
			if(event)
			{
				auto re = event_responder->Respond(*this, *event);
				if(re)
				{
					
				}
			}
		}
		return DefWindowProcW(hWnd, msg, wParam, lParam);
	}

	std::optional<FormEvent> TranslateDumplingFormMessage(UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch(msg)
		{
		case WM_NCDESTROY:
			return FormEvent{FormEventEnum::DESTORYED};
		}
		return std::nullopt;
	}
}