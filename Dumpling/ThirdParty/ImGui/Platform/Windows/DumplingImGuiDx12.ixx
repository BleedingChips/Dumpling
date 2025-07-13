
module;

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"


export module DumplingImGuiDx12;

import std;
import DumplingImGuiContext;
import DumplingImGuiWindows;

export namespace Dumpling::Gui
{
	/*
	struct HeadUpDisplayWin32Dx12 : public HeadUpDisplay, public Potato::IR::MemoryResourceRecordIntrusiveInterface
	{
		static Ptr Create(Win32::Form& form, Dx12::Device& device, Widget::Ptr top_widget, std::pmr::memory_resource* resource = std::pmr::get_default_resource());
		virtual void StartFrame();
		virtual void EndFrame();
		virtual void CommitedToRenderer(Dx12::PassRenderer& renderer) override;
	protected:
		void AddHeadUpDisplayRef() const override { MemoryResourceRecordIntrusiveInterface::AddRef(); }
		void SubHeadUpDisplayRef() const override { MemoryResourceRecordIntrusiveInterface::SubRef(); }
		HeadUpDisplayWin32Dx12(
			Potato::IR::MemoryResourceRecord record, 
			Dx12::DescriptorHeapPtr heap,
			Widget::Ptr top_widget,
			ImGuiContext* context
		)
			: HeadUpDisplay(std::move(top_widget), context), MemoryResourceRecordIntrusiveInterface(record), heap(std::move(heap)) {}
		~HeadUpDisplayWin32Dx12();
		Dx12::DescriptorHeapPtr heap;
	};
	*/
}
