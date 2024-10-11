module;


export module Dumpling;

export import DumplingFormEvent;

#ifdef _WIN32
import DumplingWindowsForm;
import DumplingDx12Renderer;
#endif
export import DumplingPipeline;
//export import DumplingRenderer;

export namespace Dumpling
{
#ifdef _WIN32
	using Dumpling::Win32::FormEventCapture;
	using Dumpling::Win32::Form;
	using Dumpling::Dx12::DevicePtr;
	using Dumpling::Dx12::DescriptorHeapPtr;

	using Dumpling::Dx12::RendererResource;
	using Dumpling::Dx12::FormWrapper;
	using Dumpling::Dx12::RenderTargetSet;
	using Dumpling::Dx12::PassRenderer;
	using Dumpling::Dx12::FrameRenderer;
	using Dumpling::Dx12::Device;
#endif
}