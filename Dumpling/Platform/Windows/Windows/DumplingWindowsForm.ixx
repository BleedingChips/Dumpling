module;

#include <Windows.h>
#include <wrl.h>

#undef max
#undef interface

export module DumplingWindowsForm;

import std;
import PotatoPointer;
import PotatoIR;
import PotatoTaskSystem;
export import DumplingFormEvent;


export namespace Dumpling
{


	export struct Form;

	struct FormEventCapture
	{
		struct Wrapper
		{
			template<typename Type> void AddRef(Type* ptr) const { ptr->AddFormEventCaptureRef(); }
			template<typename Type> void SubRef(Type* ptr) const { ptr->SubFormEventCaptureRef(); }
		};

		using Ptr = Potato::Pointer::IntrusivePtr<FormEventCapture, Wrapper>;
		FormEvent::Category GetAcceptedCategory() const { return accepted_category; }

		virtual FormEvent::Respond Receive(Form& interface, FormEvent::Modify event) { return FormEvent::Respond::PASS; }
		virtual FormEvent::Respond Receive(Form& interface, FormEvent::System event) { return FormEvent::Respond::PASS; }
		virtual FormEvent::Respond Receive(Form& interface, FormEvent::Input event) { return FormEvent::Respond::PASS; }
		virtual HRESULT ReceiveRaw(Form& interface, FormEvent::Category category, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	protected:

		FormEventCapture(FormEvent::Category accepted_category = FormEvent::Category::UNACCEPTABLE) : accepted_category(accepted_category){}

		FormEvent::Category accepted_category;

		virtual void AddFormEventCaptureRef() const = 0;
		virtual void SubFormEventCaptureRef() const = 0;

		friend struct Form;
	};

	export struct Form : protected Potato::Pointer::DefaultIntrusiveInterface
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

		bool InsertCapture(FormEventCapture::Ptr capture, std::size_t priority = 0);


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
			: record(record), captures( record.GetMemoryResource()) {}

		virtual ~Form() = default;

		Potato::IR::MemoryResourceRecord record;

		mutable std::shared_mutex mutex;
		HWND hwnd = nullptr;

		virtual void Release() override;

		virtual HRESULT HandleEvent(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

		std::size_t identity_id = 0;

		std::shared_mutex capture_mutex;


		struct CaptureTuple
		{
			std::size_t priority;
			FormEvent::Category acceptable_category;
			FormEventCapture::Ptr capture;
		};

		std::pmr::vector<CaptureTuple> captures;

		friend struct FormHelper;

		friend struct Potato::Pointer::DefaultIntrusiveWrapper;
	};

	HRESULT MarkMessageSkip(UINT msg)
	{
		return E_NOTIMPL;
	}
}



export namespace Dumpling::Windows
{

	template<typename PtrT>
	using ComPtr = Microsoft::WRL::ComPtr<PtrT>;

}
