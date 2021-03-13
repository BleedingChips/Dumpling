#pragma once
#include <variant>
#include <vector>
#include "Potato/Public/Grammar.h"
#include "ParserDefine.h"
#include <string_view>
namespace Dumpling::MscfParser
{

	using namespace Potato::Grammar;

	struct MateDataProperty {};
	
	struct ValueProperty
	{
		SymbolMask type;
		std::u32string_view type_name;
		SymbolMask sampler;
		std::u32string_view sampler_name;
		std::vector<int32_t> array_count;
		std::vector<SymbolAreaMask> mate_data;
	};
	
	struct ImportStatement
	{
		std::u32string_view path;
	};

	struct ReferencesPath
	{
		SymbolMask property_reference;
		std::u32string_view reference_name;
		std::vector<std::u32string_view> references;
	};

	struct CodeStatement
	{
		SymbolAreaMask references;
		std::u32string_view code;
	};

	struct InoutParameter
	{
		bool is_input;
		SymbolMask type;
		std::u32string_view type_name;
		std::u32string_view name;
	};
	
	struct SnippetStatement
	{
		SymbolAreaMask references;
		std::u32string_view code;
		SymbolAreaMask parameters;
	};

	struct MaterialStatement
	{
		SymbolAreaMask mate_data;
		std::u32string_view shading_mode;
		SymbolAreaMask propertys;
	};

	struct PropertyStatement
	{
		SymbolAreaMask propertys;
	};

	struct Content
	{
		SymbolAreaMask statements;
	};

	struct MaterialUsingStatement
	{
		ReferencesPath reference_path;
	};

	struct MaterialDefineStatement
	{
		std::u32string_view define_target;
		std::u32string_view define_source;
	};

	struct C_PushData{  ValueMask mask;  };
	struct C_MarkAsArray{ size_t count = 0; };
	struct C_ConverType { std::u32string_view type_name; SymbolMask mask; size_t count; };
	struct C_SetValue { SymbolMask to;  };

	struct MscfContent
	{
		MscfContent() = default;
		MscfContent(MscfContent&& MPT) = default;
		using CommandType = std::variant<C_PushData, C_MarkAsArray, C_ConverType, C_SetValue>;
		template<typename Type>
		decltype(auto) InsertValue(Type const& t){ return const_data.InsertValue(symbol, t); }
		decltype(auto) ReservedLazyValue(){ return const_data.ReservedLazyValue(); }
		void PushCommand(CommandType CT, Section Se){commands.push_back({std::move(CT), Se});}
		Parser::ParserSymbol symbol;
		Parser::ParserValue const_data;
		std::vector<std::tuple<CommandType, Section>> commands;
		Content content;
	};

	MscfContent MscfParser(std::u32string_view code);

	struct HlslStorageInfoLinker : MemoryModelMaker
	{
		using MemoryModelMaker::MemoryModelMaker;
		using MemoryModel = MemoryModel;
		virtual HandleResult Handle(MemoryModel cur, MemoryModel input) const override;
	};

	/*
	Content MscfParser(std::u32string_view code, Table& table, Commands& commands);

	void FilterAndCheck(Content& content, Table& table);
	
	void Mapping(Table& table, Commands& commands, Content& Content);
	*/

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

