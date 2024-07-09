import std;
import PotatoTaskSystem;

import Dumpling;
import std;

using namespace Dumpling;


struct TopEventResponder : public Dumpling::FormEventResponder
{
	void AddFormEventResponderRef() const override {}
	void SubFormEventResponderRef() const override {}
	FormEventRespond Respond(Form& interface, FormEvent event) override
	{
		if(event.message == FormEventEnum::DESTROY)
		{
			Form::PostFormQuitEvent();
		}
		return FormEventRespond::Default;
	}
};


int main()
{
	TopEventResponder responder;
	auto form = Form::Create(&responder);

	FormProperty pro;
	pro.title = u8"DumplingFormTest";

	form->Init(pro);

	while(true)
	{
		bool need_quit = false;
		while(
			Form::PeekMessageEventOnce([&](Form* form, FormEvent event, FormEventRespond respond)
		{
			if(event.message == FormEventEnum::QUIT)
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