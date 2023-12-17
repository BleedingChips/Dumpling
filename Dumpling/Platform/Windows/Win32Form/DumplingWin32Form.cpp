module;

#include <cassert>
#include <Windows.h>

module DumplingWin32Form;

import std;
import PotatoIR;

namespace Dumpling::Win32
{

	auto Win32Style::Create(wchar_t const* class_name, std::pmr::memory_resource* resource)
		-> Ptr
	{
		if(resource != nullptr && class_name != nullptr)
		{
			const WNDCLASSEXW static_class = { sizeof(WNDCLASSEXW), CS_HREDRAW | CS_VREDRAW , &Win32Style::WndProc, 0, 0, GetModuleHandle(0), NULL,NULL, 0, NULL, class_name, NULL };
			ATOM res = RegisterClassExW(&static_class);
			if(res != 0)
			{
				std::wstring_view name{ class_name };
				Potato::IR::Layout layout = Potato::IR::Layout::Get<Win32Style>();
				Potato::IR::Layout str_layout = Potato::IR::Layout::GetArray<wchar_t>(name.size() + 1);
				auto offset = Potato::IR::InsertLayoutCPP(layout, str_layout);
				Potato::IR::FixLayoutCPP(layout);
				assert(layout.Align == alignof(Win32Style));

				auto record = Potato::IR::MemoryResourceRecord::Allocate(resource, layout);

				if(record)
				{
					Ptr ptr =
						new (record.Get()) Win32Style{
							record,
							record.Cast<std::byte>() + offset,
							name
					};
					return ptr;
				}else
				{
					UnregisterClassW(class_name, GetModuleHandle(0));
				}
			}
		}
		return {};
	}

	Win32Style::Win32Style(Potato::IR::MemoryResourceRecord record, std::byte* offset, std::wstring_view class_type_view)
		: resource_record(record), class_type(reinterpret_cast<wchar_t*>(offset))
	{
		std::memcpy(offset, class_type_view.data(), sizeof(wchar_t) * (class_type_view.size() + 1));
	}

	Win32Style::~Win32Style()
	{
		UnregisterClassW(class_type, GetModuleHandle(0));
	}

	void Win32Style::Release()
	{
		auto record = resource_record;
		this->~Win32Style();
		record.Deallocate();
	}
	
	LRESULT CALLBACK Win32Style::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
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
					assert( ptr->status == Win32Form::Status::Opened || ptr->status == Win32Form::Status::Hidden);
					ptr->status = Win32Form::Status::Closed;
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
				std::shared_lock sl(ptr->mutex);
				if(ptr->event_channel)
				{
					
				}
			}
			break;
		}
		}
		return DefWindowProcW(hWnd, msg, wParam, lParam);
	}

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

	Win32Form::Win32Form(
		Win32Style::Ptr style,
		Win32Renderer::Ptr renderer,
		Win32FormEventChannel::Ptr event_channel,
		Potato::IR::MemoryResourceRecord record
	)
		: resource_record(record), style(std::move(style)), renderer(std::move(renderer)), event_channel(std::move(event_channel))
	{
		assert(this->style);
	}

	void Win32Form::ViewerRelease()
	{
		auto res = resource_record;
		assert(res);
		this->~Win32Form();
		res.Deallocate();
	}

	HWND Win32Form::GetWindowHandle() const
	{
		std::shared_lock sl(mutex);
		return window_handle;
	}

	auto Win32Form::CreateWin32Form(
		Win32Style::Ptr style,
		Win32Setting const& setting,
		Win32Renderer::Ptr renderer,
		Win32FormEventChannel::Ptr event_channel,
		std::pmr::memory_resource* resource
		)
		-> Ptr
	{
		if(style)
		{
			auto record = Potato::IR::MemoryResourceRecord::Allocate<Win32Form>(resource);
			if(record)
			{
				Win32Form::Ptr ptr { new (record.Get()) Win32Form{
					std::move(style), std::move(renderer), std::move(event_channel), record
				}};

				std::promise<DWORD> promise;

				auto fur = promise.get_future();

				ptr->window_thread = std::thread{ [&]() mutable {

					std::wstring title {setting.form_name};

					HWND handle = CreateWindowExW(
						0,
						ptr->GetStyle().GetStyleName(),
						setting.form_name,
						WS_VISIBLE | WS_OVERLAPPEDWINDOW,
						100, 100, setting.size_x, setting.size_y,
						NULL,
						NULL,
						GetModuleHandle(0),
						NULL
					);

					if (handle != nullptr)
					{
						ptr->window_handle = handle;
						ptr->status = Status::Opened;
						ptr->AddViewerRef();
						SetWindowLongPtr(handle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(ptr.GetPointer()));
						promise.set_value(0);
					}
					else
					{
						DWORD error_code = GetLastError();
						//Pointer = error_code;
						ptr->status = Status::Error;
						promise.set_value(error_code);
						return;
					}

					while (true)
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
						std::this_thread::sleep_for(std::chrono::milliseconds{ 1 });
					}
					return;
				}};

				auto tem = fur.get();

				if(tem == 0)
				{
					return ptr;
				}
			}
		}
		return {};
	}

	auto Win32Form::GetStatus() const
		-> Status
	{
		std::shared_lock sl(mutex);
		return status;
	}
}