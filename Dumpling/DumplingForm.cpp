module;

module DumplingForm;



#ifdef _WIN32
import DumplingWindowsForm;
#endif

namespace Dumpling
{
	bool Form::InsertCapture_AssumedLocked(FormEventCapture::Ptr capture, std::size_t priority)
	{
		if(capture)
		{
			auto ite = std::find_if(captures.begin(), captures.end(), [=](CaptureTuple& tuple)
			{
				return tuple.priority < priority;
			});
			captures.insert(ite, {priority, capture->AcceptedCategory(), capture});
			return true;
		}
		return false;
	}
	FormEvent::Respond Form::HandleEvent(FormEvent::System event)
	{
		std::shared_lock sl(capture_mutex);
		for(auto& ite : captures)
		{
			if((ite.acceptable_category & FormEvent::Category::SYSTEM) != FormEvent::Category::UNACCEPTABLE)
			{
				auto re = ite.capture->Receive(*this, event);
				if(re == FormEvent::Respond::CAPTURED)
				{
					return FormEvent::Respond::CAPTURED;
				}
			}
		}
		return FormEvent::Respond::PASS;
	}

	FormEvent::Respond Form::HandleEvent(FormEvent::Modify event)
	{
		std::shared_lock sl(capture_mutex);
		for(auto& ite : captures)
		{
			if((ite.acceptable_category & FormEvent::Category::MODIFY) != FormEvent::Category::UNACCEPTABLE)
			{
				auto re = ite.capture->Receive(*this, event);
				if(re == FormEvent::Respond::CAPTURED)
				{
					return FormEvent::Respond::CAPTURED;
				}
			}
		}
		return FormEvent::Respond::PASS;
	}

	FormEvent::Respond Form::HandleEvent(FormEvent::Input event)
	{
		std::shared_lock sl(capture_mutex);
		for(auto& ite : captures)
		{
			if((ite.acceptable_category & FormEvent::Category::INPUT) != FormEvent::Category::UNACCEPTABLE)
			{
				auto re = ite.capture->Receive(*this, event);
				if(re == FormEvent::Respond::CAPTURED)
				{
					return FormEvent::Respond::CAPTURED;
				}
			}
		}
		return FormEvent::Respond::PASS;
	}

	Form::Ptr Form::Create(
		std::size_t identity_id,
		std::pmr::memory_resource* resource
	)
	{
#ifdef _WIN32
		return Windows::Win32Form::Create(
			identity_id,
			resource
		);
#endif
	}

	bool Form::PeekMessageEventOnce(void(*func)(void*, FormEvent::System), void* data)
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