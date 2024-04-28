import std;
import PotatoTaskSystem;

import DumplingForm;
import DumplingFormInterface;
import DumplingWindowsForm;
import std;

using namespace Dumpling;

int main()
{

	{
		auto form = Dumpling::CreateGameWindows();

		Potato::Task::TaskContext context;
		form->CommitedMessageLoop(context, std::this_thread::get_id());

		context.ProcessTaskUntillNoExitsTask({});
	}
	

	return 0;
}