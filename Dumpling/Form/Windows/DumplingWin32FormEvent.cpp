module;

#include <cassert>
#include <Windows.h>

#undef max
#undef IGNORE
#undef interface

#define DUMPLING_WM_GLOBAL_MESSAGE static_cast<UINT>(WM_USER + 100)

module DumplingWin32FormEvent;

import std;


namespace Dumpling::Win32
{
	FormEvent::Respond FormEvent::RespondMarkAsHooked() const
	{
		switch (message)
		{
		case WM_CLOSE:
			return 1;
		}
		return S_OK;
	}

	FormEvent::Respond FormEvent::RespondMarkAsSkip() const
	{
		return -1;
	}

	bool FormEvent::IsRespondMarkAsHooked(Respond respond) const
	{
		return RespondMarkAsHooked() == respond;
	}

	void FormEvent::PostQuitEvent()
	{
		::PostQuitMessage(0);
	}
}