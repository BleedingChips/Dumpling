
module;

export module DumplingHLSLComplierContext;

import std;


export namespace Dumpling::HLSLCompiler
{
	struct Target
	{
		enum Category
		{
			VS,
			PS,
			CS,
			UnKnow
		};

		Category category = Category::UnKnow;
		char8_t const* entry_point = nullptr;
		char8_t const* source_name = nullptr;
	};

}
