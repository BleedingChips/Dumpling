#include "d3d12.h"
#include "dxgi1_6.h"
import DumplingForm;
import DumplingWin32Form;
import DumplingDx12Renderer;
import std;



using namespace Dumpling;

int main()
{

	{
		auto form_style = Win32::Win32Style::Create(L"Fuck1");


		auto form = Win32::Win32Form::CreateWin32Form(form_style, {}, {}, {});

		auto con = Dx12::Context::Create();

		auto ada = con->EnumAdapter(0);

		auto decive = Dx12::Device::Create(ada);

		auto queue = decive->CreateCommandQueue();

		auto render = Dx12::Renderer::Create(queue, con);


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