#include "d3d12.h"
#include "dxgi1_6.h"
import DumplingForm;
import DumplingWindowsForm;
import DumplingDx12Renderer;
import std;



using namespace Dumpling;

int main()
{

	/*
	Dx12::InitDebugLayer();

	{
		

		auto con = Dx12::HardwareDevice::Create();

		auto ada = con.EnumAdapter(0);

		auto sd = Dx12::SoftwareDevice::Create(ada);

		auto render = sd.CreateRenderer();

		auto swap = con.CreateSwapChain({}, render);

		auto form_style = Win32::Style::Create(L"Fuck1");

		auto form = Win32::Form::Create(form_style, {}, swap, {});

		form->ShowWindow();

		while (true)
		{
			if (form->GetStatus() == Win32::Form::Status::Closed)
			{
				break;
			}
			else
				std::this_thread::sleep_for(std::chrono::milliseconds{ 1 });
		}

		volatile int i = 0;

		/*
		auto ada = con->EnumAdapter(0);

		auto decive = Dx12::Device::Create(ada);

		auto queue = decive->CreateCommandQueue();

		auto render = Dx12::RendererWrapper::Create(queue, con);

		auto form = Win32::Form::Create(form_style, {}, render.GetPointer(), {});
		*/

		/*
		
		*/
	//}
	


	return 0;
}