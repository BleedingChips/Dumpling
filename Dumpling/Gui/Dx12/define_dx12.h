#pragma once
#include <d3d12.h>
#include "../Dxgi/define_dxgi.h"
#include <assert.h>
#pragma comment(lib, "d3d12.lib")
namespace Dumpling::Dx12
{

	using Dxgi::ComPtr;
	using Dxgi::void_t;

	void InitDebugLayout();


	using Device = ID3D12Device;
	using DevicePtr = ComPtr<Device>;

	std::tuple<DevicePtr, HRESULT> CreateDevice(Dxgi::Adapter* adapter = nullptr, D3D_FEATURE_LEVEL level = D3D_FEATURE_LEVEL_12_0);

	using Fence = ID3D12Fence1;
	using FencePtr = ComPtr<Fence>;

	std::tuple<FencePtr, HRESULT> CreateFence(Device* ptr, uint64_t initial_value = 0, D3D12_FENCE_FLAGS flag = D3D12_FENCE_FLAG_NONE);

	using DescriptorHeap = ID3D12DescriptorHeap;
	using DescriptorHeapPtr = ComPtr<DescriptorHeap>;

	std::tuple<DescriptorHeapPtr, HRESULT> CreateDescriptorHeap(Device* d, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t count, D3D12_DESCRIPTOR_HEAP_FLAGS flag = D3D12_DESCRIPTOR_HEAP_FLAG_NONE, uint32_t node_mask = 0);

	struct DescriptorHandleIncrementSize
	{
		uint32_t CBV_SRV_UAV;
		uint32_t Sampler;
		uint32_t RTV;
		uint32_t DSV;
		D3D12_CPU_DESCRIPTOR_HANDLE offset_CBV_SRV_UAV(DescriptorHeap* start, uint32_t index) const noexcept {
			assert(start != nullptr);
			return offset_CBV_SRV_UAV(start->GetCPUDescriptorHandleForHeapStart(), index);
		}
		D3D12_CPU_DESCRIPTOR_HANDLE offset_Sampler(DescriptorHeap* start, uint32_t index) const noexcept {
			assert(start != nullptr);
			return offset_Sampler(start->GetCPUDescriptorHandleForHeapStart(), index);
		}
		D3D12_CPU_DESCRIPTOR_HANDLE offset_RTV(DescriptorHeap* start, uint32_t index) const noexcept {
			assert(start != nullptr);
			return offset_RTV(start->GetCPUDescriptorHandleForHeapStart(), index);
		}
		D3D12_CPU_DESCRIPTOR_HANDLE offset_DSV(DescriptorHeap* start, uint32_t index) const noexcept {
			assert(start != nullptr);
			return offset_DSV(start->GetCPUDescriptorHandleForHeapStart(), index);
		}
		D3D12_CPU_DESCRIPTOR_HANDLE offset_CBV_SRV_UAV(D3D12_CPU_DESCRIPTOR_HANDLE start, uint32_t index) const noexcept {
			return { start.ptr + CBV_SRV_UAV * index };
		}
		D3D12_CPU_DESCRIPTOR_HANDLE offset_Sampler(D3D12_CPU_DESCRIPTOR_HANDLE start, uint32_t index) const noexcept {
			return { start.ptr + Sampler * index };
		}
		D3D12_CPU_DESCRIPTOR_HANDLE offset_RTV(D3D12_CPU_DESCRIPTOR_HANDLE start, uint32_t index) const noexcept {
			return { start.ptr + RTV * index };
		}
		D3D12_CPU_DESCRIPTOR_HANDLE offset_DSV(D3D12_CPU_DESCRIPTOR_HANDLE start, uint32_t index) const noexcept {
			return { start.ptr + DSV * index };
		}
	};

	DescriptorHandleIncrementSize GetDescriptorHandleIncrementSize(Device* ptr);

	using CommandQueue = ID3D12CommandQueue;
	using CommandQueuePtr = ComPtr<CommandQueue>;
	using CommandAllocator = ID3D12CommandAllocator;
	using CommandAllocatorPtr = ComPtr<CommandAllocator>;
	using GraphicCommandList = ID3D12GraphicsCommandList;
	using GraphicCommandListPtr = ComPtr<GraphicCommandList>;
	using CommandList = ID3D12CommandList;

