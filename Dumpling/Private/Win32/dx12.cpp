#include "dx12.h"
#include <assert.h>
#include <d3dcompiler.h>
#undef max
namespace Dumpling::Dx12
{

	std::tuple<D3D12_HEAP_TYPE, D3D12_HEAP_FLAGS> Trans(HeapType Input)
	{
		switch (Input)
		{
		case HeapType::Texture: {
			return { D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_DEFAULT, D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_ALLOW_ONLY_NON_RT_DS_TEXTURES };
		} break;
		case HeapType::RTDSTexture: {
			return { D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_DEFAULT, D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_ALLOW_ONLY_RT_DS_TEXTURES };
		} break;
		case HeapType::Shader: {
			return { D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_DEFAULT, D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_SHARED };
		} break;
		case HeapType::UploadBuffer: {
			return { D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_UPLOAD, D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS };
		} break;
		case HeapType::Buffer: {
			return { D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_DEFAULT, D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS };
		} break;
		default:assert(false);
			break;
		}
		return { D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_DEFAULT, D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS };
	}

	struct HeadManagerImpCommited : HeadManager
	{
		DevicePtr Dev;
		HeapType type;
		UINT VisibleNodeMask;
		HeadManagerImpCommited(DevicePtr ptr, HeapType type, UINT VNM) : Dev(std::move(ptr)), type(type), VisibleNodeMask(VNM) {}
		virtual HeapType Type() const noexcept override { return type; }
		virtual Device& GetDevice() noexcept override { return *Dev; }
		virtual size_t VisibleMask() const noexcept override { return VisibleNodeMask; }
		virtual HeapPtr Allocate(size_t size, size_t& offset) override
		{
			auto [type, flag] = Trans(Type());


			D3D12_HEAP_DESC desc{size, D3D12_HEAP_PROPERTIES{ type,
				D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
				D3D12_MEMORY_POOL_UNKNOWN,
				DeviceNodeMask(*Dev),
				VisibleNodeMask
			}, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT, flag };
			HeapPtr heap;
			HRESULT re =Dev->CreateHeap(&desc, __uuidof(Heap), heap(VoidT{}));
			Win32::ThrowIfFault(re);
			offset = 0;
			return std::move(heap);
		}
	};

	void InitDebugLayout()
	{
		ComPtr<ID3D12Debug>	debugController;
		HRESULT re = D3D12GetDebugInterface(__uuidof(ID3D12Debug), debugController(VoidT{}));
		assert(SUCCEEDED(re));
		debugController->EnableDebugLayer();
	}

	HeadManagerPtr CreateCommitedHeap(DevicePtr Dev, HeapType Type, UINT VisibleNodeMask)
	{
		return new Win32::ComBaseImplement<HeadManagerImpCommited>( std::move(Dev), Type, VisibleNodeMask | DeviceNodeMask(*Dev));
	}

	UINT DeviceNodeMask(Device& Dev)
	{
		return (1 << *Dxgi::HardwareRenderers::Instance().CalculateAdapter(Dev.GetAdapterLuid()));
	}

	DevicePtr CreateDevice(uint8_t AdapterIndex, D3D_FEATURE_LEVEL Level)
	{
		auto adapter = Dxgi::HardwareRenderers::Instance().GetAdapter(AdapterIndex);
		if (adapter)
		{
			DevicePtr Ptr;
			HRESULT re = D3D12CreateDevice(adapter, Level, __uuidof(Device), Ptr(VoidT{}));
			return Ptr;
		}
		return {};
	}

	FencePtr CreateFence(Device& Dev, uint32_t Value, D3D12_FENCE_FLAGS Flag)
	{
		FencePtr Ptr;
		HRESULT re = Dev.CreateFence(Value, Flag, __uuidof(Fence), Ptr(VoidT{}));
		return Ptr;
	}

	CommandQueuePtr CreateCommandQueue(Device& Dev, D3D12_COMMAND_LIST_TYPE Type, D3D12_COMMAND_QUEUE_PRIORITY Priority, D3D12_COMMAND_QUEUE_FLAGS Flags)
	{
		CommandQueuePtr Ptr;
		auto Index = Dxgi::HardwareRenderers::Instance().CalculateAdapter(Dev.GetAdapterLuid());
		if (Index)
		{
			D3D12_COMMAND_QUEUE_DESC desc{ Type, Priority, Flags, DeviceNodeMask(Dev) };
			HRESULT re = Dev.CreateCommandQueue(&desc, __uuidof(CommandQueue), Ptr(VoidT{}));
			return Ptr;
		}
		return {};
	}

