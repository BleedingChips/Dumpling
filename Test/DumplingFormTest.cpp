import DumplingForm;
import DumplingWin32Form;

using namespace Dumpling;

int main()
{

	{
		auto Win32 = FormManager::Create();

		auto form = FormChannel::Create();

		Win32->CreateForm(*form);
	}
	


	return 0;
}