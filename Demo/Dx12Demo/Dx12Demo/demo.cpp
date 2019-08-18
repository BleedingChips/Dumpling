#include "..//..//..//Dumpling/Gui/Dxgi/define_dxgi.h"
#include "..//..//..//Dumpling/Gui/Dx12/define_dx12.h"
#include "..//../..//Dumpling/Gui/Win32/form.h"
#include <assert.h>
#include <iostream>
#include <chrono>
#include <thread>
using namespace Dumpling;
using Dxgi::PixFormat;
int main()
{
#ifdef _DEBUG
	Dx12::InitDebugLayout();
#endif

	auto [Factory, re_f] = Dxgi::CreateFactory();
	auto AllAdapters = Dxgi::EnumAdapter(Factory);


	auto [Device, re_d] = Dx12::CreateDevice(AllAdapters[0], D3D_FEATURE_LEVEL_12_0);
	auto [Queue, re_c] = Dx12::CreateCommmandQueue(Device, D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT);
	Queue->SetName(L"WTF");
	auto [Allocator, re_a] = Dx12::CreateCommandAllocator(Device, D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT);
	Win32::Form form = Win32::Form::create();

	auto swap_chain_desc = Dxgi::CreateDefaultSwapChainDesc(cast(Dxgi::PixFormat::RGBA16_Float), 1024, 768);
	auto [SwapChain, re_s] = Dx12::CreateSwapChain(Factory, Queue, form, swap_chain_desc);
	auto [RTDescHead, re_rt1] = Dx12::CreateDescriptorHeap(Device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 1);
	auto [DTDescHead, re_rt2] = Dx12::CreateDescriptorHeap(Device, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1);

	auto DescriptorSize = Dx12::GetDescriptorHandleIncrementSize(Device);

	
	auto [DTResource, red] = Dx12::CreateDepthStencil2D(Device, cast(PixFormat::D24S8_Unorn_Uint), 1024, 768, 0);
	Dx12::CreateDepthStencilView2D(Device, DTResource, DescriptorSize.offset_DSV(DTDescHead, 0));

	auto viewport = Dx12::CreateFullScreenViewport(1024, 768);

	auto [Fence, ref] = Dx12::CreateFence(Device, 0);

	bool exit = false;
	size_t current_buffer = 0;
	while (!exit)
	{

		auto [BBResource, re1] = Dx12::GetBuffer(SwapChain, current_buffer);
		BBResource->SetName(L"sdadasd");
		Dx12::CreateRenderTargetView2D(Device, BBResource, DescriptorSize.offset_RTV(RTDescHead, 0));
		
		auto [CommandList, re_l] = Dx12::CreateGraphicCommandList(Device, Allocator, D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT);

		auto RTHandle = RTDescHead->GetCPUDescriptorHandleForHeapStart();
		auto DTHandle = DTDescHead->GetCPUDescriptorHandleForHeapStart();

		FLOAT Color[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
		
		Dx12::ResourceBarrier tem[2] = {
			Dx12::CreateResourceBarrierTransition(Device, BBResource, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET),
			Dx12::CreateResourceBarrierTransition(Device, DTResource, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_DEPTH_WRITE)
		};

		CommandList->ResourceBarrier(1, tem);
		CommandList->ClearRenderTargetView(RTDescHead->GetCPUDescriptorHandleForHeapStart(), Color, 0, nullptr);
		CommandList->ClearDepthStencilView(DTDescHead->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0, 0, 0, nullptr);
		CommandList->OMSetRenderTargets(1, &RTHandle, false, &DTHandle);
		Dx12::SwapResourceBarrierTransitionState(2, tem);
		CommandList->ResourceBarrier(1, tem);
		CommandList->Close();
		Dx12::CommandList* const CommandlListArray = CommandList;
		Queue->ExecuteCommandLists(1, &CommandlListArray);
		SwapChain->Present(1, 0);
		Queue->Signal(Fence, 1);

		while (Fence->GetCompletedValue() != 1)
		{
			std::cout << Fence->GetCompletedValue() << std::endl;
			std::this_thread::sleep_for(std::chrono::milliseconds{ 10 });
		}
			
		
		current_buffer += 1;
		current_buffer = current_buffer % 2;
		Allocator->Reset();
		CommandList->Reset(Allocator, nullptr);
		Fence->Signal(0);
		std::cout << "down" << std::endl;
		MSG msg;
		break;
		while (form.pook_event(msg))
		{
			if (msg.message == WM_CLOSE)
			{
				exit = true;
				break;
			}
		}
	}
	return 0;
}