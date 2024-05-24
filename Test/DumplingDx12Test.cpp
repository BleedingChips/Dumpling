#include "d3d12.h"
#include "dxgi1_6.h"


import DumplingForm;
import DumplingRenderer;
import std;

using namespace Dumpling;

int main()
{
	auto device = HardDevice::Create();
	auto renderer = device->CreateRenderer();
	auto output = renderer->CreateFormRenderer();
	auto form = Form::Create({}, output);

	FormProperty pro;
	pro.title = u8"DumplingDx12Test";

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


	return 0;
}