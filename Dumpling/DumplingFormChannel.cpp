module;

module DumplingForm;

namespace Dumpling
{
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
}