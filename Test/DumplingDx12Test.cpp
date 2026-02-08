#include <d3d12.h>
import Dumpling;
import std;
import Potato;
#undef GetObject

using namespace Dumpling;
using namespace Dumpling::Dx12;


struct TopHook : public Win32::FormEventHook
{
	virtual Win32::FormEvent::Respond Hook(FormEvent& event) override
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

	auto finded_path = Potato::Path::ReverseRecursiveSearchDirectory(u8"Test");
	if (!finded_path.empty())
	{
		std::filesystem::current_path(finded_path);
	}

	std::pmr::u8string shader_code;

	auto read_code = Potato::Document::BinaryStreamReader::ReadToBuffer(u8"TestShaderTexture.hlsl", [&](std::size_t size) -> std::span<std::byte> {
		shader_code.resize(size);
		return std::span(reinterpret_cast<std::byte*>(shader_code.data()), shader_code.size() * sizeof(char8_t));
	});

	if (!read_code.has_value())
		return 0;


	using Potato::IR::StructLayout;
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
	Dx12::ShaderSharedResource shader_shader_slot;

	HLSLCompiler::ComplieContext context;
	context.error_capture = [](std::u8string_view message, HLSLCompiler::ShaderTarget target) {
		std::cout << reinterpret_cast<char const*>(message.data()) << std::endl;
	};

	bool result = instance.CompileMaterial(
		compiler,
		shader_slot,
		shader_shader_slot,
		shader_output,
		{
			{shader_code, u8"VSMain", u8"Test.hlsl"},
			{shader_code, u8"PSMain", u8"Test.hlsl"}
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

	auto presets_geometry = Dumpling::Renderer::PresetGeometry::GetTriangle();

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

	auto index_buffer_offset = Potato::MemLayout::AlignTo(presets_geometry.vertex_data->GetBuffer().size(), Dx12::resource_buffer_align);
	auto cb_offset = Potato::MemLayout::AlignTo(index_buffer_offset + sizeof(std::uint32_t) * 3, 256);
	
	auto size_in_byte = presets_geometry.vertex_data->GetBuffer().size();

	Dx12::DescriptorTableMapping description_mapping;

	auto root_signature = CreateRootSignature(device, shader_slot, description_mapping);

	Dx12::ShaderSharedResourceInstance inst;
	inst.Init(shader_shader_slot);

	auto cb = inst.const_buffers[0].const_buffer;

	auto p1 = cb->MemberAs<Math::Float4>(0);
	auto p2 = cb->MemberAs<Math::Float4>(1);
	*p1 = Math::Float4{ 0.0f, 0.0f, 0.0f };
	*p2 = Math::Float4{ 0.5f, 0.5f, 0.5f, 0.0f };

	Math::Float4* typ = reinterpret_cast<Math::Float4*>(cb->GetObject());

	Dx12::MaterialState material_state;
	material_state.vs_layout = presets_geometry.vertex_data->GetStructLayout();
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

	auto description_heap = CreateDescriptorHeap(device, shader_shader_slot);
	description_heap->CreateConstBufferView(device, shader_shader_slot, 0, *vertex_buffer, { cb_offset, cb->GetBuffer().size() + cb_offset });

	{
		Dx12::PassStreamer pass_streamer;
		streamer.PopRequester(pass_streamer, {1});

		auto current_state = pass_streamer.UploadBufferResource(
			presets_geometry.vertex_data->GetBuffer(),
			*vertex_buffer
		);

		
		auto state_change2 = pass_streamer.UploadBufferResource(
			presets_geometry.index_data->GetBuffer(),
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

	auto view = Dx12::GetVertexBufferView(*presets_geometry.vertex_data, *vertex_buffer);
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
	
	output->LogicPresent();
	output->Present();

	return 0;
}