	std::tuple<CommandQueuePtr, HRESULT> CreateCommmandQueue(Device* d, 
		D3D12_COMMAND_LIST_TYPE type, D3D12_COMMAND_QUEUE_FLAGS flag = D3D12_COMMAND_QUEUE_FLAG_NONE, 
		D3D12_COMMAND_QUEUE_PRIORITY priority = D3D12_COMMAND_QUEUE_PRIORITY::D3D12_COMMAND_QUEUE_PRIORITY_NORMAL, uint32_t node_mask = 0
	);

	std::tuple<CommandAllocatorPtr, HRESULT> CreateCommandAllocator(Device* d, D3D12_COMMAND_LIST_TYPE type);

	std::tuple<GraphicCommandListPtr, HRESULT> CreateGraphicCommandList(Device* d, CommandAllocator* allocator, D3D12_COMMAND_LIST_TYPE type, const ID3D12PipelineState* pipeline_state = nullptr, uint32_t node_mask = 0);

	inline std::tuple<Dxgi::SwapChainPtr, HRESULT> CreateSwapChain(Dxgi::Factory* factory, CommandQueue* device, HWND hwnd, const Dxgi::SwapChainDesc& desc, const DXGI_SWAP_CHAIN_FULLSCREEN_DESC* fullscreen_desc = nullptr, IDXGIOutput* output = nullptr)
	{
		return Dxgi::CreateSwapChain(factory, device, hwnd, desc, fullscreen_desc, output);
	}

	D3D12_VIEWPORT CreateFullScreenViewport(float width, float height) noexcept;

	using Resource = ID3D12Resource;
	using ResourcePtr = ComPtr<Resource>;

	std::tuple<ResourcePtr, HRESULT> GetBuffer(Dxgi::SwapChain* swap_chain, uint32_t count);

	std::tuple<ResourcePtr, HRESULT> CreateDepthStencil2D(Device* device, DXGI_FORMAT format, uint32_t width, uint32_t height, uint16_t mipmap = 0, float Depth_value = 1.0, uint8_t stencil_value = 0, uint32_t node_mask = 0, uint32_t visible_node_mask = 0);
	void CreateRenderTargetView2D(Device* device, Resource* resource, D3D12_CPU_DESCRIPTOR_HANDLE handle, uint32_t mipmap = 0, DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN, uint32_t plane_slice = 0);
	void CreateDepthStencilView2D(Device* device, Resource* resource, D3D12_CPU_DESCRIPTOR_HANDLE handle, uint32_t mipmap = 0, DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN, D3D12_DSV_FLAGS flag = D3D12_DSV_FLAG_NONE);
	std::tuple<ResourcePtr, HRESULT> CreateUploadBuffer(Device* device, uint32_t buffer_count);


	using ResourceBarrier = D3D12_RESOURCE_BARRIER;
	ResourceBarrier CreateResourceBarrierTransition(Device* device, Resource* resource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after, uint32_t subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
	void SwapResourceBarrierTransitionState(size_t index, ResourceBarrier* output);

	template<typename SourceType, typename ParameterType>
	struct ElementVertex
	{
		static constexpr size_t type_size = sizeof(ParameterType);
		ParameterType SourceType::* member_shift;
		const char* semantic;
		uint32_t semantic_index ;
	};

	template<typename SourceType, typename ParameterType>
	ElementVertex(ParameterType SourceType::*, const char*, uint32_t)
		->ElementVertex<SourceType, ParameterType>;

	template<typename SourceType, typename ParameterType>
	struct ElementInstance
	{
		static constexpr size_t type_size = sizeof(ParameterType);
		ParameterType SourceType::* member_shift;
		const char* semantic;
		uint32_t semantic_index;
		uint32_t instance_repeat_count;
	};

	template<typename SourceType, typename ParameterType>
	ElementInstance(ParameterType SourceType::*, const char*, uint32_t, uint32_t)
		->ElementInstance<SourceType, ParameterType>;

	


}