import std;
import Potato;

import Dumpling;
import std;

using namespace Dumpling;

struct TopHook : public Dumpling::FormEventHook
{
	virtual FormEvent::Respond Hook(FormEvent& event) override
	{
		if (event.IsMessage(FormMessage::DESTORY))
		{
			FormEvent::PostQuitEvent();
		}
		return event.RespondMarkAsSkip();
	}
	virtual void AddFormEventHookRef() const {};
	virtual void SubFormEventHookRef() const {};
} hook;

int main()
{

	Form::Config config;
	std::u8string_view title2 = u8"中国人！";
	std::string_view title3 = "中国人！";

	config.title = u8"中国人";
	config.event_hook = &hook;

	auto form = Form::Create(config);


	while (true)
	{
		auto has_event = Form::PeekMessageEventOnce([](FormEvent const& event) ->FormEvent::Respond {
			if (event.IsMessage(FormMessage::DESTORY))
				FormEvent::PostQuitEvent();
			return event.RespondMarkAsSkip();
		});

		if (!has_event.has_value())
		{
			break;
		}
		else {
			std::this_thread::yield();
		}
	}

	return 0;
}