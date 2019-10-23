#include "define_dx12.h"
#include <assert.h>
#include <d3dcompiler.h>
#include "..//Dxgi/define_dxgi.h"
#include "descriptor_table_dx12.h"
#undef max
namespace Dumpling::Dx12
{

	void InitDebugLayout()
	{
		ComPtr<ID3D12Debug>	debugController;
		HRESULT re = D3D12GetDebugInterface(__uuidof(ID3D12Debug), debugController(VoidT{}));
		assert(SUCCEEDED(re));
		debugController->EnableDebugLayer();
	}
	
	Context::Context(uint8_t AdapterIndex, D3D_FEATURE_LEVEL Level) : m_AdapterIndex(AdapterIndex)
	{
		auto adapter = Dxgi::HardwareRenderers::Instance().GetAdapter(AdapterIndex);
		HRESULT re = D3D12CreateDevice(adapter, Level, __uuidof(Device), m_Device(VoidT{}));
		if (SUCCEEDED(re))
		{
			m_NodeMask = (1 << AdapterIndex);
		}
		else
			throw re;
	}

	void Context::Release() const noexcept { if (m_Ref.sub_ref()) delete this; }
	void Context::AddRef() const noexcept { m_Ref.add_ref(); }

	std::tuple<FencePtr, HRESULT> Context::CreateFence(uint32_t value, D3D12_FENCE_FLAGS flag)
	{
		FencePtr tem;
		HRESULT re = m_Device->CreateFence(value, flag, __uuidof(Fence), tem(VoidT{}));
		return { std::move(tem), re };
	}
	std::tuple<CommandQueuePtr, HRESULT> Context::CreateCommandQueue(CommandListType Type, CommandQueuePriority Priority, CommandQueueFlag Flags)
	{
		CommandQueuePtr result;
		D3D12_COMMAND_QUEUE_DESC desc{ *Type, *Priority, *Flags, m_NodeMask };
		HRESULT re = m_Device->CreateCommandQueue(&desc, __uuidof(CommandQueue), result(VoidT{}));
		return { std::move(result), re };
	}
	std::tuple<CommandAllocatorPtr, HRESULT> Context::CreateCommandAllocator(CommandListType Type)
	{
		CommandAllocatorPtr tem;
		HRESULT re = m_Device->CreateCommandAllocator(*Type, __uuidof(CommandAllocator), tem(VoidT{}));
		return { std::move(tem), re };
	}
	std::tuple<GraphicCommandListPtr, HRESULT> Context::CreateGraphicCommandList(CommandAllocator* allocator, CommandListType Type)
	{
		assert(allocator != nullptr);
		GraphicCommandListPtr tem;
		HRESULT re = m_Device->CreateCommandList(m_NodeMask, *Type, allocator, nullptr, __uuidof(GraphicCommandList), tem(VoidT{}));
		return {std::move(tem), re};
	}

	std::tuple<ContextPtr, HRESULT> Context::Create(uint8_t AdapterIndex, D3D_FEATURE_LEVEL Level)
	{
		try {
			ContextPtr ptr = new Context(AdapterIndex, Level);
			return { ptr, S_OK };
		}
		catch (HRESULT re)
		{
			return { ContextPtr{}, re };
		}
	}

	/*
	bool Context::SetRTAsTex2(RTDSDescriptor& Desc, Resource* Res, std::string_view Name, uint32_t MipSlice, uint32_t PlaneSlice, Dxgi::FormatPixel FP) {
		auto Index = Desc.FindElement(Name);
		if (Index.has_value())
		{
			D3D12_RENDER_TARGET_VIEW_DESC Des{ *FP, D3D12_RTV_DIMENSION_TEXTURE2D };
			Des.Texture2D = D3D12_TEX2D_RTV{ MipSlice, PlaneSlice };
			m_Device->CreateRenderTargetView(Res, &Des, Desc.RTCpuHandle(*Index));
			return true;
		}
		return false;
	}
	*/

