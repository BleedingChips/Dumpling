#include "../Public/Symbol.h"
#include <set>

namespace PineApple::Symbol
{
	Command::Data Command::Data::Make(std::u32string_view name, std::byte const* datas, size_t length)
	{
		std::vector<std::byte> data(length);
		std::memcpy(data.data(), datas, length);
		return Data{name, std::move(data)};
	}
	/*
	template<typename Storage, typename StorageTuple, typename Mapping> 
	LRTable::Mask InsertExe(Storage&& s, std::vector<StorageTuple>& stack, std::vector<Mapping>& mapping)
	{
		auto mask = mapping.size() + 1;
		stack.push_back({ std::move(s), {mask} });
		mapping.push_back({false, stack.size() - 1});
		return {mask};
	}

	auto LRTable::InsertValue(Mask type_mask, std::u32string_view value_name, Property pro) -> Mask 
	{
		return InsertExe(Value{type_mask, value_name, std::move(pro)}, value_stack, value_mapping);
	}

	auto LRTable::InsertType(std::u32string_view type_name, std::vector<Value> member) -> Mask
	{
		return InsertExe(Type{ type_name, std::move(member) }, type_stack, type_mapping);
	}

	template<typename StorageTuple>
	LRTable::Mask FindExe(std::u32string_view name, std::vector<StorageTuple> const& stack)
	{
		for (auto ite = stack.rbegin(); ite != stack.rend(); ++ite)
		{
			auto& [ref, mask] = *ite;
			if(ref.name == name)
				return mask;
		}
		return {0};
	}

	auto LRTable::FindType(std::u32string_view type_name) const -> Mask
	{
		return FindExe(type_name, type_stack);
	}

	auto LRTable::FindValue(std::u32string_view value_name) const -> Mask
	{
		return FindExe(value_name, value_stack);
	}

	template<typename StorageTuple>
	void PopActionScopeExe(size_t s, std::vector<StorageTuple>& stack, std::vector<StorageTuple>& backup_stack, std::vector<LRTable::StorageMask>& mapping)
	{
		if (s != 0)
		{
			assert(s <= stack.size());
			auto cur = backup_stack.size();
			backup_stack.insert(backup_stack.end(), std::move_iterator(stack.begin() + stack.size() - s), std::move_iterator(stack.end()));
			stack.resize(stack.size() - s);
			auto mapping_offset = mapping.size() - s;
			for (size_t i = 0; i < cur; ++i)
				mapping[mapping_offset + i] ={true,  cur + i};
		}
	}

	void LRTable::PopActionScope(Record record)
	{
		PopActionScopeExe(record.value_count, value_stack, background_value_stack, value_mapping);
		PopActionScopeExe(record.table_count, type_stack, background_type_stack, type_mapping);
	}
	*/
}