
module;

#include <d3dcommon.h>
#include "d3d12shader.h"
#include "d3d12.h"

export module DumplingHLSLComplierInstance;

import std;
import Potato;
import DumplingDx12Define;
import DumplingDx12Shader;


namespace Dumpling::HLSLCompiler
{

	struct BlobWrapper
	{
		void AddRef(void* ptr);
		void SubRef(void* ptr);
		using PotatoPointerEnablePointerAccess = void;
	};

	struct EncodingBlobWrapper
	{
		void AddRef(void* ptr);
		void SubRef(void* ptr);
		using PotatoPointerEnablePointerAccess = void;
	};

	struct ArgumentWrapper
	{
		void AddRef(void* ptr);
		void SubRef(void* ptr);
		using PotatoPointerEnablePointerAccess = void;
	};

	struct CompilerWrapper
	{
		void AddRef(void* ptr);
		void SubRef(void* ptr);
		using PotatoPointerEnablePointerAccess = void;
	};

	struct ResultWrapper
	{
		void AddRef(void* ptr);
		void SubRef(void* ptr);
		using PotatoPointerEnablePointerAccess = void;
	};

	struct UtilsWrapper
	{
		void AddRef(void* ptr);
		void SubRef(void* ptr);
		using PotatoPointerEnablePointerAccess = void;
	};
}

export namespace Dumpling::HLSLCompiler
{

	using Potato::IR::Layout;
	using Potato::IR::StructLayout;
	using Potato::IR::StructLayoutObject;

	using BlobPtr = Potato::Pointer::IntrusivePtr<void, EncodingBlobWrapper>;
	using EncodingBlobPtr = Potato::Pointer::IntrusivePtr<void, EncodingBlobWrapper>;
	using ArgumentPtr = Potato::Pointer::IntrusivePtr<void, ArgumentWrapper>;
	using CompilerPtr = Potato::Pointer::IntrusivePtr<void, CompilerWrapper>;
	using ResultPtr = Potato::Pointer::IntrusivePtr<void, ResultWrapper>;

	enum class ShaderTarget
	{
		VS_6_0,
		
		PS_6_0,

		VS_Lastest = VS_6_0,
		PS_Lastest = PS_6_0,
	};

	constexpr ShaderType TranslateShaderType(ShaderTarget target)
	{
		switch (target)
		{
		case ShaderTarget::VS_6_0:
			return ShaderType::VS;
		}
		return ShaderType::PS;
	}

	enum class ComplieTargetVersion
	{
		VERSION_5
	};

	enum class ComplierFlag
	{
		None
	};

	struct ComplieContext
	{
		ComplieTargetVersion version = ComplieTargetVersion::VERSION_5;
		ComplierFlag flag = ComplierFlag::None;
		Potato::TMP::FunctionRef<Dx12::ShaderSlot::ConstBuffer(std::u8string_view)> cbuffer_layout_override;
		Potato::TMP::FunctionRef<StructLayout::Ptr(std::u8string_view)> type_layout_override;
		Potato::TMP::FunctionRef<void(std::u8string_view, ShaderTarget)> error_capture;
	};

	struct ShaderEnterPointView
	{
		std::u8string_view code;
		std::u8string_view entry_point;
		std::u8string_view file_path;
		operator bool() const { return !code.empty() && !entry_point.empty(); }
	};

	struct MaterialShaderContext
	{
		ShaderEnterPointView vs_entry_point;
		ShaderEnterPointView ps_entry_point;
	};

	struct MaterialShaderOutput
	{
		ComPtr<ID3D10Blob> vs;
		ComPtr<ID3D10Blob> ps;
	};

	struct Instance
	{
		Instance(Instance const&) = default;
		Instance(Instance&&) = default;
		Instance& operator=(Instance&& in_complier) = default;
		Instance& operator=(Instance const& in_complier) = default;
		Instance() = default;
		operator bool() const { return utils; }

		ComPtr<ID3D10Blob> GetShaderObject(ResultPtr const& result);
		ComPtr<ID3D12ShaderReflection> CreateReflection(ResultPtr const& result);
		bool GetErrorMessage(ResultPtr const& result, Potato::TMP::FunctionRef<void(std::u8string_view)> receive_function = {});
		EncodingBlobPtr EncodeShader(std::u8string_view shader_code);
		ArgumentPtr CreateArguments(ShaderTarget target, std::u8string_view entry_point, std::u8string_view file_path, ComplierFlag flag = ComplierFlag::None);
		CompilerPtr CreateCompiler();
		ResultPtr Compile(CompilerPtr& compiler, EncodingBlobPtr const& code, ArgumentPtr const& arguments);
		static Instance Create();

		bool CompileMaterial(CompilerPtr& compiler, Dx12::ShaderSlot& out_slot, MaterialShaderOutput& out_shader, MaterialShaderContext  const& material_context, ComplieContext const& context);

	protected:

		ComPtr<ID3D10Blob> CompileShader(CompilerPtr& compiler, ShaderTarget shader_type, Dx12::ShaderSlot& out_slot, ShaderEnterPointView entry_point, ComplieContext const& context);

		using Ptr = Potato::Pointer::IntrusivePtr<void, UtilsWrapper>;

		Ptr utils;
	};
}
