#pragma once
#include <string_view>
#include <map>
#include <vector>
#include <optional>
#include <assert.h>
#include <variant>
#include <typeindex>
#include <any>
#include "Nfa.h"
#include <optional>
namespace PineApple::Symbol
{

	using Section = Nfa::Section;
	
	struct Table
	{
		struct Mask
		{
			Mask(const Mask&) = default;
			Mask(size_t inputindex) : index(inputindex + 1){}
			Mask() =default;
			Mask& operator=(Mask const&) = default;
			operator bool() const noexcept { return index != 0; }
		private:
			friend struct Table;
			size_t index = 0;
		};
		
		Mask FindActiveLast(std::u32string_view name) const noexcept;
		std::vector<Mask> FindActiveAll(std::u32string_view name) const;

		enum class ResultType
		{
			Inavailable,
			ExistButNotRequireType,
			Available
		};
		
		template<typename RequireType>
		struct Result
		{
			ResultType type = ResultType::Inavailable;
			std::u32string_view name;
			Section section;
			RequireType* require_data = nullptr;
			operator bool () const noexcept{return type == ResultType::Available;}
			RequireType* operator->(){return require_data;}
			bool Exist() const noexcept{return type != ResultType::Inavailable;}
		};

		Result<const std::any> FindRaw(Mask mask) const;

		Result<std::any> FindRaw(Mask mask)
		{
			auto result = static_cast<Table const*>(this)->FindRaw(mask);
			return {result.type, result.name, result.section, const_cast<std::any*>(result.require_data) };
		}

		template<typename RequireType>
		auto Find(Mask mask) ->Result<std::remove_reference_t<RequireType>>
		{
			auto result = static_cast<Table const*>(this)->Find<RequireType>(mask);
			auto ptr = const_cast<std::remove_reference_t<RequireType>*>(result.require_data);
			return {result.type, result.name, result.section, ptr};
		}

		template<typename RequireType>
		auto Find(Mask mask) const ->Result<std::add_const_t<std::remove_reference_t<RequireType>>>
		{
			auto result = FindRaw(mask);
			auto ptr = std::any_cast<std::add_const_t<std::remove_reference_t<RequireType>>>(result.require_data);
			if(ptr != nullptr)
				return { ResultType::Available, result.name, result.section, ptr };
			else if(result.type == ResultType::Available)
				return { ResultType::ExistButNotRequireType, result.name, result.section, ptr };
			else
				return { ResultType::Inavailable, {}, {}, nullptr };
		}

		size_t PopElementAsUnactive(size_t count);

		std::vector<Mask> PopAndReturnElementAsUnactive(size_t count);

		Mask Insert(std::u32string_view name, std::any property, Section section = {});

		Table(Table&&) = default;
		Table(Table const&) = default;
		Table() = default;

	private:

		struct Storage
		{
			std::u32string_view name;
			Mask index;
			Section section;
			std::any property;
		};
		
		struct Mapping
		{
			bool is_active;
			size_t index;
		};
		
		std::vector<Storage> unactive_scope;
		std::vector<Storage> active_scope;
		std::vector<Mapping> mapping;
	};

	struct StorageInfo
	{
		size_t align = 0;
		size_t size = 0;
	};

	struct StorageInfoLinker
	{
		
		struct HandleResult
		{
			size_t align = 0;
			size_t size_reserved = 0;
		};

		size_t operator()(StorageInfo const& info);
		operator StorageInfo() const { return Finalize(info); }

		StorageInfoLinker(StorageInfo info = {}) : info(std::move(info)) {}
		StorageInfoLinker(StorageInfoLinker const&) = default;
		StorageInfoLinker(StorageInfoLinker&&) = default;
		StorageInfoLinker& operator=(StorageInfoLinker const&) = default;
		StorageInfoLinker& operator=(StorageInfoLinker&&) = default;

		static size_t MaxAlign(StorageInfo out, StorageInfo in) noexcept;
		static size_t ReservedSize(StorageInfo out, StorageInfo in) noexcept;

