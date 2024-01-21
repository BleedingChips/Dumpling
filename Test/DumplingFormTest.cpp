import std;
import PotatoTaskSystem;

import DumplingForm;
import DumplingWin32Form;
import std;

using namespace Dumpling;

int main()
{

	{
		auto style = Dumpling::Win32::FormStyle::CreateDefaultGameplayStyle();
		auto form = Win32::Form::Create();

		Potato::Task::TaskContext context;
		form->Commit(context, std::this_thread::get_id(), style, {});

		context.ProcessTaskUntillNoExitsTask({});
	}
	

	return 0;
}