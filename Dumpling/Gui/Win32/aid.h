#pragma once
#include "..//..//..//Potato/smart_pointer.h"
namespace Dumpling::Win32
{

	template<typename Type> using intrusive_ptr = Potato::Tool::intrusive_ptr<Type>;

	struct ComWrapper
	{
		template<typename T> static void add_ref(T* com) { com->AddRef(); }
		template<typename T> static void sub_ref(T* com) { com->Release(); }

		template<typename SourceType>
		SourceType** operator ()(SourceType*& pi)
		{
			if (pi != nullptr)
			{
				sub_ref(pi);
				pi = nullptr;
			}
			return &pi;
		}
	};

	template<typename Type> using ComPtr = Potato::Tool::intrusive_ptr<Type, ComWrapper>;
}