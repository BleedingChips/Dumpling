
module;

#include "imgui_impl_dx12.h"


export module DumplingImGuiDx12;

import std;
import PotatoIR;
import PotatoPointer;
import DumplingWindowsForm;
import DumplingDx12Renderer;
import DumplingImGuiWindows;

export namespace Dumpling
{

	struct ImGuiContext : public Potato::IR::MemoryResourceRecordIntrusiveInterface
	{
		using Ptr = Potato::Pointer::IntrusivePtr<ImGuiContext>;
		static Ptr Create(Device& device, std::pmr::memory_resource* resource = std::pmr::get_default_resource());
		ImGuiFormWrapper::Ptr CreateFormWrapper(Form& form, std::pmr::memory_resource* resource = std::pmr::get_default_resource());
		void StartFrame();
		void EndFrame() { ImGui::Render(); }
		void Commited(PassRenderer& renderer);
		DescriptorHeapPtr::InterfaceType* GetHeap() { return heap.Get(); }
	protected:
		ImGuiContext(Potato::IR::MemoryResourceRecord record, DescriptorHeapPtr heap)
			: MemoryResourceRecordIntrusiveInterface(record), heap(std::move(heap)) {}
		~ImGuiContext();
		DescriptorHeapPtr heap;
	};
}
