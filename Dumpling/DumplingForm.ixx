module;


export module DumplingForm;

import std;
import PotatoMisc;
import PotatoPointer;
import PotatoTaskSystem;

export import DumplingInterfaceForm;
export import DumplingInterfaceRenderer;

export namespace Dumpling
{
	FormManager::Ptr CreateManager(
		std::pmr::memory_resource* resource = std::pmr::get_default_resource()
	);

	HardDevice::Ptr CreateRendererHardDevice(
		std::pmr::memory_resource* resource = std::pmr::get_default_resource()
	);

}