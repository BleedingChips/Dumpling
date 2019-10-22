#pragma once
#include "enum_dx12.h"
#include "../Win32/aid.h"
#include "../../../Potato/tool.h"
#include "../Dxgi/define_dxgi.h"
#include "pre_define_dx12.h"
#include <map>
#include <array>
#include <vector>
#include <memory>
namespace Dumpling::Dx12 
{
	using Win32::ComPtr;

	using DescriptorHeap = ID3D12DescriptorHeap;
	using DescriptorHeapPtr = ComPtr<DescriptorHeap>;

	enum class DescResCategory
	{
		SRV = 0,
		CBV,
		UAV,
		Sampler,
		Num
	};

	inline constexpr uint32_t operator*(DescResCategory Category) noexcept { return static_cast<uint32_t>(Category); }

	enum class DescResType
	{
		CB = 0,
		S_Tex1, S_Tex1A, S_Tex2, S_Tex2A, S_Tex2MS, S_Tex2AMS, S_Tex3, S_TexCube, S_SB,
		U_Tex1, U_Tex1A, U_Tex2, U_Tex2A, U_Tex3,
		Sampler,
		Num
	};

	struct DescriptorElement {
		std::string_view Name;
		DescResType Type;
		Dxgi::FormatPixel Format = Dxgi::FormatPixel::Unknown;
	};

	struct DescriptorMapping;
	using DescriptorMappingPtr = std::shared_ptr<DescriptorMapping>;

	struct DescriptorMapping {
		std::string_view Name() const noexcept { return m_Name; }
		uint32_t ResourceCount() const noexcept { return m_ResourceCount; }
		uint32_t SamplerCount() const noexcept { return m_SamplerCount; }
		inline static DescriptorMappingPtr Create(std::string Name, std::initializer_list<DescriptorElement> Map) { return std::make_shared<DescriptorMapping>(std::move(Name), std::move(Map)); }
		std::optional<std::tuple<DescResType, uint32_t, Dxgi::FormatPixel>> FindElement(DescResCategory ResType, std::string_view view) const;
		DescriptorMapping(std::string Name, std::initializer_list<DescriptorElement> Map);
	private:
		std::string m_Name;
		std::vector<std::tuple<std::string, DescResType, Dxgi::FormatPixel>> m_Elements;
		std::array<size_t, *DescResCategory::Num> m_ElementsOffset;
		std::array<size_t, *DescResCategory::Num> m_ElenmentCount;
		uint32_t m_ResourceCount;
		uint32_t m_SamplerCount;
	};

	struct Descriptor;
	using DescriptorPtr = std::shared_ptr<Descriptor>;

	struct Descriptor {
		Descriptor(Device* Dev, uint32_t NodeMask, DescriptorType Type, size_t Count);
		Descriptor(Descriptor&&) = default;
		~Descriptor() = default;
		DescriptorType Type() const noexcept { return m_Type; };
		operator bool() const noexcept { return m_Heap; }
		D3D12_CPU_DESCRIPTOR_HANDLE CPUHandle(uint32_t Index = 0) const noexcept { 
			assert(m_Count < Index);
			return D3D12_CPU_DESCRIPTOR_HANDLE{ m_Heap->GetCPUDescriptorHandleForHeapStart().ptr + static_cast<uint64_t>(m_Offset)* Index };
		}
	protected:
		DescriptorType m_Type;
		DescriptorHeapPtr m_Heap;
		uint32_t m_Offset;
		uint32_t m_Count;
	};

	struct RTDescriptor : Descriptor {
		using Descriptor::Descriptor;
		RTDescriptor(Device* Dev, uint32_t NodeMask, size_t Count) : Descriptor(Dev, NodeMask, DescriptorType::RenderTarget, Count) {}
		RTDescriptor(RTDescriptor&&) = default;
		RTDescriptor(Descriptor&& des) : Descriptor(std::move(des)) {}
		void SetTex2AsRTV(Device* dev, Resource* Res, uint32_t Index, uint32_t MipSlice = 0, uint32_t PlaneSlice = 0, Dxgi::FormatPixel FP = Dxgi::FormatPixel::Unknown);
	};

