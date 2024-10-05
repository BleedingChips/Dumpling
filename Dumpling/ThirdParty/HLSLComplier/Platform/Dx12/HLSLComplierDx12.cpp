
module;
#include <d3dcompiler.h>

module DumplingHLSLCompilerDx12;



namespace Dumpling::HLSLCompiler::Dx12
{
	auto Context::Create(std::pmr::memory_resource* resource)
		-> Ptr
	{
		auto re = Potato::IR::MemoryResourceRecord::Allocate<Context>(resource);
		if(re)
		{
			return new(re.Get()) Context{re};
		}
		return {};
	}


	BlobPtr Context::Compiler(std::u8string_view code, Target const& compiler_target)
	{
		BlobPtr target_blob;
		auto re = D3DCompile2(
			reinterpret_cast<LPCVOID>(code.data()),
			code.size() * sizeof(decltype(code)::value_type),
			nullptr,
			nullptr,
			nullptr,
			reinterpret_cast<LPCSTR>(compiler_target.entry_point),
			"vs_5_1",
			0, 0, 0, nullptr, 0, target_blob.GetAddressOf(), nullptr
		);
		if(SUCCEEDED(re))
		{
			return target_blob;
		}
		return {};
	}
}
