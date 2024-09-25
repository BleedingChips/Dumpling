
module;

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_win32.h"
#include "Windows.h"

#undef interface

module DumplingImGuiWindows;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Dumpling
{

	HRESULT ImGuiFormEventCapture::ReceiveRaw(Form& interface, FormEvent::Category category, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
			return MarkMessageProcessed(msg);
		return MarkMessageSkip(msg);
	}

	auto ImGuiFormEventCapture::GetInstance() -> Ptr
	{
		static ImGuiFormEventCapture temp_capture;
		return &temp_capture;
	}

	ImGuiFormWrapper::ImGuiFormWrapper(Potato::IR::MemoryResourceRecord record, Form& form)
		: MemoryResourceRecordIntrusiveInterface(record), reference_form(&form)
	{
		ImGui_ImplWin32_Init(form.GetWnd());
		form.InsertCapture(ImGuiFormEventCapture::GetInstance());
	}

	ImGuiFormWrapper::~ImGuiFormWrapper()
	{
		ImGui_ImplWin32_Shutdown();
	}

	void ImGuiFormWrapper::StartFrame()
	{
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		ImGui::ShowDemoWindow();
	}
}
