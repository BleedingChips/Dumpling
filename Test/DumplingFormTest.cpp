import DumplingForm;
import DumplingWin32Form;
import std;

using namespace Dumpling;

int main()
{

	{
		auto form_style = Win32::Win32Style::Create(L"Fuck1");


		auto form = Win32::Win32Form::CreateWin32Form(form_style, {}, {}, {});

		while(true)
		{
			if(form->GetStatus() == Win32::Win32Form::Status::Closed)
			{
				break;
			}else
				std::this_thread::sleep_for(std::chrono::milliseconds{1});
		}
	}
	


	return 0;
}