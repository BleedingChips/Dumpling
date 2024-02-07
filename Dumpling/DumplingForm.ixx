module;


export module DumplingForm;

import std;
import PotatoMisc;
import PotatoPointer;

#ifdef _WIN32
import DumplingWindows;
#endif

export namespace Dumpling
{

	


	/*
	struct FormSetting
	{
		std::size_t size_x = 1024;
		std::size_t size_y = 768;
		std::optional<std::size_t> offset_x;
		std::optional<std::size_t> offset_y;
		std::wstring_view form_name = L"Fuck You Windows";
	};

	enum class Status
	{
		Empty,
		Opened,
		Closed,
		Hidden,
		Error,
	};

	struct FormEventChannel
	{
		virtual void HandleEvent();
	};

	struct Form : public Potato::Pointer::DefaultControllerViewerInterface
	{
		using Ptr = Potato::Pointer::ControllerPtr<Form>;

		virtual void CloseWindows() = 0;
		virtual Status GetStatus() const = 0;

#ifdef _WIN32
		static Form::Ptr CreateDx12Form(FormSetting setting);
#endif
	protected:

		virtual ~Form() = default;
	};

	*/

}