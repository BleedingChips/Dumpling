#include "win32.h"
#include <optional>
using namespace Potato;
using namespace Dumpling::Win32;

namespace Dumpling::Win32
{
	const FormStyle& DefaultStyle() noexcept { static FormStyle Default;  return Default; }
	const char16_t static_class_name[] = u"po_frame_default_win32_class";
	const WNDCLASSEXW static_class = { sizeof(WNDCLASSEXW), CS_HREDRAW | CS_VREDRAW , Form::WndProc, 0, 0, GetModuleHandle(0), NULL,NULL, 0, NULL, (const wchar_t*)static_class_name, NULL };
	const struct StaticClassInitStruct
	{
		StaticClassInitStruct() { HRESULT res = RegisterClassExW(&static_class); assert(SUCCEEDED(res)); }
		~StaticClassInitStruct() { UnregisterClassW((const wchar_t*)static_class_name, GetModuleHandleW(0)); }
	};

	struct GobalManager
	{
		~GobalManager();
		HWND CreateForm(const FormSetting& Setting, const FormStyle& Style, Form* Form);
		GobalManager();

		uint32_t FormCount = 0;

	private:

		struct Request {
			std::promise<HWND> Promise;
			const FormSetting& Setting;
			const FormStyle& Style;
			Form* Form;
		};

		void ExectionFunction();
		
		std::mutex m_RequestsMutex;
		bool m_ThreadRunning;
		std::optional<Request> m_Requests;
		std::thread m_ExecuteThread;
	}GobalManagerInstance;

	struct FormImplement;

	GobalManager::GobalManager()
		: m_ThreadRunning(false) {}

	HWND GobalManager::CreateForm(const FormSetting& Setting, const FormStyle& Style, Form* Form)
	{
		std::promise<HWND> pro;
		auto fur = pro.get_future();
		while (true)
		{
			if (m_RequestsMutex.try_lock())
			{
				std::lock_guard lg(m_RequestsMutex, std::adopt_lock);
				if (!m_Requests.has_value())
				{
					m_Requests.emplace(Request{ std::move(pro), Setting, Style, Form });
					if (!m_ThreadRunning)
					{
						if (m_ExecuteThread.joinable()) m_ExecuteThread.join();
						m_ExecuteThread = std::thread([this]() {ExectionFunction(); });
						m_ThreadRunning = true;
					}
					break;
				}
			}
			std::this_thread::yield();
		}
		return fur.get();
	}

	GobalManager::~GobalManager()
	{
		volatile int i2 = 0;
		if (m_ExecuteThread.joinable())
			m_ExecuteThread.join();
		volatile int i = 0;
	}

