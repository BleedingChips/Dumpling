#pragma once
#include <d3dcompiler.h>
#include "../Dxgi/pre_define_dxgi.h"
#include <tuple>
#pragma comment(lib, "D3dcompiler.lib")
namespace Dumpling::Dx
{

	using Dumpling::Dxgi::ComPtr;

	namespace Enum
	{
		enum class ShaderTarget
		{
			PS_5_0,
			VS_5_0,
		};

		const char* operator*(ShaderTarget type) noexcept;
	}

	using Blob = ID3DBlob;
	using BlobPtr = ComPtr<Blob>;

	std::tuple<BlobPtr, BlobPtr> Complie(
		const char* shader_target,
		const char* ansi_code,
		size_t code_length,
		std::vector<std::tuple<const char*, const char*>> define
	);
}