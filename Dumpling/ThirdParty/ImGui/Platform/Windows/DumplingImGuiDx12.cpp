
module;

#include <cassert>
#include "imgui.h"
#include "d3d12.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"
#include "wrl.h"

module DumplingImGuiDx12;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Dumpling
{

	auto ImGuiHeadUpDisplayWin32Dx12::Create(Form& form, FrameRenderer& renderer, IGWidget::Ptr top_widget, std::pmr::memory_resource* resource) -> Ptr
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
			
			Dx12DescriptorHeapPtr heap;
			auto re = renderer.GetRawDevice()->CreateDescriptorHeap(
				&desc, IID_PPV_ARGS(heap.GetAddressOf())
			);

			if(SUCCEEDED(re))
			{
				if(ImGui_ImplWin32_Init(form.GetPlatformValue()))
				{
					if(ImGui_ImplDX12_Init(
						renderer.GetRawDevice().Get(),
						2, 
						DXGI_FORMAT_R8G8B8A8_UNORM, 
						heap.Get(), 
						heap->GetCPUDescriptorHandleForHeapStart(), 
						heap->GetGPUDescriptorHandleForHeapStart()
					))
					{
						auto re = Potato::IR::MemoryResourceRecord::Allocate<ImGuiHeadUpDisplayWin32Dx12>(resource);
						if(re)
						{
							return new(re.Get()) ImGuiHeadUpDisplayWin32Dx12{re, std::move(heap), std::move(top_widget), context};
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

	ImGuiHeadUpDisplayWin32Dx12::~ImGuiHeadUpDisplayWin32Dx12()
	{
		ImGui_ImplDX12_Shutdown();
		ImGui_ImplWin32_Shutdown();
		heap.Reset();
		assert(io_context != nullptr);
		ImGui::DestroyContext(io_context);
		io_context = nullptr;
	}

	bool ImGuiHeadUpDisplayWin32Dx12::DrawTo(PassRenderer& renderer)
	{
		if (io_context != nullptr)
		{
			ImGui::SetCurrentContext(io_context);
			ImGui_ImplDX12_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();
			//ImGui::Begin("");
			if (top_widget)
			{
				top_widget->Draw(renderer);
			}
			//ImGui::End();
			ImGui::Render();
			renderer->SetDescriptorHeaps(1, heap.GetAddressOf());
			ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), renderer.GetCommandList());
			return true;
		}
		return false;
	}

	FormEvent::Respond ImGuiHeadUpDisplayWin32Dx12::FormEventHook(FormEvent& event)
	{
		if (ImGui_ImplWin32_WndProcHandler(event.hwnd, event.message, event.wParam, event.lParam))
			return event.RespondMarkAsHooked();
		return event.RespondMarkAsSkip();
	}
}
