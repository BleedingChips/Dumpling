module;

#include <Windows.h>
#include <wrl.h>

#undef max
#undef interface

export module DumplingFormEvent;

import std;
import Potato;


export namespace Dumpling
{
	struct FormEvent : public MSG
	{
		FormEvent(FormEvent const&) = default;
		FormEvent(FormEvent&&) = default;
		FormEvent(MSG const& other) : MSG(other) {}
		FormEvent() = default;
		FormEvent& operator=(FormEvent const&) = default;
		FormEvent& operator=(FormEvent &&) = default;

		using Respond = HRESULT;

		Respond RespondMarkAsHooked() const;
		Respond RespondMarkAsSkip() const;
		bool IsRespondMarkAsHooked(Respond respond) const;
		bool IsFormDestory() const;
		static void PostQuitEvent();
	};

	struct FormEventHook
	{
		struct Wrapper
		{
			void AddRef(FormEventHook const* ptr) const { ptr->AddFormEventHookRef(); }
			void SubRef(FormEventHook const* ptr) const { ptr->SubFormEventHookRef(); }
		};

		using Ptr = Potato::Pointer::IntrusivePtr<FormEventHook, Wrapper>;

		virtual FormEvent::Respond Hook(FormEvent& event) = 0;
		virtual ~FormEventHook() = default;

	protected:

		virtual void AddFormEventHookRef() const = 0;
		virtual void SubFormEventHookRef() const = 0;
		
	};
}
