module;

module DumplingForm;

#ifdef _WIN32
import DumplingWin32Form;
#endif

namespace Dumpling
{

#ifdef _WIN32
	/*
	Form::Ptr Form::CreateDx12Form(FormSetting setting)
	{
		return Win32::Win32Form::CreateWin32Form(setting);
	}
	*/
#endif


	/*
	auto FormChannel::Create(std::pmr::memory_resource* resource)
		-> Ptr
	{
		if(resource != nullptr)
		{
			auto adress = resource->allocate(sizeof(FormChannel), alignof(FormChannel));
			if(adress != nullptr)
			{
				return new (adress) FormChannel{resource};
			}
		}
		return {};
	}

	void FormChannel::ViewerRelease()
	{
		auto re = resource;
		this->~FormChannel();
		re->deallocate(this, sizeof(FormChannel), alignof(FormChannel));
	}
	*/
}