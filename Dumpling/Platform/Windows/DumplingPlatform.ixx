module;
#include "unknwn.h"
export module DumplingPlatform;

import std;
import Potato;


export namespace Dumpling
{
	struct ComWrapper
	{
		void AddRef(IUnknown* ptr) { ptr->AddRef(); }
		void SubRef(IUnknown* ptr) { ptr->Release(); }
		using PotatoPointerEnablePointerAccess = void;
	};

	template<typename PointerT>
	using PlatformPtr = Potato::Pointer::IntrusivePtr<PointerT, ComWrapper>;
}