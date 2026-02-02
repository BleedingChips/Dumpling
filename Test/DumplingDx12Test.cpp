#include <d3d12.h>
import Dumpling;
import std;
import Potato;
#undef GetObject

using namespace Dumpling;


struct TopHook : public FormEventHook
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

	Dx12::Device::InitDebugLayer();

	Dx12::Device device;
	device.Init();

	Form::Config config;
	config.title = u8"FuckYou";
	config.event_hook = &hook;

	auto instance = HLSLCompiler::Instance::Create();
	auto compiler = instance.CreateCompiler();

	HLSLCompiler::MaterialShaderOutput shader_output;
	Dx12::ShaderSlot shader_slot;

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

	Dx12::FrameRenderer form_renderer;

	device.InitFrameRenderer(form_renderer);

	auto output = device.CreateFormWrapper(form, form_renderer);

	auto vertex_layout = Potato::IR::StructLayout::CreateDynamic(
		u8"default_vertex_layout",
		std::initializer_list<StructLayout::Member>{
			{
				Dx12::GetHLSLConstBufferStructLayout<Float3>(),
				u8"POSITION"
			},
			{
				Dx12::GetHLSLConstBufferStructLayout<Float3>(),
				u8"COLOR"
			}
		}
	);

	auto vertex_object = Potato::IR::StructLayoutObject::DefaultConstruct(vertex_layout, 3, Dx12::GetHLSLConstBufferPolicy());
	
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


	Float3& v1 = *vertex_object->MemberAs<Float3>(0, 0);
	Float3& v2 = *vertex_object->MemberAs<Float3>(0, 1);
	Float3& v3 = *vertex_object->MemberAs<Float3>(0, 2);
	Float3& c1 = *vertex_object->MemberAs<Float3>(1, 0);
	Float3& c2 = *vertex_object->MemberAs<Float3>(1, 1);
	Float3& c3 = *vertex_object->MemberAs<Float3>(1, 2);
	v1 = Float3{ 0.0f, 0.5f, 0.0f };
	v2 = Float3{ 0.0f, 0.0f, 0.0f };
	v3 = Float3{ 0.5f, 0.0f, 0.0f };
	c1 = Float3{ 1.0f, 0.5f, 0.0f };
	c2 = Float3{ 1.0f, 0.0f, 1.0f };
	c3 = Float3{ 0.0f, 0.0f, 1.0f };

	std::uint32_t index_object[3] = {2, 1, 0};

	auto index_buffer_offset = Potato::MemLayout::AlignTo(vertex_object->GetBuffer().size(), Dx12::resource_buffer_align);
	auto cb_offset = Potato::MemLayout::AlignTo(index_buffer_offset + sizeof(std::uint32_t) * 3, 256);
	
	auto size_in_byte = vertex_object->GetBuffer().size();

	Dx12::ShaderDefineDescriptorTableInfo description_table_description_info;
	Dx12::DescriptorTableMapping description_mapping;

	auto root_signature = CreateRootSignature(device, shader_slot, description_table_description_info, description_mapping);

	auto cb = Potato::IR::StructLayoutObject::DefaultConstruct(shader_slot.const_buffer[0].layout);

	auto p1 = cb->MemberAs<Float4>(0);
	auto p2 = cb->MemberAs<Float4>(1);
	*p1 = Float4{ -0.5f, -0.5f, 0.0f };
	*p2 = Float4{ 0.5f, 0.5f, 0.5f, 0.0f };

	Float4* typ = reinterpret_cast<Float4*>(cb->GetObject());

	Dx12::MaterialState material_state;
	material_state.vs_layout = vertex_layout;
	material_state.vs_shader = shader_output.vs;
	material_state.ps_shader = shader_output.ps;


	auto pipeline_object = CreatePipelineState(device, *root_signature, material_state);

	float R = 0.0f;
	float G = 0.0f;
	float B = 0.0f;

	bool need_loop = true;

	Dx12::ResourceStreamer streamer;

	device.InitResourceStreamer(streamer);

	auto heap = streamer.CreateDefaultHeap(Dx12::heap_align);

	auto [vertex_buffer, vertex_buffer_size] = streamer.CreateBufferResource(*heap, Dx12::heap_align);

	auto description_heap = CreateDescriptorHeap(device, description_table_description_info);
	description_heap->CreateConstBufferView(device, description_table_description_info, 0, *vertex_buffer, { cb_offset, cb->GetBuffer().size() + cb_offset });

	{
		Dx12::PassStreamer pass_streamer;
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

	auto view = Dx12::GetVertexBufferView(*vertex_object, *vertex_buffer);
	auto index_view = Dx12::GetIndexBufferView(*vertex_buffer, 3, index_buffer_offset);

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

		Dx12::PassRenderer ren;
		PassRequest request;

		if (form_renderer.PopPassRenderer(ren, request))
		{
			Dx12::RenderTargetSet sets;
			sets.AddRenderTarget(*output);
			ren.SetRenderTargets(sets);
			ren.ClearRendererTarget(0, { R, G, B, 1.0f });
			ren->SetPipelineState(pipeline_object.GetPointer());
			ren->SetGraphicsRootSignature(root_signature.GetPointer());
			ren.SetGraphicDescriptorTable(description_mapping, *description_heap);
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