	CommandAllocatorPtr CreateCommandAllocator(Device& Dev, D3D12_COMMAND_LIST_TYPE Type)
	{
		CommandAllocatorPtr Ptr;
		HRESULT re = Dev.CreateCommandAllocator(Type, __uuidof(CommandAllocator), Ptr(VoidT{}));
		return Ptr;
	}

	GraphicCommandListPtr CreateGraphicCommandList(Device& Dev, CommandAllocator& allocator, D3D12_COMMAND_LIST_TYPE Type)
	{
		GraphicCommandListPtr Ptr;
		HRESULT re = Dev.CreateCommandList(DeviceNodeMask(Dev), Type, &allocator, nullptr, __uuidof(GraphicCommandList), Ptr(VoidT{}));
		return Ptr;
	}

	ResourcePtr HeadManager::CreateTexture2D(DXGI_FORMAT Format, uint64_t Width, uint32_t Height, uint16_t Mapmap, D3D12_RESOURCE_STATES State)
	{
		D3D12_RESOURCE_DESC Desc{ D3D12_RESOURCE_DIMENSION_TEXTURE2D, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT, Width, Height, 1, Mapmap, Format, DXGI_SAMPLE_DESC {1, 0}, D3D12_TEXTURE_LAYOUT_UNKNOWN, D3D12_RESOURCE_FLAG_NONE };
		D3D12_RESOURCE_ALLOCATION_INFO Info = GetDevice().GetResourceAllocationInfo(VisibleMask(), 1, &Desc);
		size_t Offset;
		auto Head = Allocate(Info.SizeInBytes, Offset);
		ResourcePtr Result;
		HRESULT re = GetDevice().CreatePlacedResource(Head, Offset, &Desc, State, nullptr, __uuidof(Resource), Result(VoidT{}));
		Win32::ThrowIfFault(re);
		return Result;
	}

	ResourcePtr HeadManager::CreateTexture3DUAV(DXGI_FORMAT Format, uint64_t Width, uint32_t Height, uint32_t depth, D3D12_RESOURCE_STATES State)
	{
		D3D12_RESOURCE_DESC Desc{ D3D12_RESOURCE_DIMENSION_TEXTURE3D, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT, Width, Height, depth, 1, Format, DXGI_SAMPLE_DESC {1, 0}, D3D12_TEXTURE_LAYOUT_UNKNOWN, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS };
		D3D12_RESOURCE_ALLOCATION_INFO Info = GetDevice().GetResourceAllocationInfo(VisibleMask(), 1, &Desc);
		size_t Offset;
		auto Head = Allocate(Info.SizeInBytes, Offset);
		ResourcePtr Result;
		HRESULT re = GetDevice().CreatePlacedResource(Head, Offset, &Desc, State, nullptr, __uuidof(Resource), Result(VoidT{}));
		Win32::ThrowIfFault(re);
		return Result;
	}

	ResourcePtr HeadManager::CreateBuffer(uint64_t Width, D3D12_RESOURCE_STATES State)
	{
		D3D12_RESOURCE_DESC Desc{ D3D12_RESOURCE_DIMENSION_BUFFER, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT, Width, 1, 1, 1, DXGI_FORMAT_UNKNOWN, DXGI_SAMPLE_DESC {1, 0}, D3D12_TEXTURE_LAYOUT_ROW_MAJOR, D3D12_RESOURCE_FLAG_NONE };
		D3D12_RESOURCE_ALLOCATION_INFO Info = GetDevice().GetResourceAllocationInfo(VisibleMask(), 1, &Desc);
		size_t Offset;
		auto Head = Allocate(Info.SizeInBytes, Offset);
		ResourcePtr Result;
		D3D12_RESOURCE_STATES state = *ResState::Common;
		if (Type() == HeapType::UploadBuffer)
			state = *ResState::GenericRead;
		HRESULT re = GetDevice().CreatePlacedResource(Head, Offset, &Desc, state, nullptr, __uuidof(Resource), Result(VoidT{}));
		Win32::ThrowIfFault(re);
		return Result;
	}

	ResourcePtr CreateTexture2DConst(Device& Dev, DXGI_FORMAT Format, uint64_t Width, uint32_t Height, uint16_t Mapmap, D3D12_RESOURCE_STATES State)
	{
		D3D12_HEAP_PROPERTIES Pri{ D3D12_HEAP_TYPE_DEFAULT, D3D12_CPU_PAGE_PROPERTY_UNKNOWN, D3D12_MEMORY_POOL_UNKNOWN, DeviceNodeMask(Dev), DeviceNodeMask(Dev)};
		D3D12_RESOURCE_DESC Desc{ D3D12_RESOURCE_DIMENSION_TEXTURE2D, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT, Width, Height, 1, Mapmap, Format, DXGI_SAMPLE_DESC {1, 0}, D3D12_TEXTURE_LAYOUT_UNKNOWN, D3D12_RESOURCE_FLAG_NONE };
		ResourcePtr Ptr;
		Dev.CreateCommittedResource(&Pri, D3D12_HEAP_FLAG_NONE, &Desc, State, nullptr, __uuidof(Resource), Ptr(VoidT{}));
		D3D12_HEAP_FLAGS des22c;
		D3D12_HEAP_PROPERTIES pro22;
		Ptr->GetHeapProperties(&pro22, &des22c);
		return Ptr;
	}

