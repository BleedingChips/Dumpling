module;
#include <d3d12.h>

export module DumplingMaterial;

import std;
import Potato;
import DumplingPlatform;
import DumplingRendererTypes;

export namespace Dumpling
{
	struct Materail
	{
		ComPtr<ID3D12RootSignature> RootSignature;
	};
}