	void GobalManager::ExectionFunction()
	{
		static StaticClassInitStruct static_class;
		while (true)
		{
			MSG msg;
			while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessageW(&msg);
				
			}
			
			{
				std::lock_guard lg(m_RequestsMutex);
				if (m_Requests.has_value())
				{
					auto& Ref = *m_Requests;
					HWND handle = CreateWindowExW(
						0,
						(wchar_t*)(static_class_name),
						(Ref.Setting.Title),
						WS_VISIBLE | WS_OVERLAPPEDWINDOW,
						Ref.Setting.ShiftX, Ref.Setting.ShiftY, Ref.Setting.Width, Ref.Setting.Height,
						NULL,
						NULL,
						GetModuleHandle(0),
						NULL
					);
					if (handle != nullptr)
					{
						SetWindowLongPtr(handle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(Ref.Form));
						++FormCount;
						Ref.Promise.set_value(handle);
					}
					else {
						DWORD ErrorCode = GetLastError();
						Ref.Promise.set_exception(std::make_exception_ptr(Dumpling::Win32::Error::FaultToCreate{ "unable_to_create Fault" }));
					}
					m_Requests = std::nullopt;
				}
				else if (FormCount == 0)
				{
					m_ThreadRunning = false;
					break;
				}
			}
			std::this_thread::sleep_for(std::chrono::milliseconds{ 1 });
		}
	}

	Form::~Form()
	{
		SendMessage(GetHWnd(), WM_USER + 1, 0, 0);
		volatile int index = 0;
	}

	Form::Form(const FormSetting& Setting, const FormStyle& Style)
		//: m_available(true)
	{
		m_Hwnd = GobalManagerInstance.CreateForm(Setting, Style, this);
	}

	std::optional<LRESULT> Form::RespondEventInEventLoop(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
	{
		{
			std::lock_guard lg(m_EventFunctionMutex);
			if (m_EventFunction)
				return m_EventFunction(hWnd, msg, wParam, lParam);
		}
		return {};
	}

	void Form::OverwriteEventFunction(EventFunctionT event_function) noexcept
	{
		std::lock_guard lg(m_EventFunctionMutex);
		m_EventFunction = std::move(event_function);
	}

	LRESULT CALLBACK Form::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		assert(hWnd != nullptr);
		switch (msg)
		{
		case WM_USER + 1:
		{
			LONG_PTR data = GetWindowLongPtr(hWnd, GWLP_USERDATA);
			Form* ptr = reinterpret_cast<Form*>(data);
			if (ptr != nullptr)
				SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(nullptr));
			DestroyWindow(hWnd);
			break;
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
			GobalManagerInstance.FormCount -= 1;
			break;
		}
		default:
		{
			LONG_PTR data = GetWindowLongPtr(hWnd, GWLP_USERDATA);
			Form* ptr = reinterpret_cast<Form*>(data);
			std::optional<LRESULT> Result = std::nullopt;
			if (ptr != nullptr)
				Result = ptr->RespondEventInEventLoop(hWnd, msg, wParam, lParam);
			if (Result.has_value())
				return *Result;
			else if(msg != WM_CLOSE)
				return DefWindowProcW(hWnd, msg, wParam, lParam);
			return 0;
		}
		}
		return DefWindowProcW(hWnd, msg, wParam, lParam);
	}
	
	FormPtr Form::Create(const FormSetting & Setting, const FormStyle& Style)
	{
		return new Form{ Setting,  Style };
	}


	

	/*
	LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch (msg)
		{
		default:
		{
			LONG_PTR data = GetWindowLongPtr(hWnd, GWLP_USERDATA);
			FormImplement* ptr = reinterpret_cast<FormImplement*>(data);
			if (ptr != nullptr)
				return ptr->RespondEventInEventLoop(hWnd, msg, wParam, lParam);
			return DefWindowProcW(hWnd, msg, wParam, lParam);
		}
		}
	}
	*/

	/*
	std::tuple<DWORD, DWORD> translate_style(Dumpling::Win32::Style style)
	{
		switch (style)
		{
		case Dumpling::Win32::Style::Normal:
			return { 0, WS_OVERLAPPEDWINDOW };
		default:
			return { 0, 0 };
		}
	}
	*/
	

	
	/*
	void GobalManager::execute_function(GobalManager* ins)
	{
		bool available = true;
		while (available)
		{
			{
				std::lock_guard lg(ins->state_lock);
				if (!ins->requests.empty())
				{
					for (auto& ite : ins->requests)
					{
						auto& setting = std::get<1>(ite);
						auto type = translate_style(setting.style);
						HWND handle = CreateWindowExW(
							std::get<0>(type),
							(wchar_t*)(static_class_name),
							(wchar_t*)(setting.title.c_str()),
							WS_VISIBLE | std::get<1>(type),
							setting.shift_x, setting.shift_y, setting.width, setting.height,
							NULL,
							NULL,
							GetModuleHandle(0),
							NULL
						);
						if (handle != nullptr)
						{
							SetWindowLongPtr(handle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(std::get<2>(ite)));
							std::get<2>(ite)->m_handle = handle;
							std::get<0>(ite).set_value();
							++ins->count;
						}
						else 
							std::get<0>(ite).set_exception(std::make_exception_ptr(Dumpling::Win32::Error::CreateWindowFauit{ GetLastError() }));
					}
					ins->requests.clear();
				}
				else if (ins->count == 0)
				{
					ins->read_to_exit = true;
					available = false;
				}
			}
			MSG msg;
			while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
			{
				if (msg.message == UD_REQUEST_QUIT)
				{
					std::lock_guard lg(ins->state_lock);
					--ins->count;
				}
				TranslateMessage(&msg);
				DispatchMessageW(&msg);
			}
			std::this_thread::sleep_for(std::chrono::milliseconds{ 1 });
		}
	}

	intrusive_ptr<Dumpling::Win32::Implement::Control> GobalManager::create_control(const Dumpling::Win32::FormProperty& pro)
	{
		std::promise<void> promise;
		auto future = promise.get_future();
		intrusive_ptr<Implement::Control> control = new Implement::Control{};
		{
			std::lock_guard lg(state_lock);
			if (read_to_exit)
			{
				if (execute_thread.joinable())
					execute_thread.join();
				execute_thread = std::thread(execute_function, this);
			}
			requests.push_back(std::forward_as_tuple(promise, pro, control));
		}
		future.get();
		return std::move(control);
	}

	GobalManager gobal;
	*/
}



namespace Dumpling::Win32
{
	/*
	FormPtr CreateForm(const wchar_t* title, uint32_t width, uint32_t height, const FormSetting & Setting)
	{
		return new FormImplement{ title, width, height, Setting };
	}
	*/
	/*
	namespace Error
	{
		const char* CreateWindowFauit::what() const noexcept { return "unable to create windows"; }
	}

	

	namespace Implement
	{

		Control::Control(): m_handle(nullptr) {}
		void Control::add_ref() noexcept { m_ref.add_ref(); }
		void Control::sub_ref() noexcept
		{
			if (m_ref.sub_ref())
				PostMessageW(m_handle, UD_REQUEST_QUIT, 0, 0);
		}
	}

	bool Form::pook_event(MSG& out) noexcept
	{
		assert(m_ref);
		std::lock_guard lg(m_ref->m_mutex);
		if (!m_ref->m_msages.empty())
		{
			out = *m_ref->m_msages.begin();
			m_ref->m_msages.pop_front();
			return true;
		}
		return false;
	}

	Form Form::create(const FormProperty & pro)
	{
		Form result;
		result.m_ref = gobal.create_control(pro);
		return std::move(result);
	}
	*/
}