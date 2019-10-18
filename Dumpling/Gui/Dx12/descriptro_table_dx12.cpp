#include "descriptor_table_dx12.h"
#include <set>
#include "define_dx12.h"
namespace Dumpling::Dx12
{

	DescResCategory TypeToIndex(DescResType Type)
	{
		if (Type == DescResType::CB)
			return DescResCategory::CBV;
		else if (Type >= DescResType::S_Tex1 && Type <= DescResType::S_SB)
			return DescResCategory::SRV;
		else if (Type >= DescResType::U_Tex1 && Type <= DescResType::U_Tex3)
			return DescResCategory::UAV;
		else if (Type == DescResType::Sampler)
			return DescResCategory::Sampler;
		else {
			assert(false);
			return DescResCategory::Sampler;
		}
	}

	void DescriptorMapping::AddRef() const noexcept
	{
		m_Ref.add_ref();
	}

	void DescriptorMapping::Release() const noexcept
	{
		if (m_Ref.sub_ref())
			delete this;
	}

	DescriptorMappingPtr DescriptorMapping::Create(std::string Name, std::initializer_list<DescriptorElement> Mapping)
	{
		std::array<size_t, *DescResCategory::Num> Count;
		for (auto& ite : Count) ite = 0;
		for (auto& ite : Mapping)
		{
			auto Categ= TypeToIndex(ite.Type);
			++Count[*Categ];
		}
		std::array<size_t, *DescResCategory::Num> Shift;
		Shift[0] = 0;
		for (size_t i = 1; i < *DescResCategory::Num; ++i)
			Shift[i] = Shift[i-1] + Count[i - 1];
		uint32_t ResourceCount = static_cast<uint32_t>(Shift[*DescResCategory::Sampler]);
		uint32_t SamplerCount = static_cast<uint32_t>(Count[*DescResCategory::Sampler]);

		std::vector<std::tuple<std::string, DescResType, Dxgi::FormatPixel>> ResultArray;
		ResultArray.resize(ResourceCount + SamplerCount);
		std::array<size_t, *DescResCategory::Num> Index;
		for (auto& ite : Index) ite = 0;
		for (auto& ite : Mapping)
		{
			auto Categ = TypeToIndex(ite.Type);
			ResultArray[Shift[*Categ] + Index[*Categ]++] = { std::string{ite.Name}, ite.Type, ite.Format };
		}
		DescriptorMappingPtr Result = new DescriptorMapping{};
		Result->m_Name = std::move(Name);
		Result->m_Elements = std::move(ResultArray);
		Result->m_ElementsOffset = Shift;
		Result->m_ElenmentCount = Count;
		Result->m_ResourceCount = ResourceCount;
		Result->m_SamplerCount = SamplerCount;
		return Result;
	}

	std::optional<std::tuple<DescResType, uint32_t, Dxgi::FormatPixel>> DescriptorMapping::FindElement(DescResCategory ResType, std::string_view view) const
	{
		auto begin = m_Elements.begin() + m_ElementsOffset[*ResType];
		auto end = begin + m_ElenmentCount[*ResType];
		uint32_t index = 0;
		for (; begin != end; ++begin, ++index)
		{
			auto& [Name, Type, Format] = *begin;
			if (Name == view)
			{
				return std::tuple<DescResType, uint32_t, Dxgi::FormatPixel>{ Type, index, Format };
			}
		}
		return std::nullopt;
	}

	DescriptorPtr Descriptor::Create(Device& Dev, uint32_t NodeMask, DescriptorMapping* Mapping)
	{
		if (Mapping != nullptr)
		{
			ComPtr<ID3D12DescriptorHeap> Res;
			if (Mapping->ResourceCount() != 0)
			{
				D3D12_DESCRIPTOR_HEAP_DESC Desc{ D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, Mapping->ResourceCount(), D3D12_DESCRIPTOR_HEAP_FLAG_NONE, NodeMask };
				HRESULT re = Dev.CreateDescriptorHeap(&Desc, __uuidof(ID3D12DescriptorHeap), Res(Win32::VoidT{}));
				if (!SUCCEEDED(re))
					return {};
			}
			uint32_t ResOffset = Dev.GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			
			ComPtr<ID3D12DescriptorHeap> Sam;
			if (Mapping->ResourceCount() != 0)
			{
				D3D12_DESCRIPTOR_HEAP_DESC Desc{ D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, Mapping->SamplerCount(), D3D12_DESCRIPTOR_HEAP_FLAG_NONE, NodeMask };
				HRESULT re = Dev.CreateDescriptorHeap(&Desc, __uuidof(ID3D12DescriptorHeap), Res(Win32::VoidT{}));
				if (!SUCCEEDED(re))
					return {};
			}
			if (Res || Sam)
			{
				uint32_t SamOffset = Dev.GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
				DescriptorPtr Ptr = new Descriptor{};
				Ptr->m_ResHeap = std::move(Res);
				Ptr->m_SamHeap = std::move(Sam);
				Ptr->m_ResOffset = ResOffset;
				Ptr->m_SamOffset = SamOffset;
				Ptr->m_Mapping = Mapping;
				return Ptr;
			}
		}
		return {};
	}

