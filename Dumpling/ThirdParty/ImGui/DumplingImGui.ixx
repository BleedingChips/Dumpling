
module;

export module DumplingImGui;

import std;
export import DumplingImGuiContext;
#ifdef _WIN32
import DumplingWindowsForm;
import DumplingDx12Renderer;
import DumplingImGuiWindows;
import DumplingImGuiDx12;
#endif


export namespace Dumpling::Gui
{
	inline HeadUpDisplay::Ptr CreateHUD(Form& form, Device& device, Widget::Ptr top_widget, std::pmr::memory_resource* resource = std::pmr::get_default_resource())
	{
#ifdef _WIN32
		return HeadUpDisplayWin32Dx12::Create(form, device, std::move(top_widget), resource);
#else
		return {};
#endif
	}
}
