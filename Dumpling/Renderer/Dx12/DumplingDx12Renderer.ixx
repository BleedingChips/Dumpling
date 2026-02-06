module;

#include "d3d12.h"
#include "dxgi1_6.h"
#include <cassert>

#undef interface
#undef max
#undef GetObject

export module DumplingDx12Renderer;

import std;
import Potato;
import DumplingForm;
import DumplingPipeline;
import DumplingMath;
import DumplingDx12Define;
import DumplingDx12Shader;
import DumplingDx12Material;
import DumplingDx12ResourceStreamer;

export namespace Dumpling::Dx12
{
	export struct Renderer;

	struct RendererResource
	{
		struct Wrapper
		{
			template<typename Type> void AddRef(Type* ptr) const { ptr->AddRendererResourceRef(); }
			template<typename Type> void SubRef(Type* ptr) const { ptr->SubRendererResourceRef(); }
		};

		using Ptr = Potato::Pointer::IntrusivePtr<RendererResource, Wrapper>;

		struct Description
		{
			ComPtr<ID3D12Resource> resource_ptr;
			D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle;
			D3D12_RESOURCE_STATES default_state;
		};

		virtual Description GetDescription(D3D12_RESOURCE_STATES require_state) const = 0;

	protected:

		virtual void AddRendererResourceRef() const = 0;
		virtual void SubRendererResourceRef() const = 0;
	};

	struct FormWrapper : public RendererResource
	{

		struct Config
		{
			std::size_t swap_buffer_count = 2;
		};

		using Ptr = Potato::Pointer::IntrusivePtr<FormWrapper, RendererResource::Wrapper>;

		RendererResource::Ptr GetAvailableRenderResource() { return this; }

		Description GetDescription(D3D12_RESOURCE_STATES require_state) const override;
		bool Present(std::size_t syn_interval = 1);
		bool LogicPresent();

	protected:

		FormWrapper(ComPtr<IDXGISwapChain3> swap_chain, ComPtr<ID3D12DescriptorHeap> m_rtvHeap, Config config, std::size_t offset)
			: swap_chain(std::move(swap_chain)), m_rtvHeap(std::move(m_rtvHeap)), config(config), offset(offset)
		{
			current_index = this->swap_chain->GetCurrentBackBufferIndex();
			logic_current_index = current_index;
		}

		ComPtr<IDXGISwapChain3> swap_chain;
		ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
		const std::size_t offset;
		const Config config;
		mutable std::shared_mutex logic_mutex;
		std::size_t current_index = 0;
		std::size_t logic_current_index= 0;

		friend struct Renderer;
	};

	export struct FrameRenderer;
	export struct PassRenderer;

	struct RenderTargetSet
	{

		static constexpr std::size_t max_render_target_count = 8;

		void Clear();
		std::optional<std::size_t> AddRenderTarget(RendererResource const& resource);
		bool SetDepthStencil(RendererResource const& resource);
		RenderTargetSet() = default;
		RenderTargetSet(RenderTargetSet const&) = default;
		RenderTargetSet(RenderTargetSet&& set);
		std::size_t GetRenderTargetCount() const { return render_target_count; }
		bool HasDepthStencil() const { return has_depth_stencil; }

	protected:

		struct ResourceRecord
		{
			ComPtr<ID3D12Resource> reference_resource;
			D3D12_CPU_DESCRIPTOR_HANDLE handle;
			D3D12_RESOURCE_STATES default_state = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COMMON;
		};

		std::array<ResourceRecord, max_render_target_count + 1> target_data;
		std::array<D3D12_CPU_DESCRIPTOR_HANDLE, max_render_target_count + 1> target;
		std::size_t render_target_count = 0;
		bool has_depth_stencil = false;

		friend struct PassRenderer;
	};

	struct PassRenderer
	{
		PassRenderer() = default;
		ID3D12GraphicsCommandList* operator->() const { return command.GetPointer(); }
		~PassRenderer()
		{
			assert(!*this);
		}

