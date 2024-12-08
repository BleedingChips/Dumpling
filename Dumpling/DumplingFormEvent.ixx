module;

#include <assert.h>

export module DumplingFormEvent;

import std;
import PotatoMisc;
import PotatoPointer;
import PotatoTaskSystem;

export namespace Dumpling
{

	struct FormEventSystem
	{
		enum class Message
		{
			QUIT,
		};
		Message message;
	};

	struct FormEventModify
	{
		enum class Message
		{
			DESTROY
		};
		Message message;
	};


	struct FormEvent
	{
		enum class Type
		{
			SYSTEM = 0x01,
			MODIFY = 0x02,
			INPUT = 0x04,
		};

		Type type;

		union 
		{
			FormEventSystem system;
			FormEventModify modify;
		};

		bool IsSystem() const { return type == Type::SYSTEM; }
		FormEventSystem GetSystem() const { assert(IsSystem());  return system; }

		enum class Respond
		{
			PASS,
			CAPTURED,
		};
	};

}