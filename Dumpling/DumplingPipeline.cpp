module;


module DumplingPipeline;


namespace Dumpling
{
	PipelineManager::PipelineManager(MemorySetting setting)
		: passes(setting.self_resource), requests(setting.self_resource), pass_resource(setting.pass_resource)
	{
	}



	auto Pass::Create(PassProperty property, FastIndex index,  std::pmr::memory_resource* resource)
		->Ptr
	{
		auto layout = Potato::IR::Layout::Get<Pass>();
		auto offset = Potato::IR::InsertLayoutCPP(layout, Potato::IR::Layout::GetArray<char8_t>(property.name.size()));
		auto re = Potato::IR::MemoryResourceRecord::Allocate(resource, layout);
		if(re)
		{
			std::memcpy(re.GetByte() + offset, property.name.data(), property.name.size() * sizeof(char8_t));
			std::u8string_view view {
				reinterpret_cast<char8_t*>(re.GetByte() + offset),
				property.name.size()
			};
			property.name = view;
			return new (re.Get()) Pass{
				re, std::move(property), index
			};
		}
		return {};
	}

	/*
	auto Pipeline::Create(std::pmr::memory_resource* resource)
		-> Ptr
	{
		return Potato::IR::MemoryResourceRecord::AllocateAndConstruct<Pipeline>(resource);
	}
	*/

	Pass::Ptr PipelineManager::RegisterPass(PassProperty pass_property)
	{
		for(auto& ite : passes)
		{
			if(ite.pass_name == pass_property.name)
			{
				return {};
			}
		}
		auto pass_ptr = Pass::Create(pass_property, FastIndex{0, passes.size()}, pass_resource);
		if(pass_ptr)
		{
			auto str = pass_ptr->GetName();
			passes.emplace_back(str, pass_ptr);
			return pass_ptr;
		}
		return {};
	}

	bool PipelineManager::ExecutePipeline(PipelineRequester::Ptr requester, PipelineInstance const& pipeline)
	{
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

	PipelineInstance::Ptr PipelineManager::CreatPipelineInstance(Pipeline const& pipeline, std::pmr::memory_resource* resource)
	{
		return Potato::IR::MemoryResourceRecord::AllocateAndConstruct<PipelineInstance>(resource);
	}

	bool PipelineManager::UnregisterPass(Pass const& node)
	{
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

	std::optional<PipelineManager::PassRequest> PipelineManager::PopPassRequest(Pass const& node)
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
				auto Temp = std::move(*end_ite);
				requests.pop_back();
				return Temp;
			}
		}
		return {};
	}

	bool PipelineManager::PushPassRequest(PassRequest pass)
	{
		std::lock_guard lg(request_mutex);
		requests.push_back(std::move(pass));
		return true;
	}
}