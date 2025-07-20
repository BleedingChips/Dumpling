
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
	ImGuiHeadUpDisplayWin32Dx12::ImGuiHeadUpDisplayWin32Dx12(
		Potato::IR::MemoryResourceRecord record,
		Dx12DescriptorHeapPtr heap,
		IGWidget::Ptr top_widget,
		std::size_t heap_handle_increment_size,
		ImGuiContext* context
	) : IGHeadUpDisplay(std::move(top_widget)), MemoryResourceRecordIntrusiveInterface(record),
		heap(std::move(heap)),
		heap_handle_increment_size(heap_handle_increment_size),
		io_context(context)
	{

	}
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
				64,
				D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
				0
			};
			
			Dx12DescriptorHeapPtr heap;
			auto re = renderer.GetRawDevice()->CreateDescriptorHeap(
				&desc, IID_PPV_ARGS(heap.GetAddressOf())
			);

			if(SUCCEEDED(re))
			{
				std::size_t heap_handle_increment_size = renderer.GetRawDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);


				auto re = Potato::IR::MemoryResourceRecord::Allocate<ImGuiHeadUpDisplayWin32Dx12>(resource);
				if (re)
				{
					ImGuiHeadUpDisplayWin32Dx12* ptr = new(re.Get()) ImGuiHeadUpDisplayWin32Dx12{ re, std::move(heap), std::move(top_widget), heap_handle_increment_size, context };
					
					assert(ptr != nullptr);
					Ptr real_ptr{ ptr };
					if (ImGui_ImplWin32_Init(form.GetPlatformValue()))
					{
						ImGui_ImplDX12_InitInfo init_info = {};
						init_info.Device = renderer.GetRawDevice().Get();
						init_info.CommandQueue = renderer.GetRawCommandQuery().Get();
						init_info.NumFramesInFlight = 2;
						init_info.RTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM; // Or your render target format.
						init_info.UserData = ptr;
						init_info.SrvDescriptorHeap = heap.Get();
						init_info.SrvDescriptorAllocFn = [](ImGui_ImplDX12_InitInfo* info, D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu_handle) {
							ImGuiHeadUpDisplayWin32Dx12* ptr = reinterpret_cast<ImGuiHeadUpDisplayWin32Dx12*>(info->UserData);
							std::uint64_t iterator = 1;
							for (std::size_t i = 0; i < 64; ++i)
							{
								if ((ptr->available_heap_mark & iterator) == 0)
								{
									ptr->available_heap_mark |= iterator;
									out_cpu_handle->ptr = ptr->heap->GetCPUDescriptorHandleForHeapStart().ptr + ptr->heap_handle_increment_size * i;
									out_gpu_handle->ptr = ptr->heap->GetGPUDescriptorHandleForHeapStart().ptr + ptr->heap_handle_increment_size * i;
									return;
								}
								else {
									iterator = (iterator << 1);
								}
							}
							
							*out_cpu_handle = ptr->heap->GetCPUDescriptorHandleForHeapStart();
							*out_gpu_handle = ptr->heap->GetGPUDescriptorHandleForHeapStart();
						};
						init_info.SrvDescriptorFreeFn = [](ImGui_ImplDX12_InitInfo* info, D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle) {
							ImGuiHeadUpDisplayWin32Dx12* ptr = reinterpret_cast<ImGuiHeadUpDisplayWin32Dx12*>(info->UserData);
							assert((cpu_handle.ptr - ptr->heap->GetCPUDescriptorHandleForHeapStart().ptr) % ptr->heap_handle_increment_size == 0);
							assert((gpu_handle.ptr - ptr->heap->GetGPUDescriptorHandleForHeapStart().ptr) % ptr->heap_handle_increment_size == 0);
							std::size_t index = (cpu_handle.ptr - ptr->heap->GetCPUDescriptorHandleForHeapStart().ptr) / ptr->heap_handle_increment_size;
							std::uint64_t mask = (static_cast<std::uint64_t>(1) << index);
							assert((ptr->available_heap_mark & mask) != 0);
							ptr->available_heap_mark &= ~mask;
						};

						if (ImGui_ImplDX12_Init(&init_info))
						{
							return real_ptr;
						}
					}
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
