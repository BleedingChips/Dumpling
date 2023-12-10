module;


export module DumplingForm;

import std;
import PotatoMisc;
import PotatoPointer;
import DumplingWin32Form;

export namespace Dumpling
{
	using FormManager = Dumpling::Win32::FormManager;


	struct FormChannel : public Dumpling::Win32::FormChannel
	{
		using Ptr = Potato::Pointer::ControllerPtr<FormChannel>;

		static Ptr Create(std::pmr::memory_resource* resource = std::pmr::get_default_resource());

	protected:

		FormChannel(std::pmr::memory_resource* resource) : resource(resource) {}

		virtual void ViewerRelease() override;

		std::pmr::memory_resource* resource;

	};

}