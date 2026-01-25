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

struct Vertex
{
	float4 position : SV_POSITION;
};

Vertex VSMain(float3 Position : POSITION)
{
	Vertex vertex;
	vertex.position = float4(0.0f, 0.0f, 0.0f, 1.0f);
	return vertex;
}

struct Pixel
{
	float4 Color : SV_TARGET0;
};

Pixel PSMain(Vertex vertex)
{
	Pixel pixel;
	pixel.Color = float4(1.0f, 1.0f, 1.0f, 1.0f);
	return pixel;
}

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
	bool result = instance.CompileMaterial(
		compiler,
		shader_slot,
		shader_output,
		{
			{shader, u8"VSMain", u8"Test.hlsl"},
			{shader, u8"PSMain", u8"Test.hlsl"}
		},
		{}
	);

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


	CBFloat3& v1 = *vertex_layout->GetMemberView()[0].As<CBFloat3>(vertex_object->GetObject(0));
	v1 = CBFloat3{ 0.0f, 0.5f, 0.0f };
	CBFloat3& v2 = *vertex_layout->GetMemberView()[0].As<CBFloat3>(vertex_object->GetObject(1));
	v2 = CBFloat3{ 0.0f, 0.0f, 0.0f };
	CBFloat3& v3 = *vertex_layout->GetMemberView()[0].As<CBFloat3>(vertex_object->GetObject(2));
	v3 = CBFloat3{ 0.5f, 0.0f, 0.0f };
	
	auto size_in_byte = vertex_object->GetBuffer().size();

	std::array<char8_t, 1024> tem_input;
	std::array<D3D12_INPUT_ELEMENT_DESC, 1024> element_desc;

	auto size = CreateInputDescription(*vertex_layout, element_desc, tem_input);
	auto element_span = std::span(element_desc).subspan(0, *size);

	auto root_signature = CreateRootSignature(device, shader_slot.total_statics);

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

	auto heap = streamer.CreateDefaultHeap(size_in_byte);
	ComPtr<ID3D12Resource> vertex_buffer;

	{
		PassStreamer pass_streamer;
		streamer.PopRequester(pass_streamer, {});
		vertex_buffer = pass_streamer.CreateVertexBuffer(vertex_object->GetBuffer().data(), size_in_byte, *heap);
		auto version = streamer.Commited(pass_streamer);
		while (!streamer.TryFlushTo(version))
		{
			std::this_thread::yield();
		}
		streamer.PopRequester(pass_streamer, {});
		streamer.Commited(pass_streamer);
	}

	auto view = GetVertexBufferView(*vertex_object, *vertex_buffer);

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
			ren->IASetVertexBuffers(0, 1, &view);
			ren->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			ren->DrawInstanced(3, 1, 0, 0);
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