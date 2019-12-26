#pragma once
#include "pre_define_dx12.h"
//#include "descriptor_table_dx12.h"
#include <tuple>
#include <filesystem>
#include <d3d12shader.h>
namespace Dumpling::Dx12
{

	

	using DescriptorHeap = ID3D12DescriptorHeap;
	using DescriptorHeapPtr = ComPtr<DescriptorHeap>;







	enum class ShaderType {
		VS = 0,
		DS,
		HS,
		GS,
		PS,
		Num,
	};

	struct ShaderCode {
		virtual void AddRef() const noexcept = 0;
		virtual bool Release() const noexcept = 0;
		virtual const std::byte* Code() const noexcept = 0;
		virtual size_t Length() const noexcept = 0;
		virtual ComPtr<ID3D12ShaderReflection> Reflection() const noexcept = 0;
	};

	using ShaderCodePtr = ComPtr<ShaderCode>;

	ShaderCodePtr LoadEntireFile(const std::filesystem::path&);

	enum class PipeLineType
	{
		Compute,
		Default,
		StreamOutput,
		Unknow,
	};

	struct PipeLine {
		virtual void AddRef() const noexcept = 0;
		virtual bool Release() const noexcept = 0;
		virtual PipeLineType Type() const noexcept = 0;
		//virtual void Execute() const noexcept = 0;
	};

	

	using PipeLinePtr = ComPtr<PipeLine>;

	PipeLinePtr CreatePipleLineFromSperatedShader(std::initializer_list<ShaderCodePtr>);


}