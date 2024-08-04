module;

#include <Windows.h>
#include <wrl.h>

#undef max

export module DumplingWindowsForm;

import std;
import PotatoPointer;
import PotatoIR;
import PotatoTaskSystem;
import DumplingFormEvent;


export namespace Dumpling
{
	struct Form : protected Potato::Pointer::DefaultIntrusiveInterface
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

		using Ptr = Potato::Pointer::IntrusivePtr<Form>;

		HWND GetWnd() const { std::shared_lock sl(mutex); return hwnd; }

		static Form::Ptr Create(std::size_t identity_id = 0, std::pmr::memory_resource* resource = std::pmr::get_default_resource());
		virtual bool Init(FormProperty property, std::pmr::memory_resource* temp = std::pmr::get_default_resource());
		static void PostQuitEvent();

		static LRESULT CALLBACK DefaultWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

		bool InsertCapture(FormEventCapture::Ptr capture, std::size_t priority = 0) { return capture_manager.InsertCapture(std::move(capture), priority); }


		template<typename Func>
		static bool PeekMessageEventOnce(Func&& func)
			requires(std::is_invocable_r_v<void, Func, FormEvent::System>)
		{
			return Form::PeekMessageEvent(
				[](void* data, FormEvent::System event)
				{
					(*static_cast<Func*>(data))(event);
				},
				&func
			);
		}

		template<typename Func>
		static std::size_t PeekMessageEvent(Func&& func)
			requires(std::is_invocable_r_v<void, Func, FormEvent::System>)
		{
			std::size_t count = 0;
			while(PeekMessageEventOnce(std::forward<Func>(func)))
			{
				count += 1;
			}
			return count;
		}

	protected:

		static bool PeekMessageEvent(void(*func)(void*, FormEvent::System), void*);

		Form(Potato::IR::MemoryResourceRecord record, std::size_t identity_id)
			: record(record), capture_manager( record.GetMemoryResource()) {}

		virtual ~Form() = default;

		Potato::IR::MemoryResourceRecord record;

		mutable std::shared_mutex mutex;
		HWND hwnd = nullptr;

		virtual void Release() override;

		virtual HRESULT HandleEvent(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

		CaptureManager capture_manager;

		friend struct Potato::Pointer::DefaultIntrusiveWrapper;
	};
}



export namespace Dumpling::Windows
{

	template<typename PtrT>
	using ComPtr = Microsoft::WRL::ComPtr<PtrT>;

}
