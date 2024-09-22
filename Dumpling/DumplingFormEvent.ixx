module;


export module DumplingFormEvent;

import std;
import PotatoMisc;
import PotatoPointer;
import PotatoTaskSystem;

export namespace Dumpling
{

	export namespace FormEvent
	{
		enum class Category
		{
			UNACCEPTABLE = 0,
			SYSTEM = 0x01,
			MODIFY = 0x02,
			INPUT = 0x04,
			RAW = 0xFF,
		};

		struct System
		{
			enum class Message
			{
				QUIT
			};
			Message message;
		};

		struct Modify
		{
			enum class Message
			{
				DESTROY
			};
			Message message;
		};

		struct Input
		{
			/*
			enum class Message
			{
				DESTROY
			};
			Message message;
			*/
		};

		enum class Respond
		{
			PASS,
			CAPTURED,
		};

	}

	enum class FormStyle
	{
		FixedSizeWindow,
	};

	struct FormSize
	{
		std::size_t width = 1024;
		std::size_t height = 768;
	};

	struct FormProperty
	{
		FormStyle style = FormStyle::FixedSizeWindow;
		FormSize form_size;
		std::u8string_view title = u8"Default Dumping Form";
	};

}

export constexpr Dumpling::FormEvent::Category operator& (Dumpling::FormEvent::Category c1, Dumpling::FormEvent::Category c2)
{
	return static_cast<Dumpling::FormEvent::Category>(
		static_cast<std::size_t>(c1) & static_cast<std::size_t>(c2)
	);
}

export constexpr Dumpling::FormEvent::Category operator| (Dumpling::FormEvent::Category c1, Dumpling::FormEvent::Category c2)
{
	return static_cast<Dumpling::FormEvent::Category>(
		static_cast<std::size_t>(c1) | static_cast<std::size_t>(c2)
	);
}