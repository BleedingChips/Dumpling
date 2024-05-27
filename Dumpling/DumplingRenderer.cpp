module;


module DumplingRenderer;

import std;
import PotatoMisc;
import PotatoPointer;
import DumplingForm;

#ifdef _WIN32
import DumplingDx12Renderer;
#endif


namespace Dumpling
{
	auto HardDevice::Create(std::pmr::memory_resource* resource) -> Ptr
	{
#ifdef _WIN32
		return Dx12::HardDevice::Create(resource);
#endif
	}

	bool Renderer::Execute(RendererRequester::Ptr requester, Pipeline::Ptr pipeline)
	{
		return true;
	}

	std::optional<PassIdentity> Renderer::RegisterPass(PassProperty pass_property)
	{
		return std::nullopt;
	}

	bool Renderer::UnregisterPass(PassIdentity id)
	{
		return true;
	}

	SubRenderer::Ptr Renderer::EnumPass(PassIdentity id, std::size_t ite)
	{
		return {};
	}
}