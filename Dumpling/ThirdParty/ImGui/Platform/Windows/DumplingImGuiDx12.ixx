
module;

#include "imgui.h"
#include "wrl.h"
#include "D3D12.h"

export module DumplingImGuiDx12;

import Potato;
import DumplingForm;
import DumplingImGuiHUD;
import DumplingDX12;

export namespace Dumpling::IMGUI
{
	struct ImGuiHeadUpDisplayWin32Dx12 : public IGHeadUpDisplay, public Potato::IR::MemoryResourceRecordIntrusiveInterface
	{
		static Ptr Create(Win32::Form& form, Dx12::FrameRenderer& device, IGWidget::Ptr top_widget, std::pmr::memory_resource* resource = std::pmr::get_default_resource());
		static Win32::FormEvent::Respond FormEventHook(Win32::FormEvent& event);

	protected:

		virtual bool DrawTo(Dx12::PassRenderer& renderer) override;

		void AddIGHeadUpDisplayRef() const override { MemoryResourceRecordIntrusiveInterface::AddRef(); }
		void SubIGHeadUpDisplayRef() const override { MemoryResourceRecordIntrusiveInterface::SubRef(); }
		
		ImGuiHeadUpDisplayWin32Dx12(
			Potato::IR::MemoryResourceRecord record,
			Dx12::ComPtr<ID3D12DescriptorHeap> heap,
			IGWidget::Ptr top_widget,
			std::size_t heap_handle_increment_size,
			ImGuiContext* context
		);
		~ImGuiHeadUpDisplayWin32Dx12();

		ImGuiContext* io_context = nullptr;
		Dx12::ComPtr<ID3D12DescriptorHeap> heap;
		std::uint64_t available_heap_mark = 0;
		std::size_t heap_handle_increment_size = 0;
	};
}
