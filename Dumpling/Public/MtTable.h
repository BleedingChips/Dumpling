#pragma once
#include <variant>
#include <vector>
#include "ParserDefine.h"
#include <string_view>
namespace Dumpling::MscfParser
{

	using namespace Dumpling::Parser;

	using TypeProperty = Potato::Grammar::TypeProperty;


	struct ValueTypeProperty : TypeProperty
	{
		TypeProperty sampler;
	};

	using TypeSymbol = Potato::Grammar::TypeSymbol;

	struct ValueSymbol
	{
		ValueTypeProperty type;
		std::vector<std::optional<size_t>> arrays;
		std::vector<AreaMask> mate_data;
		bool is_member = false;
	};

	struct MateDataSymbol {
		std::optional<TypeProperty> type;
	};
	
	struct ImportSymbol
	{
		std::u32string_view path;
	};

	struct References
	{
		SymbolMask property_reference;
		std::u32string_view reference_name;
		std::vector<std::u32string_view> sub_references;
	};

	struct CodeSymbol
	{
		std::vector<References> reference;
		std::u32string_view code;
		AreaMask matedata;
	};

	struct InoutParameter
	{
		bool is_input;
		SymbolMask type;
		std::u32string_view type_name;
		std::u32string_view name;
	};
	
	struct SnippetSymbol
	{
		std::vector<References> reference;
		std::u32string_view code;
		std::vector<InoutParameter> parameters;
		AreaMask matedata;
	};

	struct MaterialSymbol
	{
		AreaMask mate_data;
		std::u32string_view shading_mode;
		AreaMask propertys;
	};

	struct PropertySymbol
	{
		AreaMask propertys;
		AreaMask mate_datas;
	};

	struct MaterialUsingSymbol
	{
		References reference_path;
	};

	struct MaterialDefineSymbol
	{
		std::u32string_view define_target;
		std::u32string_view define_source;
	};

	struct C_PushData{  ValueMask mask;  };
	struct C_MarkAsArray{ size_t count = 0; };
	struct C_ConverType { TypeProperty type; size_t count; };
	struct C_SetValue { SymbolMask to;  };

	struct MscfContent : Table
	{
		MscfContent() = default;
		MscfContent(MscfContent&& MPT) = default;
		using CommandType = std::variant<C_PushData, C_MarkAsArray, C_ConverType, C_SetValue>;
		void PushCommand(CommandType CT, Section Se){commands.push_back({std::move(CT), Se});}
		std::vector<std::tuple<CommandType, Section>> commands;
	};

	MscfContent MscfParser(std::u32string_view code);

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

