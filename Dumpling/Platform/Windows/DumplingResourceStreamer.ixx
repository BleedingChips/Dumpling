module;

#include "d3d12.h"
#include <cassert>

#undef interface
#undef max

export module DumplingResourceStreamer;

import std;
import Potato;
import DumplingPlatform;
import DumplingRendererTypes;

export namespace Dumpling
{
	struct PassStreamer
	{
		struct Config
		{
			std::size_t max_upload_heap_sie = 64 * 1024;
		};

		PassStreamer() = default;
		~PassStreamer()
		{
			assert(!*this);
		}
		operator bool() const { return commands && allocator && device; }

		ComPtr<ID3D12Resource> CreateVertexBuffer(void const* buffer, std::size_t size, ID3D12Heap& heap, std::size_t heap_offset = 0);

	protected:

		void PreCommited();
		void PosCommited();
		ComPtr<ID3D12GraphicsCommandList> commands;
		ComPtr<ID3D12CommandAllocator> allocator;
		ComPtr<ID3D12Device> device;

		ComPtr<ID3D12Heap> upload_heap;
		std::size_t heap_size = 0;
		std::size_t using_heap_size = 0;

		std::pmr::vector<ComPtr<ID3D12Resource>> using_resource;

		friend struct ResourceStreamer;
	};

	struct ResourceStreamer
	{
		bool Init(ComPtr<ID3D12Device> device);
		std::uint64_t Commited(PassStreamer& request);
		bool PopRequester(PassStreamer& request, PassStreamer::Config config);
		operator bool() const { return device && fence && command_queue; }
		std::uint64_t Flush();
		bool TryFlushTo(std::uint64_t target_fence_value) { return Flush() >= target_fence_value; }
		ComPtr<ID3D12Heap> CreateDefaultHeap(std::size_t heap_size);

	protected:

		struct HeapDescription
		{
			ComPtr<ID3D12Heap> heap;
			std::size_t heap_size;
		};

		struct VersionedObject
		{
			std::uint64_t version;
			std::variant<
				ComPtr<ID3D12CommandAllocator>,
				ComPtr<ID3D12Resource>,
				ComPtr<ID3D12Heap>
			> object;
		};

		ComPtr<ID3D12Device> device;
		ComPtr<ID3D12Fence> fence;
		std::size_t current_flush_frame = 2;
		ComPtr<ID3D12CommandQueue> command_queue;
		std::pmr::vector<ComPtr<ID3D12GraphicsCommandList>> idle_command_list;
		std::pmr::vector<ComPtr<ID3D12CommandAllocator>> idle_allocator;
		std::pmr::vector<HeapDescription> idle_upload_heap;
		std::pmr::vector<VersionedObject> waitting_object;
		std::uint64_t last_flush_version = 1;
		ComPtr<ID3D12GraphicsCommandList> GetCommandList(ID3D12CommandAllocator& allocator);
		ComPtr<ID3D12CommandAllocator> GetCommandAllocator();
		std::tuple<ComPtr<ID3D12Heap>, std::size_t> GetUploadHeap(std::size_t size);
	};
}