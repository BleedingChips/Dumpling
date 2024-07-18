module;


module DumplingPipeline;


namespace Dumpling
{
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


	auto Pipeline::Create(std::pmr::memory_resource* resource)
		-> Ptr
	{
		return Potato::IR::MemoryResourceRecord::AllocateAndConstruct<Pipeline>(resource);
	}
}