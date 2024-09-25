
module;

#include "imgui.h"
#include "imgui_impl_dx12.h"
#include "imgui_impl_win32.h"
#include "d3d12.h"

module DumplingImGuiDx12;

namespace Dumpling
{

	auto ImGuiContext::Create(Device& device, std::pmr::memory_resource* resource) -> Ptr
	{
		IMGUI_CHECKVERSION();
		
		//io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // IF using Docking Branch

		D3D12_DESCRIPTOR_HEAP_DESC desc{
			D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
			2,
			D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
			0
		};

		DescriptorHeapPtr heap;
		auto re = device.GetDevice()->CreateDescriptorHeap(
			&desc, IID_PPV_ARGS(heap.GetAddressOf())
		);

		if(SUCCEEDED(re))
		{
			ImGui::CreateContext();
			ImGuiIO& io = ImGui::GetIO();
			io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
			io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

			ImGui_ImplDX12_Init(
					device.GetDevice(), 
				2, 
				DXGI_FORMAT_R8G8B8A8_UNORM, 
				heap.Get(), 
				heap->GetCPUDescriptorHandleForHeapStart(), 
				heap->GetGPUDescriptorHandleForHeapStart()
			);

			auto re = Potato::IR::MemoryResourceRecord::Allocate<ImGuiContext>(resource);
			if(re)
			{
				return new(re.Get()) ImGuiContext{re, std::move(heap)};
			}else
			{
				ImGui_ImplDX12_Shutdown();
				ImGui::DestroyContext();
			}
		}
		return {};
	}

	ImGuiContext::~ImGuiContext()
	{
		ImGui_ImplDX12_Shutdown();
		ImGui::DestroyContext();
	}

	ImGuiFormWrapper::Ptr ImGuiContext::CreateFormWrapper(Form& form, std::pmr::memory_resource* resource)
	{
		auto re = Potato::IR::MemoryResourceRecord::Allocate<ImGuiFormWrapper>(resource);
		if(re)
		{
			return new(re.Get()) ImGuiFormWrapper{re, form};
		}
		return {};
	}

	void ImGuiContext::StartFrame()
	{
		ImGui_ImplDX12_NewFrame();
	}

	void ImGuiContext::Commited(PassRenderer& renderer)
	{
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), renderer.GetCommandList());
	}
}
