#pragma once
#include <d3d12.h>
namespace Dumpling::Dx12
{
	namespace Enum
	{
		enum class CommandListType
		{
			Direct = D3D12_COMMAND_LIST_TYPE_DIRECT,
			Copy = D3D12_COMMAND_LIST_TYPE_COPY,
			Compute = D3D12_COMMAND_LIST_TYPE_COMPUTE,
			Bundle = D3D12_COMMAND_LIST_TYPE_BUNDLE,
			VideoEncode = D3D12_COMMAND_LIST_TYPE_VIDEO_DECODE,
			VideoProcess = D3D12_COMMAND_LIST_TYPE_VIDEO_PROCESS
		};

		inline constexpr D3D12_COMMAND_LIST_TYPE operator *(CommandListType type) noexcept { return static_cast<D3D12_COMMAND_LIST_TYPE>(type); }
		inline constexpr CommandListType operator *(D3D12_COMMAND_LIST_TYPE type) noexcept { return static_cast<CommandListType>(type); }

		enum class CommandQueuePriority {
			Normal = D3D12_COMMAND_QUEUE_PRIORITY::D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
			High = D3D12_COMMAND_QUEUE_PRIORITY::D3D12_COMMAND_QUEUE_PRIORITY_HIGH,
			GlobalRealTime = D3D12_COMMAND_QUEUE_PRIORITY::D3D12_COMMAND_QUEUE_PRIORITY_GLOBAL_REALTIME
		};

		inline constexpr D3D12_COMMAND_QUEUE_PRIORITY operator *(CommandQueuePriority type) noexcept { return static_cast<D3D12_COMMAND_QUEUE_PRIORITY>(type); }
		inline constexpr CommandQueuePriority operator *(D3D12_COMMAND_QUEUE_PRIORITY type) noexcept { return static_cast<CommandQueuePriority>(type); }

		enum class FenseFlag
		{
			None = D3D12_FENCE_FLAG_NONE,
			Shared = D3D12_FENCE_FLAG_SHARED,
			SharedCrossAdapter = D3D12_FENCE_FLAG_SHARED_CROSS_ADAPTER,
			NonMonitored = D3D12_FENCE_FLAG_NON_MONITORED,
		};

		inline constexpr D3D12_FENCE_FLAGS operator *(FenseFlag type) noexcept { return static_cast<D3D12_FENCE_FLAGS>(type); }
		inline constexpr FenseFlag operator *(D3D12_FENCE_FLAGS type) noexcept { return static_cast<FenseFlag>(type); }
		inline constexpr FenseFlag operator |(FenseFlag type, FenseFlag type2) noexcept { return *(*type | *type2); }

		enum class ResourceState
		{
			Common = D3D12_RESOURCE_STATE_COMMON,
			VertexConstBuffer = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
			IndexBuffer = D3D12_RESOURCE_STATE_INDEX_BUFFER,
			RenderTarget = D3D12_RESOURCE_STATE_RENDER_TARGET,
			UnorderedAccess = D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
			DepthWrite = D3D12_RESOURCE_STATE_DEPTH_WRITE,
			DepthRead = D3D12_RESOURCE_STATE_DEPTH_READ,
			NonPixelResource = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
			PixelResource = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			StreamOut = D3D12_RESOURCE_STATE_STREAM_OUT,
			IndirectArgument = D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT,
			CopyDest = D3D12_RESOURCE_STATE_COPY_DEST,
			CopySource = D3D12_RESOURCE_STATE_COPY_SOURCE,
			ResolveDest = D3D12_RESOURCE_STATE_RESOLVE_DEST,
			ResolveSource = D3D12_RESOURCE_STATE_RESOLVE_SOURCE,
			RayTracingAccelerationStructure = D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
			GenericRead = D3D12_RESOURCE_STATE_GENERIC_READ,
			Present = D3D12_RESOURCE_STATE_PRESENT,
			Predication = D3D12_RESOURCE_STATE_PREDICATION,
			VideoDecodeRead = D3D12_RESOURCE_STATE_VIDEO_DECODE_READ,
			VideoDecodeWrite = D3D12_RESOURCE_STATE_VIDEO_DECODE_WRITE,
			VideoProcessRead = D3D12_RESOURCE_STATE_VIDEO_PROCESS_READ,
			VideoProcessWrite = D3D12_RESOURCE_STATE_VIDEO_PROCESS_WRITE,
			VideoEncodeRead = D3D12_RESOURCE_STATE_VIDEO_ENCODE_READ,
			VideoEncodeWrite = D3D12_RESOURCE_STATE_VIDEO_ENCODE_WRITE,
		};

		inline constexpr D3D12_RESOURCE_STATES operator *(ResourceState type) noexcept { return static_cast<D3D12_RESOURCE_STATES>(type); }
		inline constexpr ResourceState operator *(D3D12_RESOURCE_STATES type) noexcept { return static_cast<ResourceState>(type); }
		inline constexpr ResourceState operator |(ResourceState type, ResourceState type2) noexcept { return *(*type | *type2); }

		enum class CommandQueueFlag 
		{
			Non = D3D12_COMMAND_QUEUE_FLAG_NONE,
			DisableGpuTimeOut = D3D12_COMMAND_QUEUE_FLAG_DISABLE_GPU_TIMEOUT
		};

		inline constexpr D3D12_COMMAND_QUEUE_FLAGS operator *(CommandQueueFlag type) noexcept { return static_cast<D3D12_COMMAND_QUEUE_FLAGS>(type); }
		inline constexpr CommandQueueFlag operator *(D3D12_COMMAND_QUEUE_FLAGS type) noexcept { return static_cast<CommandQueueFlag>(type); }

		enum class DescriptorType
		{
			Resource = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
			Sampler = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
			RenderTarget = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
			DepthStencil = D3D12_DESCRIPTOR_HEAP_TYPE_DSV
		};

		inline constexpr D3D12_DESCRIPTOR_HEAP_TYPE operator *(DescriptorType type) noexcept { return static_cast<D3D12_DESCRIPTOR_HEAP_TYPE>(type); }
		inline constexpr DescriptorType operator *(D3D12_DESCRIPTOR_HEAP_TYPE type) noexcept { return static_cast<DescriptorType>(type); }
	}
	using namespace Enum;
}