	/*
	struct Descriptor
	{
		std::string_view Name() const noexcept { return m_Mapping->Name(); }
		D3D12_CPU_DESCRIPTOR_HANDLE ResourceCpuHandleStart() const noexcept 
		{ 
			assert(ResourceCount() > 0);
			return m_ResHeap->GetCPUDescriptorHandleForHeapStart(); 
		}
		D3D12_CPU_DESCRIPTOR_HANDLE ResourceCpuHandle(uint32_t index) const noexcept {
			assert(index < ResourceCount());
			return D3D12_CPU_DESCRIPTOR_HANDLE{ ResourceCpuHandleStart().ptr + static_cast<uint64_t>(m_ResOffset) * index };
		}
		D3D12_CPU_DESCRIPTOR_HANDLE SamplerCpuHandleStart() const noexcept {
			assert(SamplerCount() > 0);
			return m_SamHeap->GetCPUDescriptorHandleForHeapStart();
		}
		D3D12_CPU_DESCRIPTOR_HANDLE SamplerCpuHandle(uint32_t index) const noexcept {
			assert(index < SamplerCount());
			return D3D12_CPU_DESCRIPTOR_HANDLE{ SamplerCpuHandleStart().ptr + static_cast<uint64_t>(m_SamOffset) * index };
		}
		uint32_t ResourceCount() const noexcept { return m_Mapping->ResourceCount(); }
		uint32_t SamplerCount() const noexcept { return m_Mapping->SamplerCount(); }
		std::optional<std::tuple<DescResType, uint32_t, Dxgi::FormatPixel>> FindElement(DescResCategory ResType, std::string_view view) const { return m_Mapping->FindElement(ResType, view); }
		static DescriptorPtr Create(Device& Dev, uint32_t NodeMask, DescriptorMapping* Mapping);
	private:
		Descriptor() = default;
		DescriptorHeapPtr m_ResHeap;
		uint32_t m_ResOffset;
		
		DescriptorMappingPtr m_Mapping;
		mutable Potato::Tool::atomic_reference_count m_Ref;
	};

	struct SimpleDescriptorMapping {
		void AddRef() const noexcept;
		void Release() const noexcept;
		std::optional<uint32_t> FindElement(std::string_view Name) const noexcept;
	private:
		std::vector<std::string> m_Name;
		mutable Potato::Tool::atomic_reference_count m_Ref;
	};

	struct RTDSDescriptor;
	using RTDSDescriptorPtr = ComPtr<RTDSDescriptor>;

	struct RTDSDescriptor {
		void AddRef() const noexcept;
		void Release() const noexcept;
		bool UsedDS() const noexcept { return m_DSHeap; }
		static RTDSDescriptorPtr Create(Device* Dev, uint32_t NodeMask, std::string Name, std::initializer_list<std::string_view> RTName, bool UsedDT = false);
		bool SetRTAsTex2(Device* dev, Resource* Res, std::string_view Name, uint32_t MipSlice, uint32_t PlaneSlice, Dxgi::FormatPixel FP);
		std::optional<uint32_t> FindElement(std::string_view Name);
		std::string_view Name() const noexcept { return m_Name; }
		D3D12_CPU_DESCRIPTOR_HANDLE RTCpuHandleStart() const noexcept
		{
			assert(RTCount() > 0);
			return m_RTHeap->GetCPUDescriptorHandleForHeapStart();
		}
		D3D12_CPU_DESCRIPTOR_HANDLE RTCpuHandle(uint32_t index) const noexcept {
			assert(index < RTCount());
			return D3D12_CPU_DESCRIPTOR_HANDLE{ RTCpuHandleStart().ptr + static_cast<uint64_t>(m_RTOffset)* index };
		}
		D3D12_CPU_DESCRIPTOR_HANDLE DSCpuHandle() const noexcept {
			assert(DSCount() > 0);
			return m_DSHeap->GetCPUDescriptorHandleForHeapStart();
		}
		uint32_t RTCount() const noexcept { return m_RTCount; }
		uint32_t DSCount() const noexcept { return m_DSHeap ? 1 : 0; }
		void SetAsRenderTarget(GraphicCommandList* List) noexcept;
		void ChangeRTStateToRenderTarget(GraphicCommandList* List) noexcept;
		void MarkAsPresent(GraphicCommandList* List) noexcept;
		void MarkAsCommmand(GraphicCommandList* List) noexcept;
	protected:
		RTDSDescriptor() = default;
		DescriptorHeapPtr m_RTHeap;
		uint32_t m_RTOffset;
		uint32_t m_RTCount;
		DescriptorHeapPtr m_DSHeap;
		std::vector<std::tuple<ResourcePtr, ResourceState>> m_RTResources;
		std::tuple<ResourcePtr, ResourceState> m_DSResource;
		std::vector<std::string> m_Mapping;
		std::string m_Name;
		mutable Potato::Tool::atomic_reference_count m_Ref;
	};

	struct RTDSDescriptorTex2D : RTDSDescriptor
	{

	};
	*/

