module;

module DumplingForm;



#ifdef _WIN32
import DumplingWindowsForm;
#endif

namespace Dumpling
{

	FormEventRespond Form::HandleResponder(FormEvent event)
	{
		if(responder)
		{
			auto re = responder->Respond(*this, event);
			if(re != FormEventRespond::Default)
				return re;
		}
		return FormEventRespond::Default;
	}

	Form::Ptr Form::Create(
		FormEventResponder::Ptr respond,
		std::size_t identity_id,
		std::pmr::memory_resource* resource
	)
	{
#ifdef _WIN32
		return Windows::Win32Form::Create(
			std::move(respond),
			identity_id,
			resource
		);
#endif
	}

	bool Form::PeekMessageEventOnce(void(*func)(void*, Form*, FormEvent, FormEventRespond), void* data)
	{
#ifdef _WIN32
		return Windows::Win32Form::PeekMessageEvent(
			func,
			data
		);
#endif
	}

	void Form::PostFormQuitEvent()
	{
		#ifdef _WIN32
		return Windows::Win32Form::PostQuitEvent();
#endif
	}


}