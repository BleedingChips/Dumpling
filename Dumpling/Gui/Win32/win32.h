#pragma once
#include <future>
#include <string>
#include <Windows.h>
#include "../../Potato/Public/smart_pointer.h"
#include "../../Potato/Public/tool.h"
#include <functional>

namespace Dumpling::Win32
{

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

	struct ComBaseInterface {
		virtual void AddRef() const noexcept = 0;
		virtual void Release() const noexcept = 0;
	};

	template<typename Type> struct ComBaseImplement : Type {
		using Type::Type;
		virtual void AddRef() const noexcept override { m_Ref.add_ref(); }
		virtual void Release() const noexcept override { if (m_Ref.sub_ref()) delete static_cast<const Type*>(this); }
	private:
		mutable Potato::Tool::atomic_reference_count m_Ref;
	};

	template<typename Type> struct ComBase {
		void AddRef() const noexcept { m_Ref.add_ref(); }
		void Release() const noexcept { if (m_Ref.sub_ref()) delete static_cast<const Type*>(this); }
	private:
		mutable Potato::Tool::atomic_reference_count m_Ref;
	};

	template<typename Type> using ComPtr = Potato::Tool::intrusive_ptr<Type, ComWrapper>;

	struct FormStyle {

	};

	struct FormSetting {
		const wchar_t* Title = L"PO default title :>";
		uint32_t Width = 1024;
		uint32_t Height = 768;
		uint32_t ShiftX = 0;
		uint32_t ShiftY = 0;
	};

	const FormStyle& DefaultStyle() noexcept;

	struct Form;
	using FormPtr = ComPtr<Form>;

	struct Form : ComBase<Form>{
		using EventFunctionT = std::function<std::optional<LRESULT>(HWND, UINT, WPARAM, LPARAM)>;
		HWND GetHWnd() const noexcept { return m_Hwnd; }

		// std::optional<LRESULT>(HWND, UINT, WPARAM, LPARAM)
		void OverwriteEventFunction(EventFunctionT event_function) noexcept;
		//bool Available() const noexcept { return m_available; }
		static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
		static FormPtr Create(const FormSetting& Setting = FormSetting{}, const FormStyle& Style = DefaultStyle());
		virtual ~Form();
	protected:
		Form(const FormSetting& Setting, const FormStyle& Style);
	private:
		//std::atomic_bool m_available;
		std::optional<LRESULT> RespondEventInEventLoop(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
		HWND m_Hwnd;
		std::recursive_mutex m_EventFunctionMutex;
		EventFunctionT m_EventFunction;
	};

	namespace Error
	{
		struct FaultToCreate : std::exception
		{
			const char* what() const noexcept override { return m_String.c_str(); }
			FaultToCreate(std::string ErrorString) : m_String(std::move(ErrorString)) {}
		private:
			std::string m_String;
		};

		struct HRESULTError {
			HRESULT m_result;
		};	
	}

	inline void ThrowIfFault(HRESULT input) {
		if (!SUCCEEDED(input))
			throw Error::HRESULTError{ input };
	}

	template<typename Return> Return ThrowIfFault(std::tuple<Return, HRESULT> input) {
		if (SUCCEEDED(std::get<1>(input)))
			return std::move(std::get<0>(input));
		else
			throw Error::HRESULTError{ std::get<1>(input) };
	}
}