module;


module DumplingRenderer;

import std;
import PotatoMisc;
import PotatoPointer;
import DumplingForm;

#ifdef _WIN32
import DumplingDx12Renderer;
#endif


namespace Dumpling
{
	auto HardDevice::Create(std::pmr::memory_resource* resource) -> Ptr
	{
#ifdef _WIN32
		return Dx12::HardDevice::Create(resource);
#endif
	}
}