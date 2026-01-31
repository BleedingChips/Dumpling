module;

export module DumplingImGuiHUD;

import std;
import Potato;
import DumplingForm;
import DumplingRenderer;

export namespace Dumpling::IMGUI
{
	struct IGHeadUpDisplay;

	struct IGWidget
	{
		struct Wrapper
		{
			void AddRef(IGWidget const* ptr) { ptr->AddIGWidgetRef(); }
			void SubRef(IGWidget const* ptr) { ptr->SubIGWidgetRef(); }
		};
		using Ptr = Potato::Pointer::IntrusivePtr<IGWidget, Wrapper>;

		virtual void Draw(Dx12::PassRenderer& render) = 0;

	protected:

		virtual void AddIGWidgetRef() const = 0;
		virtual void SubIGWidgetRef() const = 0;

		friend struct IGHeadUpDisplay;
	};

	struct IGHeadUpDisplay
	{
		struct Wrapper
		{
			void AddRef(IGHeadUpDisplay const* ptr) { ptr->AddIGHeadUpDisplayRef(); }
			void SubRef(IGHeadUpDisplay const* ptr) { ptr->SubIGHeadUpDisplayRef(); }
		};
		using Ptr = Potato::Pointer::IntrusivePtr<IGHeadUpDisplay, Wrapper>;

		bool Draw(Dx12::PassRenderer& renderer);
		static FormEvent::Respond FormEventHook(FormEvent& event);

		static Ptr Create(Win32::Form& form, Dx12::FrameRenderer& renderer, IGWidget::Ptr top_weight, std::pmr::memory_resource* resource = std::pmr::get_default_resource());

	protected:

		IGHeadUpDisplay(IGWidget::Ptr top_widget) : top_widget(std::move(top_widget)) { }
		virtual bool DrawTo(Dx12::PassRenderer& renderer) = 0;
		virtual ~IGHeadUpDisplay() = default;
		virtual void AddIGHeadUpDisplayRef() const = 0;
		virtual void SubIGHeadUpDisplayRef() const = 0;

		IGWidget::Ptr top_widget;
	};
}
