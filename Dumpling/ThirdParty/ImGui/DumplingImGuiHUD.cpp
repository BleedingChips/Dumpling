module;

module DumplingImGuiHUD;
import std;

#ifdef _WIN32
import DumplingImGuiDx12;
#endif

namespace Dumpling::IMGUI
{

	static std::mutex ImGuiContextMutex;

	bool IGHeadUpDisplay::Draw(Dx12::PassRenderer& renderer)
	{
		std::lock_guard lg(ImGuiContextMutex);
		return DrawTo(renderer);
	}

	IGHeadUpDisplay::Ptr IGHeadUpDisplay::Create(Form& form, Dx12::FrameRenderer& renderer, IGWidget::Ptr top_weight, std::pmr::memory_resource* resource)
	{
#ifdef _WIN32
		return ImGuiHeadUpDisplayWin32Dx12::Create(form, renderer, std::move(top_weight), resource);
#else
		return {};
#endif
	}

	FormEvent::Respond IGHeadUpDisplay::FormEventHook(FormEvent& event)
	{
		std::lock_guard lg(ImGuiContextMutex);
#ifdef _WIN32
		return ImGuiHeadUpDisplayWin32Dx12::FormEventHook(event);
#else
		return event.RespondMarkAsSkip();
#endif
			
	}
}
