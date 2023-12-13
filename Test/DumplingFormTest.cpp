import DumplingForm;
import std;

using namespace Dumpling;

int main()
{

	{
		auto form = Form::CreateDx12Form({});

		while(true)
		{
			if(form->GetStatus() == Status::Closed)
			{
				break;
			}else
				std::this_thread::sleep_for(std::chrono::milliseconds{1});
		}
	}
	


	return 0;
}