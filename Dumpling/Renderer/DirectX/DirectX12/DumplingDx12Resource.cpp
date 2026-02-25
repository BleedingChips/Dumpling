module;
#include "d3d12.h"
#include <dxgi1_6.h>
#include <cassert>
#undef max

module DumplingDx12Resource;


namespace Dumpling::Dx12
{
	HeapIndexed CreatedResourceHeapIndexed(ID3D12Device& device, D3D12_HEAP_TYPE type, std::size_t require_heap_size)
	{
		require_heap_size = Potato::MemLayout::AlignTo(require_heap_size, heap_align);
		ComPtr<ID3D12Heap> heap;
		D3D12_HEAP_DESC heap_desc;
		heap_desc.Alignment = 0;
		heap_desc.Flags = D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE;
		heap_desc.SizeInBytes = require_heap_size;
		heap_desc.Properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY::D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heap_desc.Properties.Type = type;
		auto re = device.CreateHeap(&heap_desc, __uuidof(decltype(heap)::Type), heap.GetPointerVoidAdress());
		assert(SUCCEEDED(re));
		return { std::move(heap), {0, require_heap_size} };
	}

	ResourceIndexed CreatedBufferResourceIndexed(ID3D12Device& device, HeapIndexed heap, std::size_t require_buffer_size, HeapIndexed* out_heap = nullptr)
	{
		if (!heap)
			return {};
		require_buffer_size = Potato::MemLayout::AlignTo(require_buffer_size, heap_align);

		heap.index_span = {
			Potato::MemLayout::AlignTo(heap.index_span.Begin(), heap_align),
			heap.index_span.End()
		};

		if (!heap.index_span)
			return {};
		if (heap.index_span.Size() < require_buffer_size)
			return {};

		D3D12_RESOURCE_DESC resource_desc;
		resource_desc.Dimension = D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_BUFFER;
		resource_desc.Alignment = 0;
		resource_desc.Width = require_buffer_size;
		resource_desc.Height = 1;
		resource_desc.DepthOrArraySize = 1;
		resource_desc.MipLevels = 1;
		resource_desc.Format = DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;
		resource_desc.SampleDesc.Count = 1;
		resource_desc.SampleDesc.Quality = 0;
		resource_desc.Layout = D3D12_TEXTURE_LAYOUT::D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		resource_desc.Flags = D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_NONE;
		D3D12_CLEAR_VALUE clear_value;

		ComPtr<ID3D12Resource> resource;

		auto re = device.CreatePlacedResource(
			heap.heap,
			0,
			&resource_desc,
			(
				heap.heap->GetDesc().Properties.Type == D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_UPLOAD ?
				D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_GENERIC_READ :
				D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COMMON
				),
			nullptr,
			__uuidof(decltype(resource)::Type), resource.GetPointerVoidAdress()
		);

		if (resource)
		{
			assert(SUCCEEDED(re));
			return { resource, {0, require_buffer_size} };
		}
		return {};
	}

}
