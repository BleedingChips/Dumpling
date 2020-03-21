#pragma once
#include "../Dxgi/define_dxgi.h"
#include <d3d12.h>
#include <array>
#include <assert.h>
#include <optional>
#include <map>
#include <string_view>

namespace Dumpling::Dx12
{
	using Win32::ComPtr;
	using Win32::ComBase;
	using Win32::VoidT;

	using Device = ID3D12Device;
	using DevicePtr = ComPtr<Device>;

	using Fence = ID3D12Fence1;
	using FencePtr = ComPtr<Fence>;

	using CommandQueue = ID3D12CommandQueue;
	using CommandQueuePtr = ComPtr<CommandQueue>;
	using CommandAllocator = ID3D12CommandAllocator;
	using CommandAllocatorPtr = ComPtr<CommandAllocator>;
	using GraphicCommandList = ID3D12GraphicsCommandList;
	using GraphicCommandListPtr = ComPtr<GraphicCommandList>;
	using CommandList = ID3D12CommandList;
	using Resource = ID3D12Resource;
	using ResourcePtr = ComPtr<Resource>;
	
	using DescriptorHeap = ID3D12DescriptorHeap;
	using DescriptorHeapPtr = ComPtr<ID3D12DescriptorHeap>;

	using RootSignature = ID3D12RootSignature;
	using RootSignaturePtr = ComPtr<RootSignature>;

	using PipelineState = ID3D12PipelineState;
	using PipelineStatePtr = ID3D12PipelineState;

	using Blob = ID3DBlob;
	using BlobPtr = ComPtr<Blob>;

	using CommandAllocatorPtr = ComPtr<ID3D12CommandAllocator>;
	using GraphicCommandListPtr = ComPtr<ID3D12GraphicsCommandList>;
	using ResourcePtr = ComPtr<ID3D12Resource>;

