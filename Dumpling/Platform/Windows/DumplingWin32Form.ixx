module;

export module DumplingWin32Form;

import std;
import PotatoPointer;
import DumplingForm;

export namespace Dumpling::Win32
{
	struct FormManager : public Potato::Pointer::DefaultStrongWeakInterface, public FormManagerInterface
	{
		using Ptr = Potato::Pointer::StrongPtr<FormManager>;

		static Ptr Create(std::pmr::memory_resource* resource = std::pmr::get_default_resource());

		bool CreateForm(FormChannel& channel);

	protected:

		using WPtr = Potato::Pointer::WeakPtr<FormManager>;

		FormManager(std::pmr::memory_resource* resource)
			: resource(resource) {}

		~FormManager();

		void StrongRelease() override;
		void WeakRelease() override;

		virtual void AddRef() const override;
		virtual void SubRef() const override;

		void Execuet(FormManager::WPtr ptr);

		std::pmr::memory_resource* resource;

		std::atomic_size_t exist_form_count;
		std::thread threads;

		std::mutex form_init_list;

		struct FormRequest
		{
			FormChannel::WPtr form_channel_interface;
		};

		std::pmr::vector<FormRequest> requests;
	};
}
