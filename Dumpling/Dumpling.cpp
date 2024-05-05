module;

module Dumpling;

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


	
}