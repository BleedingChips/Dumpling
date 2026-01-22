#include <d3d12.h>
import Dumpling;
import std;
import Potato;

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



int main()
{
	Device::InitDebugLayer();

	Device device;
	device.Init();

	Form::Config config;
	config.title = u8"FuckYou";
	config.event_hook = &hook;

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
	auto size_in_byte = vertex_object->GetBuffer().size();

	std::array<char8_t, 1024> tem_input;
	std::array<D3D12_INPUT_ELEMENT_DESC, 1024> element_desc;

	auto size = CreateInputDescription(*vertex_layout, element_desc, tem_input);
	auto element_span = std::span(element_desc).subspan(0, *size);

	D3D12_ROOT_SIGNATURE_DESC desc;
	desc.NumParameters = 0;
	desc.NumStaticSamplers = 0;
	desc.pParameters = nullptr;
	desc.pStaticSamplers = nullptr;
	desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	ComPtr<ID3D10Blob> root_signature;
	ComPtr<ID3D10Blob> error_code;

	auto result = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, root_signature.GetPointerAdress(), error_code.GetPointerAdress());

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