	/*
	void Context::SetRTV2D(Descriptor& Heap, uint32_t Solts, Resource& Resource, uint32_t mipmap, DXGI_FORMAT format, uint32_t plane_slice)
	{
		assert(Heap.m_Type == DescriptorHeapType::RT);
		D3D12_RENDER_TARGET_VIEW_DESC desc;
		if (format == DXGI_FORMAT_UNKNOWN)
		{
			auto res_desc = Resource.GetDesc();
			desc.Format = res_desc.Format;
		}
		else
			desc.Format = format;
		desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		desc.Texture2D = { mipmap , plane_slice };
		m_Device->CreateRenderTargetView(&Resource, &desc, Heap[Solts]);
	}
	*/

	const FormStyle& Default() noexcept {
		static FormStyle Tem;
		return Tem;
	}

	Form::Form(CommandQueue& Queue, const FormSetting& Setting, const FormStyle& Style)
		: Win32::Form(Setting.Win32Setting, Style.Win32Style), m_BackBufferIndex(0)
	{
		auto& WS = Setting.Win32Setting;
		Dxgi::SwapChainDesc ChainDest{
			WS.Width, WS.Height, *Setting.Pixel, false, DXGI_SAMPLE_DESC{1, 0}, DXGI_USAGE_RENDER_TARGET_OUTPUT, 2,
			DXGI_SCALING_STRETCH, DXGI_SWAP_EFFECT_FLIP_DISCARD, DXGI_ALPHA_MODE_UNSPECIFIED, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH
		};
		m_SwapChain = Win32::ThrowIfFault(Dxgi::HardwareRenderers::Instance().CreateSwapChain(&Queue, GetHWnd(), ChainDest));
		m_MaxBufferCount = ChainDest.BufferCount;
		m_AllBackBuffer.resize(m_MaxBufferCount);
		for (size_t i = 0; i < m_MaxBufferCount; ++i)
		{
			HRESULT re;
			std::tie(m_AllBackBuffer[i], re) = GetBackBuffer(i);
			assert(SUCCEEDED(re));
		}
	}

	std::tuple<ResourcePtr, HRESULT> Form::GetBackBuffer(uint8_t index) noexcept
	{
		ResourcePtr res;
		HRESULT re = m_SwapChain->GetBuffer(index, __uuidof(Resource), res(VoidT{}));
		return {std::move(res), re};
	}

	void Form::PresentAndSwap() noexcept {
		DXGI_PRESENT_PARAMETERS Para{ 0, nullptr  , nullptr , nullptr };
		m_SwapChain->Present1(0, 0, &Para);
		m_BackBufferIndex = (m_BackBufferIndex + 1) % m_MaxBufferCount;
	}

	FormPtr Form::Create(CommandQueue& Queue, const FormSetting& Setting, const FormStyle& Style)
	{
		return new Form{ Queue, Setting, Style };
	}

