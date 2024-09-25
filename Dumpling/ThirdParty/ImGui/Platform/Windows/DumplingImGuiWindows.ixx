
module;

#include "imgui_impl_dx12.h"
#include "imgui_impl_win32.h"
#include "Windows.h"

export module DumplingImGuiWindows;

#undef interface

import std;
import DumplingWindowsForm;
import PotatoIR;
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

	export struct ImGuiContext;

	struct ImGuiFormWrapper : public Potato::IR::MemoryResourceRecordIntrusiveInterface
	{
		using Ptr = Potato::Pointer::IntrusivePtr<ImGuiFormWrapper>;
		void StartFrame();
		void EndFrame() {};
	protected:
		ImGuiFormWrapper(Potato::IR::MemoryResourceRecord record, Form& form);
		~ImGuiFormWrapper();

		

		Form::Ptr reference_form;

		friend struct ImGuiContext;
	};
}
