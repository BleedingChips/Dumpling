module;

#include <cassert>
#include "unknwn.h"

export module DumplingDx12Define;

import std;
import Potato;
import DumplingWin32Define;

export namespace Dumpling::Dx12
{
	using StructLayout = Potato::IR::StructLayout;
	using Win32::ComPtr;

	enum class ShaderType
	{
		VS,
		PS,
	};

	enum class ShaderResourceType
	{
		CONST_BUFFER,
		TEXTURE,
		UNORDER_ACCED,
		SAMPLER,
		UNKNOW
	};

	/*
	enum TexFormat
	{
		RGBA8
	};

	struct RenderTargetSize
	{
		std::size_t width = 1;
		std::size_t height = 1;
		std::size_t length = 1;
	};

	struct FormRenderTargetProperty
	{
		std::size_t swap_chin_count = 2;
		std::optional<float> present;
	};

	enum class RendererResourceType
	{
		FLOAT32,
		UNSIGNED_INT64,
		TEXTURE_RT,
		TEXTURE_DS,
		TEXTURE1D,
		TEXTURE1D_ARRAY,
		TEXTURE2D,
		TEXTURE2D_ARRAY,
		TEXTURE3D,
		TEXTURE_CUBE
	};
	*/

}