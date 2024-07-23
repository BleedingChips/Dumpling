#include "d3d12.h"
#include "dxgi1_6.h"


import Dumpling;
import std;
import DumplingDx12Renderer;

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
			Form::PostFormQuitEvent();
		}
		return FormEvent::Respond::PASS;
	}
};


int main()
{
	TopEventCapture top;

	auto device = HardDevice::Create();
	auto renderer = device->CreateRenderer();
	
	auto form = Form::Create();

	FormProperty pro;
	pro.title = u8"DumplingDx12Test";

	form->InsertCapture(&top);

	form->Init(pro);

	auto output = device->CreateFormWrapper(*form, *renderer);

	auto pipeline = Pipeline::Create();

	

	auto pass = renderer->RegisterPass(
		{
			PassProperty::Category::FRAME,
			u8"Func you",
			{}
		}
	);

	

	//renderer->Execute({}, pipeline);

	//renderer->RegisterPass();

	while(true)
	{
		bool need_quit = false;


		renderer->Commited({}, *pipeline);

		while(
			Form::PeekMessageEventOnce([&](FormEvent::System event)
		{
			if(event.message == FormEvent::System::Message::QUIT)
			{
				need_quit = true;
			}
		})
			)
		{
			
		}

		while(auto passren = renderer->PopPassRenderer(*pass))
		{
			Dumpling::Dx12::PassRenderer* rend = dynamic_cast<Dumpling::Dx12::PassRenderer*>(passren.GetPointer());
		}

		renderer->FlushFrame();

		if(need_quit)
			break;
	}
	

	return 0;


	return 0;
}