module;

#include <Windows.h>

export module DumplingWin32Form;

import std;
import PotatoPointer;

export namespace Dumpling::Win32
{

	struct FormManager;

	struct FormChannel : public Potato::Pointer::DefaultControllerViewerInterface
	{
		virtual std::u8string_view GetFormName() const;

		using Ptr = Potato::Pointer::ControllerPtr<FormChannel>;

	protected:

		virtual std::optional<LRESULT> WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) { return std::nullopt; }

		void ControllerRelease() override;

		using WPtr = Potato::Pointer::ViewerPtr<FormChannel>;

		std::mutex mutex;
		Potato::Pointer::ControllerPtr<FormManager> owner;
		HWND window_handle = nullptr;
		std::optional<DWORD> error_code;

		friend struct FormManager;
	};


	struct FormManager : public Potato::Pointer::DefaultControllerViewerInterface
	{
		using Ptr = Potato::Pointer::ControllerPtr<FormManager>;

		static Ptr Create(std::pmr::memory_resource* resource = std::pmr::get_default_resource());

		bool CreateForm(FormChannel& channel);

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
			FormChannel::WPtr form_channel_interface;
		};

		std::pmr::vector<FormRequest> requests;
	};
}
