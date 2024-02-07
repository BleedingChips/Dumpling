import std;
import PotatoTaskSystem;

import DumplingForm;
import DumplingWindowsForm;
import std;

using namespace Dumpling;

int main()
{

	{
		auto style = Dumpling::Windows::FormStyle::CreateDefaultGameplayStyle();
		auto form = Windows::Form::Create();

		Potato::Task::TaskContext context;
		form->Commit(context, std::this_thread::get_id(), style, {});

		context.ProcessTaskUntillNoExitsTask({});
	}
	

	return 0;
}