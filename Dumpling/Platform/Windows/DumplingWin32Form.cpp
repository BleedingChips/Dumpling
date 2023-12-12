module;

#include <cassert>
#include <Windows.h>

module DumplingWin32Form;

import PotatoEncode;

namespace Dumpling::Win32
{
	LRESULT CALLBACK FormManager::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		assert(hWnd != nullptr);
		switch (msg)
		{
		case WM_DESTROY:
		{
			LONG_PTR data = GetWindowLongPtr(hWnd, GWLP_USERDATA);
			FormInterface* ptr = reinterpret_cast<FormInterface*>(data);
			if (ptr != nullptr)
			{
				{
					std::lock_guard lg(ptr->mutex);
					assert(
						ptr->status == FormInterface::Status::Ready
					);
					ptr->status = FormInterface::Status::Closed;
				}
				ptr->SubViewerRef();
			}
			SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(nullptr));
			break;
		}
		default:
		{
			LONG_PTR data = GetWindowLongPtr(hWnd, GWLP_USERDATA);
			FormInterface* ptr = reinterpret_cast<FormInterface*>(data);
			if (ptr != nullptr)
			{
				auto re = ptr->WndProc(hWnd, msg, wParam, lParam);
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
	const WNDCLASSEXW static_class = { sizeof(WNDCLASSEXW), CS_HREDRAW | CS_VREDRAW , &FormManager::WndProc, 0, 0, GetModuleHandle(0), NULL,NULL, 0, NULL, (const wchar_t*)static_class_name, NULL };

	const struct StaticClassInitStruct
	{
		StaticClassInitStruct() { HRESULT res = RegisterClassExW(&static_class); assert(SUCCEEDED(res)); }
		~StaticClassInitStruct() { UnregisterClassW((const wchar_t*)static_class_name, GetModuleHandleW(0)); }
	};

	void FormInterface::ControllerRelease()
	{
		Potato::Pointer::ControllerPtr<FormManager> TemPtr;
		{
			std::lock_guard lg(mutex);
			switch (status)
			{
			case Status::Empty:
			case Status::Error:
			case Status::Closed:
				break;
			case Status::Waiting:
				status = Status::RequestExist;
				break;
			case Status::Ready:
				assert(window_handle != nullptr);
				DestroyWindow(window_handle);
				break;
			default:
				assert(false);
				break;
			}
			TemPtr = std::move(owner);
		}
		TemPtr.Reset();
	}

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
		static StaticClassInitStruct init_struct;

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

}