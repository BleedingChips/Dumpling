
module;

#include <cassert>
#include "d3dcommon.h"
#include "dxcapi.h"
#include "d3d12shader.h"

#undef min
#undef max

module DumplingHLSLComplierInstance;

namespace Dumpling::HLSLCompiler
{
	constexpr wchar_t const* Translate(ShaderTarget target)
	{
		switch (target)
		{
		case ShaderTarget::VS_6_0:
			return L"vs_6_0";
		case ShaderTarget::PS_6_0:
			return L"ps_6_0";
		default:
			return L"";
		}
	}

	wchar_t const* debug_argues = L"/Zi";
	wchar_t const* shader_output = L"-T";

	void BlobWrapper::AddRef(void* ptr)
	{
		static_cast<IDxcBlob*>(ptr)->AddRef();
	}

	void BlobWrapper::SubRef(void* ptr)
	{
		static_cast<IDxcBlob*>(ptr)->Release();
	}

	void EncodingBlobWrapper::AddRef(void* ptr)
	{
		static_cast<IDxcBlobEncoding*>(ptr)->AddRef();
	}

	void EncodingBlobWrapper::SubRef(void* ptr)
	{
		static_cast<IDxcBlobEncoding*>(ptr)->Release();
	}

	void ArgumentWrapper::AddRef(void* ptr)
	{
		static_cast<IDxcCompilerArgs*>(ptr)->AddRef();
	}

	void ArgumentWrapper::SubRef(void* ptr)
	{
		static_cast<IDxcCompilerArgs*>(ptr)->Release();
	}

	void CompilerWrapper::AddRef(void* ptr)
	{
		static_cast<IDxcCompiler3*>(ptr)->AddRef();
	}

	void CompilerWrapper::SubRef(void* ptr)
	{
		static_cast<IDxcCompiler3*>(ptr)->Release();
	}

	void UtilsWrapper::AddRef(void* ptr)
	{
		static_cast<IDxcUtils*>(ptr)->AddRef();
	}

	void UtilsWrapper::SubRef(void* ptr)
	{
		static_cast<IDxcUtils*>(ptr)->Release();
	}


	void ResultWrapper::AddRef(void* ptr)
	{
		static_cast<IDxcResult*>(ptr)->AddRef();
	}

	void ResultWrapper::SubRef(void* ptr)
	{
		static_cast<IDxcResult*>(ptr)->Release();
	}

	Instance Instance::Create()
	{
		Instance result;
		if (SUCCEEDED(DxcCreateInstance(CLSID_DxcUtils, __uuidof(IDxcUtils), &result.utils.GetPointerReference())))
		{
			return result;
		}
		return {};
	}

	CompilerPtr Instance::CreateCompiler()
	{
		CompilerPtr result;
		if (SUCCEEDED(DxcCreateInstance(CLSID_DxcCompiler, __uuidof(IDxcCompiler3), &result.GetPointerReference())))
		{
			return result;
		}
		return {};
	}

	ArgumentPtr Instance::CreateArguments(ShaderTarget target, std::u8string_view entry_point, std::u8string_view file_path, ComplierFlag flag)
	{
		if (*this)
		{

			std::array<wchar_t, 1024> entry_point_w;
			std::array<wchar_t, 1024> file_path_w;

			{
				auto Info = Potato::Encode::StrEncoder<char8_t, wchar_t>{}.Encode(entry_point, std::span(entry_point_w));
				entry_point_w[std::min(Info.target_space, entry_point_w.size() - 1)] = L'\0';
			}

			{
				auto Info = Potato::Encode::StrEncoder<char8_t, wchar_t>{}.Encode(file_path, std::span(file_path_w));
				file_path_w[std::min(Info.target_space, entry_point_w.size() - 1)] = L'\0';
			}


			ArgumentPtr result;
			bool build_result = static_cast<IDxcUtils*>(utils.GetPointer())->BuildArguments(
				file_path_w.data(),
				entry_point_w.data(),
				Translate(target),
				nullptr,
				0,
				nullptr,
				0,
				reinterpret_cast<IDxcCompilerArgs**>(&result.GetPointerReference())
			);

			if (SUCCEEDED(build_result))
			{
				return result;
			}
		}
		return {};
	}

	EncodingBlobPtr Instance::EncodeShader(std::u8string_view shader_code)
	{
		if (*this)
		{
			EncodingBlobPtr blob;
			auto ptr = static_cast<IDxcUtils*>(utils.GetPointer());
			ptr->CreateBlob(
				shader_code.data(),
				shader_code.size() * sizeof(decltype(shader_code)::value_type),
				DXC_CP_UTF8,
				reinterpret_cast<IDxcBlobEncoding**>(&blob.GetPointerReference())
			);
			return blob;
		}
		return {};
	}

