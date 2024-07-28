module;


module DumplingRenderer;

import std;
import PotatoMisc;
import PotatoPointer;
import DumplingForm;

#ifdef _WIN32
import DumplingDx12Renderer;
import DumplingDXGI;
#endif


namespace Dumpling
{

	Color Color::red = Color{1.0f, 0.0f, 0.0f};
	Color Color::white = Color{1.0f, 1.0f, 1.0f};
	Color Color::blue = Color{ 0.0f, 0.0f, 1.0f };
	Color Color::black = Color{};





	auto HardDevice::Create(std::pmr::memory_resource* resource) -> Ptr
	{
#ifdef _WIN32
		return DXGI::HardDevice::Create(resource);
#endif
	}

	Pass::Ptr Renderer::RegisterPass(PassProperty pass_property)
	{
		std::lock_guard lg(pass_mutex);
		return RegisterPass_AssumedLocked(std::move(pass_property));
	}

	Pass::Ptr Renderer::RegisterPass_AssumedLocked(PassProperty pass_property)
	{
		for(auto& ite : passes)
		{
			if(ite.pass_name == pass_property.name)
			{
				return {};
			}
		}
		auto pass_ptr = Pass::Create(pass_property, FastIndex{0, passes.size()}, std::pmr::get_default_resource());
		if(pass_ptr)
		{
			auto str = pass_ptr->GetName();
			passes.emplace_back(str, pass_ptr);
			return pass_ptr;
		}
		return {};
	}

	bool Renderer::Commited(PipelineRequester::Ptr requester, Pipeline const& pipeline)
	{
		std::shared_lock sl(pass_mutex);
		std::lock_guard lg(request_mutex);
		for(auto& ite : passes)
		{
			requests.emplace_back(
				ite.pass,
				Pipeline::Ptr{},
				requester,
				Potato::IR::StructLayoutObject::Ptr{}
			);
		}
		return true;
	}

	bool Renderer::UnregisterPass(Pass const& node)
	{
		std::lock_guard lg(pass_mutex);
		auto ite = std::find_if(passes.begin(), passes.end(), [&](
			PassTuple& tuple)
		{
			return tuple.pass.GetPointer() == &node;
		});
		if(ite != passes.end())
		{
			passes.erase(ite);
			return true;
		}
		return false;
	}

	PassRenderer::Ptr Renderer::PopPassRenderer(Pass const& node)
	{
		std::lock_guard lg(request_mutex);
		for(auto ite = requests.end(); ite != requests.begin(); --ite)
		{
			auto tar = ite - 1;
			if(tar->pass.GetPointer() == &node)
			{
				auto end_ite = requests.end() - 1;
				if(tar != end_ite)
				{
					std::swap(*tar, *(ite - 1));
				}
				auto renderer = CreatePassRenderer(end_ite->requester, end_ite->object, end_ite->pass->GetProperty(), std::pmr::get_default_resource());
				if(renderer)
				{
					requests.pop_back();
					return renderer;
				}else
				{
					return {};
				}
			}
		}
		return {};
	}

	void Renderer::FlushFrame()
	{
		
	}
}