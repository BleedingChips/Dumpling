
module;

#include "imgui_impl_win32.h"
#include "Windows.h"

export module DumplingImGuiWindows;

#undef interface

import std;
import DumplingWindowsForm;
import PotatoPointer;


export namespace Dumpling
{
	struct ImGuiFormEventCapture : public FormEventCapture
	{
		static Ptr GetInstance();

	protected:

		HRESULT ReceiveRaw(Form& interface, FormEvent::Category category, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override;

		ImGuiFormEventCapture() : FormEventCapture(FormEvent::Category::RAW) {}

		void AddFormEventCaptureRef() const override {}
		void SubFormEventCaptureRef() const override {}
	};
}
