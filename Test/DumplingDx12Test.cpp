#include "d3d12.h"
#include "dxgi1_6.h"


import DumplingForm;
import DumplingRenderer;
import std;

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

	auto output = renderer->CreateFormRenderer(*form);

	auto pipeline = Pipeline::Create();

	//renderer->Execute({}, pipeline);

	//renderer->RegisterPass();

	while(true)
	{
		bool need_quit = false;
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
		if(need_quit)
			break;
	}
	

	return 0;


	return 0;
}