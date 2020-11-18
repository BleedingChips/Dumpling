#include "../Public/Symbol.h"
#include <set>

namespace PineApple::Symbol
{

	auto Table::Insert(std::u32string_view name, std::any property) -> Table::Mask
	{
		Mask mask{mapping.size() + 1};
		active_scope.push_back({name, mask, std::move(property)});
		mapping.push_back(Mapping{true, active_scope.size() - 1});
		return mask;
	}

	auto Table::FindImp(Mask mask) const -> Storage const&
	{
		assert(mask);
		assert(*mask < mapping.size());
		auto& mapp = mapping[*mask];
		if (mapp.is_active)
			return active_scope[mapp.index];
		else
			return unactive_scope[mapp.index];
	}

	auto Table::FindActiveLast(std::u32string_view name) const noexcept -> Table::Mask
	{
		for (auto ite = active_scope.rbegin(); ite != active_scope.rend(); ++ite)
		{
			if(ite->name == name)
				return ite->index;
		}
		return {};
	}
	
	size_t Table::PopElementAsUnactive(size_t count)
	{
		assert(active_scope.size() >= count);
		auto unactive_scope_size = unactive_scope.size();
		auto offset = active_scope.size() - count;
		auto offset_ite = active_scope.begin() + offset;
		unactive_scope.insert(unactive_scope.end(), 
			std::move_iterator(offset_ite),
			std::move_iterator(active_scope.end())
		);
		active_scope.erase(offset_ite, active_scope.end());
		for (size_t i = 0; i < count; ++i)
		{
			auto& mapp = *(mapping.rbegin() + i);
			mapp.is_active = false;
			mapp.index = unactive_scope_size + count - i - 1;
		}
		return active_scope.size();
	}

	size_t StorageInfoLinker::operator()(StorageInfo const& info_i)
	{
		auto re = Handle(info, info_i);
		info.align = re.align;
		info.size += re.size_reserved;
		size_t offset = info.size;
		info.size += info_i.size;
		return offset;
	}

	size_t StorageInfoLinker::MaxAlign(StorageInfo out, StorageInfo in) noexcept
	{
		return out.align < in.align ? in.align : out.align;
	}

	size_t StorageInfoLinker::ReservedSize(StorageInfo out, StorageInfo in) noexcept
	{
		auto mod = out.size % in.align;
		if (mod == 0)
			return 0;
		else
			return out.align - mod;
	}

	StorageInfoLinker::HandleResult StorageInfoLinker::Handle(StorageInfo cur, StorageInfo input) const
	{
		return { MaxAlign(cur, input), ReservedSize(cur, input) };
	}

	StorageInfo StorageInfoLinker::Finalize(StorageInfo cur) const
	{
		auto result = cur;
		result.size += ReservedSize(result, result);
		return result;
	}


	/*
	std::tuple<StorageInfo, std::vector<size_t>> CalculateSpace_Default_C(StorageInfo const* info, size_t length, StorageInfo min = {})
	{
		std::vector<size_t> offset;
		for (size_t index = 0; index < length; ++index)
		{
			auto& ref = info[index];
			min.aligned = min.aligned < ref.aligned ? ref.aligned : min.aligned;
			auto mod = min.size % ref.aligned;
			if (mod == 0)
				offset.push_back(min.size);
			else {
				min.size += ref.aligned - mod;
				offset.push_back(min.size);
			}
			min.size += ref.size;
		}
		return {min, std::move(offset)};
	}

	std::tuple<StorageInfo, std::vector<size_t>> CalculateSpace_Default_HLSL(StorageInfo const* info, size_t length, StorageInfo min = {})
	{
		std::vector<size_t> offset;
		for (size_t index = 0; index < length; ++index)
		{
			auto& ref = info[index];
			min.aligned = min.aligned < ref.aligned ? ref.aligned : min.aligned;
			auto mod = min.size % ref.aligned;
			if (mod == 0)
				offset.push_back(min.size);
			else {
				min.size += ref.aligned - mod;
				offset.push_back(min.size);
			}
			min.size += ref.size;
		}
		return { min, std::move(offset) };
	}
	*/

	/*
	Command::Data Command::Data::Make(std::u32string_view name, std::byte const* datas, size_t length)
	{
		std::vector<std::byte> data(length);
		std::memcpy(data.data(), datas, length);
		return Data{name, std::move(data)};
	}
	*/
	
	/*
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
	*/
}