	ResultPtr Instance::Compile(CompilerPtr& compiler, EncodingBlobPtr const& code, ArgumentPtr const& arguments)
	{
		if (*this && compiler && arguments && code)
		{

			BOOL encoding_available = false;
			UINT32 encoding = 0;

			auto encoding_shader = static_cast<IDxcBlobEncoding*>(code.GetPointer());

			encoding_shader->GetEncoding(&encoding_available, &encoding);

			if (!encoding_available)
				return {};

			DxcBuffer scription{
				encoding_shader->GetBufferPointer(),
				encoding_shader->GetBufferSize(),
				encoding
			};

			ResultPtr result;

			static_cast<IDxcCompiler3*>(compiler.GetPointer())->Compile(
				&scription,
				static_cast<IDxcCompilerArgs*>(arguments.GetPointer())->GetArguments(),
				static_cast<IDxcCompilerArgs*>(arguments.GetPointer())->GetCount(),
				nullptr,
				__uuidof(IDxcResult), &result.GetPointerReference()
			);

			return result;
		}
		return {};
	}

	bool Instance::GetErrorMessage(ResultPtr const& result, Potato::TMP::FunctionRef<void(std::u8string_view)> receive_function)
	{
		if (result)
		{
			ComPtr<IDxcBlobUtf8> error_message;
			auto real_result = static_cast<IDxcResult*>(result.GetPointer());
			auto re = real_result->GetOutput(
				DXC_OUT_ERRORS,
				__uuidof(IDxcBlobUtf8),
				error_message.GetPointerVoidAdress(),
				nullptr
			);
			if (error_message)
			{
				if (receive_function && error_message->GetStringLength() > 0)
				{
					std::u8string_view error_message_view{
						reinterpret_cast<char8_t const*>(error_message->GetStringPointer()),
						error_message->GetStringLength()
					};
					receive_function(error_message_view);
				}
				return true;
			}
		}
		return {};
	}

	ComPtr<ID3D12ShaderReflection> Instance::CreateReflection(ResultPtr const& result)
	{
		if (!result)
			return {};

		BlobPtr blob;
		auto real_result = static_cast<IDxcResult*>(result.GetPointer());
		auto re = real_result->GetOutput(DXC_OUT_REFLECTION, __uuidof(IDxcBlob), blob.GetPointerVoidAdress(), nullptr);

		if (!SUCCEEDED(re))
			return {};

		auto real_blob = static_cast<IDxcBlob*>(blob.GetPointer());
		auto real_utils = static_cast<IDxcUtils*>(utils.GetPointer());
		ComPtr<ID3D12ShaderReflection> reflection;
		DxcBuffer buffer;
		buffer.Encoding = DXC_CP_ACP;
		buffer.Ptr = real_blob->GetBufferPointer();
		buffer.Size = real_blob->GetBufferSize();
		auto result_kk = real_utils->CreateReflection(
			&buffer,
			__uuidof(ID3D12ShaderReflection),
			reflection.GetPointerVoidAdress()
		);
		return reflection;
	}

	ComPtr<ID3D10Blob> Instance::GetShaderObject(ResultPtr const& result)
	{
		if (result)
		{
			ComPtr<ID3D10Blob> blob;
			auto real_result = static_cast<IDxcResult*>(result.GetPointer());
			real_result->GetOutput(DXC_OUT_OBJECT, __uuidof(decltype(blob)::Type), blob.GetPointerVoidAdress(), nullptr);
			return blob;
		}
		return {};
	}

	ComPtr<ID3D10Blob> Instance::CompileShader(CompilerPtr& compiler, ShaderTarget shader_target, Dx12::ShaderSlot& out_slot, ShaderEnterPointView entry_point, ComplieContext const& context)
	{
		auto encoded_code = EncodeShader(entry_point.code);
		auto argument = CreateArguments(shader_target, entry_point.entry_point, entry_point.file_path, context.flag);
		auto compiler_result = Compile(compiler, encoded_code, argument);

		if (!compiler_result)
			return {};

		if (!GetErrorMessage(compiler_result, [&](std::u8string_view error) {
			if (context.error_capture)
			{
				context.error_capture(error, shader_target);
			}
			}))
		{
			return {};
		}

		auto reflection = CreateReflection(compiler_result);

		if (!reflection)
			return {};

		ShaderType shader_type = TranslateShaderType(shader_target);

		Dx12::ShaderReflectionConstBufferContext reflection_context;
		reflection_context.type_layout_override = context.type_layout_override;

		if (!GetShaderSlot(shader_type, *reflection, out_slot, context.cbuffer_layout_override, reflection_context))
			return {};

		return GetShaderObject(compiler_result);
	}

	bool Instance::CompileMaterial(CompilerPtr& compiler, Dx12::ShaderSlot& out_slot, MaterialShaderOutput& out_shader, MaterialShaderContext const& material_context, ComplieContext const& context)
	{
		if (!compiler)
			return false;

		ComPtr<ID3D10Blob> vs_code;

		if (material_context.vs_entry_point)
		{
			auto target = ShaderTarget::VS_6_0;
			auto block = CompileShader(compiler, target, out_slot, material_context.vs_entry_point, context);
			if (!block)
			{
				return false;
			}
			vs_code = std::move(block);
		}

		ComPtr<ID3D10Blob> ps_code;

		if (material_context.ps_entry_point)
		{
			auto target = ShaderTarget::PS_6_0;
			auto block = CompileShader(compiler, target, out_slot, material_context.ps_entry_point, context);
			if (!block)
			{
				return false;
			}
			ps_code = std::move(block);
		}

		out_shader.vs = std::move(vs_code);
		out_shader.ps = std::move(ps_code);
		return true;
	}
}
