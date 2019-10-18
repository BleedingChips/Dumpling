#pragma once
#include "enum_dx12.h"
#include "../Dxgi/define_dxgi.h"
#include "pre_define_dx12.h"
#include "..//..//..//Potato/tool.h"
#include <array>
#include <assert.h>
#include <optional>
#include "..//Win32/form.h"
#include "../Dxgi/define_dxgi.h"
#include "descriptor_table_dx12.h"
namespace Dumpling::Dx12
{
	void InitDebugLayout();

	struct Context;
	using ContextPtr = ComPtr<Context>;

	struct Context 
	{
		
		void AddRef() const noexcept;
		void Release() const noexcept;

		std::tuple<FencePtr, HRESULT> CreateFence(uint32_t value = 0, D3D12_FENCE_FLAGS flag = D3D12_FENCE_FLAG_NONE);
		std::tuple<CommandQueuePtr, HRESULT> CreateCommandQueue(CommandListType Type = CommandListType::Direct, CommandQueuePriority Priority = CommandQueuePriority::Normal, CommandQueueFlag Flags = CommandQueueFlag::Non);
		std::tuple<CommandAllocatorPtr, HRESULT> CreateCommandAllocator(CommandListType Type);
		std::tuple<GraphicCommandListPtr, HRESULT> CreateGraphicCommandList(CommandAllocator* allocator, CommandListType Type);
		DescriptorMappingPtr CreateDescriptorMapping(std::string Name, std::initializer_list<DescriptorElement> Element) { return DescriptorMapping::Create(std::move(Name), std::move(Element)); }
		DescriptorPtr CreateDescriptor(DescriptorMapping* Mapping) { return Descriptor::Create(*m_Device, m_NodeMask, Mapping); }
		RTDSDescriptorPtr CreateRTDSDescriptor(std::string Name, std::initializer_list<std::string_view> RTName, bool UsedDT = false) { return RTDSDescriptor::Create(*m_Device, m_NodeMask, std::move(Name), RTName, UsedDT); }
		bool SetRTAsTex2(RTDSDescriptor& Desc, Resource* Res, std::string_view Name, uint32_t MipSlice = 0, uint32_t PlaneSlice = 0, Dxgi::FormatPixel FP = Dxgi::FormatPixel::Unknown);

		static std::tuple<ContextPtr, HRESULT> Create(uint8_t AdapterIndex = 0, D3D_FEATURE_LEVEL Level = D3D_FEATURE_LEVEL_12_1);
	
	private:

		friend struct Form;
		Context(uint8_t AdapterIndex, D3D_FEATURE_LEVEL Level);
		mutable Potato::Tool::atomic_reference_count m_Ref;
		DevicePtr m_Device;
		uint8_t m_AdapterIndex;
		uint32_t m_NodeMask;
		
	};

	struct FormStyle
	{
		Win32::FormStyle Win32Style;
	};

	const FormStyle& Default() noexcept;

	struct FormSetting {
		Win32::FormSetting Win32Setting;
		Dxgi::FormatPixel Pixel = Dxgi::FormatPixel::RGBA16_Float;
	};

	struct Form;
	using FormPtr = ComPtr<Form>;

	struct Form : Win32::Form
	{
		static FormPtr Create(CommandQueue& Queue, const FormSetting& Setting = FormSetting{}, const FormStyle& Style = Default());
		ResourcePtr CurrentBackBuffer() const noexcept { return m_BackBuffer; }
		void PresentAndSwap(GraphicCommandList&) noexcept;
		uint8_t CurrentBackBufferIndex() const noexcept { return m_BackBufferIndex; }
		std::tuple<ResourcePtr, HRESULT> GetBackBuffer(uint8_t index) noexcept;
	private:
		Form(CommandQueue& Queue, const FormSetting&, const FormStyle&);
		Dxgi::SwapChainPtr m_SwapChain;
		ResourcePtr m_BackBuffer;
		uint8_t m_BackBufferIndex;
		uint8_t m_MaxBufferCount;
	};

	


	//std::tuple<DevicePtr, HRESULT> CreateDevice(Dxgi::Adapter* adapter = nullptr, D3D_FEATURE_LEVEL level = D3D_FEATURE_LEVEL_12_0);
	//std::tuple<ResourcePtr, HRESULT> GetBuffer(Dxgi::SwapChain* swap_chain, uint32_t count);
}

