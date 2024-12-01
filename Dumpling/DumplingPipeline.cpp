module;

#include <cassert>
module DumplingPipeline;


namespace Dumpling
{
	PassIndex PassTable::LocatePass(std::u8string_view name) const
	{
		for(std::size_t i = 0; i < passes.size(); ++i)
		{
			auto& ref = passes[i];
			auto str = ref.pass_name.Slice(std::u8string_view(pass_name));
			if(str == name)
			{
				return {i};
			}
		}
		return {};
	}

	PassIndex PassTable::RegisterPass(std::u8string_view name, PassProperty pass_property)
	{
		auto old_str_index = pass_name.size();
		pass_name.append(name);
		auto index = passes.size();
		passes.emplace_back(
			Potato::Misc::IndexSpan<>{old_str_index, pass_name.size()},
			pass_property
		);
		return {index};
	}

	struct DefaultPipelineInstanceT : public PipelineInstance, public Potato::IR::MemoryResourceRecordIntrusiveInterface
	{

		DefaultPipelineInstanceT( 
			Potato::IR::MemoryResourceRecord record,  
			std::span<PassReference> require_index,
			std::span<std::size_t> direct_to,
			std::span<ValueMapping> value_mapping,
			Potato::IR::StructLayoutObject::Ptr struct_object
		) : MemoryResourceRecordIntrusiveInterface(record), PipelineInstance(require_index, direct_to, value_mapping, std::move(struct_object))
		{
			
		}

		~DefaultPipelineInstanceT()
		{
			for(auto& ite : require_index)
			{
				ite.~PassReference();
			}
		}

		void AddPipelineInstanceRef() const override { MemoryResourceRecordIntrusiveInterface::AddRef(); }
		void SubPipelineInstanceRef() const override { MemoryResourceRecordIntrusiveInterface::SubRef(); }
	};

	PipelineInstance::Ptr PassTable::CreatePipelineInstance(Pipeline const& pipeline, std::pmr::memory_resource* resource) const
	{
		auto require = pipeline.GetRequire();

		std::size_t str_count = 0;
		for(auto& ite : require.require_pass)
		{
			str_count += ite.pass_name.size();
		}
		auto layout_cpp = Potato::MemLayout::MemLayoutCPP::Get<DefaultPipelineInstanceT>();
		auto pass_require_offset = layout_cpp.Insert(Potato::IR::Layout::GetArray<PipelineInstance::PassReference>(str_count));
		auto direct_to_offset = layout_cpp.Insert(Potato::IR::Layout::GetArray<std::size_t>(require.direct_to.size()));
		//auto value_mapping_offset = Potato::IR::InsertLayoutCPP(layout, Potato::IR::Layout::GetArray<PipelineInstance::ValueMapping>(str_count));
		auto str_offset = layout_cpp.Insert(Potato::IR::Layout::GetArray<char8_t>(str_count));
		auto layout = layout_cpp.Get();
		auto re = Potato::IR::MemoryResourceRecord::Allocate(resource, layout);

		if(re)
		{
			auto ptr = reinterpret_cast<PipelineInstance::PassReference*>(re.GetByte() + pass_require_offset);
			auto str_ptr = reinterpret_cast<char8_t*>(re.GetByte() + str_offset);
			auto ptr_ite = ptr;

			for(auto& ite : require.require_pass)
			{
				auto pass_index = LocatePass(ite.pass_name);
				if(pass_index)
				{
					new (ptr_ite) PipelineInstance::PassReference{
						pass_index,
						ite.direct_to,
							//ite.mapping_value
							{0, 0},
						0
					};
					std::memcpy(str_ptr, ite.pass_name.data(), sizeof(char8_t) * ite.pass_name.size());
					ptr_ite += 1;
					str_ptr += ite.pass_name.size();
				}else
				{
					auto ptr_ite2 = ptr;
					while(ptr_ite2 != ptr)
					{
						ptr_ite2->~PassReference();
						ptr_ite2+= 1;
					}
					re.Deallocate();
					return {};
				}
			}

			auto direct_ptr = reinterpret_cast<std::size_t*>(re.GetByte() + direct_to_offset);
			std::memcpy(direct_ptr, require.direct_to.data(), sizeof(std::size_t) * require.direct_to.size());
			Potato::IR::StructLayoutObject::Ptr object;
			auto str_layout = pipeline.GetStructLayout();
			if(str_layout)
			{
				object = Potato::IR::StructLayoutObject::DefaultConstruct(std::move(str_layout));
			}
			auto require_span = std::span(ptr, require.require_pass.size());
			auto direct_span = std::span(direct_ptr, require.direct_to.size());
			
			for(auto ite : direct_span)
			{
				require_span[ite].indegree += 1;
			}
			return new (re.Get()) DefaultPipelineInstanceT {
				re,
				require_span,
				direct_span,
				{},
				std::move(object)
			};
		}
		return {};
	}

