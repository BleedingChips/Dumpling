module;

#include <Windows.h>

export module DumplingWin32Form;

import std;
import PotatoPointer;
import PotatoIR;


export namespace Dumpling::Win32
{

	struct Win32FormPointerWrapperT
	{
		template<typename PtrT>
		void AddRef(PtrT* ptr) const { return ptr->FormAddRef(); }
		template<typename PtrT>
		void SubRef(PtrT* ptr) const { return ptr->FormSubRef(); }
	};

	struct Win32Style : public Potato::Pointer::DefaultIntrusiveInterface
	{

		using Ptr = Potato::Pointer::IntrusivePtr<Win32Style>;

		static Ptr Create(wchar_t const* class_name, std::pmr::memory_resource* resource = std::pmr::get_default_resource());

		wchar_t const* GetStyleName() const { return class_type; }

	protected:

		static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

		Win32Style(Potato::IR::MemoryResourceRecord record, std::byte* offset, std::wstring_view class_type);
		~Win32Style();
		virtual void Release();

		Potato::IR::MemoryResourceRecord resource_record;
		wchar_t const* class_type = nullptr;
	};


	struct Win32Renderer
	{
		using Ptr = Potato::Pointer::IntrusivePtr<Win32Renderer, Win32FormPointerWrapperT>;

		virtual void FormAddRef() const = 0;
		virtual void FormSubRef() const = 0;
		virtual void OnInit(HWND) = 0;
		virtual void OnRelease(HWND) = 0;
		virtual void OnUpdate() = 0;
	};

	struct Win32Setting
	{
		std::size_t size_x = 1024;
		std::size_t size_y = 768;
		std::optional<std::size_t> offset_x;
		std::optional<std::size_t> offset_y;
		wchar_t const* form_name = L"Fuck You Windows";
	};


	struct Win32FormEventChannel
	{
		using Ptr = Potato::Pointer::IntrusivePtr<Win32FormEventChannel, Win32FormPointerWrapperT>;
		virtual void FormAddRef() const = 0;
		virtual void FormSubRef() const = 0;
	};

	struct Win32Form : public Potato::Pointer::DefaultControllerViewerInterface
	{

		enum class Status
		{
			Empty,
			Opened,
			Closed,
			Hidden,
			Error,
		};

		using Ptr = Potato::Pointer::ControllerPtr<Win32Form>;

		static auto CreateWin32Form(
			Win32Style::Ptr style,
			Win32Setting const& setting,
			Win32Renderer::Ptr renderer,
			Win32FormEventChannel::Ptr event_channel,
			std::pmr::memory_resource* resource = std::pmr::get_default_resource()
		) -> Ptr;

		void CloseWindows();

		virtual ~Win32Form();
		virtual Status GetStatus() const;
		virtual HWND GetWindowHandle() const;
		virtual Win32Style const& GetStyle() const { return *style; }

	protected:

		Win32Form(
			Win32Style::Ptr style,
			Win32Renderer::Ptr renderer,
			Win32FormEventChannel::Ptr event_channel,
			Potato::IR::MemoryResourceRecord record
			);

		virtual void ControllerRelease() override;
		virtual void ViewerRelease() override;

		Potato::IR::MemoryResourceRecord resource_record;
		Win32Style::Ptr style;
		
		std::thread window_thread;
		HWND window_handle = nullptr;

		mutable std::shared_mutex mutex;
		Status status = Status::Empty;
		Win32Renderer::Ptr renderer;
		Win32FormEventChannel::Ptr event_channel;

		friend struct Win32Style;

	};

	struct ComPointerWrapper
	{
		template<typename PtrT>
		void AddRef(PtrT* ptr) { ptr->AddRef(); }
		template<typename PtrT>
		void SubRef(PtrT* ptr) { ptr->Release(); }
	};

	template<typename PtrT>
	using ComPtr = Potato::Pointer::IntrusivePtr<PtrT, ComPointerWrapper>;

}
