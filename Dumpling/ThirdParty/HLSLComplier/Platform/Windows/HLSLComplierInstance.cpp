
module;
#include <wrl/client.h>
#include "dxcapi.h"

module DumplingHLSLComplierInstance;

using Microsoft::WRL::ComPtr;

namespace Dumpling::HLSLCompiler
{
	constexpr char const* Translate(Target::Category category)
	{
		switch (category)
		{
		case Target::Category::VS:
			return "vs_5_1";
		case Target::Category::CS:
			return "cs_5_1";
		case Target::Category::PS:
			return "ps_5_1";
		case Target::Category::MS:
			return "ps_5_1";
		}
		return nullptr;
	}

	struct Win32Instance : public Instance, public Potato::IR::MemoryResourceRecordIntrusiveInterface
	{
		Win32Instance(Potato::IR::MemoryResourceRecord record, ComPtr<IDxcCompiler3> complier, ComPtr<IDxcUtils> utils)
			:MemoryResourceRecordIntrusiveInterface(record) {
		}
		virtual CompileResult Compile(std::u8string_view code, Target const& compiler_target, char8_t const* source_name = nullptr)
		{

			ComPtr<IDxcBlobEncoding> encoding_shader;
			auto re = utils->CreateBlob(
				code.data(),
				code.size() * sizeof(decltype(code)::value_type),
				CP_UTF8,
				encoding_shader.GetAddressOf()
			);

			/*
			complier->Compile(
				encoding_shader,

			);
			*/
			/*
			complier->Compile()
			*/
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
		DxcCreateInstance(CLSID_DxcCompiler, __uuidof(IDxcUtils), reinterpret_cast<void**>(utils.GetAddressOf()));
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
			}
		}
		return {};
	}

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
