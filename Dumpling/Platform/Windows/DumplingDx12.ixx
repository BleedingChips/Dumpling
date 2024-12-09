module;

#include "d3d12.h"
#include "dxgi1_6.h"
#include <cassert>
#include "wrl.h"

#undef interface
#undef max

export module DumplingDX12;

import std;
import Potato;
import DumplingForm;
import DumplingPipeline;
export import DumplingRendererTypes;

export namespace Dumpling
{
	using Microsoft::WRL::ComPtr;

	using Dx12DevicePtr = ComPtr<ID3D12Device>;
	using Dx12CommandQueuePtr = ComPtr<ID3D12CommandQueue>;
	using Dx12CommandAllocatorPtr = ComPtr<ID3D12CommandAllocator>;
	using Dx12CommandListPtr = ComPtr<ID3D12CommandList>;
	using Dx12GraphicCommandListPtr = ComPtr<ID3D12GraphicsCommandList>;
	using Dx12FencePtr = ComPtr<ID3D12Fence>;
	using Dx12ResourcePtr = ComPtr<ID3D12Resource>;
	using Dx12DescriptorHeapPtr = ComPtr<ID3D12DescriptorHeap>;
	using Dx12FactoryPtr = ComPtr<IDXGIFactory2>;
	using Dx12SwapChainPtr = ComPtr<IDXGISwapChain3>;
	using Dx12DescriptorHeapPtr = ComPtr<ID3D12DescriptorHeap>;
}