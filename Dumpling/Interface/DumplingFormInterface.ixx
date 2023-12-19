module;


export module DumplingFormInterface;

import std;
import PotatoMisc;
import PotatoPointer;

export namespace Dumpling
{

	
	struct FormPointerWrapperT
	{
		template<typename PtrT>
		void AddRef(PtrT* ptr) const { return ptr->FormAddRef(); }
		template<typename PtrT>
		void SubRef(PtrT* ptr) const { return ptr->FormSubRef(); }
	};

	struct FormEventChannel
	{
		using Ptr = Potato::Pointer::IntrusivePtr<FormEventChannel, FormPointerWrapperT>;
		virtual void FormAddRef() const = 0;
		virtual void FormSubRef() const = 0;
	};

}