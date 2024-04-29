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
				return new (record.Get()) FormInit(
					style, size, record, wstr
				);
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

	

	void FormInit::TaskExecute(Potato::Task::ExecuteStatus& status)
	{
		static FormClassStyle class_style;

		Form* ptr = reinterpret_cast<Form*>(status.task_property.user_data[0]);
		assert(ptr != nullptr);
		HWND new_hwnd = CreateWindowExW(
			0,
			form_class_style_name,
			title.data(),
			GetWSStyle(style),
			100, 100, size.width, size.height,
			NULL,
			NULL,
			GetModuleHandle(0),
			ptr
		);
		status.task_property.user_data[0] = reinterpret_cast<std::size_t>(new_hwnd);
		status.context.CommitTask(ptr, status.task_property);
		ptr->SubFormInterfaceRef();
	}

	FormInterface::Ptr Form::CreateFormAndCommitedMessageLoop(
			Potato::Task::TaskContext& context,
			std::thread::id thread_id,
			FormProperty property,
			FormTaskProperty task_property,
			std::pmr::memory_resource* resource
		)
	{
		auto form_record = Potato::IR::MemoryResourceRecord::Allocate<Form>(resource);
		if(form_record)
		{
			Form::Ptr form = new(form_record.Get()) Form(form_record, property);
			auto init = FormInit::Create(property.style, property.form_size, property.title, resource);
			if(init)
			{
				Potato::Task::TaskProperty new_task_property
				{
					property.title,
					{reinterpret_cast<std::size_t>(form.GetPointer()), 0},
					{
						task_property.priority,
						Potato::Task::Category::THREAD_TASK,
						0,
						thread_id
					}
				};
				if(context.CommitTask(init.GetPointer(), new_task_property))
				{
					return form.GetPointer();
				}
			}
		}

		return {};
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
				SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(inter));
				break;
				{
					/*
					std::lock_guard lg(inter->mutex);
					inter->status = Status::NORMAL;
					inter->hwnd = hWnd;
					*/
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
					//assert(ptr->status == Form::Status::NORMAL);
					//ptr->status = Form::Status::CLOSED;
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

	/*
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
	*/

	void Form::TaskExecute(Potato::Task::ExecuteStatus& task_status)
	{
		
		auto handle = reinterpret_cast<HWND>(task_status.task_property.user_data[0]);
		assert(handle != nullptr);

		MSG msg;
		while (PeekMessage(&msg, handle, 0, 0, PM_REMOVE))
		{
			//TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
		/*
		std::shared_lock sl(mutex);
		if(status == Status::CLOSED)
		{
			return;
		}
		*/
		task_status.context.CommitTask(this, task_status.task_property);
	}

	void Form::Release()
	{
		auto re = record;
		this->~Form();
		re.Deallocate();
	}

	HRESULT Form::HandleEvent(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		if(event_responder)
		{
			auto re = event_responder->Respond(*this, {});
			if(re)
			{
				
			}
		}
		return DefWindowProcW(hWnd, msg, wParam, lParam);
	}
}