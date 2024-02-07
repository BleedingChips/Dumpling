module;

#include <cassert>
#include <Windows.h>

#undef max

module DumplingWindowsForm;

import std;
import PotatoIR;


namespace Dumpling::Windows
{

	struct GameplayStyle : public FormStyle
	{
		static wchar_t const* class_name;

		GameplayStyle(Potato::IR::MemoryResourceRecord record)
			: record(record) {}

		Potato::IR::MemoryResourceRecord record;

		virtual wchar_t const* GetStyleName() const override { return class_name; }
		virtual DWORD GetWSStyle(FormSetting const& setting) const override { return WS_VISIBLE | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX; }
		virtual void StrongRelease() override
		{
			UnregisterClassW(class_name, GetModuleHandle(0));
		}
		virtual void WeakRelease() override
		{
			auto re = record;
			this->~GameplayStyle();
			re.Deallocate();
		}
	};

	wchar_t const* GameplayStyle::class_name = L"Dumpling_Default_GameStyle";

	FormStyle::Ptr FormStyle::CreateDefaultGameplayStyle(std::pmr::memory_resource* resource)
	{
		static std::mutex style_mutex;
		static FormStyle::WPtr ptr;

		std::lock_guard lg(style_mutex);
		auto re = ptr.Isomer();
		if (re)
			return re;
		else
		{
			ptr.Reset();
			const WNDCLASSEXW static_class = { sizeof(WNDCLASSEXW), CS_HREDRAW | CS_VREDRAW , &FormInterface::DefaultWndProc, 0, 0, GetModuleHandle(0), NULL,NULL, 0, NULL, GameplayStyle::class_name, NULL };

			ATOM res = RegisterClassExW(&static_class);

			if (res != 0)
			{
				auto record = Potato::IR::MemoryResourceRecord::Allocate<GameplayStyle>(resource);
				FormStyle::Ptr tem_tr = new (record.Get()) GameplayStyle{ record };
				ptr = tem_tr.Isomer();
				return tem_tr;
			}
			else
			{
				return {};
			}

		}
		return {};
	}


	LRESULT CALLBACK FormInterface::DefaultWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		assert(hWnd != nullptr);
		switch (msg)
		{
		case WM_CREATE:
			{
				CREATESTRUCTA* Struct = reinterpret_cast<CREATESTRUCTA*>(lParam);
				assert(Struct != nullptr);
				FormInterface* inter = static_cast<FormInterface*>(Struct->lpCreateParams);
				assert(inter != nullptr);
				SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(inter));
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
			FormInterface* ptr = reinterpret_cast<FormInterface*>(data);
			if (ptr != nullptr)
			{
				auto re = ptr->HandleEvent(hWnd, msg, wParam, lParam);
				{
					std::lock_guard lg(ptr->mutex);
					assert(ptr->status == FormInterface::Status::NORMAL);
					ptr->status = FormInterface::Status::CLOSED;
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
			FormInterface* ptr = reinterpret_cast<FormInterface*>(data);
			if (ptr != nullptr)
			{
				return ptr->HandleEvent(hWnd, msg, wParam, lParam);
			}
			break;
		}
		}
		return DefWindowProcW(hWnd, msg, wParam, lParam);
	}

	bool FormInterface::Commit(
		Potato::Task::TaskContext& context,
		std::thread::id thread_id,
		FormStyle::Ptr init_style,
		FormSetting init_setting,
		FormTaskProperty property
	)
	{
		if(init_style && thread_id != std::thread::id{})
		{
			std::lock_guard lg(mutex);
			if(status == Status::INVALID || status == Status::CLOSED)
			{
				std::size_t real_count = 0;
				auto count = property.sleep_duration.count();
				if(count < 0)
					real_count = 0;
				else if(count < std::numeric_limits<std::size_t>::max())
					real_count = static_cast<std::size_t>(count);
				else
					real_count = std::numeric_limits<std::size_t>::max();
				Potato::Task::TaskProperty task_property
				{
					property.priority,
					property.task_name,
					{0, real_count},
					Potato::Task::Category::THREAD_TASK,
					0,
					thread_id
				};
				if(context.CommitTask(this, task_property))
				{
					setting = init_setting;
					style = std::move(init_style);
					status = Status::WAITING_CREATED;
				}
			}
		}
		return false;
	}

	void FormInterface::operator()(Potato::Task::ExecuteStatus& task_status)
	{
		auto handle = reinterpret_cast<HWND>(task_status.task_property.user_data[0]);
		if(handle == nullptr)
		{
			FormStyle::Ptr created_style;
			FormSetting created_setting;
			if(mutex.try_lock())
			{
				{
					std::lock_guard lg(mutex, std::adopt_lock);
					assert(status == Status::WAITING_CREATED);
					created_style = style;
					created_setting = setting;
					status = Status::CREATING;
				}

				handle = CreateWindowExW(
					0,
					style->GetStyleName(),
					setting.form_title,
					style->GetWSStyle(setting),
					100, 100, setting.size_x, setting.size_y,
					NULL,
					NULL,
					GetModuleHandle(0),
					this
				);

				if(handle != nullptr)
				{
					task_status.task_property.user_data[0] = reinterpret_cast<std::size_t>(handle);
				}else
				{
					std::lock_guard lg(mutex);
					status = Status::CRASH;
				}

			}
		}

		if(handle != nullptr)
		{
			MSG msg;
			while (PeekMessage(&msg, handle, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessageW(&msg);
			}

			if(mutex.try_lock())
			{
				std::lock_guard lg(mutex, std::adopt_lock);
				if(status == Status::CLOSED)
					return;
			}
		}

		auto timer = task_status.task_property.user_data[1];
		if(timer == 0)
		{
			task_status.context.CommitTask(this, task_status.task_property);
		}else
		{
			task_status.context.CommitDelayTask(this, std::chrono::microseconds{ timer }, task_status.task_property);
		}
	}

	Form::Ptr Form::Create(std::pmr::memory_resource* res)
	{
		auto re = Potato::IR::MemoryResourceRecord::Allocate<Form>(res);
		if (re)
		{
			return Form::Ptr{ new (re.Get()) Form{re} };
		}
		return {};
	}

	void Form::Release()
	{
		auto re = record;
		this->~Form();
		re.Deallocate();
	}
}