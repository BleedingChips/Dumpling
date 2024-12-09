#include "d3d12.h"
#include "dxgi1_6.h"


import Dumpling;
import std;

using namespace Dumpling;

struct TopEventCapture: public Dumpling::FormEventCapture
{
	void AddFormEventCaptureRef() const override {}
	void SubFormEventCaptureRef() const override {}
	FormEvent::Respond RespondEvent(FormEvent event) override
	{
		if (event.IsModify())
		{
			auto modify = event.GetModify();
			if (modify.message == decltype(modify.message)::DESTROY)
			{
				Form::PostQuitEvent();
			}
		}
		return FormEvent::Respond::PASS;
	}
};

/*
struct PipeI : public Dumpling::Pipeline
{

	virtual std::span<PassInfo> GetPassInfo() const { return {}; }
	virtual std::span<std::size_t> GetDependence() const  { return {}; }
	virtual Potato::IR::StructLayout::Ptr GetStruct() const  { return {}; }
	virtual void AddPipelineRef() const {}
	virtual void SubPipelineRef() const {}

	PipeI()  {}
};

*/


int main()
{
	Device::InitDebugLayer();

	TopEventCapture top;

	auto device = Device::Create();

	Form::Config config;
	config.title = L"FuckYou";
	config.event_capture = &top;

	auto form = Form::Create(config);

	auto output = device->CreateFormWrapper(form);

	auto form_renderer = device->CreateFrameRenderer();

	float R = 0.0f;
	float G = 0.0f;
	float B = 0.0f;

	bool need_loop = true;
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

		form_renderer->PopPassRenderer(ren);

		RenderTargetSet sets;
		sets.AddRenderTarget(*output);
		ren.SetRenderTargets(sets);
		ren.ClearRendererTarget(0, {R, G, B, 1.0f});
		form_renderer->FinishPassRenderer(ren);

		auto tar = form_renderer->CommitFrame();
		form_renderer->FlushToLastFrame();

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