	/*

	std::tuple<DevicePtr, HRESULT> CreateDevice(Dxgi::Adapter* adapter, D3D_FEATURE_LEVEL level)
	{
		//Device* tem;
		DevicePtr tem;
		HRESULT re = D3D12CreateDevice(adapter, level, __uuidof(Device), tem(VoidT{}));
		return {std::move(tem), re};
	}

	D3D12_VIEWPORT CreateFullScreenViewport(float width, float height) noexcept
	{
		return D3D12_VIEWPORT{0.0, 0.0, width, height, 0.0, 1.0};
	}

	std::tuple<ResourcePtr, HRESULT> GetBuffer(Dxgi::SwapChain* swap_chain, uint32_t count)
	{
		assert(swap_chain != nullptr);
		ResourcePtr tem;
		HRESULT re = swap_chain->GetBuffer(count, __uuidof(Resource), tem(VoidT{}));
		return {std::move(tem), re};
	}

	void IASetVertex(GraphicCommandList* list, uint32_t start_solts, std::initializer_list<VertexView> view)
	{
		assert(list != nullptr);
		assert(view.size() <= 16);
		assert(view.size() <= std::numeric_limits<uint32_t>::max());
		D3D12_VERTEX_BUFFER_VIEW view_list[16];
		size_t index = 0;
		for (auto& ite : view)
		{
			auto& ref = view_list[index++];
			assert(ite.res != nullptr);
			ref.BufferLocation = ite.res->GetGPUVirtualAddress();
			ref.SizeInBytes = ite.size;
			ref.StrideInBytes = ite.stride;
		}
		list->IASetVertexBuffers(start_solts, static_cast<uint32_t>(view.size()), view_list);
	}

	ResourceBarrier TransitionState(Resource* resource, D3D12_RESOURCE_STATES  before, D3D12_RESOURCE_STATES after,uint32_t subresource)
	{
		return { D3D12_RESOURCE_BARRIER_TYPE_TRANSITION, D3D12_RESOURCE_BARRIER_FLAG_NONE ,  D3D12_RESOURCE_TRANSITION_BARRIER {resource, subresource, before, after} };
	}

	void SwapTransitionState(size_t index, ResourceBarrier* output)
	{
		for (size_t i = 0; i < index; ++i)
		{
			auto& output_ref = output[i];
			if(output_ref.Type == D3D12_RESOURCE_BARRIER_TYPE_TRANSITION)
				std::swap(output_ref.Transition.StateBefore, output_ref.Transition.StateAfter);
		}
	}

	std::tuple<ReflectionPtr, HRESULT> Reflect(std::byte* code, size_t code_length)
	{
		ReflectionPtr tem;
		HRESULT re = D3DReflect(code, code_length, __uuidof(Reflection), tem(void_t{}));
		return { std::move(tem), re };
	}
}

namespace Dumpling::Win32
{
	Wrapper<Device>::Wrapper(Device* device) noexcept : m_device(device) {
		assert(m_device != nullptr);
	}


	std::tuple<FencePtr, HRESULT>  Wrapper<Device>::CreateFence(uint64_t initial_value, D3D12_FENCE_FLAGS flag)
	{
		FencePtr tem;
		HRESULT re = m_device->CreateFence(initial_value, flag, __uuidof(Fence), tem(void_t{}));
		return { std::move(tem), re };
	}

	std::tuple<DescriptorHeapPtr, HRESULT> Wrapper<Device>::CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t count, D3D12_DESCRIPTOR_HEAP_FLAGS flag, uint32_t node_mask)
	{
		D3D12_DESCRIPTOR_HEAP_DESC desc{ type , count, flag, node_mask };
		DescriptorHeapPtr tem;
		HRESULT re = m_device->CreateDescriptorHeap(&desc, __uuidof(DescriptorHeap), tem(void_t{}));
		return { std::move(tem), re };
	}

	DescriptorHandleIncrementSize Wrapper<Device>::GetDescriptorHandleIncrementSize()
	{
		return {
			m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV),
			m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER),
			m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV),
			m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV)
		};
	}

	std::tuple<CommandQueuePtr, HRESULT> Wrapper<Device>::CreateCommmandQueue(
		D3D12_COMMAND_LIST_TYPE type, D3D12_COMMAND_QUEUE_FLAGS flag,
		D3D12_COMMAND_QUEUE_PRIORITY priority, uint32_t node_mask
	)
	{
		CommandQueuePtr result;
		D3D12_COMMAND_QUEUE_DESC desc{ type, priority, flag,node_mask };
		HRESULT re = m_device->CreateCommandQueue(&desc, __uuidof(CommandQueue), result(void_t{}));
		return { std::move(result), re };
	}

	std::tuple<CommandAllocatorPtr, HRESULT> Wrapper<Device>::CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE type)
	{
		CommandAllocatorPtr tem;
		HRESULT re = m_device->CreateCommandAllocator(type, __uuidof(CommandAllocator), tem(void_t{}));
		return { std::move(tem), re };
	}

	std::tuple<GraphicCommandListPtr, HRESULT> Wrapper<Device>::CreateGraphicCommandList(CommandAllocator* allocator, D3D12_COMMAND_LIST_TYPE type, const ID3D12PipelineState* pipeline_state, uint32_t node_mask)
	{
		assert(allocator != nullptr);
		GraphicCommandListPtr tem;
		HRESULT re = m_device->CreateCommandList(node_mask, type, allocator, nullptr, __uuidof(GraphicCommandList), tem(void_t{}));
		return { std::move(tem), re };
	}

	std::tuple<ResourcePtr, HRESULT> Wrapper<Device>::CreateDepthStencil2DCommitted(DXGI_FORMAT format, uint32_t width, uint32_t height, uint16_t mipmap, float Depth_value, uint8_t stencil_value, uint32_t node_mask, uint32_t visible_node_mask, D3D12_RESOURCE_STATES default_state)
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
		HRESULT re = m_device->CreateCommittedResource(&heap_properties, D3D12_HEAP_FLAG_NONE, &desc, default_state, &value, __uuidof(Resource), tem(void_t{}));
		return { std::move(tem), re };
	}

	std::tuple<ResourcePtr, HRESULT> Wrapper<Device>::CreateBufferUploadCommitted(uint32_t width, uint32_t node_mask, uint32_t available_node_mask)
	{
		assert(width >= 1);
		D3D12_RESOURCE_DESC desc{
			D3D12_RESOURCE_DIMENSION_BUFFER,
			D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT,
			width, 1, 1, 1, DXGI_FORMAT_UNKNOWN, DXGI_SAMPLE_DESC {1, 0}, D3D12_TEXTURE_LAYOUT_ROW_MAJOR, D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE
		};
		D3D12_HEAP_PROPERTIES heap{
			D3D12_HEAP_TYPE_UPLOAD,
			D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
			D3D12_MEMORY_POOL_UNKNOWN,
			node_mask, available_node_mask
		};
		ResourcePtr tem;
		HRESULT re = m_device->CreateCommittedResource(&heap, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, __uuidof(Resource), tem(void_t{}));
		return { std::move(tem), re };
	}

	std::tuple<ResourcePtr, HRESULT> Wrapper<Device>::CreateBufferVertexCommitted(uint32_t width, uint32_t node_mask, uint32_t available_node_mask, D3D12_RESOURCE_STATES default_state)
	{
		assert(width >= 1);
		D3D12_RESOURCE_DESC desc{
			D3D12_RESOURCE_DIMENSION_BUFFER,
			D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT,
			width, 1, 1, 1, DXGI_FORMAT_UNKNOWN, DXGI_SAMPLE_DESC {1, 0}, D3D12_TEXTURE_LAYOUT_ROW_MAJOR, D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE
		};
		D3D12_HEAP_PROPERTIES heap{
			D3D12_HEAP_TYPE_DEFAULT,
			D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
			D3D12_MEMORY_POOL_UNKNOWN,
			node_mask, available_node_mask
		};
		ResourcePtr tem;
		HRESULT re = m_device->CreateCommittedResource(&heap, D3D12_HEAP_FLAG_NONE, &desc, default_state, nullptr, __uuidof(Resource), tem(void_t{}));
		return { std::move(tem), re };
	}

	void Wrapper<Device>::CreateRenderTargetView2D(Resource* resource, D3D12_CPU_DESCRIPTOR_HANDLE handle, uint32_t mipmap, DXGI_FORMAT format, uint32_t plane_slice)
	{
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
		m_device->CreateRenderTargetView(resource, &desc, handle);
	}

	void Wrapper<Device>::CreateDepthStencilView2D(Resource* resource, D3D12_CPU_DESCRIPTOR_HANDLE handle, uint32_t mipmap, DXGI_FORMAT format, D3D12_DSV_FLAGS flag)
	{
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
		m_device->CreateDepthStencilView(resource, &desc, handle);
	}
	*/
}