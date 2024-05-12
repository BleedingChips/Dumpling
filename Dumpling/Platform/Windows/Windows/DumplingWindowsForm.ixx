module;

#include <Windows.h>
#include <wrl.h>

#undef max

export module DumplingWindowsForm;

import std;
import PotatoPointer;
import PotatoIR;
import PotatoTaskSystem;
import DumplingInterfaceForm;

export namespace Dumpling::Windows
{

	struct FormClassStyle
	{
		FormClassStyle();
		~FormClassStyle();
	};

	export struct FormManager;

	struct FormInit
	{
		using Ptr = Potato::Pointer::UniquePtr<FormInit>;

		static Ptr Create(FormStyle style, FormSize size, std::u8string_view title, std::pmr::memory_resource* resource);

		

		FormInit(FormStyle style, FormSize size, Potato::IR::MemoryResourceRecord record, std::span<wchar_t const> str)
			: record(record), style(style), size(size), title(str) {}

		virtual void Release();
		
		Potato::IR::MemoryResourceRecord record;
		std::span<wchar_t const> title;
		FormStyle style;
		FormSize size;
	};

	struct Form : public FormInterface, protected Potato::Pointer::DefaultIntrusiveInterface
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

		using Ptr = Potato::Pointer::IntrusivePtr<Form, FormInterface::Wrapper>;

		HWND GetWnd() const { std::shared_lock sl(mutex); return hwnd; }

	protected:

		Form(Potato::IR::MemoryResourceRecord record, FormEventResponder::Ptr respond, FormRenderTarget::Ptr form_renderer)
			: record(record), event_responder(std::move(respond)), form_renderer(std::move(form_renderer)){}

		virtual ~Form() = default;

		Potato::IR::MemoryResourceRecord record;

		mutable std::shared_mutex mutex;
		HWND hwnd = nullptr;
		FormEventResponder::Ptr event_responder;
		FormRenderTarget::Ptr form_renderer;

		virtual void AddFormInterfaceRef() const override { DefaultIntrusiveInterface::AddRef(); }
		virtual void SubFormInterfaceRef() const override { DefaultIntrusiveInterface::SubRef(); }
		virtual void Release() override;

		virtual HRESULT HandleEvent(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

		static LRESULT CALLBACK DefaultWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

		friend struct FormClassStyle;
		friend struct FormInterface::Wrapper;
		friend struct FormInit;
		friend struct FormManager;
	};

	export struct FormManager : public Dumpling::FormManager, public Potato::Task::Task, protected Potato::Pointer::DefaultIntrusiveInterface
	{
		using Ptr = Dumpling::FormManager::Ptr;


		static auto CreateManager(
			std::pmr::memory_resource* resource
			) -> Ptr;

		virtual FormInterface::Ptr CreateForm(
			FormProperty property,
			FormEventResponder::Ptr responder,
			FormRenderTarget::Ptr renderer,
			std::pmr::memory_resource* resource
		) override;

		virtual bool Commite(
			Potato::Task::TaskContext& context,
			std::thread::id thread_id,
			FormTaskProperty property
		) override;

		void CloseMessageLoop() override{ PostQuitMessage(0); };

	protected:

		FormManager(Potato::IR::MemoryResourceRecord record)
			: record(record) {}

		virtual void TaskExecute(Potato::Task::ExecuteStatus& status) override;
		virtual void TaskTerminal(Potato::Task::TaskProperty property) noexcept override;

		Potato::IR::MemoryResourceRecord record;

		std::mutex mutex;
		std::optional<std::thread::id> current_thread_id;
		std::pmr::vector<std::tuple<Form::Ptr, FormInit::Ptr>> init_requires;


		void AddFormManagerRef() const override{ DefaultIntrusiveInterface::AddRef(); }
		void SubFormManagerRef() const override { DefaultIntrusiveInterface::SubRef(); }
		void AddTaskRef() const override { DefaultIntrusiveInterface::AddRef(); }
		void SubTaskRef() const override { DefaultIntrusiveInterface::SubRef(); }
		void Release() override;
	};

	template<typename PtrT>
	using ComPtr = Microsoft::WRL::ComPtr<PtrT>;

}
