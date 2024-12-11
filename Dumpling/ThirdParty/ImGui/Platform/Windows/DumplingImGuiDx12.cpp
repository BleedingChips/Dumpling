
module;

#include "imgui.h"
#include "imgui_impl_dx12.h"
#include "imgui_impl_win32.h"
#include "d3d12.h"

module DumplingImGuiDx12;

namespace Dumpling::Gui
{
	/*
	auto HeadUpDisplayWin32Dx12::Create(Win32::Form& form, Dx12::Device& device, Widget::Ptr top_widget, std::pmr::memory_resource* resource) -> Ptr
	{
		IMGUI_CHECKVERSION();

		auto context = ImGui::CreateContext();
		if(context != nullptr)
		{
			ImGuiIO& io = ImGui::GetIO();
			io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
			io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

			D3D12_DESCRIPTOR_HEAP_DESC desc{
				D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
				2,
				D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
				0
			};
			
			Dx12::DescriptorHeapPtr heap;
			auto re = device.GetDevice()->CreateDescriptorHeap(
				&desc, IID_PPV_ARGS(heap.GetAddressOf())
			);

			if(SUCCEEDED(re))
			{
				if(ImGui_ImplWin32_Init(form.GetWnd()))
				{
					if(ImGui_ImplDX12_Init(
						device.GetDevice(), 
						2, 
						DXGI_FORMAT_R8G8B8A8_UNORM, 
						heap.Get(), 
						heap->GetCPUDescriptorHandleForHeapStart(), 
						heap->GetGPUDescriptorHandleForHeapStart()
					))
					{
						auto re = Potato::IR::MemoryResourceRecord::Allocate<HeadUpDisplayWin32Dx12>(resource);
						if(re)
						{
							form.InsertCapture(ImGuiFormEventCapture::GetInstance());
							return new(re.Get()) HeadUpDisplayWin32Dx12{re, std::move(heap), std::move(top_widget), context};
						}
						ImGui_ImplDX12_Shutdown();
					}

					ImGui_ImplWin32_Shutdown();
				}
			}
			ImGui::DestroyContext(context);
			return {};
		}
		return {};
	}

	HeadUpDisplayWin32Dx12::~HeadUpDisplayWin32Dx12()
	{
		ImGui_ImplDX12_Shutdown();
		ImGui_ImplWin32_Shutdown();
		heap.Reset();
	}

	void HeadUpDisplayWin32Dx12::StartFrame()
	{
		ImGui::SetCurrentContext(context);
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
	}

	void HeadUpDisplayWin32Dx12::CommitedToRenderer(Dx12::PassRenderer& renderer)
	{
		renderer->SetDescriptorHeaps(1, heap.GetAddressOf());
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), renderer.GetCommandList());
	}

	void HeadUpDisplayWin32Dx12::EndFrame()
	{
		ImGui::Render();
	}
	*/
}
