module;


export module Dumpling;

import std;
import PotatoMisc;
import PotatoPointer;
import PotatoTaskSystem;
export import DumplingFormEvent;
export import DumplingForm;
export import DumplingRenderer;

export namespace Dumpling
{
	FormManager::Ptr CreateManager(
		std::pmr::memory_resource* resource = std::pmr::get_default_resource()
	);

	HardDevice::Ptr CreateRendererHardDevice(
		std::pmr::memory_resource* resource = std::pmr::get_default_resource()
	);

}