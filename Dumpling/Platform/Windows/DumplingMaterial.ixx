module;
#include <d3d12.h>

export module DumplingMaterial;

import std;
import Potato;
import DumplingPlatform;
import DumplingRendererTypes;

export namespace Dumpling
{

	struct MaterialState
	{
		Potato::IR::StructLayout::Ptr vs_layout;
	};
}