	/*
	enum class DescriptorHeapType
	{
		CBSRUA = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
		Sampler = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
		RT = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
		DS = D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
		Nums = D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES
	};

	inline constexpr D3D12_DESCRIPTOR_HEAP_TYPE operator *(DescriptorHeapType type) noexcept { return static_cast<D3D12_DESCRIPTOR_HEAP_TYPE>(type); }
	inline constexpr DescriptorHeapType operator *(D3D12_DESCRIPTOR_HEAP_TYPE type) noexcept { return static_cast<DescriptorHeapType>(type); }

	enum class DescriptorHeapFlag
	{
		None = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
		ShaderVisible = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
	};

	inline constexpr D3D12_DESCRIPTOR_HEAP_FLAGS operator *(DescriptorHeapFlag type) noexcept { return static_cast<D3D12_DESCRIPTOR_HEAP_FLAGS>(type); }
	inline constexpr DescriptorHeapFlag operator *(D3D12_DESCRIPTOR_HEAP_FLAGS type) noexcept { return static_cast<DescriptorHeapFlag>(type); }
	inline constexpr DescriptorHeapFlag operator |(DescriptorHeapFlag type, DescriptorHeapFlag type2) noexcept { return *(*type | *type2); }

	using DescriptorHeap = ID3D12DescriptorHeap;
	using DescriptorHeapPtr = ComPtr<DescriptorHeap>;

	struct Descriptor {
		DescriptorHeapType Type() const noexcept { return m_Type; }
		D3D12_CPU_DESCRIPTOR_HANDLE operator[](size_t) const noexcept { return D3D12_CPU_DESCRIPTOR_HANDLE{ GetCPUHandleStart().ptr + m_Offset }; }
		D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandleStart() const noexcept { return m_Heap->GetCPUDescriptorHandleForHeapStart(); }
		D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandleStart() const noexcept { return m_Heap->GetGPUDescriptorHandleForHeapStart(); }

		DescriptorHeapType m_Type;
		DescriptorHeapPtr m_Heap;
		size_t m_Offset;
		size_t m_Count;
	};

	

	inline constexpr D3D12_DESCRIPTOR_RANGE_TYPE operator*(DescriptorResourceType input) noexcept {
		if (input == DescriptorResourceType::CB)
			return D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
		else if (input >= DescriptorResourceType::S_Tex1 && input <= DescriptorResourceType::S_SB)
			return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		else
			return   D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	}

	struct DescriptorTableMapping {
		size_t CBVCount = 0;
		size_t SRVCount = 0;
		size_t UAVCount = 0;
		struct Solt {
			DescriptorResourceType Type;
			Dxgi::FormatPixel Format;
			size_t SoltIndex;
		};
		std::map<std::string, Solt> Mapping;
	};

	//using DescTableMappingPtr = ComPtr<DescriptorTableMapping>;

	struct DescriptorTableMappingElement {
		std::string_view Name;
		DescriptorResourceType Type;
		Dxgi::FormatPixel Format = Dxgi::FormatPixel::Unknown;
	};

	void InsertDescriptorTableMapping(DescriptorTableMapping& mapping, const DescriptorTableMappingElement& Element);

	DescriptorTableMapping CreateDescriptorTableMapping(std::initializer_list<DescriptorTableMappingElement> list) {
		DescriptorTableMapping mapping;
		for (auto& ite : list)
			InsertDescriptorTableMapping(mapping, ite);
		return std::move(mapping);
	}
	*/
}