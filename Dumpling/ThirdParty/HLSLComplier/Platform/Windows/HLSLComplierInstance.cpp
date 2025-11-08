
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
		case ShaderTarget::VS_Lastest:
		case ShaderTarget::VS_6_0:
			return L"vs_6_0";
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
			PlatformPtr<IDxcBlobUtf8> error_message;
			auto real_result = static_cast<IDxcResult*>(result.GetPointer());
			auto re = real_result->GetOutput(
				DXC_OUT_ERRORS,
				__uuidof(IDxcBlobUtf8),
				error_message.GetPointerVoidAdress(),
				nullptr
			);
			if (error_message)
			{
				if (receive_function)
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

	BlobPtr Instance::GetShaderObject(ResultPtr const& result)
	{
		if (result)
		{
			BlobPtr blob;
			auto real_result = static_cast<IDxcResult*>(result.GetPointer());
			real_result->GetOutput(DXC_OUT_REFLECTION, __uuidof(IDxcBlob), blob.GetPointerVoidAdress(), nullptr);
			return blob;
		}
		return {};
	}

	/*
	std::optional<std::size_t> Instance::GetConstBufferStructLayoutFromReflection(
		ShaderReflectionPtr const& target_reflection,
		std::span<Potato::IR::StructLayout::Ptr> output_layout,
		Potato::TMP::FunctionRef<Potato::IR::StructLayout::Ptr(std::u8string_view)> layout_mapping,
		std::pmr::memory_resource* layout_resource,
		std::pmr::memory_resource* temporary_resource
	)
	{
		if (target_reflection && layout_resource != nullptr)
		{
			D3D12_SHADER_DESC shader_desc;

			if(!SUCCEEDED(target_reflection->GetDesc(&shader_desc)))
				return std::nullopt;

			for (std::size_t layout_count = 0; layout_count < shader_desc.ConstantBuffers && layout_count < output_layout.size(); ++layout_count)
			{
				ID3D12ShaderReflectionConstantBuffer* const_buffer = target_reflection->GetConstantBufferByIndex(layout_count);
				assert(const_buffer);
				D3D12_SHADER_BUFFER_DESC buffer_desc;
				const_buffer->GetDesc(&buffer_desc);
				std::u8string_view cbuffer_name{reinterpret_cast<char8_t const*>(buffer_desc.Name)};
				if (layout_mapping)
				{
					auto layout = layout_mapping(cbuffer_name);
					if (layout)
					{
						output_layout[layout_count] = std::move(layout);
						continue;
					}
				}
			}
		}
		return std::nullopt;
	}
	*/

	ShaderReflectionPtr Instance::CreateReflection(BlobPtr const& shader_object)
	{
		if (*this && shader_object)
		{
			auto real_blob = static_cast<IDxcBlob*>(shader_object.GetPointer());
			auto real_utils = static_cast<IDxcUtils*>(utils.GetPointer());
			ShaderReflectionPtr reflection;
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
		return {};
	}
}
