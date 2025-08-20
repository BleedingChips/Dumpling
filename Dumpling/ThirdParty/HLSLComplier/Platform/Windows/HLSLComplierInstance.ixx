
module;
#include <wrl/client.h>
#include <d3dcommon.h>

export module DumplingHLSLComplierInstance;

import std;
import Potato;
import DumplingRendererTypes;


export namespace Dumpling::HLSLCompiler
{
	using Microsoft::WRL::ComPtr;

	struct Target
	{
		enum Category
		{
			VS,
			PS,
			CS,
			MS,
			UnKnow
		};

		Category category = Category::UnKnow;
		char8_t const* entry_point = nullptr;
	};






	struct Shader
	{

	};

	struct CompileResult
	{
		ComPtr<ID3DBlob> blob;
		ComPtr<ID3DBlob> error_message;
	};

	struct Instance
	{
		struct Wrapper
		{
			void AddRef(Instance const* ptr) { ptr->AddInstanceRef(); }
			void SubRef(Instance const* ptr) { ptr->SubInstanceRef(); }
		};

		using Ptr = Potato::Pointer::IntrusivePtr<Instance, Wrapper>;



		static Ptr Create(std::pmr::memory_resource* resource = std::pmr::get_default_resource());
		virtual CompileResult Compile(std::u8string_view code, Target const& compiler_target, char8_t const* source_name = nullptr) = 0;
	protected:
		virtual void AddInstanceRef() const = 0;
		virtual void SubInstanceRef() const = 0;
	};
}
