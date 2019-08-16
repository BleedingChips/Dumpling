#include "define_dx12.h"
#include <assert.h>
namespace Dumpling::Dx12
{
	std::tuple<DevicePtr, HRESULT> CreateDevice(Dxgi::Adapter* adapter, D3D_FEATURE_LEVEL level)
	{
#ifdef _DEBUG
		{
			ComPtr<ID3D12Debug>	debugController;
			HRESULT re = D3D12GetDebugInterface(__uuidof(ID3D12Debug), debugController(void_t{}));
			assert(SUCCEEDED(re));
			debugController->EnableDebugLayer();
		}
#endif
		DevicePtr tem;
		HRESULT re = D3D12CreateDevice(adapter, level, __uuidof(Device), tem(void_t{}));
		return {std::move(tem), re};
	}

	std::tuple<FencePtr, HRESULT> CreateFence(Device* ptr, uint64_t initial_value, D3D12_FENCE_FLAGS flag)
	{
		assert(ptr != nullptr);
		FencePtr tem;
		HRESULT re = ptr->CreateFence(initial_value, flag, __uuidof(Fence), tem(void_t{}));
		return { std::move(tem), re };
	}

	DescriptorHandleIncrementSize GetDescriptorHandleIncrementSize(Device* ptr)
	{
		assert(ptr != nullptr);
		return {
			ptr->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV),
			ptr->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER),
			ptr->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV),
			ptr->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV)
		};
	}

	std::tuple<CommandQueuePtr, HRESULT> CreateCommmandQueue(Device* d,
		D3D12_COMMAND_LIST_TYPE type, D3D12_COMMAND_QUEUE_FLAGS flag,
		D3D12_COMMAND_QUEUE_PRIORITY priority, uint32_t node_mask
	)
	{
		assert(d != nullptr);
		CommandQueuePtr result;
		D3D12_COMMAND_QUEUE_DESC desc{ type, priority, flag,node_mask };
		HRESULT re = d->CreateCommandQueue(&desc, __uuidof(CommandQueue), result(void_t{}));
		return { std::move(result), re };
	}

	std::tuple<CommandAllocatorPtr, HRESULT> CreateCommandAllocator(Device* d, D3D12_COMMAND_LIST_TYPE type)
	{
		assert(d != nullptr);
		CommandAllocatorPtr tem;
		HRESULT re = d->CreateCommandAllocator(type, __uuidof(CommandAllocator), tem(void_t{}));
		return {std::move(tem), re};
	}

	std::tuple<GraphicCommandListPtr, HRESULT> CreateGraphicCommandList(Device* d, CommandAllocator* allocator, D3D12_COMMAND_LIST_TYPE type, const ID3D12PipelineState* pipeline_state, uint32_t node_mask)
	{
		assert(d != nullptr && allocator != nullptr);
		GraphicCommandListPtr tem;
		HRESULT re = d->CreateCommandList(node_mask, type, allocator, nullptr, __uuidof(GraphicCommandList), tem(void_t{}));
		return {std::move(tem), re};
	}

	std::tuple<DescriptorHeapPtr, HRESULT> CreateDescriptorHeap(Device* d, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t count, D3D12_DESCRIPTOR_HEAP_FLAGS flag, uint32_t node_mask)
	{
		assert(d != nullptr);
		D3D12_DESCRIPTOR_HEAP_DESC desc{ type , count, flag, node_mask};
		DescriptorHeapPtr tem;
		HRESULT re = d->CreateDescriptorHeap(&desc, __uuidof(DescriptorHeap), tem(void_t{}));
		return { std::move(tem), re };
	}

	D3D12_VIEWPORT CreateFullScreenViewport(float width, float height) noexcept
	{
		return D3D12_VIEWPORT{0.0, 0.0, width, height, 0.0, 1.0};
	}

	std::tuple<ResourcePtr, HRESULT> GetBuffer(Dxgi::SwapChain* swap_chain, uint32_t count)
	{
		assert(swap_chain != nullptr);
		ResourcePtr tem;
		HRESULT re = swap_chain->GetBuffer(count, __uuidof(Resource), tem(void_t{}));
		return {std::move(tem), re};
	}

	std::tuple<ResourcePtr, HRESULT> CreateDepthStencil2D(Device* device, DXGI_FORMAT format, uint32_t width, uint32_t height, uint16_t mipmap, float Depth_value, uint8_t stencil_value, uint32_t node_mask, uint32_t visible_node_mask)
	{
		D3D12_RESOURCE_DESC desc{ D3D12_RESOURCE_DIMENSION_TEXTURE2D, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT, 
			width, height, 1, mipmap, format, DXGI_SAMPLE_DESC {1, 0},  D3D12_TEXTURE_LAYOUT_UNKNOWN , 
			D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL 
		};
		D3D12_HEAP_PROPERTIES heap_properties{
			D3D12_HEAP_TYPE_DEFAULT,
			D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
			D3D12_MEMORY_POOL_UNKNOWN,
			node_mask,
			visible_node_mask
		};
		D3D12_CLEAR_VALUE value;
		value.Format = format;
		value.DepthStencil.Depth = Depth_value;
		value.DepthStencil.Stencil = stencil_value;
		ResourcePtr tem;
		HRESULT re = device->CreateCommittedResource(&heap_properties, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &value, __uuidof(Resource), tem(void_t{}));
		return {std::move(tem), re};
	}

	void CreateRenderTargetView2D(Device* device, Resource* resource, D3D12_CPU_DESCRIPTOR_HANDLE handle, uint32_t mipmap, DXGI_FORMAT format, uint32_t plane_slice)
	{
		assert(device != nullptr);
		assert(resource != nullptr);
		D3D12_RENDER_TARGET_VIEW_DESC desc;
		if (format == DXGI_FORMAT_UNKNOWN)
		{
			auto res_desc = resource->GetDesc();
			desc.Format = res_desc.Format;
		}
		else
			desc.Format = format;
		desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		desc.Texture2D = { mipmap , plane_slice };
		device->CreateRenderTargetView(resource, &desc, handle);
	}

	void CreateDepthStencilView2D(Device* device, Resource* resource, D3D12_CPU_DESCRIPTOR_HANDLE handle, uint32_t mipmap, DXGI_FORMAT format, D3D12_DSV_FLAGS flag)
	{
		assert(device != nullptr);
		assert(resource != nullptr);
		D3D12_DEPTH_STENCIL_VIEW_DESC desc;
		if (format == DXGI_FORMAT_UNKNOWN)
		{
			auto res_desc = resource->GetDesc();
			desc.Format = res_desc.Format;
		}
		else
			desc.Format = format;
		desc.Flags = flag;
		desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		desc.Texture2D = { mipmap };
		device->CreateDepthStencilView(resource, &desc, handle);
	}

	D3D12_RESOURCE_BARRIER CreateResourceBarrierTransition(Device* device, Resource* resource, D3D12_RESOURCE_STATES  before, D3D12_RESOURCE_STATES after,uint32_t subresource)
	{
		return { D3D12_RESOURCE_BARRIER_TYPE_TRANSITION, D3D12_RESOURCE_BARRIER_FLAG_NONE ,  D3D12_RESOURCE_TRANSITION_BARRIER {resource, subresource, before, after} };
	}

	void SwapCreateResourceBarrierTransitionState(size_t index, ResourceBarrier* output)
	{
		for (size_t i = 0; i < index; ++i)
		{
			auto& output_ref = output[i];
			std::swap(output_ref.Transition.StateBefore, output_ref.Transition.StateAfter);
		}
	}
}