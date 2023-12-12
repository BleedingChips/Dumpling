module;

#include <Windows.h>

export module DumplingWin32Form;

import std;
import PotatoPointer;

export namespace Dumpling::Win32
{

	struct FormManager;

	struct FormInterface : public Potato::Pointer::DefaultControllerViewerInterface
	{
		virtual std::u8string_view GetFormName() const;

		using Ptr = Potato::Pointer::ControllerPtr<FormInterface>;

		void WaitUntilWindowClosed(std::chrono::microseconds check_duration_time = std::chrono::microseconds{1});

	protected:

		virtual std::optional<LRESULT> WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) { return std::nullopt; }

		void ControllerRelease() override;

		using WPtr = Potato::Pointer::ViewerPtr<FormInterface>;

		std::mutex mutex;
		Potato::Pointer::ControllerPtr<FormManager> owner;
		enum class Status
		{
			Empty,
			Error,
			Waiting,
			Ready,
			RequestExist,
			Closed,
		};

		Status status = Status::Empty;
		HWND window_handle = nullptr;
		DWORD error_code = 0;

		friend struct FormManager;
	};


	struct FormManager : public Potato::Pointer::DefaultControllerViewerInterface
	{
		using Ptr = Potato::Pointer::ControllerPtr<FormManager>;

		static Ptr Create(std::pmr::memory_resource* resource = std::pmr::get_default_resource());

		bool CreateForm(FormInterface& channel);

		static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	protected:

		using WPtr = Potato::Pointer::ViewerPtr<FormManager>;

		FormManager(std::pmr::memory_resource* resource)
			: resource(resource) {}

		~FormManager();

		virtual void ControllerRelease() override;
		virtual void ViewerRelease() override;

		void Execute();

		std::pmr::memory_resource* resource;

		std::atomic_size_t exist_form_count;
		std::thread threads;

		std::mutex form_init_list;

		struct FormRequest
		{
			FormInterface::WPtr form;
		};

		std::pmr::vector<FormRequest> requests;
	};
}
