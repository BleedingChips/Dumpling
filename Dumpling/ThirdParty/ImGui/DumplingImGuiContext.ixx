
module;
#include <cassert>
#include "imgui.h"
export module DumplingImGuiContext;

import std;
import PotatoPointer;

import Dumpling;

export namespace Dumpling::Gui
{
	
	export struct HeadUpDisplay;

	struct Widget
	{
		struct Wrapper
		{
			void AddRef(Widget const* ptr)  { ptr->AddWeightRef(); }
			void SubRef(Widget const* ptr)  { ptr->SubWeightRef(); }
		};
		using Ptr = Potato::Pointer::IntrusivePtr<Widget, Wrapper>;
		static Ptr GetDemoWidget();
	protected:
		virtual void AddWeightRef() const = 0;
		virtual void SubWeightRef() const = 0;
		virtual void DrawUI() = 0;

		friend struct HeadUpDisplay;
	};

	export struct HeadUpDisplay
	{
		struct Wrapper
		{
			void AddRef(HeadUpDisplay const* ptr)  { ptr->AddHeadUpDisplayRef(); }
			void SubRef(HeadUpDisplay const* ptr)  { ptr->SubHeadUpDisplayRef(); }
		};
		using Ptr = Potato::Pointer::IntrusivePtr<HeadUpDisplay, Wrapper>;
		bool Commited(Dumpling::PassRenderer& renderer);
	protected:
		HeadUpDisplay(Widget::Ptr top_widget, ImGuiContext* context) : top_widget(std::move(top_widget)), context(context) {assert(context != nullptr);}
		virtual ~HeadUpDisplay();
		virtual void AddHeadUpDisplayRef() const = 0;
		virtual void SubHeadUpDisplayRef() const = 0;
		virtual void CommitedToRenderer(Dumpling::PassRenderer& renderer) = 0;
		virtual void StartFrame() = 0;
		virtual void EndFrame() = 0;

		ImGuiContext* context = nullptr;
		Widget::Ptr top_widget;
		
	};

}


