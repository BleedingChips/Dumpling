
module;

export module DumplingHLSLComplier;

import std;
export import DumplingHLSLComplierContext;

#ifdef _WIN32
export import DumplingHLSLCompilerDx12;
#endif


export namespace Dumpling::HLSLCompiler
{
#ifdef _WIN32
	using HLSLCompiler::Dx12::BlobPtr;
	using HLSLCompiler::Dx12::Context;
#endif
}
