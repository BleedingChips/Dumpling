import std;
import PotatoTaskSystem;

import Dumpling;
import std;

using namespace Dumpling;


struct TopEventCapture: public Dumpling::FormEventCapture
{
	void AddFormEventCaptureRef() const override {}
	void SubFormEventCaptureRef() const override {}
	FormEvent::Respond RespondEvent(FormEvent event) override
	{
		if(event.IsModify())
		{
			auto systm = event.GetModify();
			if(systm.message == FormEventModify::Message::DESTROY)
			{
				Form::PostQuitEvent();
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
	config.event_capture = &responder;

	auto form = Form::Create(config);

	bool MessageLoop = true;
	while (MessageLoop)
	{
		while (true)
		{
			auto re = Form::PeekMessageEventOnce();
			if (!re.has_value())
			{
				MessageLoop = false;
			}else if (!*re)
			{
				break;
			}
		}
		std::this_thread::yield();
	}
	

	return 0;
}