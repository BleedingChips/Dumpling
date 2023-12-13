module;

#include <Windows.h>

export module DumplingWin32Form;

import std;
import PotatoPointer;
import DumplingForm;

export namespace Dumpling::Win32
{

	struct Win32Form : public Form
	{
		virtual void CloseWindows() override;

		static Form::Ptr CreateWin32Form(FormSetting setting, std::pmr::memory_resource* resource = std::pmr::get_default_resource());
		virtual ~Win32Form();
		virtual Status GetStatus() const override;

		static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	protected:

		virtual void ControllerRelease() override;
		virtual void ViewerRelease() override;

		mutable std::shared_mutex mutex;
		std::thread window_thread;
		HWND window_handle = nullptr;
		Status status = Status::Empty;
		std::optional<LRESULT> SelfHandleProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) { return std::nullopt; }

	private:

		
	};

}
