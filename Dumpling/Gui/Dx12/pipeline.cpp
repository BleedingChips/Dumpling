#include "pipeline.h"
#include <fstream>
#include <d3d12shader.h>
#include <d3dcompiler.h>
namespace Dumpling::Dx12
{

	namespace Implement {
		struct SingleShaderFile : ShaderCode
		{
			ShaderType mType;
			std::vector<std::byte> mCode;
			mutable Potato::Tool::atomic_reference_count mRef;
			virtual void AddRef() const noexcept override { mRef.add_ref(); }
			virtual bool Release() const noexcept override { return mRef.sub_ref(); }
			virtual ShaderType Type() const noexcept override { return mType; }
			virtual std::tuple<const std::byte*, size_t> Code() const noexcept override { return { mCode.data(), mCode.size() }; }
		};
	}

	ShaderCodePtr LoadEntireFile(const std::filesystem::path& Path)
	{
		std::ifstream input(Path, std::ios::binary);
		if (input.is_open())
		{
			size_t file_size = std::filesystem::file_size(Path);
			std::vector<std::byte> Temporary(file_size, std::byte{ 0 });
			input.read(reinterpret_cast<char*>(Temporary.data()), Temporary.size());
			input.close();
			ComPtr<ID3D12ShaderReflection> Reflection;
			HRESULT re = D3DReflect(Temporary.data(), Temporary.size(), __uuidof(ID3D12ShaderReflection), Reflection(VoidT{}));
			if (SUCCEEDED(re))
			{
				ShaderCodePtr Ptr = new Implement::SingleShaderFile{};
				D3D12_SHADER_DESC desc;
				HRESULT re = Reflection->GetDesc(&desc);
				assert(SUCCEEDED(re));
			}
		}
		return {};
	}
}