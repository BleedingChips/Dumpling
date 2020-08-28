#include "../Interface/Variable.h"
#include <set>

namespace PineApple::VariableManager
{


	Variable TypeInfoStorage::Construct(std::u32string Name, std::vector<std::byte> value, VariableProperty pro) const
	{
		Variable Result{ info, pro, std::move(Name), {} };
		if (value.size() != Result.info.size)
			Result.value = default_value;
		return Result;
	}

	bool TypeInfoManager::Insert(TypeInfoStorage input)
	{
		std::u32string_view TypeName = input.info.type_name;
		auto Re = all_types.insert({ TypeName, std::move(input) });
		return Re.second;
	}


	TypeInfoStorage const* TypeInfoManager::Find(std::u32string_view Name) const
	{
		auto ite = all_types.find(Name);
		if (ite != all_types.end())
			return &ite->second;
		return nullptr;
	}

	TypeInfoStorage TypeInfoStorageLinker::Link(std::u32string_view type_name, Variable const* element, size_t size)
	{
		TypeInfoStorage Result;
		Result.info.type_name = type_name;
		Result.info.align_size = min_align;
		std::set<std::u32string_view> NameList;
		if (!adjest_member_order)
		{
			for (size_t i = 0; i < size; ++i)
			{
				auto& ref = element[i];
				if (refuse_same_name)
				{
					auto re = NameList.insert(ref.name);
					if (!re.second)
						throw Error::MemberWithSameName{ std::u32string(ref.name) };
				}

				size_t mer_align = ref.info.align_size;
				size_t mer_size = ref.value.size();
				Result.info.align_size = (Result.info.align_size < mer_align ? mer_align : Result.info.align_size);
				size_t require_size = (Result.default_value.size() % mer_align);
				size_t start_adress = Result.default_value.size();
				if (require_size != 0)
				{
					require_size = mer_align - require_size;
					start_adress = Result.default_value.size() + require_size;
					Result.default_value.resize(Result.default_value.size() + require_size + ref.value.size());
				}
				else {
					Result.default_value.resize(Result.default_value.size() + ref.value.size());
				}
				std::memcpy(Result.default_value.data() + start_adress, ref.value.data(), ref.value.size());
				Result.members.push_back({ ref.info, ref.name, ref.property, start_adress });
			}
			size_t align_size = Result.default_value.size() % Result.info.align_size;
			if (align_size != 0)
				Result.default_value.resize(Result.default_value.size() + Result.info.align_size - align_size);
			if (Result.default_value.size() < Result.info.align_size)
				Result.default_value.resize(Result.info.align_size);
			Result.info.size = Result.default_value.size();
		}
		else {
			throw 1;
		}
		

		return Result;
	}
}