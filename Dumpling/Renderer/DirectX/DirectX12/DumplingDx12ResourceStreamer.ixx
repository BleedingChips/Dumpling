module;

#include "d3d12.h"
#include <cassert>

#undef interface
#undef max

export module DumplingDx12ResourceStreamer;

import std;
import Potato;
import DumplingDxDefine;
import DumplingDx12Define;

export namespace Dumpling::Dx12
{
	constexpr std::size_t heap_align = 64 * 1024;
	constexpr std::size_t resource_buffer_align = 64;

	struct ResourceState
	{
		D3D12_RESOURCE_STATES original = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COMMON;
		std::optional<D3D12_RESOURCE_STATES> target = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COMMON;
	};

	struct PassStreamer
	{
		struct Config
		{
			std::size_t buffer_size = 0;
		};

		PassStreamer() = default;
		~PassStreamer()
		{
			assert(!*this);
		}
		operator bool() const { return commands && allocator && device; }

		std::optional<D3D12_RESOURCE_STATES> UploadBufferResource(void const* buffer, std::size_t size, ID3D12Resource& target_resource, std::size_t target_offset = 0, ResourceState state = {});

		std::optional<D3D12_RESOURCE_STATES> UploadBufferResource(std::span<std::byte> buffer, ID3D12Resource& target_resource, std::size_t target_offset = 0, ResourceState state = {}) { return UploadBufferResource(buffer.data(), buffer.size(), target_resource, target_offset, state); }
		//ComPtr<ID3D12Resource> CreateVertexBuffer(void const* buffer, std::size_t size, ID3D12Heap& heap, std::size_t heap_offset = 0);

	protected:

		void PreCommited();
		void PosCommited();
		ComPtr<ID3D12GraphicsCommandList> commands;
		ComPtr<ID3D12CommandAllocator> allocator;
		ComPtr<ID3D12Device> device;

		struct ResourceDescription
		{
			ComPtr<ID3D12Resource> resource;
			std::size_t max_size = 0;
			std::size_t using_size = 0;
			void Reset() { resource.Reset(); max_size = 0; using_size = 0; }
			operator bool() const { return resource; }
		};

		ResourceDescription upload_resource;
		ComPtr<ID3D12Heap> upload_heap;

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
		std::tuple<ComPtr<ID3D12Resource>, std::size_t> CreateBufferResource(ID3D12Heap& heap, std::size_t buffer_size);

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