	private:
		
		virtual HandleResult Handle(StorageInfo cur, StorageInfo input) const;
		virtual StorageInfo Finalize(StorageInfo cur) const;
		StorageInfo info;
	};










	/*
	struct Mask
	{
		size_t index = 0;
		operator bool() const noexcept { return index != 0; }
	};

	struct Value
	{
		Mask type;
		std::u32string name;
	};

	struct Data
	{
		Mask type;
		std::u32string name;
	};

	struct ValueData
	{
		Mask type;
	};
	*/













	/*
	struct Mask
	{
		size_t index = 0;
		operator bool() const noexcept { return index != 0; }
	};

	struct Record
	{
		size_t table_count = 0;
		size_t value_count = 0;
		Record& operator+= (Record const& i) { table_count += i.table_count; value_count += i.value_count; return *this; }
		Record operator+ (Record const& i) { Record result(*this); return result += i; }
	};

	struct Value
	{
		Mask type;
		std::u32string_view name;
		std::any pro;
	};

	struct Type
	{
		std::u32string_view name;
		std::vector<Value> members;
		std::any pro;
	};

	struct ValueMask
	{
		size_t offset;
		size_t length;
	};

	struct Table
	{

		std::optional<Value> MakeValue(std::u32string_view const& type, std::u32string_view value_name, std::any pro = {}) const{
			auto mask = FindType(type);
			if(mask)
				return Value{mask, std::move(value_name), std::move(pro)};
			else
				return {};
		}

		Mask InsertValue(Value value);
		Mask InsertType(std::u32string_view type_name, std::vector<Value> member, std::any pro = {});
		Mask FindType(std::u32string_view const& type_name) const;
		Mask FindValue(std::u32string_view const& value_name) const;
		void PopActionScope(Record record);

	private:

		struct StorageMask
		{
			bool in_background;
			size_t index;
		};

		std::vector<std::tuple<Value, Mask>> background_value_stack;
		std::vector<std::tuple<Value, Mask>> value_stack;
		std::vector<StorageMask> value_mapping;

		std::vector<std::tuple<Type, Mask>> background_type_stack;
		std::vector<std::tuple<Type, Mask>> type_stack;
		std::vector<StorageMask> type_mapping;

		std::vector<std::byte> all_data;
	};

	struct ValueBuffer
	{
		ValueMask Insert(std::byte const* data, size_t length);
		ValueMask Allocate(size_t length);
	private:
		std::vector<std::byte> all_data;
	};
	*/

	/*
	struct Value
	{
		std::u32string_view type_name;
		std::u32string_view name;
		std::any property;
	};

	

	struct CommmandList
	{
		struct Element
		{
			std::any command;
			Nfa::Location loc;
		};

		template<typename Type>
		size_t Push(Type&& type, Nfa::Location loc)
		{
			size_t cur_index = commands.size();
			commands.push_back({std::forward<Type>(type), loc});
			return cur_index;
		}
	private:
		std::vector<Element> commands;
	};

	namespace Command
	{
		struct Data
		{
			template<typename Type>
			static Data Make(std::u32string_view name, Type&& type)
			{
				return Make(name, reinterpret_cast<std::byte const*>(&type), sizeof(type));
			}
			static Data Make(std::u32string_view name, std::byte const* datas, size_t length);
			std::u32string_view type;
			std::vector<std::byte> data;
		};

		struct DefaultValue
		{
			std::u32string_view type;
		};

		struct MakeNoNameValue
		{
			size_t count;
		};

		struct LinkNoNameValue
		{
			size_t count;
		};

		struct DefineValue
		{
			std::u32string_view type;
			std::u32string_view name;
			std::any property;
		};

		struct EqualValue
		{
			std::u32string_view name;
			size_t data_used;
		};

		struct DefineType
		{
			std::u32string_view type;
			size_t data_used;
		};
	}
	*/
	

	

	

	

}