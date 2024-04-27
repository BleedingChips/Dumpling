module;

#include <Windows.h>
#include <wrl.h>

#undef max

export module DumplingWindowsForm;

import std;
import PotatoPointer;
import PotatoIR;
import PotatoTaskSystem;
import DumplingFormInterface;

export namespace Dumpling::Windows
{

	enum class Window
	{
		WINDOW,
		WINDOW_FULL_SCREEN,
		FULL_SCREEN
	};

	struct FormSetting
	{
		std::size_t size_x = 1024;
		std::size_t size_y = 768;
		std::optional<std::size_t> offset_x;
		std::optional<std::size_t> offset_y;
		Window windows = Window::WINDOW;
		wchar_t const* form_title = L"DumplingDefaultForm";
	};

	struct FormStyle : public Potato::Pointer::DefaultStrongWeakInterface
	{
		using Ptr = Potato::Pointer::StrongPtr<FormStyle>;
		using WPtr = Potato::Pointer::WeakPtr<FormStyle>;

		static FormStyle::Ptr CreateDefaultGameplayStyle(std::pmr::memory_resource* resource = std::pmr::get_default_resource());

		virtual ~FormStyle() = default;
		virtual wchar_t const* GetStyleName() const = 0;
		virtual DWORD GetWSStyle(FormSetting const& setting) const = 0;
	};

	struct FormTaskProperty
	{
		Potato::Task::Priority priority = Potato::Task::Priority::Normal;
		std::chrono::microseconds sleep_duration = std::chrono::microseconds{ 100 };
		std::u8string_view task_name = u8"Form Task";
	};

	struct Form : protected Potato::Task::Task, protected FormInterface
	{

		enum class Status
		{
			INVALID,
			WAITING_CREATED,
			CREATING,
			NORMAL,
			CLOSED,
			CRASH,
		};

		bool Commit(
			Potato::Task::TaskContext& context, 
			std::thread::id thread_id,
			FormStyle::Ptr style,
			FormSetting setting,
			FormTaskProperty property = {}
			);

		Status GetStatus() const
		{
			std::shared_lock sl(mutex);
			return status;
		}

		using Ptr = Potato::Pointer::IntrusivePtr<Form, FormInterface::Wrapper>;

	protected:

		Form() {}
		virtual ~Form() = default;

		virtual void TaskExecute(Potato::Task::ExecuteStatus& status) override;

		mutable std::shared_mutex mutex;
		HWND hwnd = nullptr;
		Status status = Status::INVALID;
		FormSetting setting;
		FormStyle::Ptr style;

		virtual void AddTaskRef() const override { AddFormInterfaceRef(); }
		virtual void SubTaskRef() const override { SubFormInterfaceRef(); }

		virtual HRESULT HandleEvent(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) { return DefWindowProcW(hWnd, msg, wParam, lParam); }

		static LRESULT CALLBACK DefaultWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

		friend struct FormStyle;
		friend struct FormInterface::Wrapper;
	};

	
	struct EventResponderForm : public Form, public Potato::Pointer::DefaultIntrusiveInterface
	{
		using Ptr = Form::Ptr;

		static Ptr Create(FormEventResponder::Ptr ref, std::pmr::memory_resource* res = std::pmr::get_default_resource());

	protected:

		virtual HRESULT HandleEvent(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

		EventResponderForm(FormEventResponder::Ptr res, Potato::IR::MemoryResourceRecord record)
			: res(std::move(res)), record(record)
		{
			
		}

		Potato::IR::MemoryResourceRecord record;
		FormEventResponder::Ptr res;

		virtual void AddFormInterfaceRef() const override{ DefaultIntrusiveInterface::AddRef(); }
		virtual void SubFormInterfaceRef() const override { DefaultIntrusiveInterface::SubRef(); }
		virtual void Release() override;

		friend struct Potato::Pointer::DefaultIntrusiveWrapper;
	};

	/*
	struct WindowsSetting
	{
		std::size_t size_x = 1024;
		std::size_t size_y = 768;
		std::optional<std::size_t> offset_x;
		std::optional<std::size_t> offset_y;
		wchar_t const* form_name = L"Fuck You Windows";
	};

	struct RendererWrapper
	{
		using Ptr = Potato::Pointer::IntrusivePtr<RendererWrapper, FormPointerWrapperT>;

		virtual void OnReInit(HWND, std::size_t size_x, std::size_t size_y) = 0;
		virtual void OnRelease(HWND) = 0;

		virtual void FormAddRef() const = 0;
		virtual void FormSubRef() const = 0;
		virtual void OnUpdate() const = 0;

		virtual ~RendererWrapper() = default;
	};
	*/

	/*
	struct SwapChain
	{
		using Ptr = Potato::Pointer::IntrusivePtr<SwapChain, FormPointerWrapperT>;

		virtual void FormAddRef() const = 0;
		virtual void FormSubRef() const = 0;
		virtual void OnReInit(HWND, std::size_t size_x, std::size_t size_y) = 0;
		virtual void OnRelease(HWND) = 0;
		virtual void OnUpdate() = 0;
		virtual ~SwapChain() = default;
	};
	*/


	/*
	struct TaskSetting
	{
		Potato::Task::Priority priority = Potato::Task::Priority::Normal;
		std::u8string_view name = u8"win32_message_loop";
	};

	struct Form : protected Potato::Task::Task, protected Potato::Pointer::DefaultControllerViewerInterface
	{
		using Ptr = Potato::Pointer::IntrusivePtr<Form>;

		static Ptr Create(Style::Ptr style, RendererWrapper::Ptr render_wrapper = {}, std::pmr::memory_resource* resource = std::pmr::get_default_resource());

		bool CreateWindows(Potato::Task::TaskContext& context, std::thread::id thread_id, TaskSetting const& task_setting, Setting const& setting);

		virtual void AddRef() const override { DefaultControllerViewerInterface::AddViewerRef(); }
		virtual void SubRef() const override { DefaultControllerViewerInterface::SubViewerRef(); }
	};
	*/


	
	

	

	/*
	struct Form : public Potato::Pointer::DefaultControllerViewerInterface
	{

		enum class Status
		{
			Empty,
			Opened,
			Closed,
			Hidden,
			Error,
		};

		using Ptr = Potato::Pointer::ControllerPtr<Form>;

		static auto Create(
			Style::Ptr style,
			Setting const& setting,
			SwapChain::Ptr swap_chain,
			FormEventChannel::Ptr event_channel,
			std::pmr::memory_resource* resource = std::pmr::get_default_resource()
		) -> Ptr;

		void CloseWindows();

		virtual ~Form();
		virtual Status GetStatus() const;
		virtual HWND GetWindowHandle() const;
		virtual Style const& GetStyle() const { return *style; }

		void ShowWindow();
		void HideWindow();

	protected:

		Form(
			Style::Ptr style,
			SwapChain::Ptr swap_chain,
			FormEventChannel::Ptr event_channel,
			Potato::IR::MemoryResourceRecord record
			);


		virtual std::optional<HRESULT> HandleEvent(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

		virtual void ControllerRelease() override;
		virtual void ViewerRelease() override;

		Potato::IR::MemoryResourceRecord resource_record;
		Style::Ptr style;
		
		std::thread window_thread;
		HWND window_handle = nullptr;

		mutable std::shared_mutex mutex;
		Status status = Status::Empty;
		SwapChain::Ptr swap_chain;
		FormEventChannel::Ptr event_channel;

		friend struct Style;

	};
	*/

	template<typename PtrT>
	using ComPtr = Microsoft::WRL::ComPtr<PtrT>;

}