	ResourcePtr CreateUploadBuffer(Device& Dev, uint64_t Length)
	{
		D3D12_HEAP_PROPERTIES Pri{ D3D12_HEAP_TYPE_UPLOAD, D3D12_CPU_PAGE_PROPERTY_UNKNOWN, D3D12_MEMORY_POOL_UNKNOWN, DeviceNodeMask(Dev), DeviceNodeMask(Dev) };
		D3D12_RESOURCE_DESC Desc{ D3D12_RESOURCE_DIMENSION_BUFFER, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT, Length, 1, 1, 1, DXGI_FORMAT_UNKNOWN, DXGI_SAMPLE_DESC {1, 0}, D3D12_TEXTURE_LAYOUT_ROW_MAJOR, D3D12_RESOURCE_FLAG_NONE };
		ResourcePtr Ptr;
		Dev.CreateCommittedResource(&Pri, D3D12_HEAP_FLAG_NONE, &Desc, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, __uuidof(Resource), Ptr(VoidT{}));
		return Ptr;
	}

	void ChangeState(GraphicCommandList& List, std::initializer_list<Resource*> Res, ResState OldState, ResState NewState, uint32_t SubResource)
	{
		for (auto& ite : Res)
		{
			if (ite != nullptr)
			{
				D3D12_RESOURCE_BARRIER Barr{
					D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
					D3D12_RESOURCE_BARRIER_FLAG_NONE
				};
				Barr.Transition = D3D12_RESOURCE_TRANSITION_BARRIER{ ite, SubResource, *OldState, *NewState };
				List.ResourceBarrier(1, &Barr);
			}
		}
	}

	ComPtr<DescriptorMapping> CreateDescriptorMapping(const std::vector<std::tuple<std::string_view, ResourceType>>& ResourceName)
	{
		size_t ResourceCount = 0;
		size_t SamplerCount = 0;
		DescriptorMapping::StorageMapping TemplateMapping;
		size_t Index = 0;
		for (auto& ite : ResourceName)
		{
			auto& [Name, Type] = ite;
			if (Type == ResourceType::Sampler)
				++SamplerCount;
			else
				++ResourceCount;
			auto Result = TemplateMapping.insert({ std::tuple<ResourceType, std::string>{Type, Name}, Index });
			if (Result.second)
				++Index;
		}
		return new DescriptorMapping{std::move(TemplateMapping), ResourceCount, SamplerCount };
	}

	std::optional<size_t> DescriptorMapping::Find(std::string_view View, ResourceType RT) const
	{
		auto Result = Mapping.find({RT, View});
		if (Result != Mapping.end())
			return Result->second;
		else
			return std::nullopt;
	}

	ResourceDescriptor::ResourceDescriptor(Device& Dev, DescriptorMappingPtr Ptr)
	{
		assert(Ptr);
		HRESULT Result = S_OK;
		if (Ptr->ResourceCount() != 0)
		{
			D3D12_DESCRIPTOR_HEAP_DESC Desc{ D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, static_cast<UINT>(Ptr->ResourceCount()), D3D12_DESCRIPTOR_HEAP_FLAG_NONE, DeviceNodeMask(Dev) };
			Result = Dev.CreateDescriptorHeap(&Desc, __uuidof(DescriptorHeap), mResource(VoidT{}));
		}
		assert(SUCCEEDED(Result));
		
		if (Ptr->SamplerCount() != 0)
		{
			D3D12_DESCRIPTOR_HEAP_DESC Desc{ D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, static_cast<UINT>(Ptr->SamplerCount()), D3D12_DESCRIPTOR_HEAP_FLAG_NONE, DeviceNodeMask(Dev) };
			Result = Dev.CreateDescriptorHeap(&Desc, __uuidof(DescriptorHeap), mResource(VoidT{}));
		}
		assert(SUCCEEDED(Result));
		mMapping = std::move(Ptr);
	}

