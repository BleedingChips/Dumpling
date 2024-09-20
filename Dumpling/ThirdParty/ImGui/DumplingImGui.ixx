
module;

#include "imgui.h"

export module DumplingImGui;

import std;

#ifdef _WIN32
export import DumplingImGuiWin32Dx12;
#endif
