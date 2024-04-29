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
		

		Potato::Task::TaskContext context;

		auto form = Dumpling::CreateFormAndCommitedMessageLoop(context, std::this_thread::get_id());

		context.ProcessTaskUntillNoExitsTask({});
	}
	

	return 0;
}