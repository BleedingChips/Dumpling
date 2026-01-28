#include <d3d12.h>
import Dumpling;
import std;
import Potato;
#undef GetObject

using namespace Dumpling;


struct TopHook : public Dumpling::FormEventHook
{
	virtual FormEvent::Respond Hook(FormEvent& event) override
	{
		if (event.IsMessage(FormMessage::DESTORY))
		{
			FormEvent::PostQuitEvent();
		}
		return event.RespondMarkAsSkip();
	}
	virtual void AddFormEventHookRef() const {};
	virtual void SubFormEventHookRef() const {};
} hook;

std::u8string_view shader = u8R"(

cbuffer UserDefine  {
	float4 PositionOffset;
	float4 ColorOffset;
};

struct Vertex
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

struct InputVertex
{
	float3 Position : POSITION;
	float3 Color : COLOR;
};

Vertex VSMain(InputVertex in_vertex)
{
	Vertex vertex;
	vertex.position = float4(in_vertex.Position.x, in_vertex.Position.y, in_vertex.Position.z, 1.0f) + PositionOffset;
	vertex.color = float4(in_vertex.Color.x, in_vertex.Color.y, in_vertex.Color.z, 1.0f);
	return vertex;
};

struct Pixel
{
	float4 Color : SV_TARGET0;
};

Pixel PSMain(Vertex vertex)
{
	Pixel pixel;
	pixel.Color = vertex.color + ColorOffset;
	return pixel;
};

)";

