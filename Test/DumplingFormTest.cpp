import DumplingForm;
import DumplingWin32Form;
import std;

using namespace Dumpling;

int main()
{

	{
		auto form_style = Win32::Style::Create(L"Fuck1");


		auto form = Win32::Form::Create(form_style, {}, {}, {});

		while(true)
		{
			if(form->GetStatus() == Win32::Form::Status::Closed)
			{
				break;
			}else
				std::this_thread::sleep_for(std::chrono::milliseconds{1});
		}
	}
	


	return 0;
}