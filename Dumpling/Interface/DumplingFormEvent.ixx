module;


export module DumplingFormEvent;


export namespace Dumpling
{

	enum class FormEventEnum
	{
		DESTORYED,
	};



	struct FormEvent
	{
		FormEventEnum message;
	};

	struct FormEventRespond
	{
		
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