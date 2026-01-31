module;

#include <cassert>
#include "unknwn.h"

export module DumplingDx12Define;

import std;
import Potato;

export namespace Dumpling
{
	using StructLayout = Potato::IR::StructLayout;

	struct ComWrapper
	{
		void AddRef(IUnknown* ptr) { ptr->AddRef(); }
		void SubRef(IUnknown* ptr) { ptr->Release(); }
		using PotatoPointerEnablePointerAccess = void;
	};

	template<typename PointerT>
	using ComPtr = Potato::Pointer::IntrusivePtr<PointerT, ComWrapper>;

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
		SAMPLER
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