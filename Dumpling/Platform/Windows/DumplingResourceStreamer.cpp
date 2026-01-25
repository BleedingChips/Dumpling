module;
#include "d3d12.h"
#include <dxgi1_6.h>
#include <cassert>
#undef max

module DumplingResourceStreamer;


namespace Dumpling
{
	bool ResourceStreamer::Init(ComPtr<ID3D12Device> in_device)
	{
		if (*this || !in_device)
			return false;

		ComPtr<ID3D12CommandQueue> command_queue;
		D3D12_COMMAND_QUEUE_DESC desc{
			D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_COPY,
			D3D12_COMMAND_QUEUE_PRIORITY::D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
			D3D12_COMMAND_QUEUE_FLAGS::D3D12_COMMAND_QUEUE_FLAG_NONE,
		0
		};

		auto result = in_device->CreateCommandQueue(
			&desc, __uuidof(decltype(command_queue)::Type), command_queue.GetPointerVoidAdress()
		);
		if (SUCCEEDED(result))
		{
			ComPtr<ID3D12Fence> fence;
			auto re = in_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, __uuidof(decltype(fence)::Type), fence.GetPointerVoidAdress());
			if (SUCCEEDED(re))
			{
				this->device = std::move(in_device);
				this->command_queue = std::move(command_queue);
				this->fence = std::move(fence);
				return true;
			}
		}
		return {};
	}

	std::uint64_t ResourceStreamer::Commited(PassStreamer& request)
	{
		if (!request)
			return std::numeric_limits<std::uint64_t>::max();

		request.PreCommited();

		auto list = std::move(request.commands);
		auto allo = std::move(request.allocator);
		auto heap = std::move(request.upload_heap);
		std::size_t heap_size = request.heap_size;
		if (request.using_heap_size == 0)
		{
			idle_upload_heap.emplace_back(
				std::move(heap),
				heap_size
			);
		}

		ID3D12CommandList* temporary_list = list.GetPointer();

		command_queue->ExecuteCommandLists(1, &temporary_list);
		command_queue->Signal(fence.GetPointer(), current_flush_frame);

		idle_command_list.emplace_back(std::move(list));
		waitting_object.emplace_back(current_flush_frame, std::move(allo));
		for (auto& ite : request.using_resource)
		{
			waitting_object.emplace_back(
				current_flush_frame,
				std::move(ite)
			);
		}
		request.using_resource.clear();
		if (heap)
		{
			waitting_object.emplace_back(
				current_flush_frame,
				std::move(heap)
			);
		}
		request.PosCommited();
		return current_flush_frame++;
	}

	void PassStreamer::PreCommited() 
	{ 
		commands->Close(); 
	}

	void PassStreamer::PosCommited() 
	{
		commands.Reset();
		device.Reset(); 
		allocator.Reset();
		upload_heap.Reset();
		using_resource.clear();
	}

	ComPtr<ID3D12Resource> PassStreamer::CreateVertexBuffer(void const* buffer, std::size_t size, ID3D12Heap& heap, std::size_t heap_offset)
	{
		size = Potato::MemLayout::AlignTo(size, sizeof(float) * 16);
		if (size > 0 && using_heap_size + size <= heap_size)
		{
			D3D12_RESOURCE_DESC resource_desc;
			resource_desc.Dimension = D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_BUFFER;
			resource_desc.Alignment = 0;
			resource_desc.Width = size;
			resource_desc.Height = 1;
			resource_desc.DepthOrArraySize = 1;
			resource_desc.MipLevels = 1;
			resource_desc.Format = DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;
			resource_desc.SampleDesc.Count = 1;
			resource_desc.SampleDesc.Quality = 0;
			resource_desc.Layout = D3D12_TEXTURE_LAYOUT::D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
			resource_desc.Flags = D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_NONE;
			D3D12_CLEAR_VALUE clear_value;

			ComPtr<ID3D12Resource> upload_resource;

			device->CreatePlacedResource(
				upload_heap,
				using_heap_size,
				&resource_desc,
				D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				__uuidof(decltype(upload_resource)::Type), upload_resource.GetPointerVoidAdress()
			);

			if (upload_heap)
			{
				ComPtr<ID3D12Resource> default_resource;

				device->CreatePlacedResource(
					&heap,
					using_heap_size,
					&resource_desc,
					D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COMMON,
					nullptr,
					__uuidof(decltype(default_resource)::Type), default_resource.GetPointerVoidAdress()
				);

				if (default_resource)
				{
					D3D12_RANGE range{
						0,
						size
					};
					void* target_buffer = nullptr;
					upload_resource->Map(0, &range, &target_buffer);
					std::memcpy(target_buffer, buffer, size);
					upload_resource->Unmap(0, &range);
					using_heap_size += size;

					std::array<D3D12_RESOURCE_BARRIER, 2> barrier;
					barrier[0].Type = D3D12_RESOURCE_BARRIER_TYPE::D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
					barrier[0].Flags = D3D12_RESOURCE_BARRIER_FLAGS::D3D12_RESOURCE_BARRIER_FLAG_NONE;
					barrier[0].Transition = D3D12_RESOURCE_TRANSITION_BARRIER{
						upload_resource,
						0,
						D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_GENERIC_READ,
						D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COPY_SOURCE
					};
					barrier[1].Type = D3D12_RESOURCE_BARRIER_TYPE::D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
					barrier[1].Flags = D3D12_RESOURCE_BARRIER_FLAGS::D3D12_RESOURCE_BARRIER_FLAG_NONE;
					barrier[1].Transition = D3D12_RESOURCE_TRANSITION_BARRIER{
						default_resource,
						0,
						D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COMMON,
						D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COPY_DEST
					};

					commands->ResourceBarrier(1, barrier.data() + 1);
					commands->CopyBufferRegion(
						default_resource,
						0,
						upload_resource,
						0,
						size
					);
					std::swap(barrier[0].Transition.StateAfter, barrier[0].Transition.StateBefore);
					std::swap(barrier[1].Transition.StateAfter, barrier[1].Transition.StateBefore);
					commands->ResourceBarrier(1, barrier.data() + 1);
					using_resource.emplace_back(upload_resource);
					using_resource.emplace_back(default_resource);
					return default_resource;
				}
			}
		}
		return {};
	}

	ComPtr<ID3D12GraphicsCommandList> ResourceStreamer::GetCommandList(ID3D12CommandAllocator& allocator)
	{
		if (!idle_command_list.empty())
		{
			auto current_command = std::move(*idle_command_list.rbegin());
			idle_command_list.pop_back();
			if (current_command)
			{
				current_command->Reset(&allocator, nullptr);
			}
		}

		ComPtr<ID3D12GraphicsCommandList> current_command;

		if (!current_command)
		{
			auto re = device->CreateCommandList(
				0, D3D12_COMMAND_LIST_TYPE_COPY, &allocator, nullptr,
				__uuidof(decltype(current_command)::Type), current_command.GetPointerVoidAdress()
			);
			assert(SUCCEEDED(re));
		}

		return current_command;
	}

	ComPtr<ID3D12CommandAllocator> ResourceStreamer::GetCommandAllocator()
	{
		if (!idle_allocator.empty())
		{
			auto current_allocator = std::move(*idle_allocator.rbegin());
			idle_allocator.pop_back();
			assert(current_allocator);
			current_allocator->Reset();
			return current_allocator;
		}

		ComPtr<ID3D12CommandAllocator> allocator;

		auto re = device->CreateCommandAllocator(
			D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_COPY,
			__uuidof(decltype(allocator)::Type), allocator.GetPointerVoidAdress()
		);
		assert(SUCCEEDED(re));

		return allocator;
	}

	std::tuple<ComPtr<ID3D12Heap>, std::size_t> ResourceStreamer::GetUploadHeap(std::size_t heap_size)
	{
		heap_size = Potato::MemLayout::AlignTo(heap_size, 1024);
		for (auto ite = idle_upload_heap.begin(); ite != idle_upload_heap.end(); ++ite)
		{
			if (ite->heap_size >= heap_size)
			{
				auto current_heap = std::move(*ite);
				idle_upload_heap.erase(ite);
				return { current_heap.heap, current_heap.heap_size };
				break;
			}
		}

		ComPtr<ID3D12Heap> current_heap;

		D3D12_HEAP_DESC heap_desc;
		heap_desc.Alignment = 0;
		heap_desc.Flags = D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE;
		heap_desc.SizeInBytes = heap_size;
		heap_desc.Properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY::D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heap_desc.Properties.Type = D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_UPLOAD;
		heap_desc.Properties.CreationNodeMask = 1;
		heap_desc.Properties.VisibleNodeMask = 1;
		heap_desc.Properties.MemoryPoolPreference = D3D12_MEMORY_POOL::D3D12_MEMORY_POOL_UNKNOWN;

		auto re = device->CreateHeap(
			&heap_desc,
			__uuidof(decltype(current_heap)::Type), current_heap.GetPointerVoidAdress()
		);

		return { std::move(current_heap), heap_size };
	}

	bool ResourceStreamer::PopRequester(PassStreamer& request, PassStreamer::Config config)
	{
		if (request)
			return false;

		bool done = false;

		ComPtr<ID3D12CommandAllocator> current_allocator;
		ComPtr<ID3D12GraphicsCommandList> current_command_list;
		ComPtr<ID3D12Heap> current_heap;
		std::size_t heap_size = 0;

		current_allocator = GetCommandAllocator();

		if (current_allocator)
		{
			current_command_list = GetCommandList(*current_allocator);
		}

		if (current_command_list)
		{
			std::tie(current_heap, heap_size) = GetUploadHeap(config.max_upload_heap_sie);
		}

		if (current_allocator && current_command_list && current_heap)
		{
			request.commands = std::move(current_command_list);
			request.allocator = std::move(current_allocator);
			request.device = device;
			request.upload_heap = std::move(current_heap);
			request.heap_size = heap_size;
			request.using_heap_size = 0;
			return true;
		}

		if (current_allocator)
		{
			idle_allocator.emplace_back(std::move(current_allocator));
		}

		if (current_command_list)
		{
			idle_command_list.emplace_back(std::move(current_command_list));
		}

		if (current_heap)
		{
			idle_upload_heap.emplace_back(std::move(current_heap), heap_size);
		}

		return false;
	}

	ComPtr<ID3D12Heap> ResourceStreamer::CreateDefaultHeap(std::size_t heap_size)
	{
		heap_size = Potato::MemLayout::AlignTo(heap_size, 64);
		ComPtr<ID3D12Heap> heap;
		D3D12_HEAP_DESC heap_desc;
		heap_desc.Alignment = 0;
		heap_desc.Flags = D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE;
		heap_desc.SizeInBytes = heap_size;
		heap_desc.Properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY::D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heap_desc.Properties.Type = D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_DEFAULT;
		heap_desc.Properties.CreationNodeMask = 1;
		heap_desc.Properties.VisibleNodeMask = 1;
		heap_desc.Properties.MemoryPoolPreference = D3D12_MEMORY_POOL::D3D12_MEMORY_POOL_UNKNOWN;

		auto re = device->CreateHeap(
			&heap_desc,
			__uuidof(decltype(heap)::Type), heap.GetPointerVoidAdress()
		);

		if (SUCCEEDED(re))
		{
			return heap;
		}
		return {};
	}

	std::uint64_t ResourceStreamer::Flush()
	{
		auto current_version = fence->GetCompletedValue();
		if (current_version != last_flush_version)
		{
			last_flush_version = current_version;

			bool change = false;
			auto begin = waitting_object.begin();

			for (;begin != waitting_object.end();++begin)
			{
				if (begin->version <= current_version)
				{
					if (std::holds_alternative<ComPtr<ID3D12CommandAllocator>>(begin->object))
					{
						idle_allocator.emplace_back(
							std::move(
								std::get<ComPtr<ID3D12CommandAllocator>>(
									begin->object
								)
							)
						);
					}
					else if (std::holds_alternative<ComPtr<ID3D12Heap>>(begin->object))
					{
						ComPtr<ID3D12Heap> heap = std::move(std::get<ComPtr<ID3D12Heap>>(begin->object));
						D3D12_HEAP_DESC heap_desc = heap->GetDesc();
						idle_upload_heap.emplace_back(std::move(heap), heap_desc.SizeInBytes);
					}
				}
				else {
					break;
				}
			}
			waitting_object.erase(
				waitting_object.begin(),
				begin
			);

		}
		return current_version;
	}
}
