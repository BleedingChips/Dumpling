#pragma once
#include <string_view>
#include <map>
#include <vector>
#include <optional>
#include <assert.h>
#include <variant>
#include <typeindex>
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

		struct DismatchDataLength
		{
			std::u32string type;
			std::u32string name;
			size_t type_length;
			size_t input_length;
		};
	}

	struct ScopeName
	{
		size_t action_scope = 0;
		std::u32string_view name;
		bool operator <(ScopeName const& input) const noexcept {
			return action_scope < input.action_scope || (action_scope == input.action_scope && name < input.name);
		}
	};

	struct Property
	{
		std::vector<uint64_t> propertys;
	};

	struct ValueWrapper
	{
		size_t offset;
		size_t length;
	};

	struct Value
	{
		ScopeName Type;
		Property pro;
	};

	struct Type
	{
		struct Member
		{
			ScopeName Type;
			Property pro;
			std::u32string_view name;
		};
		std::vector<Member> members;
	};

	template<typename Key, typename StorageType>
	struct ActionScopeTable
	{
		struct Node
		{
			size_t upper_action_scope;
			std::map<Key, StorageType> datas;
		};
		void Push() {
			nodes.push_back(Node{ cur_action_scope, {} });
			cur_action_scope = nodes.size();
		}
		void Pop(){
			assert(!nodes.empty());
			cur_action_scope = nodes.rbegin()->upper_action_scope;
		}
		void Clear(){
			nodes.clear();
			cur_action_scope = 0;
		}
		auto Insert(Key k, StorageType storage)
		{
			assert(cur_action_scope != 0);
			auto& ref = nodes[cur_action_scope - 1];
			return ref.datas.insert({std::move(k), std::move(storage)});
		}
		StorageType const* Find(size_t scope, Key const& type) const
		{
			if (scope != 0)
			{
				auto& ref = nodes[scope - 1];
				auto re = ref.datas.find(type);
				if(re != ref.datas.end())
					return &(re->second);
			}
			return nullptr;
		}
		std::tuple<size_t, StorageType const*> Find(Key const& type) const
		{
			size_t scope = cur_action_scope;
			while (scope != 0)
			{
				auto& ref = nodes[scope - 1];
				auto re = ref.datas.find(type);
				if (re != ref.datas.end())
					return { scope, &(re->second)};
				else
					scope = ref.upper_action_scope;
			}
			return nullptr;
		}
		size_t ActionScope() const {return cur_action_scope;}
	private:
		std::vector<Node> nodes;
		size_t cur_action_scope = 0;
	};

	struct ConstValue
	{
		ScopeName type;
		ValueWrapper value;
	};

	struct DefaultValue
	{
		ScopeName type;
	};

	struct MakeValue
	{
		ScopeName type;
		size_t parameter_count;
	};

	struct EqualValue
	{
		ScopeName value;
	};

	struct EqualTypeDefaultValue
	{
		ScopeName type;
	};

	using Command = std::variant<ConstValue, DefaultValue, MakeValue, EqualValue, EqualTypeDefaultValue>;

	struct InsideType
	{
		std::type_index type;
		std::u32string_view name;
		std::vector<std::byte> data;
	};

	template<typename ClassType>
	static InsideType MakeInsideType(std::u32string_view name, ClassType ref) {
		std::vector<std::byte> datas(sizeof(ClassType));
		std::memcpy(datas.data(), &ref, datas.size());
		return { typeid(ClassType), name, std::move(datas) };
	}

	struct Table
	{
		
		

		

		Table(std::initializer_list<InsideType> inside_type);

		template<typename Type>
		void PushData(Type const& value);
		void PushData(ScopeName type, std::byte const*, size_t length);
		template<typename Type>
		void PushDefultData();
		void PushDefaultData(ScopeName type);

		/*
		void RegisterInsideType


		std::map<std::type_index, std::u32string_view> inside_mapping;

		ValueWrapper PushData(std::byte const* data, size_t length);



		
		template<typename Type> void AddInsideValue(std::u32string_view type, Type const& data) {
			std::byte data_array[sizeof(Type)];
			std::memcpy(data_array, &data, sizeof(Type));
			AddInsideValue(type, data_array, sizeof(Type));
		}

		void AddInsideValue(std::u32string_view type, std::byte const*, size_t length);
		template<typename Type>
		size_t InsertRowValue(ScopeName type, Type const& data)
		{
			std::byte data_array[sizeof(Type)];
			std::memcpy(data_array, &data, sizeof(Type));
			return InsertRowValue(type, data_array, sizeof(Type));
		}
		size_t InsertRowValue(ScopeName type, std::byte const* data, size_t length);
		NamedValue CreateTemplateDefaultValue(std::u32string_view type, std::u32string_view name, Property pro);

		template<typename Type>
		size_t CreateTemplateValue(std::u32string_view type, std::u32string_view name, Property pro, Type const& data)
		{
			std::byte data_array[sizeof(Type)];
			std::memcpy(data_array, &data, sizeof(Type));
			return CreateTemplateValue(type, name, pro, data_array, sizeof(Type));
		}
		NamedValue CreateTemplateValue(std::u32string_view type, std::u32string_view name, Property pro, std::byte const* data, size_t length);

		ScopeName CreateType(ScopeName type, std::vector<NamedValue> members);

		void Push();
		void Pop();
		*/

		/*
		ValueWrapper AddConstData(std::byte const*, size_t);
		std::tuple<std::byte const*, size_t> ReadConstData(ValueWrapper wrapper);
		size_t AddRowValue(RowValue value);
		RowValue const* ReadRowValue(size_t index) const;
		ScopeName InsertType(Type type);
		Type const* ReadType(ScopeName name) const;
		ScopeName InsertValue(Value type);
		Type const* ReadType(ScopeName name) const;
		*/
		
	protected:
		ValueWrapper InsertData(std::byte const* data, size_t length);
		//size_t InsertRowValue(ScopeName type, std::vector<ValueReference> ref);
		//RowValue const* ReadRowData(size_t index) const;
	private:
		std::vector<std::byte> const_datas;
		std::vector<Command> commands;
		std::map<std::type_index, ScopeName> inside_type_mapping;
		//std::vector<RowValue> row_values;
		ActionScopeTable<std::u32string_view, Value> values;
		ActionScopeTable<std::u32string_view, Type> types;
	};

	/*
	struct ValueStorage
	{
		ValueWrapper Insert(std::byte const* data, size_t length);
	private:
		std::vector<std::byte> datas;
	};

	struct Value
	{
		ScopeName type_name;
		Property property;
		std::vector<std::variant<ScopeName, ValueWrapper>> value;
	};

	struct Type
	{
		struct Member
		{
			ScopeName type_name;
			std::u32string_view name;
			Property property;
			ScopeName default_Value;
		};
		std::vector<Member> members;
	};

	struct SymbolTable
	{
	private:
		std::vector<ValueWrapper> default_value;
	};



	struct ValueData
	{
		std::vector<std::byte> datas;
	};

	

	struct Property
	{
		std::vector<uint64_t> propertys;
	};

	struct Type
	{
		struct Member
		{
			ScopeName type_name;
			Property property;
			size_t offset;
		};
		std::vector<Member> member;
		ValueData default_datas;
	};

	struct Value
	{
		ScopeName type_name;
		Property property;
		ValueData default_datas;
	};

	struct Variable
	{
		ScopeName name;

	};

	struct ValueStorage
	{
		ValueWrapper insert(std::byte const* data, size_t index);
	private:
		std::vector<std::byte> all_byte;
	};

	

	

	struct Value
	{
		ScopeName type;
		Property property;
		std::variant<ValueWrapper, std::vector<ScopeName>> value;
	};

	

	struct TypePair
	{
		std::u32string_view name;
		Type type;
	};

	struct ValuePair
	{
		std::u32string_view name;
		Value value;
	};

	struct TypeTable
	{
		struct Node
		{
			size_t upper_action_scope;
			std::map<std::u32string_view, Type> types;
		};
		void Push();
	private:
		size_t current_action_scope = 0;
		std::vector<Node> table;
	};

	struct ValueTable
	{
		struct Node
		{
			size_t upper_action_scope;
			std::map<std::u32string_view, Value> types;
		};
		size_t Push();
		ScopeName Add(std::u32string_view name, );
	private:
		size_t current_action_scope = 0;
		size_t action_scope_new = 1;
		std::vector<Node> table;
	};

	struct SymbolTable
	{
		void TypePush()		{ type_table.Push(); }
		void ValuePush()	{ value_table.Push(); }

		struct Node
		{
			std::map<std::u32string_view, Value> values;
			std::map<std::u32string_view, Type> types;
		};

		struct InsideValue
		{
			Value values;
		};

	private:

		std::map<std::u32string_view, Value> InsideValue;
		ValueStorage storage;
		TypeTable type_table;
		ValueTable value_table;
	};
	*/


	



	/*
	struct Type
	{
		std::u32string_view name;
		size_t size;
		size_t align;
	};

	struct TypeTable
	{
		struct Member
		{
			Type info;
			size_t size;
			size_t align;
		};
	};


	struct TypeDescription
	{

	};


	struct TypeDescription;
	using TypeDescriptionMapping = std::map<std::u32string_view, TypeDescription>;

	struct Space
	{
		size_t align = 1;
		size_t size = 0;
	};

	struct Property
	{
		bool is_pointer : 1 = false;
		std::optional<size_t> array_count;
	};

	struct TypeDescriptionManager
	{
		
	private:
		std::vector<TypeDescriptionMapping> mapping;
	};

	struct TypeDescription
	{
		struct Member
		{
			TypeDescriptionMapping::const_iterator type;
			std::u32string name;
			Space space;
			Property pro;
		};
	};









	namespace Implement
	{
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
			if (data != nullptr)
				std::memcpy(datas.data(), reinterpret_cast<std::byte const*>(data), size);
			return std::move(datas);
		}
	}

	

	

	struct TypeInfo
	{
		std::u32string type_name;
		Space space;
	};

	struct NoNamed
	{
		TypeInfo type;
		Space space;
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

	struct Pattern
	{
		TypeInfo info;
		std::vector<std::byte> default_value;
		NoNamed Construct();
		NoNamed Construct(std::vector<std::byte> data);
	};

	template<typename StorageType> 
	struct PatternWrapper
	{
		PatternWrapper(std::u32string type_name, StorageType&& type)
			: pattern(Pattern{ TypeInfo{std::move(type_name), Space{ alignof(StorageType), sizeof(StorageType)}}, {} })
		{
			pattern.default_value.resize(sizeof(StorageType));
			std::memcpy(pattern.default_value.data(), &type, sizeof(StorageType));
		}
		PatternWrapper(std::u32string type_name)
			: pattern(Pattern{ TypeInfo{std::move(type_name), Space{ alignof(StorageType), sizeof(StorageType)}}, {} })
		{
			pattern.default_value.resize(sizeof(StorageType));
		}
		NoNamed Construct(){return pattern.Construct();}
		NoNamed Construct(StorageType&& Type){ return pattern.Construct(Implement::ToData(Type));}
	private:
		Pattern pattern;
	};

	

	struct Member
	{
		std::u32string type_name;
		std::u32string name;
		size_t offset;
		size_t size;
	};
	



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

	

	

	template<typename Type>
	std::vector<std::byte> ToZeroData() {
		return ToArrayData<Type>(nullptr, 0);
	}

	

	
	
	struct TypeDefine
	{
		struct Member
		{
			TypeInfo type;
			Property property;
			std::u32string name;
			size_t offset;
		};

		TypeInfo info;
		std::vector<Member> members;
		std::vector<std::byte> default_value;
		NoNamed Construct(std::vector<std::byte> datas = {}, Property pro = {}) const;
		Named ConstructNamed(std::u32string name, std::vector<std::byte> datas = {}, Property pro = {}) const {
			return Named{ Construct(std::move(datas), pro), std::move(name) };
		}
	};
	*/

	/*
	template<typename StorageType>
	struct Pattern
	{
		Pattern(std::u32string type_name, StorageType const& data) : 
			info_storage({ TypeInfo{ std::move(type_name), alignof(StorageType), sizeof(StorageType) }, {}, ToData(data) }) {}
		Pattern(std::u32string type_name) :
			info_storage({ TypeInfo{ std::move(type_name), alignof(StorageType), sizeof(StorageType) }, {}, ToArrayData<StorageType>(nullptr, 0) }) {}
		operator TypeInfoStorage() { return info_storage; }
		TypeInfo const& Info() const { return info_storage.info; }
		NoNamed Construct(StorageType const& datas, Property pro = {}) const {
			return info_storage.Construct(ToData(datas), pro);
		}
		NoNamed Construct(Property pro = {}) const {
			return info_storage.Construct({}, pro);
		}
		Named ConstructNamed(std::u32string name, StorageType const& datas, Property pro = {}) const {
			return info_storage.Construct(std::move(name), ToData(datas), pro);
		}
		Named ConstructNamed(std::u32string name, Property pro = {}) const {
			return info_storage.Construct(std::move(name), {}, pro);
		}
	private:
		TypeDefine info_storage;
	};
	*/

	/*
	struct TypeDefineManager
	{
		TypeDefineManager() = default;
		TypeDefineManager(TypeDefineManager&&) = default;
		TypeDefineManager(TypeDefineManager const&) = default;
		TypeDefineManager(std::initializer_list<TypeDefine> input) {
			for (auto& Ite : input)
				Insert(Ite);
		}
		TypeDefine const* Find(std::u32string_view Name) const;
		bool Insert(TypeDefine input_type);
		TypeDefine& InsertLinq(TypeDefine input_type);
	private:
		std::map<std::u32string_view, TypeDefine> all_types;
	};

	enum class InfoSearchingType
	{
		Forwark,
		Backwark,
	};

	struct TypeDefineLinker
	{
		Space pointer_space = { alignof(nullptr), sizeof(nullptr) };
		Space default_space = { 1, 0};
		TypeDefine Link(std::u32string type_name, std::vector<Named> members);
		virtual void AdjustMember(std::vector<Named>& meb_list);
		virtual bool ShouldIgnoreSameNameMember(Named& old_one, Named& new_one);
		virtual void HandleAligned(Space& offset, Named& element);
	};

	struct VariableManager
	{
		Named* Find(std::u32string_view Name);
		bool Insert(Named var);
	public:
		std::map<std::u32string_view, Named> all_value;
	};
	*/
}