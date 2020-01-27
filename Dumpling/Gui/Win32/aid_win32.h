#pragma once
#include "..//..//..//Potato/smart_pointer.h"
#include "..//..//..//Potato/tool.h"
#include <Windows.h>
#include <list>
#include <filesystem>
namespace Dumpling::Win32
{
	namespace Error {
		struct HRESULTError {
			HRESULT m_result;
		};
		
	}

	template<typename Return> Return ThrowIfFault(std::tuple<Return, HRESULT> input) {
		if (SUCCEEDED(std::get<1>(input)))
			return std::move(std::get<0>(input));
		else
			throw Error::HRESULTError{ std::get<1>(input) };
	}


	struct VoidT {};

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
		void** operator ()(SourceType*& pi, VoidT)
		{
			return reinterpret_cast<void**>(this->operator()(pi));
		}

		template<typename SourceType>
		Wrapper<SourceType> self_add_pos(SourceType* type) noexcept { return Wrapper<SourceType>{type}; }
	};

	template<typename Type> struct ComBase {
		void AddRef() const noexcept { m_Ref.add_ref(); }
		void Release() const noexcept { if (m_Ref.sub_ref()) delete static_cast<const Type*>(this); }
	private:
		mutable Potato::Tool::atomic_reference_count m_Ref;
	};

	template<typename Type> using ComPtr = Potato::Tool::intrusive_ptr<Type, ComWrapper>;

	std::list<std::filesystem::path> SearchVisualStudioPath();
}