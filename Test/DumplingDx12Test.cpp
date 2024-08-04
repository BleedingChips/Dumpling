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

		while(auto passren = renderer->PopPassRenderer(*pass))
		{
			auto rs = output->GetAvailableRenderResource();
			passren->ClearRendererTarget(*rs);
		}

		auto frame = renderer->CommitedAndSwapContext();

		while(true)
		{
			auto [b, i] = renderer->TryFlushFrame(*frame);
			if(b)
			{
				break;
			}else
			{
				std::this_thread::yield();
			}
		}

		if(need_quit)
			break;
	}
	

	return 0;


	return 0;
}