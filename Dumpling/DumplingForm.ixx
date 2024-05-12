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

	struct FormEventCollector : public FormEventResponder 
	{
		FormManager::Ptr manager;
		struct Element
		{
			FormInterface::Ptr form;
			FormEvent events;
		};

		std::mutex mutex;
		std::pmr::vector<Element> events;

		FormEventCollector(std::pmr::memory_resource* resource = std::pmr::get_default_resource())
			: events(resource) {}

	protected:

		std::optional<FormEventRespond> Respond(FormInterface& interface, FormEvent event) override;

		void AddFormEventResponderRef() const override {}
		void SubFormEventResponderRef() const override {}
	};


}