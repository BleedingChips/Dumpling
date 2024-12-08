
module;

#include "d3d12.h"

export module DumplingHLSLCompilerDx12;

import std;
import PotatoIR;
import PotatoPointer;
import DumplingWindowsForm;
import DumplingDx12Renderer;
import DumplingHLSLComplierContext;

export namespace Dumpling::HLSLCompiler::Dx12
{

	using BlobPtr = Win32::ComPtr<ID3DBlob>;

	struct CompileResult
	{
		BlobPtr code;
		BlobPtr error;
		operator bool() const { return code; }
		std::u8string_view GetErrorMessage() const;
	};

	struct Context : public Potato::IR::MemoryResourceRecordIntrusiveInterface
	{
		using Ptr = Potato::Pointer::IntrusivePtr<Context>;
		static Ptr Create(std::pmr::memory_resource* resource = std::pmr::get_default_resource());
		CompileResult Compile(std::u8string_view code, Target const& compiler_target, char8_t const* source_name = nullptr);
	protected:
		Context(Potato::IR::MemoryResourceRecord record) : MemoryResourceRecordIntrusiveInterface(record) {}
	};


	
}
