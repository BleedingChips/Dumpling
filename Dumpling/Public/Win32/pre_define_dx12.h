#pragma once
#include <d3d12.h>
#include <assert.h>
#include "dxgi.h"
namespace Dumpling::Dx12
{
	using Dxgi::ComPtr;
	using Dxgi::VoidT;

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

	using ResourceDesc = D3D12_RESOURCE_DESC;
	using HeapProprties = D3D12_HEAP_PROPERTIES;

	using DescriptorRange = D3D12_DESCRIPTOR_RANGE;

	using RootDescriptorTable = D3D12_ROOT_DESCRIPTOR_TABLE;
	using RootParameter = D3D12_ROOT_PARAMETER;
	using RootSignatureDesc = D3D12_ROOT_SIGNATURE_DESC;

	using RootSignature = ID3D12RootSignature;
	using RootSignaturePtr = ComPtr<RootSignature>;

	using PipelineState = ID3D12PipelineState;
	using PipelineStatePtr = ID3D12PipelineState;

	using Blob = ID3DBlob;
	using BlobPtr = ComPtr<Blob>;

	/*
	struct DescriptorHandleIncrementSize
	{
		uint32_t CBV_SRV_UAV;
		uint32_t Sampler;
		uint32_t RTV;
		uint32_t DSV;
		D3D12_CPU_DESCRIPTOR_HANDLE CBVOffset(DescriptorHeap* start, uint32_t index) const noexcept { 
			assert(start != nullptr); 
			return { start->GetCPUDescriptorHandleForHeapStart().ptr + CBV_SRV_UAV * index };
		}
		D3D12_CPU_DESCRIPTOR_HANDLE SRVOffset(DescriptorHeap* start, uint32_t index) const noexcept {
			assert(start != nullptr);
			return { start->GetCPUDescriptorHandleForHeapStart().ptr + CBV_SRV_UAV * index };
		}
		D3D12_CPU_DESCRIPTOR_HANDLE UAVOffset(DescriptorHeap* start, uint32_t index) const noexcept {
			assert(start != nullptr);
			return { start->GetCPUDescriptorHandleForHeapStart().ptr + CBV_SRV_UAV * index };
		}
		D3D12_CPU_DESCRIPTOR_HANDLE SamplerOffset(DescriptorHeap* start, uint32_t index) const noexcept {
			assert(start != nullptr);
			return { start->GetCPUDescriptorHandleForHeapStart().ptr + Sampler * index };
		}
		D3D12_CPU_DESCRIPTOR_HANDLE RTVOffset(DescriptorHeap* start, uint32_t index) const noexcept {
			assert(start != nullptr);
			return { start->GetCPUDescriptorHandleForHeapStart().ptr + RTV * index };
		}
		D3D12_CPU_DESCRIPTOR_HANDLE DSVOffset(DescriptorHeap* start, uint32_t index) const noexcept {
			assert(start != nullptr);
			return { start->GetCPUDescriptorHandleForHeapStart().ptr + RTV * index };
		}
	};
	*/
}