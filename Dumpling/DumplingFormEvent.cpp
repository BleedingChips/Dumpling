module;

module DumplingFormEvent;

namespace Dumpling
{
	bool CaptureManager::InsertCapture_AssumedLocked(FormEventCapture::Ptr capture, std::size_t priority)
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
	FormEvent::Respond CaptureManager::HandleEvent(Form& form, FormEvent::System event)
	{
		std::shared_lock sl(capture_mutex);
		for(auto& ite : captures)
		{
			if((ite.acceptable_category & FormEvent::Category::SYSTEM) != FormEvent::Category::UNACCEPTABLE)
			{
				auto re = ite.capture->Receive(form, event);
				if(re == FormEvent::Respond::CAPTURED)
				{
					return FormEvent::Respond::CAPTURED;
				}
			}
		}
		return FormEvent::Respond::PASS;
	}

	FormEvent::Respond CaptureManager::HandleEvent(Form& form, FormEvent::Modify event)
	{
		std::shared_lock sl(capture_mutex);
		for(auto& ite : captures)
		{
			if((ite.acceptable_category & FormEvent::Category::MODIFY) != FormEvent::Category::UNACCEPTABLE)
			{
				auto re = ite.capture->Receive(form, event);
				if(re == FormEvent::Respond::CAPTURED)
				{
					return FormEvent::Respond::CAPTURED;
				}
			}
		}
		return FormEvent::Respond::PASS;
	}

	FormEvent::Respond CaptureManager::HandleEvent(Form& form, FormEvent::Input event)
	{
		std::shared_lock sl(capture_mutex);
		for(auto& ite : captures)
		{
			if((ite.acceptable_category & FormEvent::Category::INPUT) != FormEvent::Category::UNACCEPTABLE)
			{
				auto re = ite.capture->Receive(form, event);
				if(re == FormEvent::Respond::CAPTURED)
				{
					return FormEvent::Respond::CAPTURED;
				}
			}
		}
		return FormEvent::Respond::PASS;
	}

}