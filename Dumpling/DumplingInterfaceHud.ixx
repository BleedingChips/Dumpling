module;


export module DumplingInterfaceHud;


export namespace Dumpling
{
	struct FormHud
	{
		struct Wrapper
		{
			template<typename Type> void AddRef(Type* ptr) const { ptr->AddFormHudRef(); }
			template<typename Type> void SubRef(Type* ptr) const { ptr->SubFormHudRef(); }
		};

		
		virtual ~FormHud() = default;

	protected:

		virtual void AddFormHudRef() const = 0;
		virtual void SubFormHudRef() const = 0;
	};

}