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
		auto style = Dumpling::Windows::FormStyle::CreateDefaultGameplayStyle();

		Dumpling::FormEventResponder resp;

		auto form = Dumpling::Windows::EventResponderForm::Create(&resp);

		Potato::Task::TaskContext context;
		form->Commit(context, std::this_thread::get_id(), style, {});

		context.ProcessTaskUntillNoExitsTask({});
	}
	

	return 0;
}