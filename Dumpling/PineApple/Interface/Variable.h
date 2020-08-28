#pragma once
#include <string_view>
#include <map>
#include <vector>
#include <optional>
#include <assert.h>
namespace PineApple::VariableManager
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

	enum class VariablePropertyFlag
	{
		Value = 0b1,
		Array = 0b10,
		Pointer = 0b100,
	};

	constexpr VariablePropertyFlag operator&(VariablePropertyFlag i1, VariablePropertyFlag i2)
	{
		return static_cast<VariablePropertyFlag>(static_cast<size_t>(i1) & static_cast<size_t>(i2));
	}

	constexpr VariablePropertyFlag operator|(VariablePropertyFlag i1, VariablePropertyFlag i2)
	{
		return static_cast<VariablePropertyFlag>(static_cast<size_t>(i1) | static_cast<size_t>(i2));
	}

	struct VariableProperty
	{
		using Flag = VariablePropertyFlag;
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

	template<class Type> struct TypeInfoPattern { std::u32string_view operator()() const; };

	struct Variable
	{
		TypeInfo info;
		VariableProperty property;
		std::u32string name;
		std::vector<std::byte> value;

		template<class Type>
		static Variable MakeArray(std::u32string Name, Type const* P, VariableProperty pro = {})
		{
			using PureType = std::remove_cvref_t<Type>;
			Variable var{
				{std::u32string(TypeInfoPattern<PureType>{}()), alignof(PureType), sizeof(PureType) },
				pro, Name, {}
			};
			size_t size = pro.CalculateSize(sizeof(PureType));
			var.value.resize(size);
			std::memcpy(var.value.data(), reinterpret_cast<std::byte const*>(P), size);
			return std::move(var);
		}

		template<class Type>
		static Variable Make(std::u32string Name, Type const& P, VariableProperty pro = {})
		{
			using PureType = std::remove_cvref_t<Type>;
			Variable var{
				{std::u32string(TypeInfoPattern<PureType>{}()), alignof(PureType), sizeof(PureType) },
				pro, Name, {}
			};
			size_t size = pro.CalculateSize(sizeof(PureType));
			var.value.resize(size);
			std::memcpy(var.value.data(), reinterpret_cast<std::byte const*>(&P), size);
			return std::move(var);
		}

		/*
		template<class Type>
		static Variable Make(std::u32string Name, Type const& P, VariableProperty pro = {}) { return Variable::Make(std::move(Name), &P, pro); }
		*/
	};
	
	struct TypeInfoStorage
	{
		TypeInfo info;
		struct Member
		{
			TypeInfo info;
			std::u32string name;
			VariableProperty property;
			size_t offset;
		};
		std::vector<Member> members;
		std::vector<std::byte> default_value;
		Variable Construct(std::u32string Name, std::vector<std::byte> value = {}, VariableProperty pro = {}) const;
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
		TypeInfoStorage Link(std::u32string_view type_name, Variable const* element, size_t size);
	};

	
}