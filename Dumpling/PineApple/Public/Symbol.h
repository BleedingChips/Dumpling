#pragma once
#include <string_view>
#include <map>
#include <vector>
#include <optional>
#include <assert.h>
#include <variant>
#include <typeindex>
#include <any>
namespace PineApple::Symbol
{
	namespace Error
	{
		struct RedefineType
		{
			std::u32string name;
		};

		struct UndefineType
		{
			std::u32string name;
		};
	}

	struct Property
	{
		template<typename ProT>
		Property(ProT prot) : pro(std::move(prot)){}
		Property() = default;
		Property(Property const&) = default;
		Property(Property&&) = default;
		Property& operator=(Property&&) = default;
		Property& operator=(Property const&) = default;
		operator bool() const noexcept{return pro.has_value();}
		template<typename ProT>
		ProT* TryCast(){ return std::any_cast<ProT>(&pro); }
		template<typename ProT>
		ProT& Cast() { return std::any_cast<ProT>(pro); }
	private:
		std::any pro;
	};

	struct LRTable
	{

		struct Record
		{
			size_t table_count = 0;
			size_t value_count = 0;
			Record& operator+= (Record const& i){ table_count += i.table_count; value_count += i.value_count; return *this;}
			Record operator+ (Record const& i) { Record result(*this); return result += i; }
		};

		struct Mask
		{
			size_t mask = 0;
			operator bool() const noexcept{return mask != 0;}
		};

		struct Value
		{
			Mask type;
			std::u32string_view name;
			Property pro;
		};

		struct Type
		{
			std::u32string_view name;
			std::vector<Value> members;
		};

		static Value MakeValue(Mask type, std::u32string_view name, Property pro = {}){return Value{ type, name, std::move(pro)};}

		Mask InsertValue(Mask type_mask, std::u32string_view value_name, Property pro = {});
		Mask InsertType(std::u32string_view type_name, std::vector<Value> member);
		Mask FindType(std::u32string_view type_name) const;
		Mask FindValue(std::u32string_view value_name) const;

		void PopActionScope(Record record);

		struct StorageMask
		{
			bool in_background;
			size_t index;
		};

	private:

		std::vector<std::tuple<Value, Mask>> background_value_stack;
		std::vector<std::tuple<Value, Mask>> value_stack;
		std::vector<StorageMask> value_mapping;

		std::vector<std::tuple<Type, Mask>> background_type_stack;
		std::vector<std::tuple<Type, Mask>> type_stack;
		std::vector<StorageMask> type_mapping;
	};
}