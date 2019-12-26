#include "pipeline.h"
#include <fstream>
#include <d3d12shader.h>
#include <d3dcompiler.h>

inline D3D12_SHADER_VERSION_TYPE GetType(const D3D12_SHADER_DESC& desc) noexcept { return static_cast<D3D12_SHADER_VERSION_TYPE>((desc.Version & 0xFFFF0000) >> 16); }
inline UINT GetMajorVersion(const D3D12_SHADER_DESC& desc) noexcept { return (desc.Version & 0x000000F0) >> 4; }
inline UINT GetMinVersion(const D3D12_SHADER_DESC& desc) noexcept { return desc.Version & 0x0000000F; }


namespace Dumpling::Dx12
{

	namespace Implement {
		struct SingleShaderFile : ShaderCode
		{
			std::vector<std::byte> mCode;
			ComPtr<ID3D12ShaderReflection> mReflection;
			mutable Potato::Tool::atomic_reference_count mRef;
			virtual void AddRef() const noexcept override { mRef.add_ref(); }
			virtual bool Release() const noexcept override { return mRef.sub_ref(); }
			virtual const std::byte* Code() const noexcept override { return mCode.data(); }
			virtual size_t Length() const noexcept override { return mCode.size(); }
			virtual ComPtr<ID3D12ShaderReflection> Reflection() const noexcept { return mReflection; }
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
				ComPtr<Implement::SingleShaderFile> Ptr = new Implement::SingleShaderFile{};
				Ptr->mCode = std::move(Temporary);
				Ptr->mReflection = std::move(Reflection);
				return Ptr;
				D3D12_SHADER_DESC desc;
				HRESULT re = Reflection->GetDesc(&desc);
				assert(SUCCEEDED(re));
				D3D12_SHADER_VERSION_TYPE Type = static_cast<D3D12_SHADER_VERSION_TYPE>((desc.Version & 0xFFFF0000) >> 16);
				uint64_t MajorVersion = (desc.Version & 0x000000F0) >> 4;
				uint64_t MinVersiobn = desc.Version & 0x0000000F;
				auto InputElement = desc.InputParameters;
				for (UINT i = 0; i < desc.InputParameters; ++i)
				{
					D3D12_SIGNATURE_PARAMETER_DESC desc;
					Reflection->GetInputParameterDesc(i, &desc);
					volatile int k = 0;
				}
			}
		}
		return {};
	}

	namespace Implement {
		struct SimplePipeline : PipeLine {
			mutable Potato::Tool::atomic_reference_count mRef;
			ShaderCodePtr mCodes[6];
			virtual void AddRef() const noexcept override { mRef.add_ref(); }
			virtual bool Release() const noexcept override { return mRef.sub_ref(); }
		};
	}

	PipeLineType CheckPipelineType(const std::array<ShaderCodePtr, 6>& InputShader)
	{
		static std::array<bool, 6> AllPipeLineState[7]
		{
			// Default,
			{ true, true, false, false, false, false },
			{ true, true, true, false, false, false },
			{ true, true, false, true, true, false },
			{ true, true, true, true, true, false },

			// Compute
			{ false, false, false, false, false, true },
			// StreamOutput
			{ true, true, true, false, false, false },
			{ false, true, true, true, true, false },
		};

		size_t i = 0;
		for (; i < 7; ++i)
		{
			bool Mark = true;
			for (size_t j = 0; j < 6; ++j)
			{
				if (AllPipeLineState[i][j] != static_cast<bool>(InputShader[j]))
				{
					Mark = false;
					break;
				}
			}
			if (Mark)
				break;
		}

		if (i >= 0 && i < 4) return PipeLineType::Default;
		else if (i >= 4 && i < 5) return PipeLineType::Compute;
		else if (i < 7) return PipeLineType::StreamOutput;
		else return PipeLineType::Unknow;
	}

	PipeLinePtr CreatePipleLineFromSperatedShader(std::initializer_list<ShaderCodePtr> List) {
		ComPtr<Implement::SimplePipeline> Ptr = new  Implement::SimplePipeline{};
		std::array<ShaderCodePtr, 6> AllCode;
		std::array<D3D12_SHADER_DESC, 6> AllType;
		for (auto& ite : List)
		{
			if (ite)
			{
				D3D12_SHADER_DESC Tem;
				ite->Reflection()->GetDesc(&Tem);
				auto Type = GetType(Tem);
				AllCode[Type] = ite;
				AllType[Type] = Tem;
			}
		}
		PipeLineType Type = CheckPipelineType(AllCode);
		if (Type != PipeLineType::Unknow)
		{
			struct DataType {
				std::string Name;
			};
			std::vector<DataType> AllDataType;
			for (size_t i = 0; i < 6; ++i)
			{

			}
		}
		else
			return {};
	};
}