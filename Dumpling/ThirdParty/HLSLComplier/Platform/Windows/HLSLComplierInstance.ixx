
module;

#include <d3dcommon.h>
#include "d3d12shader.h"

export module DumplingHLSLComplierInstance;

import std;
import Potato;
import DumplingPlatform;
import DumplingRendererTypes;


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

	enum class ShaderTarget
	{
		VS_6_0,
		VS_Lastest,
	};

	enum class ComplierFlag
	{
		None
	};

	using BlobPtr = Potato::Pointer::IntrusivePtr<void, EncodingBlobWrapper>;
	using EncodingBlobPtr = Potato::Pointer::IntrusivePtr<void, EncodingBlobWrapper>;
	using ArgumentPtr = Potato::Pointer::IntrusivePtr<void, ArgumentWrapper>;
	using CompilerPtr = Potato::Pointer::IntrusivePtr<void, CompilerWrapper>;
	using ResultPtr = Potato::Pointer::IntrusivePtr<void, ResultWrapper>;

	using ShaderReflection = ID3D12ShaderReflection;
	using ShaderReflectionPtr = PlatformPtr<ShaderReflection>;

	struct ShaderStatistics
	{
		std::size_t const_buffer_count = 0;
	};

	struct Instance
	{
		Instance(Instance const&) = default;
		Instance(Instance&&) = default;
		Instance& operator=(Instance&& in_complier) = default;
		Instance& operator=(Instance const& in_complier) = default;
		Instance() = default;
		operator bool() const { return utils; }
		static BlobPtr GetShaderObject(ResultPtr const& result);
		bool GetErrorMessage(ResultPtr const& result, Potato::TMP::FunctionRef<void(std::u8string_view)> receive_function = {});
		EncodingBlobPtr EncodeShader(std::u8string_view shader_code);
		ArgumentPtr CreateArguments(ShaderTarget target, std::u8string_view entry_point, std::u8string_view file_path, ComplierFlag flag = ComplierFlag::None);
		CompilerPtr CreateCompiler();
		ResultPtr Compile(CompilerPtr& compiler, EncodingBlobPtr const& code, ArgumentPtr const& arguments);
		ShaderReflectionPtr CreateReflection(BlobPtr const& shader_object);
		
		static std::optional<ShaderStatistics> GetShaderStatistics(ShaderReflection& target_reflection);
		static StructLayoutObject::Ptr CreateLayoutFromCBuffer(
			ShaderReflection& target_reflection,
			std::size_t cbuffer_index,
			Potato::TMP::FunctionRef<StructLayoutObject::Ptr(std::u8string_view)> cbuffer_layout_override = {},
			Potato::TMP::FunctionRef<std::tuple<StructLayout::Ptr, Layout>(std::u8string_view)> type_layout_override = {},
			std::pmr::memory_resource* layout_resource = std::pmr::get_default_resource(),
			std::pmr::memory_resource* temporary_resource = std::pmr::get_default_resource()
		);

		static Instance Create();
	protected:

		using Ptr = Potato::Pointer::IntrusivePtr<void, UtilsWrapper>;

		Ptr utils;
	};
}
