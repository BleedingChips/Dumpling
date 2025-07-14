module;

#include <cassert>
#include <Windows.h>

#undef max
#undef IGNORE
#undef interface

#define DUMPLING_WM_GLOBAL_MESSAGE static_cast<UINT>(WM_USER + 100)

module DumplingFormEvent;

import std;


namespace Dumpling
{
	FormEvent::Respond FormEvent::RespondMarkAsHooked() const
	{
		return S_OK;
	}

	FormEvent::Respond FormEvent::RespondMarkAsSkip() const
	{
		return -1;
	}

	bool FormEvent::IsRespondMarkAsHooked(Respond respond) const
	{
		return SUCCEEDED(respond);
	}

	bool FormEvent::IsFormDestory() const
	{
		return message == WM_DESTROY;
	}

	void FormEvent::PostQuitEvent()
	{
		::PostQuitMessage(0);
	}
}