module;


export module DumplingFormInterface;

import std;
import PotatoMisc;
import PotatoPointer;

export namespace Dumpling
{

	struct FormEvent
	{
		
	};

	struct FormEventRespond
	{
		
	};

	struct FormInterface
	{
		struct Wrapper
		{
			template<typename Type> void AddRef(Type* ptr) const { ptr->AddFormInterfaceRef(); }
			template<typename Type> void SubRef(Type* ptr) const { ptr->SubFormInterfaceRef(); }
		};

		using Ptr = Potato::Pointer::IntrusivePtr<FormInterface, Wrapper>;
		virtual void AddFormInterfaceRef() const = 0;
		virtual void SubFormInterfaceRef() const = 0;
	};

	struct FormEventResponder
	{
		struct Wrapper
		{
			template<typename Type> void AddRef(Type* ptr) const { ptr->AddFormEventResponderRef(); }
			template<typename Type> void SubRef(Type* ptr) const { ptr->SubFormEventResponderRef(); }
		};

		using Ptr = Potato::Pointer::IntrusivePtr<FormEventResponder, Wrapper>;
		virtual std::optional<FormEventRespond> Respond(FormInterface const& interface, FormEvent event) { return std::nullopt; }

	protected:

		virtual void AddFormEventResponderRef() const {};
		virtual void SubFormEventResponderRef() const {};

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