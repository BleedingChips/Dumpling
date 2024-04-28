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


	FormClassStyle::FormClassStyle()
	{
		const WNDCLASSEXW static_class = {
			sizeof(WNDCLASSEXW),
			CS_HREDRAW | CS_VREDRAW ,
			&Form::DefaultWndProc, 0, 0, GetModuleHandle(0), NULL,NULL, 0, NULL, form_class_style_name, NULL };

		ATOM res = RegisterClassExW(&static_class);
		assert(res != 0);
	}

	FormClassStyle::~FormClassStyle()
	{
		UnregisterClassW(form_class_style_name, GetModuleHandle(0));
	}

	LRESULT CALLBACK Form::DefaultWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		assert(hWnd != nullptr);
		switch (msg)
		{
		case WM_NCCREATE:
			break;
		case WM_CREATE:
			{
				CREATESTRUCTA* Struct = reinterpret_cast<CREATESTRUCTA*>(lParam);
				assert(Struct != nullptr);
				Form* inter = static_cast<Form*>(Struct->lpCreateParams);
				assert(inter != nullptr);
				SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(inter));
				break;
				{
					std::lock_guard lg(inter->mutex);
					inter->status = Status::NORMAL;
					inter->hwnd = hWnd;
				}
				return inter->HandleEvent(hWnd, msg, wParam, lParam);
			}
		case WM_NCDESTROY:
		{
			LONG_PTR data = GetWindowLongPtr(hWnd, GWLP_USERDATA);
			Form* ptr = reinterpret_cast<Form*>(data);
			if (ptr != nullptr)
			{
				auto re = ptr->HandleEvent(hWnd, msg, wParam, lParam);
				{
					std::lock_guard lg(ptr->mutex);
					assert(ptr->status == Form::Status::NORMAL);
					ptr->status = Form::Status::CLOSED;
					ptr->hwnd = nullptr;
				}
				SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(nullptr));
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

	bool Form::CommitedMessageLoop(Potato::Task::TaskContext& context, std::thread::id require_thread_id, FormTaskProperty task_property)
	{
		if(require_thread_id != std::thread::id{})
		{
			std::lock_guard lg(mutex);
			if(status == Status::INVALID)
			{
				Potato::Task::TaskProperty new_task_property
				{
					property.title,
					{0, 0},
					{
						task_property.priority,
						Potato::Task::Category::THREAD_TASK,
						0,
						require_thread_id
					}
				};
				if(context.CommitTask(this, new_task_property))
				{
					status = Status::WAITING_CREATED;
				}
			}
		}
		return false;
	}

	void Form::TaskExecute(Potato::Task::ExecuteStatus& task_status)
	{
		static FormClassStyle class_style;
		auto handle = reinterpret_cast<HWND>(task_status.task_property.user_data[0]);
		if(handle == nullptr)
		{
			std::lock_guard lg(mutex);
			auto title = *Potato::Encode::StrEncoder<char8_t, wchar_t>::EncodeToString(property.title);
			handle = CreateWindowExW(
					0,
					form_class_style_name,
					L"Fuck",
					GetWSStyle(property.style),
					100, 100, property.form_size.width, property.form_size.height,
					NULL,
					NULL,
					GetModuleHandle(0),
					this
				);
			if(handle == nullptr)
			{

				task_status.task_property.user_data[0] = reinterpret_cast<std::size_t>(handle);
				if(task_status.context.CommitTask(this, task_status.task_property))
				{
					status = Status::NORMAL;
					hwnd = handle;
				}
				return;
			}else
			{
				status = Status::CRASH;
				return;
			}
		}else
		{
			MSG msg;
			while (PeekMessage(&msg, handle, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessageW(&msg);
			}

			std::shared_lock sl(mutex);
			if(status == Status::CLOSED)
			{
				return;
			}
			task_status.context.CommitTask(this, task_status.task_property);
		}
	}


	FormInterface::Ptr Form::CreateGameWindows(FormProperty property, std::pmr::memory_resource* memory_resource)
	{
		auto re = Potato::IR::MemoryResourceRecord::Allocate<Form>(memory_resource);
		if (re)
		{
			return FormInterface::Ptr{ new (re.Get()) Form{re, std::move(property)} };
		}
		return {};
	}

	void Form::Release()
	{
		auto re = record;
		this->~Form();
		re.Deallocate();
	}

	HRESULT Form::HandleEvent(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		std::shared_lock sl(mutex);
		if(property.responder)
		{
			auto re = property.responder->Respond(*this, {});
			if(re.has_value())
			{
				
			}
		}
		return DefWindowProcW(hWnd, msg, wParam, lParam);
	}
}