
module;

#include "imgui_impl_dx12.h"
#include "imgui_impl_win32.h"

export module DumplingImGuiWin32Dx12;

import std;
import DumplingDx12Renderer;

export namespace Dumpling
{
	void InitImGui(Device& decive);
	//void InitImGui();
}
