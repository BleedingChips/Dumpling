module;

#include <Windows.h>
#include <wrl.h>

export module DumplingWin32Form;

import std;
import PotatoPointer;
import PotatoIR;


export namespace Dumpling::Win32
{

	struct FormPointerWrapperT
	{
		template<typename PtrT>
		void AddRef(PtrT* ptr) const { return ptr->FormAddRef(); }
		template<typename PtrT>
		void SubRef(PtrT* ptr) const { return ptr->FormSubRef(); }
	};

	struct Style : public Potato::Pointer::DefaultIntrusiveInterface
	{

		using Ptr = Potato::Pointer::IntrusivePtr<Style>;

		static Ptr Create(wchar_t const* class_name, std::pmr::memory_resource* resource = std::pmr::get_default_resource());

		wchar_t const* GetStyleName() const { return class_type; }

	protected:

		static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

		Style(Potato::IR::MemoryResourceRecord record, std::byte* offset, std::wstring_view class_type);
		~Style();
		virtual void Release();

		Potato::IR::MemoryResourceRecord resource_record;
		wchar_t const* class_type = nullptr;
	};


	struct Renderer
	{
		using Ptr = Potato::Pointer::IntrusivePtr<Renderer, FormPointerWrapperT>;

		virtual void FormAddRef() const = 0;
		virtual void FormSubRef() const = 0;
		virtual void OnInit(HWND) = 0;
		virtual void OnRelease(HWND) = 0;
		virtual void OnUpdate() = 0;
	};

	struct Setting
	{
		std::size_t size_x = 1024;
		std::size_t size_y = 768;
		std::optional<std::size_t> offset_x;
		std::optional<std::size_t> offset_y;
		wchar_t const* form_name = L"Fuck You Windows";
	};


	struct FormEventChannel
	{
		using Ptr = Potato::Pointer::IntrusivePtr<FormEventChannel, FormPointerWrapperT>;
		virtual void FormAddRef() const = 0;
		virtual void FormSubRef() const = 0;
	};

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
			Renderer::Ptr renderer,
			FormEventChannel::Ptr event_channel,
			std::pmr::memory_resource* resource = std::pmr::get_default_resource()
		) -> Ptr;

		void CloseWindows();

		virtual ~Form();
		virtual Status GetStatus() const;
		virtual HWND GetWindowHandle() const;
		virtual Style const& GetStyle() const { return *style; }

	protected:

		Form(
			Style::Ptr style,
			Renderer::Ptr renderer,
			FormEventChannel::Ptr event_channel,
			Potato::IR::MemoryResourceRecord record
			);

		virtual void ControllerRelease() override;
		virtual void ViewerRelease() override;

		Potato::IR::MemoryResourceRecord resource_record;
		Style::Ptr style;
		
		std::thread window_thread;
		HWND window_handle = nullptr;

		mutable std::shared_mutex mutex;
		Status status = Status::Empty;
		Renderer::Ptr renderer;
		FormEventChannel::Ptr event_channel;

		friend struct Style;

	};

	template<typename PtrT>
	using ComPtr = Microsoft::WRL::ComPtr<PtrT>;

}
