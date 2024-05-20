module;

module DumplingForm;



#ifdef _WIN32
import DumplingWindows;
import DumplingDx12;
#endif

namespace Dumpling
{
	FormManager::Ptr CreateManager(
		std::pmr::memory_resource* resource
	)
	{
#ifdef _WIN32
		return Windows::FormManager::CreateManager(
			resource
		);
#endif
	}

	HardDevice::Ptr CreateRendererHardDevice(
		std::pmr::memory_resource* resource
	)
	{
#ifdef _WIN32
		return Dx12::HardDevice::Create(
			resource
		);
#endif
	}

	/*
	std::optional<FormEventRespond> FormEventCollector::Respond(FormInterface& interface, FormEvent event)
	{
		if(event.message == FormEventEnum::DESTORYED)
		{
			std::lock_guard lg(mutex);
			events.emplace_back(
				FormInterface::Ptr{&interface},
				event
			);
		}
		return std::nullopt;
	}
	*/
	
}