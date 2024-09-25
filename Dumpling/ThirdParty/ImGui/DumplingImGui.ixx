
module;

#include "imgui.h"

export module DumplingImGui;

import std;

#ifdef _WIN32
export import DumplingImGuiWindows;
export import DumplingImGuiDx12;
#endif

