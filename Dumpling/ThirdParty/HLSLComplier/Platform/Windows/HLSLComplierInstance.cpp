
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

	ShaderComplierArguments::ShaderComplierArguments(ShaderComplierArguments const& arguments)
	{
		argument = arguments.argument;
		if (argument != nullptr)
		{
			static_cast<IDxcCompilerArgs*>(argument)->AddRef();
		}
	}

	ShaderComplierArguments::ShaderComplierArguments(ShaderComplierArguments&& arguments)
	{
		argument = arguments.argument;
		arguments.argument = nullptr;
	}

	ShaderComplierArguments& ShaderComplierArguments::operator=(ShaderComplierArguments&& arguments)
	{
		auto temp = arguments.argument;
		arguments.argument = nullptr;
		argument = temp;
		return *this;
	}

	ShaderComplierArguments& ShaderComplierArguments::operator=(ShaderComplierArguments const& arguments)
	{
		auto temp = arguments.argument;
		if (temp != nullptr)
		{
			static_cast<IDxcCompilerArgs*>(temp)->AddRef();
		}
		if (argument != nullptr)
		{
			static_cast<IDxcCompilerArgs*>(temp)->Release();
		}
		argument = temp;
		return *this;
	}

	ShaderComplierArguments::~ShaderComplierArguments()
	{
		if (argument != nullptr)
		{
			static_cast<IDxcCompilerArgs*>(argument)->Release();
			argument = nullptr;
		}
	}

	ShaderComplier::ShaderComplier(ShaderComplier const& arguments)
	{
		complier = arguments.complier;
		if (complier != nullptr)
		{
			static_cast<IDxcComplier3*>(complier)->AddRef();
		}
	}

	ShaderComplier::ShaderComplier(ShaderComplier&& arguments)
	{
		complier = arguments.complier;
		arguments.complier = nullptr;
	}

	ShaderComplier& ShaderComplier::operator=(ShaderComplier&& arguments)
	{
		auto temp = arguments.complier;
		arguments.complier = nullptr;
		complier = temp;
		return *this;
	}

	ShaderComplier& ShaderComplier::operator=(ShaderComplier const& arguments)
	{
		auto temp = arguments.complier;
		if (temp != nullptr)
		{
			static_cast<IDxcComplier3*>(temp)->AddRef();
		}
		if (complier != nullptr)
		{
			static_cast<IDxcComplier3*>(temp)->Release();
		}
		complier = temp;
		return *this;
	}

	ShaderComplier::~ShaderComplier()
	{
		if (complier != nullptr)
		{
			static_cast<IDxcComplier3*>(complier)->Release();
			complier = nullptr;
		}
	}


	Instance::Instance(Instance const& arguments)
	{
		utils = arguments.utils;
		if (utils != nullptr)
		{
			static_cast<IDxcUtils*>(utils)->AddRef();
		}
	}

	Instance::Instance(Instance&& arguments)
	{
		utils = arguments.utils;
		arguments.utils = nullptr;
	}

	Instance& Instance::operator=(Instance&& arguments)
	{
		auto temp = arguments.utils;
		arguments.utils = nullptr;
		utils = temp;
		return *this;
	}

	Instance& Instance::operator=(Instance const& arguments)
	{
		auto temp = arguments.utils;
		if (temp != nullptr)
		{
			static_cast<IDxcUtils*>(temp)->AddRef();
		}
		if (utils != nullptr)
		{
			static_cast<IDxcUtils*>(temp)->Release();
		}
		utils = temp;
		return *this;
	}

	Instance::~Instance()
	{
		if (utils != nullptr)
		{
			static_cast<IDxcUtils*>(utils)->Release();
			utils = nullptr;
		}
	}

	Instance Instance::Create()
	{
		Instance result;
		if (SUCCEEDED(DxcCreateInstance(CLSID_DxcUtils, __uuidof(IDxcUtils), &result.utils)))
		{
			return result;
		}
		return {};
	}

	ShaderComplier Instance::CreateComplier()
	{
		ShaderComplier result;
		if (SUCCEEDED(DxcCreateInstance(CLSID_DxcCompiler, __uuidof(IDxcCompiler3), &result.complier)))
		{
			return result;
		}
		return {};
	}

	ShaderComplierArguments Instance::CreateArguments(ShaderTarget target, wchar_t const* entry_point, wchar_t const* file_path, ComplierFlag flag)
	{
		if (*this)
		{
			ShaderComplierArguments result;
			bool build_result = static_cast<IDxcUtils*>(utils)->BuildArguments(
				file_path,
				entry_point,
				Translate(target),
				nullptr,
				0,
				nullptr,
				0,
				reinterpret_cast<IDxcCompilerArgs**>(&result.argument)
			);

			if (SUCCEEDED(build_result))
			{
				return result;
			}
		}
		return {};
	}

	ShaderBufferPtr Instance::Complier(ShaderComplier& complier, std::wstring_view code, ShaderComplierArguments const& arguments, Potato::TMP::FunctionRef<void(std::wstring_view)> error)
	{
		if (*this && complier && arguments)
		{
			ComPtr<IDxcBlobEncoding> encoding_shader;
			auto ptr = static_cast<IDxcUtils*>(utils);
			auto encodeing_result = ptr->CreateBlob(
				code.data(),
				code.size() * sizeof(decltype(code)::value_type),
				CP_WINUNICODE,
				encoding_shader.GetAddressOf()
			);

			if (!SUCCEEDED(encodeing_result))
			{
				if (error)
				{
					error(L"source code encoding wrong");
				}
				return {};
			}

			BOOL encoding_available;
			UINT32 encoding = 0;

			if (!SUCCEEDED(encoding_shader->GetEncoding(&encoding_available, &encoding)))
			{
				if (error)
				{
					error(L"source code encoding wrong");
				}
				return {};
			}

			DxcBuffer scription{
				encoding_shader->GetBufferPointer(),
				encoding_shader->GetBufferSize(),
				encoding
			};

			ComPtr<IDxcResult> result;

			auto complier_result = static_cast<IDxcCompiler3*>(complier.complier)->Compile(
				&scription,
				static_cast<IDxcCompilerArgs*>(arguments.argument)->GetArguments(),
				static_cast<IDxcCompilerArgs*>(arguments.argument)->GetCount(),
				nullptr,
				__uuidof(IDxcResult), reinterpret_cast<void**>(result.GetAddressOf())
			);

			if (!result || !SUCCEEDED(complier_result))
			{
				if (error)
				{
					error(L"unable to complier shader");
				}
				return {};
			}

			HRESULT state;
			result->GetStatus(&state);

			if (!SUCCEEDED(state))
			{
				ComPtr<IDxcBlobEncoding> error_buffer;
				result->GetErrorBuffer(error_buffer.GetAddressOf());
				ComPtr<IDxcBlobWide> wchar_error;
				ptr->GetBlobAsWide(
					error_buffer.Get(),
					wchar_error.GetAddressOf()
				);
				if (wchar_error)
				{
					if (error)
					{
						error(std::wstring_view{ wchar_error->GetStringPointer(), wchar_error->GetStringLength()});
					}
					return {};
				}
			}

			ShaderBufferPtr output_shader;

			//result->Get(output_shader.GetAddressOf());
			return output_shader;
		}
		return {};
	}

	//ShaderComplierArguments CreateArguments(ShaderTarget target, ShaderLevel level, wchar_t const* entry_point, wchar_t const* file_path, ComplierFlag flag = ComplierFlag::None);

	/*
	struct Win32Instance : public Instance, public Potato::IR::MemoryResourceRecordIntrusiveInterface
	{
		Win32Instance(Potato::IR::MemoryResourceRecord record)
			:MemoryResourceRecordIntrusiveInterface(record) {
		}
		virtual ComPtr<ID3DBlob> Compile(std::wstring_view code, ShaderComplierProperty const& property, std::pmr::wstring* error_output)
		{






			ComPtr<IDxcBlobEncoding> encoding_shader;
			if (!SUCCEEDED(utils->CreateBlob(
				code.data(),
				code.size() * sizeof(decltype(code)::value_type),
				CP_UTF8,
				encoding_shader.GetAddressOf()
			)))
			{
				return {};
			}




			std::array<wchar_t const*, 2> argument_list
				= {
				L"-T",
				L"/Zi"
			};

			wchar_t const** target_argume = argument_list.data();


			ComPtr<IDxcCompilerArgs> argues;

			utils->BuildArguments(
				L"xxx.hlsl",
				L"main",
				L"vs_6_0",
				argument_list.data(),
				2,
				nullptr,
				0,
				argues.GetAddressOf()
			);

			auto k = argues->GetArguments();
			auto count = argues->GetCount();

			for (std::size_t i = 0; i < count; ++i)
			{
				auto kcc = k[i];
				volatile int i2 = 0;
			}

			BOOL EncodingAvailable;
			UINT32 Encoding = 0;
			encoding_shader->GetEncoding(&EncodingAvailable, &Encoding);

			DxcBuffer scription{
				encoding_shader->GetBufferPointer(),
				encoding_shader->GetBufferSize(),
				Encoding
			};

			ComPtr<IDxcResult> result;

			complier->Compile(
				&scription,
				argues->GetArguments(),
				argues->GetCount(),
				nullptr,
				__uuidof(IDxcResult), reinterpret_cast<void**>(result.GetAddressOf())
			);

			ComPtr<IDxcBlobEncoding> error;

			result->GetErrorBuffer(error.GetAddressOf());
			HRESULT state;
			result->GetStatus(&state);

			if (error)
			{
				ComPtr<IDxcBlobWide> wchar_error;
				utils->GetBlobAsWide(
					error.Get(),
					wchar_error.GetAddressOf()
				);
				std::wstring_view error_str = {
					wchar_error->GetStringPointer(),
					wchar_error->GetStringLength()
				};
				volatile int o = 0;
			}

			return { };
			return {};
		}

		ComPtr<IDxcCompiler3> complier;
		ComPtr<IDxcUtils> utils;
	protected:

		
		virtual void AddInstanceRef() const { MemoryResourceRecordIntrusiveInterface::AddRef(); }
		virtual void SubInstanceRef() const { MemoryResourceRecordIntrusiveInterface::SubRef(); }
	};


	auto Instance::Create(std::pmr::memory_resource* resource)
		-> Ptr
	{
		ComPtr<IDxcUtils> utils;
		DxcCreateInstance(CLSID_DxcUtils, __uuidof(IDxcUtils), reinterpret_cast<void**>(utils.GetAddressOf()));
		ComPtr<IDxcCompiler3> complier;
		DxcCreateInstance(CLSID_DxcCompiler, __uuidof(IDxcCompiler3), reinterpret_cast<void**>(complier.GetAddressOf()));
		if (complier && utils)
		{
			auto re = Potato::IR::MemoryResourceRecord::Allocate<Win32Instance>(resource);
			if (re)
			{
				Win32Instance* instance = new(re.Get()) Win32Instance{ re };
				instance->complier = std::move(complier);
				instance->utils = std::move(utils);
				return instance;
			}
		}
		return {};
	}
	*/

	/*
	std::u8string_view CompileResult::GetErrorMessage() const
	{
		if(error)
		{
			return std::u8string_view{
				static_cast<char8_t const*>(error->GetBufferPointer()),
				error->GetBufferSize()
			};
		}
		return {};
	}
	*/
}
