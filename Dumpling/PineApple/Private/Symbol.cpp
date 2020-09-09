#include "../Interface/Symbol.h"
#include <set>

namespace PineApple::Symbol
{

	Table::Table(std::initializer_list<InsideType> inside_type)
	{
		types.Push();
		for (auto& ite : inside_type)
		{
			auto find = inside_type_mapping.find(ite.type);
			if (find != inside_type_mapping.end())
			{
				auto [scope, result] = types.Insert(ite.name, {});
				assert(result);
				auto scope_name = ScopeName{ types.ActionScope(), ite.name };
				auto re = inside_type_mapping.insert({ite.type, scope_name });
				assert(re.second);
				auto datas = InsertData(ite.data.data(), ite.data.size());
				commands.push_back(ConstValue{ scope_name, datas });
				commands.push_back(DefaultValue{ scope_name });
			}else
				throw Error::RedefineType{ std::u32string(ite.name) };
		}
	}

	ValueWrapper Table::InsertData(std::byte const* data, size_t length)
	{
		auto old_size = const_datas.size();
		const_datas.resize(old_size + length);
		std::memcpy(const_datas.data(), data, length);
		return { old_size, length };
	}

	/*
	void Table::AddInsideValue(std::u32string_view type, std::byte const* data, size_t length)
	{
		auto value = InsertData(data, length);
		size_t row_value = InsertRowValue({}, {value});
		auto ite = types.Insert(type, Type{{}, row_value});
		if (ite.second)
		{
			row_values[row_value - 1].type = ScopeName{ types.ActionScope(), type };
		}
		else {
			throw Error::RedefineType{ std::u32string(type) };
		}
	}

	NamedValue Table::CreateTemplateDefaultValue(std::u32string_view type, std::u32string_view name, Property pro)
	{
		auto [scope, type_p] = types.Find(type);
		if (type_p != nullptr)
		{
			return { name, { {scope, type}, pro, type_p->default_row_value} };
		}
		throw Error::UndefineType{std::u32string(type)};
	}

	NamedValue Table::CreateTemplateValue(std::u32string_view type, std::u32string_view name, Property pro, size_t row_data)
	{
		auto [scope, type_p] = types.Find(type);
		if (type_p != nullptr)
		{
			
		}
		throw Error::UndefineType{ std::u32string(type) };
	}


	size_t Table::InsertRowValue(ScopeName type, std::byte const* data, size_t length)
	{
		auto wrapper = InsertData(data, length);
		return InsertRowValue(type, {wrapper});
	}

	RowValue const* Table::ReadRowData(size_t index) const
	{
		assert(index > 0);
		return &row_values[index - 1];
	}

	void Table::Push()
	{
		values.Push();
		types.Push();
	}

	void Table::Pop()
	{
		values.Pop();
		types.Pop();
	}

	

	size_t Table::InsertRowValue(ScopeName type, std::vector<ValueReference> ref)
	{
		row_values.push_back(RowValue{type, std::move(ref)});
		return row_values.size();
	}
	*/
		
	/*
	Space Property::Calculate(Space element, Space pointer) const
	{
		if(is_pointer)
			element = pointer;
		if (is_array)
			element.size *= array_count;
		return element;
	}


	NoNamed TypeDefine::Construct(std::vector<std::byte> datas, Property pro, Space pointer) const
	{
		if (datas.empty() || datas.size() != pro.Calculate(info.space, pointer))
			datas = default_value;
		return NoNamed{ info, pro, std::move(datas) };
	}

	bool TypeDefineManager::Insert(TypeDefine input)
	{
		std::u32string_view TypeName = input.info.type_name;
		auto Re = all_types.insert({ TypeName, std::move(input) });
		return Re.second;
	}

	TypeDefine const* TypeDefineManager::Find(std::u32string_view Name) const
	{
		auto ite = all_types.find(Name);
		if (ite != all_types.end())
			return &ite->second;
		return nullptr;
	}

	void TypeDefineLinker::AdjustMember(std::vector<Named>& meb_list) { return; }

	bool TypeDefineLinker::ShouldIgnoreSameNameMember(Named& old_one, Named& new_one) {
		throw Error::MemberWithSameName{old_one.name};
	}

	void TypeDefineLinker::HandleAligned(Space& space, Named& element)
	{
		auto tspace = element.property.Calculate(element.type.space, pointer_space);
		space.align = space.align < tspace.align ? tspace.align: space.align;
		auto need = space.size % tspace.align;
		if (need != 0)
			space.size += tspace.align - need;
	}

	TypeDefine TypeDefineLinker::Link(std::u32string type_name, std::vector<Named> members)
	{
		AdjustMember(members);
		TypeInfo info{ default_space,  std::move(type_name) };
		std::map<std::u32string_view, Named*> name_set;
		std::vector<TypeDefine::Member> all_members;
		std::vector<std::byte> datas;
		for (auto& ite : members)
		{
			auto re = name_set.insert({ite.name, &ite});
			if (re.second || !ShouldIgnoreSameNameMember(*re.first->second, ite))
			{
				HandleAligned(info.space, ite);
				all_members.push_back(TypeDefine::Member{std::move(ite.type), ite.property, std::move(ite.name), info.space.size });
				datas.resize(info.space.size + ite.value.size());
				std::memcpy(datas.data(), ite.value.data(), ite.value.size());
			}
		}
		return TypeDefine{std::move(info), std::move(all_members), std::move(datas)};
	}

	Named* VariableManager::Find(std::u32string_view Name)
	{
		auto P = all_value.find(Name);
		if (P != all_value.end())
			return &(P->second);
		return nullptr;
	}

	bool VariableManager::Insert(Named var)
	{
		std::u32string_view name = var.name;
		auto re = all_value.insert({ name , std::move(var)});
		return re.second;
	}
	*/

}