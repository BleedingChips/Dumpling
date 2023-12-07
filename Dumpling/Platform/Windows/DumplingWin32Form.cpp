module;

#include <cassert>
#include <Windows.h>
module DumplingWin32Form;

import PotatoEncode;

namespace Dumpling::Win32
{
	LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		assert(hWnd != nullptr);
		switch (msg)
		{
		case WM_USER + 1:
		{
			/*
		LONG_PTR data = GetWindowLongPtr(hWnd, GWLP_USERDATA);
		FormChannel* ptr = reinterpret_cast<FormChannel*>(data);
		if (ptr != nullptr)
			SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(nullptr));
		DestroyWindow(hWnd);
		break;
		*/
		}
		case WM_DESTROY:
		{
			/*
			LONG_PTR data = GetWindowLongPtr(hWnd, GWLP_USERDATA);
			Form* ptr = reinterpret_cast<Form*>(data);
			if (ptr != nullptr)
			{
				//ptr->m_available = false;
				SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(nullptr));
			}
			*/
			//GobalManagerInstance.FormCount -= 1;
			break;
		}
		default:
		{
			/*
		LONG_PTR data = GetWindowLongPtr(hWnd, GWLP_USERDATA);
		FormChannel* ptr = reinterpret_cast<FormChannel*>(data);
		std::optional<LRESULT> Result = std::nullopt;
		if (ptr != nullptr)
			Result = ptr->RespondEventInEventLoop(hWnd, msg, wParam, lParam);
		if (Result.has_value())
			return *Result;
		else if (msg != WM_CLOSE)
			return DefWindowProcW(hWnd, msg, wParam, lParam);
		return 0;
		*/
		}
		}
		return DefWindowProcW(hWnd, msg, wParam, lParam);
	}

	//const FormStyle& DefaultStyle() noexcept { static FormStyle Default;  return Default; }
	const char16_t static_class_name[] = u"dumpling_default_win32_class";
	const WNDCLASSEXW static_class = { sizeof(WNDCLASSEXW), CS_HREDRAW | CS_VREDRAW , WndProc, 0, 0, GetModuleHandle(0), NULL,NULL, 0, NULL, (const wchar_t*)static_class_name, NULL };
	const struct StaticClassInitStruct
	{
		StaticClassInitStruct() { HRESULT res = RegisterClassExW(&static_class); assert(SUCCEEDED(res)); }
		~StaticClassInitStruct() { UnregisterClassW((const wchar_t*)static_class_name, GetModuleHandleW(0)); }
	};



	auto FormManager::Create(std::pmr::memory_resource* resource)
		->Ptr
	{
		if(resource != nullptr)
		{
			auto adress = resource->allocate(sizeof(FormManager), alignof(FormManager));
			if(adress != nullptr)
			{
				return new (adress) FormManager{resource};
			}
		}
		return {};
	}

	void FormManager::AddRef() const
	{
		AddWeakRef();
		AddStrongRef();
	}

	void FormManager::SubRef() const
	{
		SubStrongRef();
		SubWeakRef();
	}

	FormManager::~FormManager()
	{
		if(threads.joinable())
		{
			threads.join();
		}
	}

	void FormManager::StrongRelease()
	{
		assert(exist_form_count == 0);
	}

	void FormManager::WeakRelease()
	{
		auto old_resource = resource;
		assert(resource != nullptr);
		this->~FormManager();
		old_resource->deallocate(this, sizeof(FormManager), alignof(FormManager));
	}

	void FormManager::Execuet(FormManager::WPtr ptr)
	{
		if(ptr)
		{
			while(exist_form_count != 0)
			{
				if(form_init_list.try_lock())
				{
					std::lock_guard lg(form_init_list);
					while(!requests.empty())
					{
						auto top = std::move(*requests.rbegin());
						if(top.form_channel_interface)
						{

							auto form_name = top.form_channel_interface->FormName();


							auto temp_buffer =
							Potato::Encode::StrEncoder<char8_t, wchar_t>::EncodeToString(form_name, std::pmr::polymorphic_allocator<wchar_t>{});


							HWND handle = CreateWindowExW(
								0,
								(wchar_t*)(static_class_name),
								temp_buffer->c_str(),
								WS_VISIBLE | WS_OVERLAPPEDWINDOW,
								100, 100, 1024, 768,
								NULL,
								NULL,
								GetModuleHandle(0),
								NULL
							);
							if (handle != nullptr)
							{
								/*
								SetWindowLongPtr(handle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(Ref.Form));
								++FormCount;
								Ref.Promise.set_value(handle);
								*/
							}
							else {
								/*
								DWORD ErrorCode = GetLastError();
								Ref.Promise.set_exception(std::make_exception_ptr(Dumpling::Win32::Error::FaultToCreate{ "unable_to_create Fault" }));
								*/
							}
						}
					}
				}

				MSG msg;
				while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
				{
					TranslateMessage(&msg);
					DispatchMessageW(&msg);
				}

				std::this_thread::yield();
			}
		}
	}

}