namespace Dumpling::Win32
{
	/*
	using namespace Dx12;
	template<> struct Wrapper<Device>
	{
		std::tuple<FencePtr, HRESULT> CreateFence(uint64_t initial_value = 0, D3D12_FENCE_FLAGS flag = D3D12_FENCE_FLAG_NONE);
		std::tuple<DescriptorHeapPtr, HRESULT> CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t count, D3D12_DESCRIPTOR_HEAP_FLAGS flag = D3D12_DESCRIPTOR_HEAP_FLAG_NONE, uint32_t node_mask = 0);
		DescriptorHandleIncrementSize GetDescriptorHandleIncrementSize();
		std::tuple<CommandQueuePtr, HRESULT> CreateCommmandQueue(
			D3D12_COMMAND_LIST_TYPE type, D3D12_COMMAND_QUEUE_FLAGS flag = D3D12_COMMAND_QUEUE_FLAG_NONE,
			D3D12_COMMAND_QUEUE_PRIORITY priority = D3D12_COMMAND_QUEUE_PRIORITY::D3D12_COMMAND_QUEUE_PRIORITY_NORMAL, uint32_t node_mask = 0
		);
		std::tuple<CommandAllocatorPtr, HRESULT> CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE type);
		std::tuple<GraphicCommandListPtr, HRESULT> CreateGraphicCommandList(CommandAllocator* allocator, D3D12_COMMAND_LIST_TYPE type, const ID3D12PipelineState* pipeline_state = nullptr, uint32_t node_mask = 0);
		
		std::tuple<ResourcePtr, HRESULT> CreateDepthStencil2DCommitted(DXGI_FORMAT format, uint32_t width, uint32_t height, uint16_t mipmap = 0, float Depth_value = 1.0, uint8_t stencil_value = 0, uint32_t node_mask = 0, uint32_t visible_node_mask = 0, D3D12_RESOURCE_STATES default_state = D3D12_RESOURCE_STATE_COMMON);
		std::tuple<ResourcePtr, HRESULT> CreateBufferUploadCommitted(uint32_t width, uint32_t node_mask = 0, uint32_t available_node_mask = 0);
		std::tuple<ResourcePtr, HRESULT> CreateBufferVertexCommitted(uint32_t width, uint32_t node_mask = 0, uint32_t available_node_mask = 0, D3D12_RESOURCE_STATES default_state = *ResourceState::Common);

		void CreateRenderTargetView2D(Resource* resource, D3D12_CPU_DESCRIPTOR_HANDLE handle, uint32_t mipmap = 0, DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN, uint32_t plane_slice = 0);
		void CreateDepthStencilView2D(Resource* resource, D3D12_CPU_DESCRIPTOR_HANDLE handle, uint32_t mipmap = 0, DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN, D3D12_DSV_FLAGS flag = D3D12_DSV_FLAG_NONE);
		
		Wrapper(Device*) noexcept;
	private:
		Device* m_device;
	};
	*/
}

namespace Dumpling::Dx12
{
	
	//D3D12_VIEWPORT CreateFullScreenViewport(float width, float height) noexcept;

	

	/*
	template<typename Func> HRESULT MappingBufferByte(Resource* resource, Func&& f, uint32_t byte_start, uint32_t byte_count)
	{
		assert(resource != nullptr);
		D3D12_RANGE range{ byte_start , byte_start + byte_count };
		void* data = nullptr;
		HRESULT re = resource->Map(0, &range, &data);
		if (SUCCEEDED(re))
		{
			Potato::Tool::scope_guard sq([&]() noexcept {
				resource->Unmap(0, &range);
			});
			f(reinterpret_cast<std::byte*>(data));
		}
		return re;
	}

	template<typename Func>
	HRESULT MappingBufferArray(Resource* resource, Func&& f, uint32_t element_start, uint32_t element_count) {
		using FunctionType = typename Potato::Tmp::function_type_extractor<Func>;
		static_assert(FunctionType::parameter_count == 1, "mapping buffer array require function like [](Type*){...}");
		using ParameterType = typename FunctionType::template extract_parameter<Potato::Tmp::type_placeholder>::type;
		using TrueType = std::remove_pointer_t<ParameterType>;
		return MappingBufferByte(resource, [&](std::byte* input) {
			std::forward<Func>(f)(reinterpret_cast<ParameterType>(input));
		}, sizeof(TrueType) * element_start, sizeof(TrueType) * (element_start + element_count));
	}

	template<typename Func>
	HRESULT MappingBufferSingle(Resource* resource, Func&& f, uint32_t element_start = 0) {
		return MappingBufferArray(resource, std::forward<Func>(f), element_start, 1);
	}

	struct VertexView {
		Resource* res;
		uint32_t size;
		uint32_t stride;
	};



	void IASetVertex(CommandList* list, uint32_t start_solts, std::initializer_list<VertexView> view);

	using ResourceBarrier = D3D12_RESOURCE_BARRIER;

	ResourceBarrier TransitionState(Resource* resource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after, uint32_t subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
	void SwapTransitionState(size_t index, ResourceBarrier* output);
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

	

	//std::tuple<PipelineStatePtr, HRESULT> CreateGraphicPipelineState(Device* dev);

	std::tuple<ReflectionPtr, HRESULT> Reflect(std::byte* code, size_t code_length);
	*/

}