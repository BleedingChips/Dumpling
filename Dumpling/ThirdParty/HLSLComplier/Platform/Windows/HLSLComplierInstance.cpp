
module;

#include <cassert>
#include "d3dcommon.h"
#include "dxcapi.h"
#include "d3d12shader.h"

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
				DXC_CP_WIDE,
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

	std::optional<ShaderStatistics> Instance::GetShaderStatistics(ShaderReflection& target_reflection)
	{
		D3D12_SHADER_DESC desc;
		if(SUCCEEDED(target_reflection.GetDesc(&desc)))
		{
			ShaderStatistics statistics;
			statistics.const_buffer_count = desc.ConstantBuffers;
			return statistics;
		}
		return std::nullopt;
	}

	Potato::IR::StructLayout::Ptr CreateLayoutFromVariable(
		ID3D12ShaderReflectionVariable& variable,
		Potato::TMP::FunctionRef<Potato::IR::StructLayout::Ptr(std::u8string_view)> type_layout_override,
		std::pmr::memory_resource* layout_resource,
		std::pmr::memory_resource* temporary_resource
	)
	{
		auto var_type = variable.GetType();
		assert(var_type != nullptr);

		D3D12_SHADER_TYPE_DESC type_desc;
		if (SUCCEEDED(var_type->GetDesc(&type_desc)))
		{
			if (type_layout_override)
			{
				std::u8string_view type_name{ reinterpret_cast<char8_t const*>(type_desc.Name) };
				auto layout = type_layout_override(type_name);
			}
		}


		/*
		D3D12_SHADER_VARIABLE_DESC var_desc;
		if (SUCCEEDED(variable.GetDesc(&var_desc)))
		{
			std::u8string_view var_name{
				static_cast<wchar_t>(var_desc.Name)
			};
		}
		*/
		return {};
	}


	Potato::IR::StructLayoutObject::Ptr Instance::CreateLayoutFromCBuffer(
		ShaderReflection& target_reflection,
		std::size_t cbuffer_index,
		Potato::TMP::FunctionRef<Potato::IR::StructLayoutObject::Ptr(std::u8string_view)> cbuffer_layout_override,
		Potato::TMP::FunctionRef<Potato::IR::StructLayout::Ptr(std::u8string_view)> type_layout_override,
		std::pmr::memory_resource* layout_resource,
		std::pmr::memory_resource* temporary_resource
	)
	{
		ID3D12ShaderReflectionConstantBuffer* const_buffer = target_reflection.GetConstantBufferByIndex(cbuffer_index);

		if (const_buffer != nullptr)
		{
			D3D12_SHADER_BUFFER_DESC buffer_desc;
			const_buffer->GetDesc(&buffer_desc);
			std::u8string_view cbuffer_name{ reinterpret_cast<char8_t const*>(buffer_desc.Name) };
			if (cbuffer_layout_override)
			{
				auto layout_object = cbuffer_layout_override(cbuffer_name);
				if (layout_object && layout_object->GetStructLayout()->GetLayout().size == buffer_desc.Size)
				{
					return layout_object;
				}
			}
			std::pmr::vector<Potato::IR::StructLayout::Member> members(temporary_resource);
			members.reserve(buffer_desc.Variables);
			for (std::size_t index = 0; index < buffer_desc.Variables; ++index)
			{
				auto ver = const_buffer->GetVariableByIndex(index);
				assert(ver != nullptr);
				auto layout = CreateLayoutFromVariable(*ver, type_layout_override, temporary_resource, temporary_resource);
				volatile int i = 0;
				//ver->GetDesc();
			}
		}


		D3D12_SHADER_DESC shader_desc;

		if (!SUCCEEDED(target_reflection.GetDesc(&shader_desc)))
			return {};

		return {};

		/*
		for (std::size_t layout_count = 0; layout_count < shader_desc.ConstantBuffers && layout_count < output_layout.size(); ++layout_count)
		{
			ID3D12ShaderReflectionConstantBuffer* const_buffer = target_reflection->GetConstantBufferByIndex(layout_count);
			assert(const_buffer);
			D3D12_SHADER_BUFFER_DESC buffer_desc;
			const_buffer->GetDesc(&buffer_desc);
			std::u8string_view cbuffer_name{ reinterpret_cast<char8_t const*>(buffer_desc.Name) };
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
		*/
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
