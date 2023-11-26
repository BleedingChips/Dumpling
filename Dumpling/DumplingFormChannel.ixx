module;


export module NoodleForm;

import std;
import PotatoMisc;
import PotatoPointer;

namespace Dumpling
{
	struct FormInterface
	{
		virtual void AddRef() const = 0;
		virtual void SubRef() const = 0;
	};


	struct FormChannel : public Potato::Pointer::DefaultStrongWeakInterface
	{
		using Ptr = Potato::Pointer::StrongPtr<FormChannel>;
		using WPtr = Potato::Pointer::WeakPtr<FormChannel>;
	protected:
		std::shared_mutex mutex;
		Potato::Pointer::IntrusivePtr<FormInterface> owner;
		virtual void StrongRelease() override {}
		virtual void WeakRelease() override {}
		friend struct FormInterface;
	};
}