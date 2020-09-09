#include "../../../Dumpling/Gui/Win32/dx12_form.h"
#include "../../../Dumpling/FrameWork/Public/path_system.h"
#include "../../../Dumpling/Msc/Public/mscf.h"
#include "../../../Dumpling/PineApple/Public/CharEncode.h"
#include "../../../Dumpling/PineApple/Public/Symbol.h"
#include <assert.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <filesystem>
#include <fstream>
#include <d3d12shader.h>
#include <d3dcompiler.h>
#include <sstream>


using namespace Dumpling;
using Dxgi::FormatPixel;

using namespace Dxgi::DataType;
using Win32::ThrowIfFault;
using namespace Potato;
using namespace PineApple;
using namespace Dx12;

namespace fs = std::filesystem;


int main()
{

	







	using namespace Dumpling::Path;

	auto Re2 = UpSearch(U"Content");
	assert(Re2);
	auto Finded = Search(U"base.mscf", *Re2);
	assert(Finded);
	auto Data = LoadEntireFile(*Finded);
	assert(Data);

	std::u32string AT = CharEncode::Wrapper(Data->data(), Data->size()).To<char32_t>();
	auto E = Dumpling::Mscf::translate(AT);


	//auto String = LoadEntireBinaryFile();


#ifdef _DEBUG
	Dx12::InitDebugLayout();
#endif

	//auto Cur = std::filesystem::current_path();
	auto Cur = *Path::UpSearch(U"Demo");
	auto Target = *Path::Search(U"DFGenerator.cso", Cur);
	auto CSData = *Path::LoadEntireFile(Target);

	auto vs_shader = *Path::LoadEntireFile(*Path::Search(U"VertexShader.cso", Cur));
	auto ps_shader = *Path::LoadEntireFile(*Path::Search(U"PixelShader.cso", Cur));

	Potato::Tool::span<std::byte> b(ps_shader.data(), ps_shader.size());

	//Dumpling::Dx12::CreateRootSignature({ b });

	Dx12::ComPtr<ID3D12ShaderReflection> Ref;
	HRESULT rer = D3DReflect(ps_shader.data(), ps_shader.size(), __uuidof(ID3D12ShaderReflection), Ref(Dx12::VoidT{}));
	D3D12_SHADER_DESC Desc;
	Ref->GetDesc(&Desc);

	for (size_t i = 0; i < Desc.BoundResources; ++i)
	{
		D3D12_SHADER_INPUT_BIND_DESC Desc;
		Ref->GetResourceBindingDesc(i, &Desc);
		volatile int k = 0;
	}

	for (size_t i = 0; i < Desc.ConstantBuffers; ++i)
	{
		auto Buffer  = Ref->GetConstantBufferByIndex(i);
		D3D12_SHADER_BUFFER_DESC Desc;
		Buffer->GetDesc(&Desc);
		for (size_t k = 0; k < Desc.Variables; ++k)
		{
			auto Ref = Buffer->GetVariableByIndex(k);
			if (Ref != nullptr)
			{
				D3D12_SHADER_VARIABLE_DESC Desc2;
				Ref->GetDesc(&Desc2);
				volatile int k = 0;
			}
		}
		volatile int k = 0;
	}

	auto Dev = Dx12::CreateDevice();
	auto Que = Dx12::CreateCommandQueue(*Dev, D3D12_COMMAND_LIST_TYPE_DIRECT);
	auto Allo = Dx12::CreateCommandAllocator(*Dev, D3D12_COMMAND_LIST_TYPE_DIRECT);
	auto Command = Dx12::CreateGraphicCommandList(*Dev, *Allo, D3D12_COMMAND_LIST_TYPE_DIRECT);
	auto Heap = Dx12::CreateCommitedHeap(Dev, Dx12::HeapType::Texture, 0);
	auto Texture2 = Heap->CreateTexture3DUAV(DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_FLOAT, 256, 256, 256);
	std::vector<std::array<float, 4>> AllBuffer{ {1.0, 1.0, 1.0, 1.0} };
	
	size_t Length = AllBuffer.size() * sizeof(decltype(AllBuffer)::value_type);
	auto BufferHeap = Dx12::CreateCommitedHeap(Dev, Dx12::HeapType::Buffer);
	auto Buffer = BufferHeap->CreateBuffer(Length);
	auto UploadHeap = Dx12::CreateCommitedHeap(Dev, Dx12::HeapType::UploadBuffer);
	auto UploadBuffer = UploadHeap->CreateBuffer(Length);
	auto Re = Dx12::MappingBuffer(*UploadBuffer, 0, 0, Length, [&](std::byte* Data) {
		std::memcpy(Data, AllBuffer.data(), Length);
	});
	

	Dx12::ChangeState(*Command, { UploadBuffer }, ResState::Common, ResState::CopySour);
	Dx12::ChangeState(*Command, { Buffer }, ResState::Common, ResState::CopyDest);

	Command->CopyBufferRegion(UploadBuffer, 0, Buffer, 0, Length);

	Dx12::ChangeState(*Command, { UploadBuffer }, ResState::CopySour, ResState::Common);
	Dx12::ChangeState(*Command, { Buffer }, ResState::CopyDest, ResState::PixelShaderRes);




	D3D12_ROOT_SIGNATURE_DESC1 Desc1;

	//D3D12SerializeVersionedRootSignature(    );


	auto Form = Dx12::CreateForm(*Que);
	std::atomic_bool Exit = false;
	Form->OverwriteEventFunction([&](HWND, UINT msg, WPARAM, LPARAM) -> std::optional<LRESULT> {
		if (msg == WM_CLOSE)
		{
			Exit = true;
			return 0;
		}
		else
			return std::nullopt;
	});

	auto Mapping = Dx12::CreateDescriptorMapping({
		{"DefaultTexture", Dx12::ResourceType::Tex2D}
		});

	auto Description = Dx12::CreateResourceDescriptor(*Dev, Mapping);



	auto Texture = Dx12::CreateTexture2DConst(*Dev, DXGI_FORMAT_R32G32B32A32_FLOAT, 1024, 1024, 1);
	auto UpLoadBuffer = Dx12::CreateUploadBuffer(*Dev, 1024 * 1024 * 128);
	
	//Dx12::ChangeState(*Command, { Texture }, ResState::Common, ResState::);
	//Dx12::ChangeState(*Command, { UpLoadBuffer }, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_SOURCE);
	auto Fence = Dx12::CreateFence(*Dev);
	//Command->CopyBufferRegion(Texture, 0, UpLoadBuffer, 0, 1024 * 1024 * 128);
	Command->Close();
	ID3D12CommandList* List[] = { Command };
	Que->ExecuteCommandLists(1, List);
	Que->Signal(Fence, 1);
	//while (Fence->GetCompletedValue() != 1);

	while (!Exit)
	{
		
	}

	//auto KK = Dx12::LoadEntireFile(total_path);

	//auto Context = ThrowIfFault(Dx12::Context::Create(0));
	//auto CommandQueue = ThrowIfFault(Context->CreateCommandQueue(CommandListType::Direct));
	//auto Form = Dx12::Form::Create(*CommandQueue);

	//auto CommandAllocator = ThrowIfFault(Context->CreateCommandAllocator(CommandListType::Direct));
	//auto DescMap = Context->CreateDescriptorMapping("WTF", { {"Data", Dx12::DescResType::CB} });

	//std::atomic_bool Exit = false;

	/*
	Form->OverwriteEventFunction([&](HWND, UINT msg, WPARAM, LPARAM) -> std::optional<LRESULT> {
		if (msg == WM_CLOSE)
		{
			Exit = true;
			return 0;
		}
		else
			return std::nullopt;
	});
	*/

	/*
	while (!Exit)
	{
		auto Fen = ThrowIfFault(Context->CreateFence());
		auto List = ThrowIfFault(Context->CreateGraphicCommandList(CommandAllocator, Dx12::CommandListType::Direct));
		auto Heap = Context->CreateRTDescriptor(1);
		Context->SetTex2AsRTV(Heap, Form->CurrentBackBuffer(), 0);
		Dx12::StateLog SL{ Form->CurrentBackBuffer(), ResourceState::Present };
		SL.ChangeState(List, ResourceState::RenderTarget);
		FLOAT Color[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
		List->ClearRenderTargetView(Heap.CPUHandle(), Color, 0, nullptr);
		SL.ChangeState(List, ResourceState::Present);
		List->Close();
		Dx12::CommandList* CL[] = {List};
		CommandQueue->ExecuteCommandLists(1, CL);
		Form->PresentAndSwap();
		CommandQueue->Signal(Fen, 1);
		while(Fen->GetCompletedValue() != 1)
			std::this_thread::sleep_for(std::chrono::milliseconds{ 10 });
		CommandAllocator->Reset();
	}
	volatile int i = 0;
	*/

	//std::this_thread::sleep_for(std::chrono::milliseconds{ 5000 });
	std::cout << "Down" << std::endl;

	/*

	auto [Reflect, ReR] = Dx12::Reflect(ps_shader.data(), ps_shader.size());

	auto [Factory, re_f] = Dxgi::CreateFactory();b
	auto AllAdapters = Dxgi::EnumAdapter(Factory);
	DXGI_ADAPTER_DESC desc;
	AllAdapters[0]->GetDesc(&desc);

	auto [Device, re_d] = Dx12::CreateDevice(AllAdapters[0], D3D_FEATURE_LEVEL_12_0);

	auto [Queue, re_c] = Device++.CreateCommmandQueue(*CommandListType::Direct);
	Queue->SetName(L"WTF");
	auto [Allocator, re_a] = Device++.CreateCommandAllocator(*CommandListType::Direct);
	Win32::Form form = Win32::Form::create();

	auto swap_chain_desc = Dxgi::CreateDefaultSwapChainDesc(*Dxgi::FormatPixel::RGBA16_Float, 1024, 768);
	auto [SwapChain, re_s] = Dx12::CreateSwapChain(Factory, Queue, form, swap_chain_desc);
	auto [RTDescHead, re_rt1] = Device++.CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 1);
	auto [DTDescHead, re_rt2] = Device++.CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1);

	auto DescriptorSize = Device++.GetDescriptorHandleIncrementSize();
	
	auto [DTResource, red] = Device++.CreateDepthStencil2DCommitted(*Dxgi::FormatPixel::D24S8_Unorn_Uint, 1024, 768, 0);
	Device++.CreateDepthStencilView2D(DTResource, DescriptorSize.DSVOffset(DTDescHead, 0));

	auto viewport = Dx12::CreateFullScreenViewport(1024, 768);

	auto [Fence, ref] = Device++.CreateFence(0);

	bool exit = false;
	uint32_t current_buffer = 0;

	struct Point
	{
		float3 Position;
		float2 UV;
	};

	struct Instance
	{
		float2 Shift;
	};

	auto AllInputElement = std::tuple{
		Dx12::ElementVertex{&Point::Position, "Position", 0},
		Dx12::ElementVertex{&Point::UV, "UV", 0},
	};

	auto InstanceElement = std::tuple{
		Dx12::ElementInstance{&Instance::Shift, "Shift", 0, 3}
	};

	Point Rec[] = {
		{float3{0.0, 0.2f, 0.0}, float2{0.5f, 0.0}},
		{float3{0.2f, 0.0, 0.0},float2{0.5f, 0.0} },
		{float3{-0.2f, 0.0, 0.0}, float2{ 0.5f, 0.0 }},
	};

	auto [UploadBuffer, RrUB] = Device++.CreateBufferUploadCommitted(sizeof(Point) * 3);

	HRESULT MappingResult = Dx12::MappingBufferArray(UploadBuffer, [&](Point* input) {
		for (size_t i = 0; i < 3; ++i)
			input[i] = Rec[i];
	}, 0, 3);

	auto [VertexBuffer, re_VB] = Device++.CreateBufferVertexCommitted(sizeof(Point) * 3, 0, 0);

	auto [UpdateCommandList, re_UC] = Device++.CreateGraphicCommandList(Allocator, *CommandListType::Direct);

	UpdateCommandList->CopyResource(VertexBuffer, UploadBuffer);

	UpdateCommandList->Close();

	Dx12::CommandList* const CL[] = { UpdateCommandList };

	Queue->ExecuteCommandLists(0, CL);

	while (!exit)
	{

		auto [BBResource, re1] = Dx12::GetBuffer(SwapChain, current_buffer);
		Device++.CreateRenderTargetView2D(BBResource, DescriptorSize.RTVOffset(RTDescHead, 0));
		
		auto [CommandList, re_l] = Device++.CreateGraphicCommandList(Allocator, *CommandListType::Direct);

		auto RTHandle = RTDescHead->GetCPUDescriptorHandleForHeapStart();
		auto DTHandle = DTDescHead->GetCPUDescriptorHandleForHeapStart();

		FLOAT Color[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
		
		Dx12::ResourceBarrier tem[2] = {
			Dx12::TransitionState(BBResource, *ResourceState::Present, *ResourceState::RenderTarget),
			Dx12::TransitionState(DTResource, *ResourceState::Common, *ResourceState::DepthWrite)
		};

		CommandList->ResourceBarrier(2, tem);
		CommandList->ClearRenderTargetView(RTDescHead->GetCPUDescriptorHandleForHeapStart(), Color, 0, nullptr);
		CommandList->ClearDepthStencilView(DTDescHead->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0, 0, 0, nullptr);
		CommandList->OMSetRenderTargets(1, &RTHandle, false, &DTHandle);
		Dx12::SwapTransitionState(2, tem);
		CommandList->ResourceBarrier(2, tem);
		CommandList->Close();
		Dx12::CommandList* const CommandlListArray = CommandList;
		Queue->ExecuteCommandLists(1, &CommandlListArray);
		SwapChain->Present(1, 0);
		Queue->Signal(Fence, 1);

		D3D12_DESCRIPTOR_RANGE range{ D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0,  D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND };

		D3D12_ROOT_PARAMETER tempara;
		tempara.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		tempara.Descriptor = D3D12_ROOT_DESCRIPTOR{ 0, 0 };
		tempara.ShaderVisibility = D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_PIXEL;

		D3D12_ROOT_PARAMETER RootParameter[] = {
			tempara
		};

		D3D12_ROOT_SIGNATURE_DESC RootSignatureDesc;
		RootSignatureDesc.NumParameters = 1;
		RootSignatureDesc.pParameters = RootParameter;
		RootSignatureDesc.NumStaticSamplers = 0;
		RootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;

		Dx12::BlobPtr Data, Error;

		HRESULT re = D3D12SerializeRootSignature(&RootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, Data(), Error());

		if (Error)
		{
			const char* buffer = (const char*)Error->GetBufferPointer();
			__debugbreak();
		}

		//CommandList->SetGraphicsRootDescriptorTable();

		//Device

		while (Fence->GetCompletedValue() != 1)
		{
			std::cout << Fence->GetCompletedValue() << std::endl;
			std::this_thread::sleep_for(std::chrono::milliseconds{ 10 });
		}
		
		current_buffer += 1;
		current_buffer = current_buffer % 2;
		Allocator->Reset();
		CommandList->Reset(Allocator, nullptr);
		Fence->Signal(0);
		std::cout << "down" << std::endl;
		MSG msg;
		while (form.pook_event(msg))
		{
			if (msg.message == WM_CLOSE)
			{
				exit = true;
				break;
			}
		}
	}
	*/
	return 0;
}