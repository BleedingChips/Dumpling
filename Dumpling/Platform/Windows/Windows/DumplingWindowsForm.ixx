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
	/*
	struct FormStyle : public Potato::Pointer::DefaultStrongWeakInterface
	{
		using Ptr = Potato::Pointer::StrongPtr<FormStyle>;
		using WPtr = Potato::Pointer::WeakPtr<FormStyle>;

		static FormStyle::Ptr CreateDefaultGameplayStyle(std::pmr::memory_resource* resource = std::pmr::get_default_resource());

		virtual ~FormStyle() = default;
		virtual wchar_t const* GetStyleName() const = 0;
		virtual DWORD GetWSStyle(Dumpling::FormStyle style) const = 0;
	};
	*/

	struct FormClassStyle
	{
		FormClassStyle();
		~FormClassStyle();
	};

	struct FormInit : public Potato::Task::Task, public Potato::Pointer::DefaultIntrusiveInterface
	{
		using Ptr = Potato::Pointer::IntrusivePtr<FormInit>;

		static Ptr Create(FormStyle style, FormSize size, std::u8string_view title, std::pmr::memory_resource* resource);

		bool Commited(Potato::Task::TaskContext& context, Potato::Task::TaskProperty property);

	protected:

		FormInit(FormStyle style, FormSize size, Potato::IR::MemoryResourceRecord record, std::span<wchar_t const> str)
			: record(record), style(style), size(size), title(str) {}

		virtual void Release() override;

		Potato::IR::MemoryResourceRecord record;
		std::span<wchar_t const> title;
		FormStyle style;
		FormSize size;

		virtual void TaskExecute(Potato::Task::ExecuteStatus& status) override;
		//virtual void TaskTerminal(Potato::Task::TaskProperty property) noexcept override;
		virtual void AddTaskRef() const override { DefaultIntrusiveInterface::AddRef(); }
		virtual void SubTaskRef() const override { DefaultIntrusiveInterface::SubRef(); }
	};

	struct Form : protected FormInterface, protected Potato::Task::Task, protected Potato::Pointer::DefaultIntrusiveInterface
	{
		static FormInterface::Ptr CreateFormAndCommitedMessageLoop(
			Potato::Task::TaskContext& context,
			std::thread::id thread_id,
			FormProperty property,
			FormTaskProperty task_property,
			std::pmr::memory_resource* resource
		);

		enum class Status
		{
			INVALID,
			WAITING_CREATED,
			CREATING,
			NORMAL,
			CLOSED,
			CRASH,
		};

		virtual bool CommitedMessageLoop(Potato::Task::TaskContext& context, std::thread::id require_thread_id, FormTaskProperty property) override;

		/*
		Status GetStatus() const
		{
			std::shared_lock sl(mutex);
			return status;
		}
		*/

		using Ptr = Potato::Pointer::IntrusivePtr<Form, FormInterface::Wrapper>;

	protected:

		Form(Potato::IR::MemoryResourceRecord record, FormProperty property)
			: record(record) {}

		virtual ~Form() = default;

		virtual void TaskExecute(Potato::Task::ExecuteStatus& status) override;

		Potato::IR::MemoryResourceRecord record;

		mutable std::shared_mutex mutex;
		HWND hwnd = nullptr;
		FormEventResponder::Ptr event_responder;
		FormRenderer::Ptr form_renderer;

		virtual void AddTaskRef() const override { DefaultIntrusiveInterface::AddRef(); }
		virtual void SubTaskRef() const override { DefaultIntrusiveInterface::SubRef(); }
		virtual void AddFormInterfaceRef() const override { DefaultIntrusiveInterface::AddRef(); }
		virtual void SubFormInterfaceRef() const override { DefaultIntrusiveInterface::SubRef(); }
		virtual void Release() override;

		virtual HRESULT HandleEvent(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

		static LRESULT CALLBACK DefaultWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

		friend struct FormClassStyle;
		friend struct FormInterface::Wrapper;
		friend struct FormInit;
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
