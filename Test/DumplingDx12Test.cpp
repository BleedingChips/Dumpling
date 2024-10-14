#include "d3d12.h"
#include "dxgi1_6.h"


import Dumpling;
import std;

using namespace Dumpling;

struct TopEventCapture: public Dumpling::FormEventCapture
{
	void AddFormEventCaptureRef() const override {}
	void SubFormEventCaptureRef() const override {}
	TopEventCapture() : FormEventCapture(FormEvent::Category::MODIFY)
	{
		volatile int i = 0;
	}
	FormEvent::Respond Receive(Form& form, FormEvent::Modify event) override
	{
		if(event.message == FormEvent::Modify::Message::DESTROY)
		{
			Form::PostQuitEvent();
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

	auto form = Form::Create();

	FormProperty pro;
	pro.title = u8"DumplingDx12Test";
	pro.form_size = {1024, 600};

	form->InsertCapture(&top);

	form->Init(pro);

	auto output = device->CreateFormWrapper(*form);

	auto form_renderer = device->CreateFrameRenderer();

	float R = 0.0f;
	float G = 0.0f;
	float B = 0.0f;

	while(true)
	{
		bool need_quit = false;


		Form::PeekMessageEvent([&](FormEvent::System event)
		{
			if (event.message == FormEvent::System::Message::QUIT)
			{
				need_quit = true;
			}
		});

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

		if(need_quit)
			break;
	}
	

	return 0;
}