module;

#include <Windows.h>
#include <wrl.h>

#undef max

export module DumplingWindowsForm;

import std;
import PotatoPointer;
import PotatoIR;
import PotatoTaskSystem;
import DumplingForm;

export namespace Dumpling::Windows
{

	struct FormClassStyle
	{
		FormClassStyle();
		~FormClassStyle();
	};

	struct Win32Form : public Form, protected Potato::Pointer::DefaultIntrusiveInterface
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

		using Ptr = Potato::Pointer::IntrusivePtr<Win32Form, Form::Wrapper>;

		HWND GetWnd() const { std::shared_lock sl(mutex); return hwnd; }

		static bool PeekMessageEvent(void(*func)(void*, FormEvent::System), void*);
		static Form::Ptr Create(std::size_t identity_id, std::pmr::memory_resource* resource);
		virtual bool Init(FormProperty property, std::pmr::memory_resource* temp) override;
		static void PostQuitEvent();

	protected:

		Win32Form(Potato::IR::MemoryResourceRecord record, std::size_t identity_id)
			: record(record), Form(identity_id, record.GetMemoryResource()) {}

		virtual ~Win32Form() = default;

		Potato::IR::MemoryResourceRecord record;

		mutable std::shared_mutex mutex;
		HWND hwnd = nullptr;

		virtual void AddFormRef() const override { DefaultIntrusiveInterface::AddRef(); }
		virtual void SubFormRef() const override { DefaultIntrusiveInterface::SubRef(); }
		virtual void Release() override;

		virtual HRESULT HandleEvent(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

		static LRESULT CALLBACK DefaultWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

		friend struct FormClassStyle;
		friend struct Form::Wrapper;
		friend struct FormInitTask;
		friend struct FormManager;
	};

	template<typename PtrT>
	using ComPtr = Microsoft::WRL::ComPtr<PtrT>;

}
