#pragma once
#include <d3d12shader.h>
#include "..//Dx/complier_dx.h"
#include "define_dx12.h"

namespace Dumpling::Dx12
{
	using Reflection = ID3D12ShaderReflection;
	using ReflectionPtr = ComPtr<Reflection>;
	std::tuple<ReflectionPtr, HRESULT> Reflect(std::byte* code, size_t code_length);
	std::tuple<std::vector<std::byte>, HRESULT> ComplieShader(const char* string, size_t string_length);
}
