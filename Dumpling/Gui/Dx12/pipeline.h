#pragma once
#include "pre_define_dx12.h"
#include "descriptor_table_dx12.h"
#include <tuple>
#include <filesystem>
namespace Dumpling::Dx12
{

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
		virtual ShaderType Type() const noexcept = 0;
		virtual std::tuple<const std::byte*, size_t> Code() const noexcept = 0;
	};

	using ShaderCodePtr = ComPtr<ShaderCode>;

	ShaderCodePtr LoadEntireFile(const std::filesystem::path&);

	struct PipelineInterface {
		virtual void AddRef() const noexcept = 0;
		virtual void Release() const noexcept = 0;
		virtual void Execute() const noexcept = 0;
	};
}