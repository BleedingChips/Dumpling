module;

export module DumplingForm;

#ifdef _WIN32
export import DumplingWin32FormEvent;
export import DumplingWin32Form;

export namespace Dumpling
{
	using Form = Win32::Form;
	using FormMessage = Win32::FormMessage;
	using FormEvent = Win32::FormEvent;
	using FormEventHook = Win32::FormEventHook;
}
#endif // _WIN32
