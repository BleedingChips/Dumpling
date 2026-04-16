module;

#include "d3d12.h"
#include <cassert>

#undef interface
#undef max

export module DumplingDx12Resource;

import std;
import Potato;
import DumplingDxDefine;
import DumplingDx12Define;

export namespace Dumpling::Dx12
{
	constexpr std::size_t heap_align = 64 * 1024;
	constexpr std::size_t resource_buffer_align = 64;

	struct HeapIndexed
	{
		ComPtr<ID3D12Heap> heap;
		std::size_t size;
		operator bool() const { return heap; }
	};

	struct ResourceIndexed
	{
		ComPtr<ID3D12Resource> resource;
		std::size_t size;
		operator bool() const { return resource; }
	};

	struct ResourceState
	{
		D3D12_RESOURCE_STATES original = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COMMON;
		std::optional<D3D12_RESOURCE_STATES> target = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COMMON;
	};

	ComPtr<ID3D12Heap> CreatedResourceHeapIndexed(ID3D12Device& device, D3D12_HEAP_TYPE type, std::size_t min_heap_size);

	ResourceIndexed CreatedBufferResourceIndexed(ID3D12Device& device, HeapIndexed& heap, std::size_t require_buffer_size, HeapIndexed* out_heap = nullptr);

	struct ResourceSpanAllocator
	{
		struct ResourceSpan
		{
			ComPtr<ID3D12Resource> resource;
			Potato::Misc::IndexSpan<> sub_span;
			std::size_t identity;
		};

		struct Wrapper
		{
			void AddRef(ResourceSpanAllocator const* ptr) { ptr->AddResourceSpanAllocatorRef(); }
			void SubRef(ResourceSpanAllocator const* ptr) { ptr->AddResourceSpanAllocatorRef(); }
		};

		using Ptr = Potato::Pointer::IntrusivePtr<ResourceSpanAllocator>;

		virtual ResourceSpan AllocateBuffer(std::size_t buffer_size) = 0;
		virtual void DeallocateBuffer(std::size_t identity) = 0;
	protected:
		virtual void AddResourceSpanAllocatorRef() const = 0;
		virtual void SubResourceSpanAllocatorRef() const = 0;
	};


}