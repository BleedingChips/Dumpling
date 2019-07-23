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
		static SourceType** overwrite_static_cast(SourceType*& source, Potato::Tmp::type_placeholder<SourceType**>) {
			if (source != nullptr)
			{
				sub_ref(source);
				source = nullptr;
			}
			return &source;
		}
	};

	template<typename Type> using ComPtr = Potato::Tool::intrusive_ptr<Type, ComWrapper>;
}