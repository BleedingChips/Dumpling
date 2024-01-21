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


	/*
	struct FormInterface
	{
		using Ptr = Potato::Pointer::IntrusivePtr<FormInterface, FormPointerWrapperT>;


		virtual void FormAddRef() const = 0;
		virtual void FormSubRef() const = 0;
	};
	*/




}