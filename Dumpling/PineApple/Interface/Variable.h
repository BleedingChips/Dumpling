#pragma once
#include <string_view>
#include <map>
#include <vector>
#include <optional>
#include <assert.h>
namespace PineApple::Variable
{
	namespace Error
	{
		struct UndefineType
		{
			std::u32string type_name;
		};

		struct MemberWithSameName
		{
			std::u32string name;
		};
	}

	enum class PropertyFlag
	{
		Value = 0b1,
		Array = 0b10,
		Pointer = 0b100,
	};

	constexpr PropertyFlag operator&(PropertyFlag i1, PropertyFlag i2)
	{
		return static_cast<PropertyFlag>(static_cast<size_t>(i1) & static_cast<size_t>(i2));
	}

	constexpr PropertyFlag operator|(PropertyFlag i1, PropertyFlag i2)
	{
		return static_cast<PropertyFlag>(static_cast<size_t>(i1) | static_cast<size_t>(i2));
	}

	struct Property
	{
		using Flag = PropertyFlag;
		Flag flag = Flag::Value;
		size_t array_count = 1;
		constexpr bool Is(Flag flag) const { return (this->flag & flag) == flag; }
		constexpr bool IsNormalValue() const { return Is(Flag::Value); }
		constexpr bool IsArray() const { return Is(Flag::Array); }
		constexpr bool IsPointer() const { return Is(Flag::Pointer); }
		size_t CalculateSize(size_t element_size) const {
			if (IsNormalValue())
				return element_size;
			else if (IsArray())
				return element_size * array_count;
			else {
				assert(false);
			}
			return 0;
		}
	};

	struct TypeInfo
	{
		std::u32string type_name;
		size_t align_size;
		size_t size;
	};

	template<typename Type>
	std::vector<std::byte> ToData(Type const& P)
	{
		using PureType = std::remove_cvref_t<Type>;
		std::vector<std::byte> Result;
		size_t size = sizeof(PureType);
		std::vector<std::byte> datas(size);
		std::memcpy(datas.data(), reinterpret_cast<std::byte const*>(&P), size);
		return std::move(datas);
	}

	template<typename Type>
	std::vector<std::byte> ToArrayData(Type const* data, size_t array_count)
	{
		using PureType = std::remove_cvref_t<Type>;
		std::vector<std::byte> Result;
		size_t size = sizeof(PureType) * array_count;
		std::vector<std::byte> datas(size);
		if(data != nullptr)
			std::memcpy(datas.data(), reinterpret_cast<std::byte const*>(data), size);
		return std::move(datas);
	}

	template<typename Type>
	std::vector<std::byte> ToZeroData() {
		return ToArrayData<Type>(nullptr, 0);
	}

	struct NoNamed
	{
		TypeInfo info;
		Property property;
		std::vector<std::byte> value;
	};

	struct Named : NoNamed
	{
		Named& operator=(NoNamed const& ref) { static_cast<NoNamed&>(*this).operator=(ref); return *this; }
		Named& operator=(NoNamed&& ref) { static_cast<NoNamed&>(*this).operator=(std::move(ref)); return *this; }
		Named(NoNamed nonamed, std::u32string name) : NoNamed(std::move(nonamed)), name(std::move(name)) {}
		Named(Named&&) = default;
		Named(Named const&) = default;
		std::u32string name;
	};
	
	struct TypeInfoStorage
	{

		struct Member
		{
			TypeInfo info;
			std::u32string name;
			Property property;
			size_t offset;
		};

		TypeInfo info;
		std::vector<Member> members;
		std::vector<std::byte> default_value;
		NoNamed Construct(std::vector<std::byte> datas = {}, Property pro = {}) const;
		Named Construct(std::u32string name, std::vector<std::byte> datas = {}, Property pro = {}) const {
			return Named{ Construct(std::move(datas), pro), std::move(name) };
		}
	};

	template<typename StorageType>
	struct Pattern
	{
		Pattern(std::u32string type_name, StorageType const& data) : 
			info_storage({ TypeInfo{ std::move(type_name), alignof(StorageType), sizeof(StorageType) }, {}, ToData(data) }) {}
		Pattern(std::u32string type_name) :
			info_storage({ TypeInfo{ std::move(type_name), alignof(StorageType), sizeof(StorageType) }, {}, ToArrayData<StorageType>(nullptr, 0) }) {}

		TypeInfo Info() const { return info_storage.info; }
		NoNamed Construct(StorageType const& datas, Property pro = {}) const {
			return info_storage.Construct(ToData(datas), pro);
		}
		NoNamed Construct(Property pro = {}) const {
			return info_storage.Construct({}, pro);
		}
		Named Construct(std::u32string name, StorageType const& datas, Property pro = {}) const {
			return info_storage.Construct(std::move(name), ToData(datas), pro);
		}
		Named Construct(std::u32string name, Property pro = {}) const {
			return info_storage.Construct(std::move(name), {}, pro);
		}
	private:
		TypeInfoStorage info_storage;
	};

	struct TypeInfoManager
	{
		TypeInfoStorage const* Find(std::u32string_view Name) const;
		bool Insert(TypeInfoStorage input_type);
	private:
		std::map<std::u32string_view, TypeInfoStorage> all_types;
	};

	enum class InfoSearchingType
	{
		Forwark,
		Backwark,
	};

	struct TypeInfoStorageLinker
	{
		InfoSearchingType search_type = InfoSearchingType::Backwark;
		size_t min_align = 0;
		size_t align_platform = sizeof(nullptr_t);
		size_t adjest_member_order = false;
		size_t refuse_same_name = true;
		TypeInfoStorage Link(std::u32string_view type_name, Named const* element, size_t size);
	};

	struct VariableManager
	{
		Named* Find(std::u32string_view Name);
		bool Insert(Named var);
	public:
		std::map<std::u32string_view, Named> all_value;
	};

}