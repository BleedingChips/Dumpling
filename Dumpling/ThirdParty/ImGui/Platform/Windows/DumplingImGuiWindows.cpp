
module;

#include "imgui.h"

module DumplingImGuiWindows;

namespace Dumpling
{
	HRESULT ImGuiFormEventCapture::ReceiveRaw(Form& interface, FormEvent::Category category, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override
	{
		return MarkMessageSkip(msg);
	}
}
