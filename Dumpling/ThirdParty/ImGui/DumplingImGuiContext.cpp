
module;

#include "imgui.h"

module DumplingImGuiContext;


namespace Dumpling::Gui
{
	struct DemoWidgetT : public Widget
	{
		void AddWeightRef() const override {}
		void SubWeightRef() const override {}
		void DrawUI() override { ImGui::ShowDemoWindow(); }
	};

	auto Widget::DemoWidget() ->Ptr
	{
		static DemoWidgetT widget;
		return &widget;
	}

	static std::mutex ImGuiContextMutex;

	bool HeadUpDisplay::Commited(PassRenderer& renderer)
	{
		std::lock_guard lg(ImGuiContextMutex);
		StartFrame();
		if(top_widget)
		{
			top_widget->DrawUI();
		}
		EndFrame();
		CommitedToRenderer(renderer);
		return true;
	}

	HeadUpDisplay::~HeadUpDisplay()
	{
		assert(context != nullptr);
		ImGui::DestroyContext(context);
	}
}
