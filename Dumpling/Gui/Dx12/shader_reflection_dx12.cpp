#include "shader_reflection_dx12.h"
namespace Dumpling::Dx12
{
	std::tuple<ReflectionPtr, HRESULT> Reflect(std::byte* code, size_t code_length)
	{
		ReflectionPtr tem;
		HRESULT re = D3DReflect(code, code_length, __uuidof(Reflection), tem(void_t{}));
		return { std::move(tem), re };
	}
}