		ID3D12GraphicsCommandList* GetCommandList() { assert(*this); return command.GetPointer(); }

		void SetRenderTargets(RenderTargetSet const& render_targets);
		bool ClearRendererTarget(std::size_t index, Float4 color = Color::black_rgba);
		bool SetGraphicDescriptorTable(
			DescriptorTableMapping const& descriptor_table_mapping, 
			ShaderDefineDescriptorTable& shared_resource,
			Potato::TMP::FunctionRef<ID3D12DescriptorHeap* (D3D12_DESCRIPTOR_HEAP_TYPE, std::size_t identity)> func = {}
		);
		//bool ClearDepthStencil(RendererTargetCarrier const& render_target, float depth, uint8_t stencil);
		operator bool() const { return command && allocator; }

	protected:

		ComPtr<ID3D12GraphicsCommandList> command;
		ComPtr<ID3D12CommandAllocator> allocator;
		std::size_t frame = 0;
		std::optional<std::size_t> order;

		std::array<D3D12_RESOURCE_BARRIER, (RenderTargetSet::max_render_target_count + 1) * 2> render_target_barriers;
		std::array<D3D12_CPU_DESCRIPTOR_HANDLE, RenderTargetSet::max_render_target_count + 1> cache_render_target;
		std::size_t render_target_barriers_count = 0;

		void PreFinishRender();
		void PosFinishRender();
		void Reset();

		friend struct FrameRenderer;
	};

	struct FrameRenderer
	{
		bool PopPassRenderer(PassRenderer& output, PassRequest const& request);
		bool FinishPassRenderer(PassRenderer& output);
		std::optional<std::size_t> CommitFrame();
		std::uint64_t FlushFrame();
		operator bool() const { return device && command_queue && fence; }
		bool Init(ComPtr<ID3D12Device> devive);
		~FrameRenderer();
		ID3D12Device* GetDevice() { return device.GetPointer(); }
		ID3D12CommandQueue* GetCommandQueue() { return command_queue.GetPointer(); }
		std::uint64_t GetCurrentFrame() const { return current_frame; }

	protected:

		ComPtr<ID3D12Device> device;
		ComPtr<ID3D12CommandQueue> command_queue;
		ComPtr<ID3D12Fence> fence;

		struct OrderedCommandList
		{
			std::size_t order = std::numeric_limits<std::size_t>::max();
			ComPtr<ID3D12GraphicsCommandList> command_list;
		};

		std::pmr::vector<OrderedCommandList> current_frame_command_lists;
		std::pmr::vector<ComPtr<ID3D12CommandAllocator>> current_frame_allocators;

		std::pmr::vector<ComPtr<ID3D12GraphicsCommandList>> idle_command_list;
		std::pmr::vector<ComPtr<ID3D12CommandAllocator>> idle_allocator;

		std::pmr::vector<std::tuple<ComPtr<ID3D12CommandAllocator>, std::uint64_t>> last_frame_allocator;
		std::pmr::vector<std::tuple<ComPtr<ID3D12GraphicsCommandList>, std::uint64_t>> last_frame_command_list;
		
		std::pmr::vector<ID3D12CommandList*> template_buffer;
		std::uint64_t current_frame = 1;

		friend struct Device;
	};

	

	struct Device
	{

		struct Config
		{

		};

		bool Init(Config config = {});
		FormWrapper::Ptr CreateFormWrapper(Form const& form, FrameRenderer& render, FormWrapper::Config fig = {}, std::pmr::memory_resource* resource = std::pmr::get_default_resource());
		bool InitFrameRenderer(FrameRenderer& target_frame_renderer);
		bool InitResourceStreamer(ResourceStreamer& target_resource_streamer);
		static bool InitDebugLayer();
		operator bool() const { return factory && device; }
		operator ID3D12Device& () { return *device; }

	protected:

		ComPtr<IDXGIFactory3> factory;
		ComPtr<ID3D12Device> device;
	};
}