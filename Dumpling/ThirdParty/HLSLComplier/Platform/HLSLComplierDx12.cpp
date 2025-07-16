
module;
#include <d3dcompiler.h>

module DumplingHLSLCompilerDx12;



namespace Dumpling::HLSLCompiler::Dx12
{

	constexpr char const* Translate(Target::Category category)
	{
		switch(category)
		{
		case Target::Category::VS:
			return "vs_5_1";
		case Target::Category::CS:
			return "cs_5_1";
		case Target::Category::PS:
			return "ps_5_1";
		case Target::Category::MS:
			return "ps_5_1";
		}
		return nullptr;
	}


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

	std::u8string_view CompileResult::GetErrorMessage() const
	{
		if(error)
		{
			return std::u8string_view{
				static_cast<char8_t const*>(error->GetBufferPointer()),
				error->GetBufferSize()
			};
		}
		return {};
	}


	CompileResult Context::Compile(std::u8string_view code, Target const& compiler_target, char8_t const* source_name)
	{
		BlobPtr target_blob;
		BlobPtr error;
		auto re = D3DCompile2(
			reinterpret_cast<LPCVOID>(code.data()),
			code.size() * sizeof(decltype(code)::value_type),
			reinterpret_cast<LPCSTR>(source_name),
			nullptr,
			nullptr,
			reinterpret_cast<LPCSTR>(compiler_target.entry_point),
			Translate(compiler_target.category),
			0, 0, 0, nullptr, 0, target_blob.GetAddressOf(), error.GetAddressOf()
		);
		return {std::move(target_blob), std::move(error)};
	}
}
