import std;
import PotatoTaskSystem;

import Dumpling;
import std;

using namespace Dumpling;


struct TopEventCapture: public Dumpling::FormEventCapture
{
	void AddFormEventCaptureRef() const override {}
	void SubFormEventCaptureRef() const override {}
	FormEvent::Category AcceptedCategory() const override { return FormEvent::Category::MODIFY; }
	FormEvent::Respond Receive(Form& interface, FormEvent::Modify event) override
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
	TopEventCapture responder;
	auto form = Form::Create();

	form->InsertCapture(&responder);

	FormProperty pro;
	pro.title = u8"DumplingFormTest";

	form->Init(pro);

	while(true)
	{
		bool need_quit = false;
		while(
			Form::PeekMessageEventOnce([&](FormEvent::System event)
		{
			if(event.message == decltype(event.message)::QUIT)
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
}