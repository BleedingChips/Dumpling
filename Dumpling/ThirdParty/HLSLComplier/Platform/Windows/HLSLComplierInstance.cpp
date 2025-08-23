
module;
#include <wrl/client.h>
#include "d3dcommon.h"
#include "dxcapi.h"

module DumplingHLSLComplierInstance;

using Microsoft::WRL::ComPtr;

namespace Dumpling::HLSLCompiler
{
	constexpr wchar_t const* Translate(ShaderTarget target)
	{
		switch (target)
		{
		case ShaderTarget::VS_Lastest:
		case ShaderTarget::VS_6_0:
			return L"vs_6_0";
		default:
			return L"";
		}
	}

	wchar_t const* debug_argues = L"/Zi";
	wchar_t const* shader_output = L"-T";

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

	ArgumentPtr Instance::CreateArguments(ShaderTarget target, wchar_t const* entry_point, wchar_t const* file_path, ComplierFlag flag)
	{
		if (*this)
		{
			ArgumentPtr result;
			bool build_result = static_cast<IDxcUtils*>(utils.GetPointer())->BuildArguments(
				file_path,
				entry_point,
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

	EncodingBlobPtr Instance::EncodeShader(std::wstring_view shader_code)
	{
		if (*this)
		{
			EncodingBlobPtr blob;
			auto ptr = static_cast<IDxcUtils*>(utils.GetPointer());
			ptr->CreateBlob(
				shader_code.data(),
				shader_code.size() * sizeof(decltype(shader_code)::value_type),
				CP_WINUNICODE,
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

	EncodingBlobPtr Instance::GetErrorMessage(ResultPtr const& result)
	{
		if (result)
		{
			EncodingBlobPtr error;
			auto real_result = static_cast<IDxcResult*>(result.GetPointer());
			real_result->GetErrorBuffer(reinterpret_cast<IDxcBlobEncoding**>(&error.GetPointerReference()));
			return error;
		}
		return {};
	}

	bool Instance::CastToWCharString(EncodingBlobPtr const& blob, Potato::TMP::FunctionRef<void(std::wstring_view)> func)
	{
		if (*this && blob)
		{
			auto real_bloc = static_cast<IDxcBlobEncoding*>(blob.GetPointer());
			auto real_utils = static_cast<IDxcUtils*>(utils.GetPointer());
			
			ComPtr<IDxcBlobWide> output;
			real_utils->GetBlobAsWide(
				real_bloc,
				output.GetAddressOf()
			);

			if (output)
			{
				if (func)
				{
					std::wstring_view view{output->GetStringPointer(), output->GetStringLength()};
					func(view);
				}
				return true;
			}
		}
		return false;
	}
}
