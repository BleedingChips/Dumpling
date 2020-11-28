#pragma once
#include <variant>
#include <vector>
#include "../../PineApple/Public/Symbol.h"
namespace Dumpling::Mscf
{
	
	using Mask = PineApple::Symbol::Table::Mask;
	using Section = PineApple::Symbol::Section;

	struct ValueProperty
	{
		Mask type;
		Mask sampler;
		std::vector<int64_t> array_count;
		std::vector<Mask> mate_data;
	};

	struct TypeProperty
	{
		std::vector<Mask> values;
	};

	enum class TextureType
	{
		Tex1,
		Tex2,
		Tex3,
	};

	struct TextureProperty
	{
		TextureType type;
	};

	struct SamplerProperty {};

	struct MateDataProperty {};

	struct UnTypedListProperty {};
	
	struct ImportProperty
	{
		std::u32string_view path;
	};

	struct ReferencesPath
	{
		Mask property_reference;
		std::vector<std::u32string_view> references;
	};

	struct CodeProperty
	{
		std::vector<ReferencesPath> reference;
		std::u32string_view code;
	};

	struct InoutParameter
	{
		bool is_input;
		Mask type;
		std::u32string_view name;
		Section section;
	};
	
	struct SnippetProperty
	{
		std::vector<ReferencesPath> references;
		std::u32string_view code;
		std::vector<InoutParameter> parameters;
	};

	struct MaterialProperty
	{
		std::variant<std::u32string_view, ReferencesPath> complie_type;
		std::vector<Mask> property;
		std::vector<Mask> snippets;
	};

	struct Content
	{
		std::vector<Mask> propertys;
		std::vector<Mask> statement;
	};

	struct Commands
	{
		using DataType = std::variant<int64_t, float, std::u32string_view, bool>;
	private:
		struct Data { DataType datas; };
		struct CoverType { Mask type; size_t parameter; };
		struct EqualToData { Mask value; };
	public:
		void PushData(DataType command, Section section){ AllCommands.push_back({Data{command}, section }); }
		void CoverToType(Mask type, size_t parameter, Section section){ AllCommands.push_back({CoverType{type, parameter}, section }); }
		void EqualData(Mask type, Section section) { AllCommands.push_back({EqualToData{ type }, section }); }
	private:
		using CommandType = std::variant<Data, CoverType, EqualToData>;
		std::vector<std::tuple<CommandType, Section>> AllCommands;
	};

	using Table = PineApple::Symbol::Table;

	std::tuple<Table, Commands> CreateDefaultContent();

	struct HlslStorageInfoLinker : PineApple::Symbol::StorageInfoLinker
	{
		using PineApple::Symbol::StorageInfoLinker::StorageInfoLinker;
		using StorageInfo = PineApple::Symbol::StorageInfo;
		virtual HandleResult Handle(StorageInfo cur, StorageInfo input) const override;
	};

	Content Parser(std::u32string_view code, Table& table, Commands& commands);

	/*

	struct SymbolTable
	{
		SymbolTable();
		void InsertType(std::u32string name, std::vector<Value> const& all_value);
	private:
		PineApple::Symbol::ValueBuffer buffer;
		PineApple::Symbol::Table table;
	};

	*/



	/*
	LRTable DefaultTable();

	
	struct DataWrapper
	{
		size_t offset;
		size_t length;
	};

	struct PushDataC {
		LRTable::Mask type;
		DataWrapper datas;
	};

	struct PushDefaultValueC
	{
		LRTable::Mask type;
	};

	struct MakeDefaultValueC
	{
		LRTable::Mask type;
		size_t parameter;
	};

	struct EqualValueC
	{
		LRTable::Mask name;
		size_t parameter;
	};

	struct MakePushValueC
	{
		size_t parameter;
	};

	struct LinkPushValueC
	{
		size_t parameter;
	};

	struct ValueBuildCommand
	{
		void PushData(LRTable const& table, int32_t data);
		void PushData(LRTable const& table, float data);
		void PushData(LRTable const& table, std::u32string_view data);
		void PushData(LRTable const& table, bool);
		void PushData(LRTable const& table, uint32_t);
		void PushDefaultValue(LRTable const& table, std::u32string_view type_name);
		void MakeDefaultValue(LRTable const& table, std::u32string_view type_name, size_t count);
		void EqualValue(LRTable const& table, std::u32string_view value_name, size_t count);
		void MakePushValue(size_t count);
		void LinkPushValue(size_t count);
	private:
		DataWrapper InsertData(std::vector<std::byte> const& data);
		using Command = std::variant<
			PushDataC,
			PushDefaultValueC,
			MakeDefaultValueC,
			EqualValueC,
			MakePushValueC,
			LinkPushValueC
		>;
		std::vector<Command> commands;
		std::vector<std::byte> datas;
	};

	ValueBuildCommand DefaultCommand(LRTable const& Table);
	*/
}