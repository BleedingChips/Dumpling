import std;
import PotatoTaskSystem;

import Dumpling;
import std;

using namespace Dumpling;


bool need_quit = false;

struct TopEventCapture: public Dumpling::FormEventCapture
{
	void AddFormEventCaptureRef() const override {}
	void SubFormEventCaptureRef() const override {}
	FormEvent::Respond RespondEvent(FormEvent event) override
	{
		if(event.IsSystem())
		{
			auto systm = event.GetSystem();
			if(systm.message == FormEventSystem::Message::QUIT)
			{
				need_quit = true;
				return FormEvent::Respond::PASS;
			}
		}
		return FormEvent::Respond::PASS;
	}
};


int main()
{
	TopEventCapture responder;

	Form::Config config;

	config.title = L"Fuck You Dumpling!";

	auto form = Form::Create(config);

	while(!need_quit)
	{
		Form::PeekMessageEvent(responder);
		std::this_thread::yield();
	}
	

	return 0;
}