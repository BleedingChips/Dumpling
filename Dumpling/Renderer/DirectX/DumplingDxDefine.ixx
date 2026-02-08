module;

export module DumplingDxDefine;
import Potato;
import DumplingWin32Define;
import DumplingRendererDefine;

export namespace Dumpling::Dx
{
	using Dumpling::Renderer::ShaderResourceType;
	using Dumpling::Renderer::ShaderType;
	using StructLayout = Potato::IR::StructLayout;
	using Win32::ComPtr;
}