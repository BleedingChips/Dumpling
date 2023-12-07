module;


export module DumplingForm;

import std;
import PotatoMisc;
import PotatoPointer;

export namespace Dumpling
{
	struct FormManagerInterface
	{
		virtual void AddRef() const = 0;
		virtual void SubRef() const = 0;

		friend struct FormChannel;
	};


	struct FormChannel : public Potato::Pointer::DefaultStrongWeakInterface
	{
		using Ptr = Potato::Pointer::StrongPtr<FormChannel>;
		using WPtr = Potato::Pointer::WeakPtr<FormChannel>;

		virtual std::u8string_view FormName() const;

	protected:

		std::shared_mutex mutex;
		Potato::Pointer::IntrusivePtr<FormManagerInterface> owner;
		virtual void StrongRelease() override {}
		virtual void WeakRelease() override {}

		friend struct FormManagerInterface;
	};
}