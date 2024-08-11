#include "d3d12.h"
#include "dxgi1_6.h"


import Dumpling;
import std;
import DumplingDx12Renderer;
import DumplingWindowsForm;

using namespace Dumpling;

struct TopEventCapture: public Dumpling::FormEventCapture
{
	void AddFormEventCaptureRef() const override {}
	void SubFormEventCaptureRef() const override {}
	FormEvent::Category AcceptedCategory() const override { return FormEvent::Category::MODIFY; }
	FormEvent::Respond Receive(Form& form, FormEvent::Modify event) override
	{
		if(event.message == FormEvent::Modify::Message::DESTROY)
		{
			Form::PostQuitEvent();
		}
		return FormEvent::Respond::PASS;
	}
};


int main()
{
	HardDevice::InitDebugLayout();

	TopEventCapture top;

	auto device = HardDevice::Create();
	auto renderer = Renderer::Create();
	
	auto form = Form::Create();

	FormProperty pro;
	pro.title = u8"DumplingDx12Test";

	form->InsertCapture(&top);

	form->Init(pro);

	auto output = renderer->CreateFormWrapper(*device, *form);

	auto pipeline = Pipeline::Create();

	

	auto pass = renderer->RegisterPass(
		{
			PassProperty::Category::FRAME,
			u8"Func you",
			{}
		}
	);

	
	//#if defined(DEBUG) || defined(_DEBUG)
	
	//#endif
	//renderer->Execute({}, pipeline);

	//renderer->RegisterPass();

	float R = 0.0f;
	float G = 0.0f;
	float B = 0.0f;

	while(true)
	{
		bool need_quit = false;


		renderer->ExecutePipeline({}, *pipeline);


		Form::PeekMessageEvent([&](FormEvent::System event)
		{
			if (event.message == FormEvent::System::Message::QUIT)
			{
				need_quit = true;
			}
		});

		PassRenderer ren;

		while(renderer->PopPassRenderer(ren, *pass))
		{
			auto rs = output->GetAvailableRenderResource();
			ren.ClearRendererTarget(*rs,
					{
						R,
						G,
						B,
						1.0f
					}
				
				);
		}

		renderer->FinishPassRenderer(ren);

		auto frame = renderer->CommitedAndSwapContext();

		while(true)
		{
			auto [b, i] = renderer->TryFlushFrame(*frame);
			if(b)
			{
				renderer->FlushWindows(*output);
				break;
			}else
			{
				std::this_thread::yield();
			}
		}

		std::this_thread::sleep_for(std::chrono::milliseconds{10});

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


	return 0;
}