	UINT DeviceNodeMask(Device& Dev);
	DevicePtr CreateDevice(uint8_t AdapterIndex = 0, D3D_FEATURE_LEVEL Level = D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_12_1);
	FencePtr CreateFence(Device& Dev, uint32_t Value = 0, D3D12_FENCE_FLAGS Flag = D3D12_FENCE_FLAGS::D3D12_FENCE_FLAG_NONE);
	CommandQueuePtr CreateCommandQueue(Device& Dev, D3D12_COMMAND_LIST_TYPE Type, D3D12_COMMAND_QUEUE_PRIORITY Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL, D3D12_COMMAND_QUEUE_FLAGS Flags = D3D12_COMMAND_QUEUE_FLAG_NONE);
	CommandAllocatorPtr CreateCommandAllocator(Device& Dev, D3D12_COMMAND_LIST_TYPE Type);
	GraphicCommandListPtr CreateGraphicCommandList(Device& Dev, CommandAllocator& allocator, D3D12_COMMAND_LIST_TYPE Type);
	ResourcePtr CreateTexture2DConst(Device& Dev, DXGI_FORMAT Format, uint64_t Width, uint32_t Height, uint16_t Mapmap = 0, D3D12_RESOURCE_STATES State = D3D12_RESOURCE_STATE_COMMON);
	ResourcePtr CreateUploadBuffer(Device& Dev, uint64_t Length);
	template<typename Function>
	bool MappingBuffer(Resource& Res, UINT SubResource, size_t Begin, size_t Length, Function&& F)
	{
		D3D12_RANGE Range{ Begin , Begin + Length };
		
		void* Pointer = nullptr;
		HRESULT re = Res.Map(SubResource, &Range, &Pointer);
		if (SUCCEEDED(re))
		{
			Potato::Tool::scope_guard SG([&]() noexcept {
				Res.Unmap(SubResource, &Range);
			});
			F(reinterpret_cast<std::byte*>(Pointer));
			return true;
		}
		return false;
	}
	void ChangeState(GraphicCommandList& List, std::initializer_list<Resource*>, D3D12_RESOURCE_STATES OldState, D3D12_RESOURCE_STATES NewState, uint32_t SubResource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
	void InitDebugLayout();

	enum class ResourceType 
	{
		Tex2D = 0,
		Sampler,
	};

	struct DescriptorMapping : ComBase<DescriptorMapping>
	{
		struct CustomLess 
		{
			bool operator()(const std::tuple<ResourceType, std::string_view>& i1, const std::tuple<ResourceType, std::string_view>& i2) const
			{
				auto i1i = static_cast<uint32_t>(std::get<0>(i1));
				auto i2i = static_cast<uint32_t>(std::get<0>(i2));
				return i1i < i2i || (i1i == i2i) && std::get<1>(i1) < std::get<1>(i2);
			}
		};
		size_t ResourceCount() const { return mResourceCount; }
		size_t SamplerCount() const { return mSamplerCount; }
		std::optional<size_t> Find(std::string_view View, ResourceType) const;
	private:
		friend ComPtr<DescriptorMapping> CreateDescriptorMapping(const std::vector<std::tuple<std::string_view, ResourceType>>& ResourceName);
		using StorageMapping = std::map<std::tuple<ResourceType, std::string_view>, size_t, CustomLess>;
		DescriptorMapping(StorageMapping InputMap, size_t ResourceCount, size_t SamplerCount)
			: Mapping(std::move(InputMap)), mResourceCount(ResourceCount), mSamplerCount(SamplerCount) {}
		StorageMapping Mapping;
		size_t mResourceCount;
		size_t mSamplerCount;
	};

	ComPtr<DescriptorMapping> CreateDescriptorMapping(const std::vector<std::tuple<std::string_view, ResourceType>>& ResourceName);
	inline ComPtr<DescriptorMapping> CreateDescriptorMapping(std::initializer_list<std::tuple<std::string_view, ResourceType>> ResourceName) { return CreateDescriptorMapping(std::vector<std::tuple<std::string_view, ResourceType>>(ResourceName)); }
	

	using DescriptorMappingPtr = ComPtr<DescriptorMapping>;

	struct ResourceDescriptor {
		ResourceDescriptor(Device& Dev, DescriptorMappingPtr Ptr);
		bool SetTex2D(Device& Dev, std::string_view view, ResourcePtr Re, Dxgi::FormatPixel Format, std::tuple<size_t, size_t> Mipmap = { 0, static_cast<size_t>(-1) });
		operator bool() const { return mMapping; };
	private:
		DescriptorMappingPtr mMapping;
		DescriptorHeapPtr mResource;
		DescriptorHeapPtr mSampler;
	};

	inline ResourceDescriptor CreateResourceDescriptor(Device& Dev, DescriptorMappingPtr Ptr) { return ResourceDescriptor{Dev, std::move(Ptr)}; }





	/*
	struct Context;
	using ContextPtr = ComPtr<Context>;

	struct StateLog {
		void ChangeState(GraphicCommandList* List, ResourceState NewState, uint32_t SubResource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES) {
			assert(List != nullptr);
			if (m_Res && m_State != NewState)
			{
				D3D12_RESOURCE_BARRIER Barr{
					D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
					D3D12_RESOURCE_BARRIER_FLAG_NONE
				};
				Barr.Transition = D3D12_RESOURCE_TRANSITION_BARRIER{ m_Res, SubResource, *m_State, *NewState };
				List->ResourceBarrier(1, &Barr);
				m_State = NewState;
			}
		}
		ResourcePtr m_Res;
		ResourceState m_State = ResourceState::Common;
	};

	

	struct Context 
	{
		
		void AddRef() const noexcept;
		void Release() const noexcept;

		std::tuple<FencePtr, HRESULT> CreateFence(uint32_t value = 0, D3D12_FENCE_FLAGS flag = D3D12_FENCE_FLAG_NONE);
		std::tuple<CommandQueuePtr, HRESULT> CreateCommandQueue(CommandListType Type = CommandListType::Direct, CommandQueuePriority Priority = CommandQueuePriority::Normal, CommandQueueFlag Flags = CommandQueueFlag::Non);
		std::tuple<CommandAllocatorPtr, HRESULT> CreateCommandAllocator(CommandListType Type);
		std::tuple<GraphicCommandListPtr, HRESULT> CreateGraphicCommandList(CommandAllocator* allocator, CommandListType Type);
		DescriptorMappingPtr CreateDescriptorMapping(std::string Name, std::initializer_list<DescriptorElement> Element) { return DescriptorMapping::Create(std::move(Name), std::move(Element)); }
		Descriptor CreateDescriptor(DescriptorType Type, size_t Count) { return Descriptor{ m_Device, m_NodeMask, Type, Count }; }
		RTDescriptor CreateRTDescriptor(size_t Count) { return RTDescriptor{ m_Device, m_NodeMask, Count }; }
		void SetTex2AsRTV(RTDescriptor& Desc, Resource* Res, uint32_t Index, uint32_t MipSlice = 0, uint32_t PlaneSlice = 0, Dxgi::FormatPixel FP = Dxgi::FormatPixel::Unknown) {
			return Desc.SetTex2AsRTV(m_Device, Res, Index, MipSlice, PlaneSlice, FP);
		}
		
		
		
		//RTDSDescriptorPtr CreateRTDSDescriptor(std::string Name, std::initializer_list<std::string_view> RTName, bool UsedDT = false) { return RTDSDescriptor::Create(m_Device, m_NodeMask, std::move(Name), RTName, UsedDT); }
		

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
		ResourcePtr CurrentBackBuffer() const noexcept { return m_AllBackBuffer[m_BackBufferIndex]; }
		void PresentAndSwap() noexcept;
		uint8_t CurrentBackBufferIndex() const noexcept { return m_BackBufferIndex; }
		std::tuple<ResourcePtr, HRESULT> GetBackBuffer(uint8_t index) noexcept;
	private:
		Form(CommandQueue& Queue, const FormSetting&, const FormStyle&);
		Dxgi::SwapChainPtr m_SwapChain;
		std::vector<ResourcePtr> m_AllBackBuffer;
		ComPtr<DescriptorHeap> m_Heap;
		uint8_t m_BackBufferIndex;
		uint8_t m_MaxBufferCount;
	};
	*/

	


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