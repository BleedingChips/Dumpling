#include "../Public/Symbol.h"
#include <set>

namespace PineApple::Symbol
{
	/*
	Command::Data Command::Data::Make(std::u32string_view name, std::byte const* datas, size_t length)
	{
		std::vector<std::byte> data(length);
		std::memcpy(data.data(), datas, length);
		return Data{name, std::move(data)};
	}
	*/
	
	template<typename Storage, typename StorageTuple, typename Mapping> 
	Mask InsertExe(Storage&& s, std::vector<StorageTuple>& stack, std::vector<Mapping>& mapping)
	{
		auto mask = mapping.size() + 1;
		stack.push_back({ std::move(s), {mask} });
		mapping.push_back({false, stack.size() - 1});
		return {mask};
	}

	auto Table::InsertValue(Value value) -> Mask
	{
		return InsertExe(std::move(value), value_stack, value_mapping);
	}

	auto Table::InsertType(std::u32string type_name, std::vector<Value> member, std::any pro = {}) -> Mask
	{
		return InsertExe(Type{ std::move(type_name), std::move(member), std::move(pro) }, type_stack, type_mapping);
	}

	template<typename StorageTuple>
	Mask FindExe(std::u32string_view name, std::vector<StorageTuple> const& stack)
	{
		for (auto ite = stack.rbegin(); ite != stack.rend(); ++ite)
		{
			auto& [ref, mask] = *ite;
			if(ref.name == name)
				return mask;
		}
		return {0};
	}

	auto Table::FindType(std::u32string const& type_name) const -> Mask
	{
		return FindExe(type_name, type_stack);
	}

	auto Table::FindValue(std::u32string const& value_name) const -> Mask
	{
		return FindExe(value_name, value_stack);
	}

	template<typename StorageTuple>
	void PopActionScopeExe(size_t s, std::vector<StorageTuple>& stack, std::vector<StorageTuple>& backup_stack, std::vector<Table::StorageMask>& mapping)
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

	void Table::PopActionScope(Record record)
	{
		PopActionScopeExe(record.value_count, value_stack, background_value_stack, value_mapping);
		PopActionScopeExe(record.table_count, type_stack, background_type_stack, type_mapping);
	}
}