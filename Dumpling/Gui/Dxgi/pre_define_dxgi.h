#pragma once
#include "..//..//..//Potato/smart_pointer.h"
#include <dxgi1_4.h>
namespace Dumpling::Dxgi
{

	struct void_t {};

	template<typename Type> struct Wrapper;

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

		template<typename SourceType>
		void** operator ()(SourceType*& pi, void_t)
		{
			return reinterpret_cast<void**>(this->operator()(pi));
		}

		template<typename SourceType>
		Wrapper<SourceType> self_add_pos(SourceType* type) noexcept { return Wrapper<SourceType>{type}; }
	};

	template<typename Type> using ComPtr = Potato::Tool::intrusive_ptr<Type, ComWrapper>;

	using Factory = IDXGIFactory4;
	using Adapter = IDXGIAdapter1;
	using Output = IDXGIOutput;

	using SwapChain = IDXGISwapChain1;
	using SwapChainDesc = DXGI_SWAP_CHAIN_DESC1;

	using FactoryPtr = ComPtr<Factory>;
	using AdapterPtr = ComPtr<Adapter>;
	using OutputPtr = ComPtr<Output>;
	using SwapChainPtr = ComPtr<SwapChain>;
}