	bool ResourceDescriptor::SetTex2D(Device& Dev, std::string_view view, ResourcePtr Re, Dxgi::FormatPixel Format, std::tuple<size_t, size_t> Mipmap)
	{
		assert(mMapping);
		if (mResource)
		{
			auto Find = mMapping->Find(view, ResourceType::Tex2D);
			if (Find)
			{
				UINT Size = Dev.GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
				auto [MaxLevel, MipLevel] = Mipmap;
				D3D12_TEX2D_SRV SRV{
					static_cast<UINT>(MaxLevel),
					static_cast<UINT>(MipLevel),
					0,
					0.f
				};
				D3D12_SHADER_RESOURCE_VIEW_DESC Desc{
					*Format,
					D3D12_SRV_DIMENSION::D3D12_SRV_DIMENSION_TEXTURE2D,
					D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING
				};
				Desc.Texture2D = SRV;
				Dev.CreateShaderResourceView(Re, &Desc, { mResource->GetCPUDescriptorHandleForHeapStart().ptr + *Find * Size });
				return true;
			}
		}
		return false;
	}

	void CreateRootSignature(std::vector<Potato::Tool::span<std::byte>> Type)
	{


		struct Element 
		{
			D3D12_DESCRIPTOR_RANGE_TYPE Type;
			size_t regeister;
			size_t space;
			size_t offset;
			size_t size;
		};

		D3D12_ROOT_SIGNATURE_DESC1 Desc1;
		D3D12_ROOT_PARAMETER1 Paras;
		std::array<std::vector<D3D12_DESCRIPTOR_RANGE1>, D3D12_SHVER_COMPUTE_SHADER + 1> AllInputRange;

		for (auto code : Type)
		{
			std::vector<D3D12_DESCRIPTOR_RANGE1> AllRange;
			Dx12::ComPtr<ID3D12ShaderReflection> Ref;
			HRESULT re = D3DReflect(code.data(), code.size(), __uuidof(ID3D12ShaderReflection), Ref(VoidT{}));
			Win32::ThrowIfFault(re);
			D3D12_SHADER_DESC Desc;
			Ref->GetDesc(&Desc);
			auto Type = (Desc.Version & 0xFFFF0000) >> 16;
			auto& ref = AllInputRange[Type];

			for (size_t i = 0; i < Desc.BoundResources; ++i)
			{
				D3D12_SHADER_INPUT_BIND_DESC Desc;
				Ref->GetResourceBindingDesc(i, &Desc);
				switch (Desc.Type)
				{
				case D3D_SIT_STRUCTURED:
				case D3D_SIT_TBUFFER:
				case D3D_SIT_TEXTURE: {
					D3D12_DESCRIPTOR_RANGE1 De{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, Desc.BindCount, Desc.BindPoint, Desc.Space, D3D12_DESCRIPTOR_RANGE_FLAG_NONE, 0 };
					ref.push_back(De);
				} break;
				case D3D_SIT_UAV_RWBYTEADDRESS:
				case D3D_SIT_UAV_RWSTRUCTURED:
				case D3D_SIT_UAV_APPEND_STRUCTURED:
				case D3D_SIT_UAV_CONSUME_STRUCTURED:
				case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
				case D3D_SIT_UAV_RWTYPED: {
					D3D12_DESCRIPTOR_RANGE1 De{ D3D12_DESCRIPTOR_RANGE_TYPE_UAV, Desc.BindCount, Desc.BindPoint, Desc.Space, D3D12_DESCRIPTOR_RANGE_FLAG_NONE, 0 };
					ref.push_back(De);
				}break;
				case D3D_SIT_BYTEADDRESS:
				case D3D_SIT_CBUFFER: {
					D3D12_DESCRIPTOR_RANGE1 De{ D3D12_DESCRIPTOR_RANGE_TYPE_CBV, Desc.BindCount, Desc.BindPoint, Desc.Space, D3D12_DESCRIPTOR_RANGE_FLAG_NONE, 0 };
					ref.push_back(De);
				} break;
				case D3D_SIT_SAMPLER: {
					D3D12_DESCRIPTOR_RANGE1 De{ D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, Desc.BindCount, Desc.BindPoint, Desc.Space, D3D12_DESCRIPTOR_RANGE_FLAG_NONE, 0 };
					ref.push_back(De);
				} break;
				}
			}

			size_t start = 0;
			while (start < ref.size())
			{

			}


			//AllInputRange[]

			for (size_t i = 0; i < Desc.ConstantBuffers; ++i)
			{
				auto Buffer = Ref->GetConstantBufferByIndex(i);
				D3D12_SHADER_BUFFER_DESC Desc;
				Buffer->GetDesc(&Desc);
				for (size_t k = 0; k < Desc.Variables; ++k)
				{
					auto Ref = Buffer->GetVariableByIndex(k);
					if (Ref != nullptr)
					{
						D3D12_SHADER_VARIABLE_DESC Desc2;
						Ref->GetDesc(&Desc2);
						volatile int k = 0;
					}
				}
				volatile int k = 0;
			}
		}
	}

	
	/*
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
	*/

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

	/*
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
	*/

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