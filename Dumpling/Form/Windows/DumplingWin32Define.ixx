module;



export module DumplingWin32Define;

import std;
import Potato;


export namespace Dumpling::Win32
{
	struct ComWrapper
	{
		template<typename PointerT>
		void AddRef(PointerT* ptr) { ptr->AddRef(); }
		template<typename PointerT>
		void SubRef(PointerT* ptr) { ptr->Release(); }
		using PotatoPointerEnablePointerAccess = void;
	};

	template<typename PointerT>
	using ComPtr = Potato::Pointer::IntrusivePtr<PointerT, ComWrapper>;
}
