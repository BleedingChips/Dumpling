import std;
import PotatoTaskSystem;

import Dumpling;
import Noodles;
import std;

using namespace Dumpling;



int main()
{
	auto form = Form::Create();

	form->Init();

	while(true)
	{
		bool need_quit = false;
		while(
			Form::PeekMessageEventOnce([&](Form*, FormEvent event)->FormEventRespond
		{
			if(event.message == FormEventEnum::DESTORYED)
			{
				need_quit = true;
			}
				return FormEventRespond::ignore;
		})
			)
		{
			
		}
		if(need_quit)
			break;
	}
	

	return 0;
}