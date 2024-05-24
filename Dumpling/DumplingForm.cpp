module;

module DumplingForm;



#ifdef _WIN32
import DumplingWindowsForm;
#endif

namespace Dumpling
{

	bool Form::OverrideTemporaryOutputFunction(
			FormEventRespond(*output_func)(void*, Form*, FormEvent),
			void* output_func_data,
			bool set
		)
	{
		std::lock_guard lg(mutex);
		if(set)
		{
			if(output_event_responder == nullptr)
			{
				output_event_responder = output_func;
				output_event_responder_data = output_func_data;
				return true;
			}
		}else
		{
			if(output_event_responder == output_func && output_event_responder_data == output_func_data)
			{
				output_event_responder = nullptr;
				output_event_responder_data = nullptr;
				return true;
			}
		}
		
		return false;
	}

	FormEventRespond Form::HandleResponder(FormEvent event)
	{
		if(responder)
		{
			auto re = responder->Respond(*this, event);
			if(re != FormEventRespond::Default)
				return re;
		}
		std::shared_lock sl(mutex);
		if(output_event_responder != nullptr)
		{
			return (*output_event_responder)(output_event_responder_data, this, event);
		}
		return FormEventRespond::Default;
	}

	Form::Ptr Form::Create(
		FormEventResponder::Ptr respond,
		FormRenderer::Ptr render_target,
		std::size_t identity_id,
		std::pmr::memory_resource* resource
	)
	{
#ifdef _WIN32
		return Windows::Win32Form::Create(
			std::move(respond),
			std::move(render_target),
			identity_id,
			resource
		);
#endif
	}

	bool Form::PeekMessageEventOnce(FormEventRespond(*func)(void*, Form*, FormEvent), void* data)
	{
#ifdef _WIN32
		return Windows::Win32Form::PeekMessageEvent(
			func,
			data
		);
#endif
	}
}