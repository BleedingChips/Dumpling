module;

#include <cassert>
#include <Windows.h>

module DumplingWin32Form;

import PotatoEncode;

namespace Dumpling::Win32
{
	LRESULT CALLBACK Win32Form::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		assert(hWnd != nullptr);
		switch (msg)
		{
		case WM_DESTROY:
		{
			LONG_PTR data = GetWindowLongPtr(hWnd, GWLP_USERDATA);
			Win32Form* ptr = reinterpret_cast<Win32Form*>(data);
			if (ptr != nullptr)
			{
				{
					std::lock_guard lg(ptr->mutex);
					assert( ptr->status == Status::Opened || ptr->status == Status::Hidden);
					ptr->status = Status::Closed;
				}
				ptr->SubViewerRef();
			}
			SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(nullptr));
				PostQuitMessage(0);
			break;
		}
		default:
		{
			LONG_PTR data = GetWindowLongPtr(hWnd, GWLP_USERDATA);
			Win32Form* ptr = reinterpret_cast<Win32Form*>(data);
			if (ptr != nullptr)
			{
				auto re = ptr->SelfHandleProc(hWnd, msg, wParam, lParam);
				if (re.has_value())
				{
					return *re;
				}
			}
			break;
		}
		}
		return DefWindowProcW(hWnd, msg, wParam, lParam);
	}

	//const FormStyle& DefaultStyle() noexcept { static FormStyle Default;  return Default; }
	const wchar_t static_class_name[] = L"dumpling_default_win32_class";
	const WNDCLASSEXW static_class = { sizeof(WNDCLASSEXW), CS_HREDRAW | CS_VREDRAW , &Win32Form::WndProc, 0, 0, GetModuleHandle(0), NULL,NULL, 0, NULL, (const wchar_t*)static_class_name, NULL };

	const struct StaticClassInitStruct
	{
		StaticClassInitStruct() { HRESULT res = RegisterClassExW(&static_class); assert(SUCCEEDED(res)); }
		~StaticClassInitStruct() { UnregisterClassW((const wchar_t*)static_class_name, GetModuleHandleW(0)); }
	}init_struct;

	void Win32Form::ControllerRelease()
	{
		CloseWindows();
	}

	void Win32Form::CloseWindows()
	{
		{
			std::shared_lock lg(mutex);
			if (status == Status::Opened || status == Status::Hidden)
			{
				assert(window_handle != nullptr);
				PostMessage(window_handle, WM_DESTROY, 0, 0);
			}
		}
		if(window_thread.joinable())
		{
			window_thread.join();
		}
	}

	Win32Form::~Win32Form()
	{
		assert(!window_thread.joinable());
	}

	void Win32Form::ViewerRelease()
	{
		this->~Win32Form();
		delete this;
	}

	Form::Ptr Win32Form::CreateWin32Form(FormSetting setting, std::pmr::memory_resource* resource)
	{
		Potato::Pointer::ControllerPtr<Win32Form> ptr = new Win32Form{};
		auto lp = ptr.Isomer();

		std::promise<std::variant<HWND, DWORD>> promise;

		auto fur = promise.get_future();

		ptr->window_thread = std::thread{[&]() mutable
		{

			std::wstring title {setting.form_name};

			HWND handle = CreateWindowExW(
				0,
				(wchar_t*)(static_class_name),
				title.c_str(),
				WS_VISIBLE | WS_OVERLAPPEDWINDOW,
				100, 100, setting.size_x, setting.size_y,
				NULL,
				NULL,
				GetModuleHandle(0),
				NULL
			);

			if(handle != nullptr)
			{
				auto Pointer = lp.GetPointer();
				Pointer->window_handle = handle;
				Pointer->status = Status::Opened;
				Pointer->AddViewerRef();
				SetWindowLongPtr(handle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(Pointer));
				promise.set_value(handle);
			}else
			{
				DWORD error_code = GetLastError();
				auto Pointer = lp.GetPointer();
				//Pointer = error_code;
				Pointer->status = Status::Error;
				promise.set_value(error_code);
				return;
			}

			
			while(true)
			{
				MSG msg;
				while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
				{
					if (msg.message == WM_QUIT)
					{
						break;
					}
					TranslateMessage(&msg);
					DispatchMessageW(&msg);
				}
				if (msg.message == WM_QUIT)
				{
					break;
				}
				std::this_thread::sleep_for(std::chrono::milliseconds{1});
			}
			

			return;
		}};

		auto tem = fur.get();

		if(std::holds_alternative<HWND>(tem))
		{
			return ptr;
		}else
		{
			return {};
		}
	}

	Status Win32Form::GetStatus() const
	{
		std::shared_lock sl(mutex);
		return status;
	}

	/*
	void FormInterface::WaitUntilWindowClosed(std::chrono::microseconds check_duration_time)
	{
		while(true)
		{
			if(mutex.try_lock())
			{
				std::lock_guard lg(mutex, std::adopt_lock);
				if(status == Status::Closed || status == Status::Error)
				{
					return;
				}
			}
			std::this_thread::sleep_for(check_duration_time);
		}
	}

	std::u8string_view FormInterface::GetFormName() const
	{
		return u8"Fuck You Windows";
	}

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

	FormManager::~FormManager()
	{
		if(threads.joinable())
		{
			threads.join();
		}
	}

	void FormManager::ControllerRelease()
	{
		if (threads.joinable())
		{
			threads.join();
		}
	}

	void FormManager::ViewerRelease()
	{
		auto old_resource = resource;
		assert(resource != nullptr);
		this->~FormManager();
		old_resource->deallocate(this, sizeof(FormManager), alignof(FormManager));
	}

	void FormManager::Execute()
	{
		

		while(true)
		{
			MSG msg;
			while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				
				TranslateMessage(&msg);

				if (msg.message == WM_CLOSE)
				{
					volatile int i = 0;
				}

				DispatchMessageW(&msg);
				if (msg.message == WM_CLOSE)
				{
					std::size_t E = 1;
					while (!exist_form_count.compare_exchange_strong(E, E - 1)) { assert(E > 0); }
					if(E == 1)
					{
						PostQuitMessage(0);
					}
				}else if(msg.message == WM_CLOSE)
				{
					break;
				}
			}
			if (form_init_list.try_lock())
			{
				std::lock_guard lg(form_init_list, std::adopt_lock);
				while (!requests.empty())
				{
					auto top = std::move(*requests.rbegin());
					requests.pop_back();
					if (top.form)
					{
						auto form_name = top.form->GetFormName();

						auto temp_buffer =
							Potato::Encode::StrEncoder<char8_t, wchar_t>::EncodeToString(form_name, std::pmr::polymorphic_allocator<wchar_t>{});

						std::lock_guard lg(top.form->mutex);
						if (top.form->status == FormInterface::Status::RequestExist)
						{
							std::size_t E = 1;
							while (!exist_form_count.compare_exchange_strong(E, E - 1)) { assert(E > 0); }
							if (E == 1)
							{
								PostQuitMessage(0);
							}
						}else
						{
							assert(top.form->status == FormInterface::Status::Waiting);
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
								auto Pointer = top.form.GetPointer();
								Pointer->window_handle = handle;
								Pointer->error_code = 0;
								Pointer->status = FormInterface::Status::Ready;
								Pointer->AddViewerRef();
								SetWindowLongPtr(handle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(Pointer));
							}
							else {
								DWORD error_code = GetLastError();
								auto Pointer = top.form.GetPointer();
								Pointer->error_code = error_code;
								Pointer->status = FormInterface::Status::Error;
							}
						}
					}
				}
			}
			std::this_thread::sleep_for(std::chrono::nanoseconds{1});
		}
	}

	bool FormManager::CreateForm(FormInterface& channel)
	{
		std::lock_guard lg(channel.mutex);
		switch(channel.status)
		{
		case FormInterface::Status::Ready:
		case FormInterface::Status::RequestExist:
		case FormInterface::Status::Waiting:
			return false;
		default:
			
		{
			channel.owner = this;
			channel.status = FormInterface::Status::Waiting;
			channel.window_handle = nullptr;
			channel.error_code = 0;
			{
				std::lock_guard lg(form_init_list);
				FormInterface::WPtr p{ &channel };
				requests.push_back({ std::move(p) });
				std::size_t E = 0;
				while (!exist_form_count.compare_exchange_strong(E, E + 1)) {}

				if (E == 0)
				{
					if (threads.joinable())
					{
						threads.join();
					}
					FormManager::WPtr WThis{ this };
					threads = std::thread{ [WThis = std::move(WThis), this]() mutable
					{
						WThis->Execute();
					} };
				}
			}
			return true;
		}
			
		}
	}
	*/

}