module;

module DumplingForm;

#ifdef _WIN32
import DumplingWindows;
#endif

namespace Dumpling
{

	FormInterface::Ptr CreateFormAndCommitedMessageLoop(
		Potato::Task::TaskContext& context,
		std::thread::id thread_id,
		FormProperty property ,
		FormTaskProperty task_property,
		std::pmr::memory_resource* resource
	)
	{
#ifdef _WIN32
		return Windows::Form::CreateFormAndCommitedMessageLoop(
			context,
			thread_id,
			property,
			task_property,
			resource
		);
#endif
	}


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