
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
			MS,
			UnKnow
		};

		Category category = Category::UnKnow;
		char8_t const* entry_point = nullptr;
	};
}
