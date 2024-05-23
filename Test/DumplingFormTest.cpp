import std;
import PotatoTaskSystem;

import Dumpling;
import std;

using namespace Dumpling;



int main()
{
	auto form = Form::Create();

	FormProperty pro;
	pro.title = u8"DumplingFormTest";

	form->Init(pro);

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
				return FormEventRespond::Default;
		})
			)
		{
			
		}
		if(need_quit)
			break;
	}
	

	return 0;
}