int main()
{
	Device::InitDebugLayer();

	Device device;
	device.Init();

	Form::Config config;
	config.title = u8"FuckYou";
	config.event_hook = &hook;

	auto instance = HLSLCompiler::Instance::Create();
	auto compiler = instance.CreateCompiler();

	HLSLCompiler::MaterialShaderOutput shader_output;
	ShaderSlot shader_slot;

	HLSLCompiler::ComplieContext context;
	context.error_capture = [](std::u8string_view message, HLSLCompiler::ShaderTarget target) {
		std::cout << reinterpret_cast<char const*>(message.data()) << std::endl;
	};

	bool result = instance.CompileMaterial(
		compiler,
		shader_slot,
		shader_output,
		{
			{shader, u8"VSMain", u8"Test.hlsl"},
			{shader, u8"PSMain", u8"Test.hlsl"}
		},
		context
	);

	if (!result)
	{
		return 0;
	}

	auto form = Form::Create(config);

	FrameRenderer form_renderer;

	device.InitFrameRenderer(form_renderer);

	auto output = device.CreateFormWrapper(form, form_renderer);

	auto vertex_layout = Potato::IR::StructLayout::CreateDynamic(
		u8"default_vertex_layout",
		std::initializer_list<StructLayout::Member>{
			{
				GetHLSLConstBufferStructLayout<CBFloat3>(),
				u8"POSITION"
			},
			{
				GetHLSLConstBufferStructLayout<CBFloat3>(),
				u8"COLOR"
			}
		},
		GetHLSLConstBufferPolicy()
	);

	auto vertex_object = Potato::IR::StructLayoutObject::DefaultConstruct(vertex_layout, 3, GetHLSLConstBufferPolicy());
	
	auto p = vertex_object->GetObject(0);

	D3D12_VIEWPORT view_port{
		0.0f,
		0.0f,
		1024.0f,
		768.0f,
		0.0f,
		1.0f
	};
	D3D12_RECT rect{
		0,
		0,
		1024,
		768
	};


	CBFloat3& v1 = *vertex_object->MemberAs<CBFloat3>(0, 0);
	CBFloat3& v2 = *vertex_object->MemberAs<CBFloat3>(0, 1);
	CBFloat3& v3 = *vertex_object->MemberAs<CBFloat3>(0, 2);
	CBFloat3& c1 = *vertex_object->MemberAs<CBFloat3>(1, 0);
	CBFloat3& c2 = *vertex_object->MemberAs<CBFloat3>(1, 1);
	CBFloat3& c3 = *vertex_object->MemberAs<CBFloat3>(1, 2);
	v1 = CBFloat3{ 0.0f, 0.5f, 0.0f };
	v2 = CBFloat3{ 0.0f, 0.0f, 0.0f };
	v3 = CBFloat3{ 0.5f, 0.0f, 0.0f };
	c1 = CBFloat3{ 1.0f, 0.5f, 0.0f };
	c2 = CBFloat3{ 1.0f, 0.0f, 1.0f };
	c3 = CBFloat3{ 0.0f, 0.0f, 1.0f };

	std::uint32_t index_object[3] = {2, 1, 0};

	auto index_buffer_offset = Potato::MemLayout::AlignTo(vertex_object->GetBuffer().size(), resource_buffer_align);
	auto cb_offset = Potato::MemLayout::AlignTo(index_buffer_offset + sizeof(std::uint32_t) * 3, 256);
	
	auto size_in_byte = vertex_object->GetBuffer().size();

	auto root_signature = CreateRootSignature(device, shader_slot);

	auto cb = Potato::IR::StructLayoutObject::DefaultConstruct(shader_slot.const_buffer[0].layout);

	auto p1 = cb->MemberAs<CBFloat4>(0);
	auto p2 = cb->MemberAs<CBFloat4>(1);
	*p1 = CBFloat4{ -0.5f, -0.5f, -0.5f };
	*p2 = CBFloat4{ 0.5f, 0.5f, 0.5f, 0.0f };

	CBFloat4* typ = reinterpret_cast<CBFloat4*>(cb->GetObject());

	MaterialState material_state;
	material_state.vs_layout = vertex_layout;
	material_state.vs_shader = shader_output.vs;
	material_state.ps_shader = shader_output.ps;


	auto pipeline_object = CreatePipelineState(device, *root_signature, material_state);

	float R = 0.0f;
	float G = 0.0f;
	float B = 0.0f;

	bool need_loop = true;

	ResourceStreamer streamer;

	device.InitResourceStreamer(streamer);

	auto heap = streamer.CreateDefaultHeap(heap_align);

	auto [vertex_buffer, vertex_buffer_size] = streamer.CreateBufferResource(*heap, heap_align);

	auto description_heap = CreateDescriptorHeap(device, shader_slot);

	ID3D12Device& raw_device = device;

	D3D12_CONSTANT_BUFFER_VIEW_DESC desc;
	desc.BufferLocation = vertex_buffer->GetGPUVirtualAddress() + cb_offset;
	desc.SizeInBytes = cb->GetBuffer().size();

	raw_device.CreateConstantBufferView(
		&desc,
		description_heap->GetCPUDescriptorHandleForHeapStart()
	);

	raw_device.CreateConstantBufferView(
		&desc,
		D3D12_CPU_DESCRIPTOR_HANDLE{ description_heap->GetCPUDescriptorHandleForHeapStart().ptr + raw_device.GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) }
	);

	{
		PassStreamer pass_streamer;
		streamer.PopRequester(pass_streamer, {1, 0});

		auto current_state = pass_streamer.UploadBufferResource(
			vertex_object->GetBuffer(),
			*vertex_buffer
		);

		
		auto state_change2 = pass_streamer.UploadBufferResource(
			&index_object, sizeof(std::uint32_t) * 3,
			*vertex_buffer,
			index_buffer_offset
		);

		auto s3 = pass_streamer.UploadBufferResource(
			cb->GetBuffer(),
			*vertex_buffer,
			cb_offset
		);

		auto version = streamer.Commited(pass_streamer);
		while (!streamer.TryFlushTo(version))
		{
			std::this_thread::yield();
		}
		streamer.PopRequester(pass_streamer, {});
		streamer.Commited(pass_streamer);
	}

	auto view = GetVertexBufferView(*vertex_object, *vertex_buffer);
	auto index_view = GetIndexBufferView(*vertex_buffer, 3, index_buffer_offset);

	while(need_loop)
	{

		while (true)
		{
			auto re = Form::PeekMessageEventOnce();
			if (re.has_value())
			{
				if (!*re)
					break;
			}else
			{
				need_loop = false;
			}
		}

		PassRenderer ren;
		PassRequest request;

		if (form_renderer.PopPassRenderer(ren, request))
		{
			RenderTargetSet sets;
			sets.AddRenderTarget(*output);
			ren.SetRenderTargets(sets);
			ren.ClearRendererTarget(0, { R, G, B, 1.0f });
			ren->SetPipelineState(pipeline_object.GetPointer());
			ren->SetGraphicsRootSignature(root_signature.GetPointer());
			ID3D12DescriptorHeap* ppHeaps[] = { description_heap };
			ren->SetDescriptorHeaps(1, ppHeaps);
			ren->SetGraphicsRootDescriptorTable(0, description_heap->GetGPUDescriptorHandleForHeapStart());
			ren->SetGraphicsRootDescriptorTable(1, description_heap->GetGPUDescriptorHandleForHeapStart());
			ren->IASetVertexBuffers(0, 1, &view);
			ren->IASetIndexBuffer(&index_view);
			ren->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			ren->RSSetViewports(1, &view_port);
			ren->RSSetScissorRects(1, &rect);
			ren->DrawIndexedInstanced(3, 1, 0, 0, 0);
			//ren->DrawInstanced(3, 1, 0, 0);
			form_renderer.FinishPassRenderer(ren);
		}

		auto tar = form_renderer.CommitFrame();

		while (true)
		{
			if (form_renderer.FlushFrame() + 1 < form_renderer.GetCurrentFrame())
			{
				std::this_thread::yield();
			}
			else
			{
				break;
			}
		}

		output->LogicPresent();
		output->Present();
		

		R += 0.03f;
		G += 0.06f;
		B += 0.09f;

		if(R >= 1.0f)
			R -= 1.0f;
		if(G >= 1.0f)
			G -= 1.0f;
		if(B >= 1.0f)
			B -= 1.0f;
	}
	

	return 0;
}