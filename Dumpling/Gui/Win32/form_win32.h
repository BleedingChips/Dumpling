#pragma once
#include <future>
#include <string>
#include <Windows.h>
#include "aid_win32.h"
#include "..//..//..//Potato//tool.h"
#include <functional>
namespace Dumpling::Win32
{
	namespace Error {
		struct FaultToCreate : std::exception
		{
			const char* what() const noexcept override { return m_String.c_str(); }
			FaultToCreate(std::string ErrorString) : m_String(std::move(ErrorString)) {}
		private:
			std::string m_String;
		};
	}

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

	
}