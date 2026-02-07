module;

#include <Windows.h>
#include <wrl.h>

#undef max
#undef interface

export module DumplingWin32Define;

import std;
import Potato;


export namespace Dumpling::Win32
{
	struct ComWrapper
	{
		void AddRef(IUnknown* ptr) { ptr->AddRef(); }
		void SubRef(IUnknown* ptr) { ptr->Release(); }
		using PotatoPointerEnablePointerAccess = void;
	};

	template<typename PointerT>
	using ComPtr = Potato::Pointer::IntrusivePtr<PointerT, ComWrapper>;
}
