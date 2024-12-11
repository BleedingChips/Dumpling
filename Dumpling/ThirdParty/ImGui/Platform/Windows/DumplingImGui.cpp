
module;

#include "imgui.h"
#include "Windows.h"

#undef interface

module DumplingImGuiWindows;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Dumpling
{
	/*
	HRESULT ImGuiFormEventCapture::ReceiveRaw(Win32::Form& interface, FormEvent::Category category, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
			return Win32::MarkMessageProcessed(msg);
		return Win32::MarkMessageSkip(msg);
	}

	auto ImGuiFormEventCapture::GetInstance() -> Ptr
	{
		static ImGuiFormEventCapture temp_capture;
		return &temp_capture;
	}
	*/
}
