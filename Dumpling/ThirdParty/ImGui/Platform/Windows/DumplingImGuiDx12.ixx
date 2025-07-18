
module;

#include "imgui.h"
#include "wrl.h"

export module DumplingImGuiDx12;

import Potato;
import DumplingFormEvent;
import DumplingImGuiHUD;
import DumplingDX12;

export namespace Dumpling
{
	struct ImGuiHeadUpDisplayWin32Dx12 : public IGHeadUpDisplay, public Potato::IR::MemoryResourceRecordIntrusiveInterface
	{
		static Ptr Create(Form& form, FrameRenderer& device, IGWidget::Ptr top_widget, std::pmr::memory_resource* resource = std::pmr::get_default_resource());
		static FormEvent::Respond FormEventHook(FormEvent& event);

	protected:

		virtual bool DrawTo(PassRenderer& renderer) override;

		void AddIGHeadUpDisplayRef() const override { MemoryResourceRecordIntrusiveInterface::AddRef(); }
		void SubIGHeadUpDisplayRef() const override { MemoryResourceRecordIntrusiveInterface::SubRef(); }
		
		ImGuiHeadUpDisplayWin32Dx12(
			Potato::IR::MemoryResourceRecord record,
			Dx12DescriptorHeapPtr heap,
			IGWidget::Ptr top_widget,
			std::size_t heap_handle_increment_size,
			ImGuiContext* context
		);
		~ImGuiHeadUpDisplayWin32Dx12();

		ImGuiContext* io_context = nullptr;
		Dx12DescriptorHeapPtr heap;
		std::uint64_t available_heap_mark = 0;
		std::size_t heap_handle_increment_size = 0;
	};
}
