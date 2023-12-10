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
			FormChannel* ptr = reinterpret_cast<FormChannel*>(data);
			if (ptr != nullptr)
			{
				{
					std::lock_guard lg(ptr->mutex);
					ptr->owner.Reset();
				}
				ptr->SubViewerRef();
			}
			SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(nullptr));
			break;
		}
		default:
		{
			LONG_PTR data = GetWindowLongPtr(hWnd, GWLP_USERDATA);
			FormChannel* ptr = reinterpret_cast<FormChannel*>(data);
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

	void FormChannel::ControllerRelease()
	{
		std::lock_guard lg(mutex);
		if(window_handle != nullptr)
		{
			DestroyWindow(window_handle);
		}else if(owner)
		{
			
		}
	}

	std::u8string_view FormChannel::GetFormName() const
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
		while (exist_form_count != 0)
		{
			if (form_init_list.try_lock())
			{
				std::lock_guard lg(form_init_list, std::adopt_lock);
				while (!requests.empty())
				{
					auto top = std::move(*requests.rbegin());
					requests.pop_back();
					if (top.form_channel_interface)
					{

						auto form_name = top.form_channel_interface->GetFormName();

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
							auto Pointer = top.form_channel_interface.GetPointer();
							std::lock_guard lg(Pointer->mutex);
							Pointer->window_handle = handle;
							Pointer->AddViewerRef();
							SetWindowLongPtr(handle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(Pointer));
						}
						else {
							DWORD error_code = GetLastError();
							auto Pointer = top.form_channel_interface.GetPointer();
							std::lock_guard lg(Pointer->mutex);
							Pointer->error_code = error_code;
						}
					}
				}
			}

			MSG msg;
			while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessageW(&msg);
				if(msg.message == WM_CLOSE)
				{
					std::size_t E = 1;
					while(!exist_form_count.compare_exchange_strong(E, E - 1)){ assert(E > 0); }
				}
			}

			std::this_thread::yield();
		}
	}

	bool FormManager::CreateForm(FormChannel& channel)
	{
		std::lock_guard lg(channel.mutex);
		if(channel.owner)
		{
			return false;
		}else
		{
			channel.owner = this;
			channel.window_handle = nullptr;
			channel.error_code.reset();
			{
				std::lock_guard lg(form_init_list);
				FormChannel::Ptr p{ &channel };
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
		return false;
	}

}