	bool PipelineRecorder::CommitPipeline(PassTable const& table, PipelineInstance const& pipeline, PipelineRequester::Ptr requester, std::pmr::memory_resource* resource)
	{
		auto require = pipeline.GetRequiresReference();
		auto old_request_count = writing_frame.request_count;
		for(auto& ite : require.require_index)
		{
			assert(ite.index.index < table.GetPassSize());
			auto new_direct_to = ite.direct_to;
			new_direct_to.WholeOffset(writing_frame.direct_to_count);
			requests.emplace_back(
				ite.index,
				requester,
				Potato::IR::StructLayoutObject::Ptr{},
				new_direct_to,
				State::Ready,
				ite.indegree
			);
			writing_frame.request_count += 1;
		}
		auto old_direct_to_count = direct_to.size();
		direct_to.append_range(require.direct_to);
		auto span = std::span(direct_to).subspan(old_direct_to_count);
		for(auto& ite : span)
			ite += old_request_count;
		writing_frame.direct_to_count += require.direct_to.size();
		return true;
	}


	std::tuple<std::size_t, PipelineRecorder::RequestState> PipelineRecorder::PopRequest(std::span<PassRequest> output)
	{
		std::size_t pop_request = 0;
		std::size_t pass_offset = 0;
		for(auto& ite : output)
		{
			bool Finded = false;
			if(current_state.finish_count < current_state.total_count)
			{
				for(; pass_offset < current_state.total_count; ++pass_offset)
				{
					auto& ref = requests[pass_offset];
					if(ref.state == State::Ready && ref.indegree == 0)
					{
						ite.pass_index = ref.pass_index;
						ite.object = std::move(ref.object);
						ite.requester = std::move(ref.requester);
						ite.reference_index = pass_offset;
						Finded = true;
						current_state.running_count += 1;
						pop_request += 1;
						ref.state = State::Running;
						break;
					}
				}
			}
			if(!Finded)
			{
				return {pop_request, current_state};
			}
		}
		return {pop_request, current_state};
	}

	PipelineRecorder::RequestState PipelineRecorder::FinishRequest(PassRequest& request)
	{
		assert(request.reference_index <= requests.size());
		auto& ref = requests[request.reference_index];
		assert(ref.state == State::Running);
		ref.state = State::Done;
		auto span = ref.direct_to.Slice(std::span(direct_to));
		for(auto ite : span)
		{
			assert(requests[ite].indegree >= 1);
			requests[ite].indegree -= 1;
		}
		current_state.running_count -= 1;
		current_state.finish_count += 1;
		return current_state;
	}

	bool PipelineRecorder::PushFrame()
	{
		if(writing_frame.request_count > 0)
		{
			frame_count.emplace_back(writing_frame);
			writing_frame.request_count = 0;
			writing_frame.direct_to_count = 0;
			return true;
		}else
		{
			return false;
		}
	}

	bool PipelineRecorder::PopFrame()
	{
		requests.erase(
			requests.begin(),
			requests.begin() + reading_frame.request_count
		);
		direct_to.erase(
			direct_to.begin(),
			direct_to.begin() + reading_frame.direct_to_count
		);
		
		if(!frame_count.empty())
		{
			reading_frame = frame_count.front();
			current_state.total_count = reading_frame.request_count;
			current_state.finish_count = 0;
			current_state.running_count = 0;
			frame_count.pop_front();
			return true;
		}else
		{
			reading_frame.direct_to_count = 0;
			reading_frame.request_count = 0;
			current_state.total_count = 0;
			current_state.finish_count = 0;
			current_state.running_count = 0;
			return false;
		}
	}


