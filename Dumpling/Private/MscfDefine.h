#pragma once
#include <variant>
#include <vector>
#include "Potato/Public/Symbol.h"
#include "ParserDefine.h"
namespace Dumpling::Mscf
{

	using namespace Potato::Symbol;
	
	struct ValueProperty
	{
		Table::Mask type;
		Table::Mask sampler;
		std::vector<int64_t> array_count;
	};
	
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
		std::variant<std::u32string_view, ReferencesPath> compile_type;
		std::vector<Table::Mask> property;
		std::vector<Table::Mask> snippets;
	};

	struct Content
	{
		std::vector<Table::Mask> property;
		std::vector<Table::Mask> statement;
	};

	struct C_PushData{  ConstDataTable::Mask mask;  };
	struct C_MakeArray{  size_t count; };
	struct C_ConverType { Table::Mask mask; size_t count; };
	struct C_SetValue { Table::Mask mask; };

	namespace Exception
	{
	}

	template<typename Type>
	auto MakeException(Type&& type) { return Potato::Misc::create_exception_tuple<Exception::Interface>(type); }

	struct MscfContent
	{
		MscfContent() = default;
		MscfContent(MscfContent&& MPT) = default;
		using CommandType = std::variant<C_PushData, C_MakeArray, C_ConverType, C_SetValue>;
		Parser::ParserSymbolTable symbol;
		Parser::ParserConstData const_data;
		std::vector<CommandType> commands;
		std::vector<Table::Mask> propertys;
		std::vector<Table::Mask> statements;
	};

	struct HlslStorageInfoLinker : MemoryModelMaker
	{
		using MemoryModelMaker::MemoryModelMaker;
		using MemoryModel = MemoryModel;
		virtual HandleResult Handle(MemoryModel cur, MemoryModel input) const override;
	};

	Content MscfParser(std::u32string_view code, Table& table, Commands& commands);

	void FilterAndCheck(Content& content, Table& table);
	
	void Mapping(Table& table, Commands& commands, Content& Content);

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