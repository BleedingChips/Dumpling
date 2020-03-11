#pragma once
namespace Dumpling 
{
	enum class MscToken {
		Work, // Work
		IntergateNumber, 
		Equal, // =
		Import, // Import
		String, // String
		Command, // #....
		Code, // @[...]@
		BraceLeft, // {
		BraceRight, // }

	};
}