	void Descriptor::AddRef() const noexcept { m_Ref.add_ref(); }
	void Descriptor::Release() const noexcept { 
		if (m_Ref.sub_ref()) {
			this->~Descriptor();
			delete[](reinterpret_cast<const std::byte*>(this));
		} 
	}

	void RTDSDescriptor::AddRef() const noexcept
	{
		m_Ref.add_ref();
	}
	void RTDSDescriptor::Release() const noexcept
	{
		if (m_Ref.sub_ref())
			delete this;
	}

	std::optional<uint32_t> RTDSDescriptor::FindElement(std::string_view Name)
	{
		uint32_t index = 0;
		for (auto& ite : m_Mapping)
		{
			if (ite == Name)
				return index;
			++index;
		}
		return std::nullopt;
	}

	RTDSDescriptorPtr RTDSDescriptor::Create(Device* Dev, uint32_t NodeMask, std::string Name, std::initializer_list<std::string_view> RTName, bool UsedDT)
	{
		std::vector<std::string> AllName;
		AllName.reserve(RTName.size());
		for (auto& ite : RTName) AllName.push_back(std::string{ ite });
		uint32_t RTCount = static_cast<uint32_t>(AllName.size());
		ComPtr<ID3D12DescriptorHeap> RTHeap;
		if (RTCount != 0)
		{
			D3D12_DESCRIPTOR_HEAP_DESC Desc{ D3D12_DESCRIPTOR_HEAP_TYPE_RTV, RTCount, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, NodeMask };
			HRESULT re = Dev->CreateDescriptorHeap(&Desc, __uuidof(ID3D12DescriptorHeap), RTHeap(Win32::VoidT{}));
			if (!SUCCEEDED(re))
				return {};
		}
		ComPtr<ID3D12DescriptorHeap> DTHeap;
		if (UsedDT)
		{
			D3D12_DESCRIPTOR_HEAP_DESC Desc{ D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, NodeMask };
			HRESULT re = Dev->CreateDescriptorHeap(&Desc, __uuidof(ID3D12DescriptorHeap), DTHeap(Win32::VoidT{}));
			if (!SUCCEEDED(re))
				return {};
		}
		if (RTHeap || DTHeap)
		{
			uint32_t RTOffset = Dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			RTDSDescriptorPtr Ptr = new RTDSDescriptor{};
			Ptr->m_Resource.resize(RTCount + (DTHeap ? 1 : 0));
			Ptr->m_RTHeap = std::move(RTHeap);
			Ptr->m_DTHeap = std::move(DTHeap);
			Ptr->m_RTOffset = RTOffset;
			Ptr->m_Mapping = std::move(AllName);
			Ptr->m_Name = std::move(Name);
			Ptr->m_RTCount = RTCount;
			return Ptr;
		}
		return {};
	}

	bool RTDSDescriptor::SetRTAsTex2(Device* dev, Resource* Res, std::string_view Name, uint32_t MipSlice, uint32_t PlaneSlice, Dxgi::FormatPixel FP)
	{
		auto Index = FindElement(Name);
		if (Index.has_value())
		{
			D3D12_RENDER_TARGET_VIEW_DESC Des{ *FP, D3D12_RTV_DIMENSION_TEXTURE2D };
			Des.Texture2D = D3D12_TEX2D_RTV{ MipSlice, PlaneSlice };
			dev->CreateRenderTargetView(Res, &Des, RTCpuHandle(*Index));
			m_Resource[*Index] = {Res, ResourceState::Common};
			return true;
		}
		return false;
	}









	/*
	void InsertDescriptorTableMapping(DescriptorTableMapping& mapping, const DescriptorTableMappingElement& Element)
	{
		D3D12_DESCRIPTOR_RANGE_TYPE type = *Element.Type;
		size_t Index;
		switch (type)
		{
		case D3D12_DESCRIPTOR_RANGE_TYPE_CBV:
			Index = mapping.CBVCount++;
			break;
		case D3D12_DESCRIPTOR_RANGE_TYPE_SRV:
			Index = mapping.SRVCount++;
			break;
		case D3D12_DESCRIPTOR_RANGE_TYPE_UAV:
			Index = mapping.UAVCount++;
			break;
		}
		std::string Name = std::string(Element.Name);
		auto find = mapping.Mapping.find(Name);
		if (find != mapping.Mapping.end())
		{
			D3D12_DESCRIPTOR_RANGE_TYPE OldType = *find->second.Type;
			size_t OldIndex = find->second.SoltIndex;
			switch (type)
			{
			case D3D12_DESCRIPTOR_RANGE_TYPE_CBV:
				--mapping.CBVCount;
				break;
			case D3D12_DESCRIPTOR_RANGE_TYPE_SRV:
				--mapping.SRVCount;
				break;
			case D3D12_DESCRIPTOR_RANGE_TYPE_UAV:
				--mapping.UAVCount;
				break;
			}
			for (auto Ite2 = mapping.Mapping.begin(); Ite2 != mapping.Mapping.end(); ++Ite2)
			{
				if (Ite2 != find && *Ite2->second.Type == OldType && Ite2->second.SoltIndex > OldIndex)
					--Ite2->second.SoltIndex;
			}
			find->second = DescriptorTableMapping::Solt{Element.Type, Element.Format, Index};
		}
		else {
			mapping.Mapping.insert({std::move(Name), DescriptorTableMapping::Solt{Element.Type, Element.Format, Index} });
		}
	}
	*/
}