	/*
	PipelineManager::PipelineManager(MemorySetting setting)
		: passes(setting.self_resource), requests(setting.self_resource), pass_resource(setting.pass_resource)
	{
	}

	PassIndex PipelineManager::RegisterPass(std::u8string_view name, PassProperty pass_property)
	{
		std::lock_guard lg(pass_mutex);
		return RegisterPass_AssumedLocked(name, pass_property);
	}

	PassIndex PipelineManager::RegisterPass_AssumedLocked(std::u8string_view name, PassProperty pass_property)
	{
		auto old_str_index = pass_name.size();
		pass_name.append(name);
		auto index = passes.size();
		passes.emplace_back(
			Potato::Misc::IndexSpan<>{old_str_index, pass_name.size()},
			pass_property
		);
		return {index};
	}

	PipelineInstance::Ptr PipelineManager::CreatePipelineInstance(Pipeline const& pipeline, std::pmr::memory_resource* resource)
	{
		auto require = pipeline.GetRequire();

		std::size_t str_count = 0;
		for(auto& ite : require.require_pass)
		{
			str_count += ite.pass_name.size();
		}
		auto layout = Potato::IR::Layout::Get<PipelineInstance>();
		auto pass_require_offset = Potato::IR::InsertLayoutCPP(layout, Potato::IR::Layout::GetArray<PipelineInstance::PassReference>(str_count));
		auto direct_to_offset = Potato::IR::InsertLayoutCPP(layout, Potato::IR::Layout::GetArray<std::size_t>(require.direct_to.size()));
		//auto value_mapping_offset = Potato::IR::InsertLayoutCPP(layout, Potato::IR::Layout::GetArray<PipelineInstance::ValueMapping>(str_count));
		auto str_offset = Potato::IR::InsertLayoutCPP(layout, Potato::IR::Layout::GetArray<char8_t>(str_count));
		Potato::IR::FixLayoutCPP(layout);

		auto re = Potato::IR::MemoryResourceRecord::Allocate(resource, layout);

		if(re)
		{
			std::shared_lock sl(pass_mutex);
			auto ptr = reinterpret_cast<PipelineInstance::PassReference*>(re.GetByte() + pass_require_offset);
			auto str_ptr = reinterpret_cast<char8_t*>(re.GetByte() + str_offset);
			auto ptr_ite = ptr;

			for(auto& ite : require.require_pass)
			{
				bool Find = false;
				for(std::size_t i = 0; i < passes.size(); ++i)
				{
					auto& ref = passes[i];
					auto str = ref.pass_name.Slice(std::u8string_view(pass_name));
					if(str == ite.pass_name)
					{
						new (ptr_ite) PipelineInstance::PassReference{
							PassIndex{i},
							ite.direct_to,
							//ite.mapping_value
							{0, 0}
						};
						Find = true;
						std::memcpy(str_ptr, ite.pass_name.data(), sizeof(char8_t) * ite.pass_name.size());
						ptr_ite += 1;
						str_ptr += ite.pass_name.size();
						break;
					}
				}
				if(!Find)
				{
					auto ptr_ite2 = ptr;
					while(ptr_ite2 != ptr)
					{
						ptr_ite2->~PassReference();
						ptr_ite2+= 1;
					}
					re.Deallocate();
					return {};
				}
			}

			auto direct_ptr = reinterpret_cast<std::size_t*>(re.GetByte() + direct_to_offset);
			std::memcpy(direct_ptr, require.direct_to.data(), sizeof(std::size_t) * require.direct_to.size());
			Potato::IR::StructLayoutObject::Ptr object;
			auto str_layout = pipeline.GetStructLayout();
			if(str_layout)
			{
				object = Potato::IR::StructLayoutObject::DefaultConstruct(std::move(str_layout));
			}
			return new (re.Get()) PipelineInstance {
				re,
				{ptr, require.require_pass.size()},
				{direct_ptr, require.direct_to.size()},
				{},
				std::move(object)
			};
		}
		return {};
	}
	*/

	/*
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
	*/
}