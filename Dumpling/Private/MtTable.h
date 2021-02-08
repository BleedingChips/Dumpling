#pragma once
#include <variant>
#include <vector>
#include "Potato/Public/Grammar.h"
#include "ParserDefine.h"
#include <string_view>
namespace Dumpling::Mscf
{

	using namespace Potato::Grammar;

	struct MateDataProperty {};
	
	struct ValueProperty
	{
		SymbolMask type;
		SymbolMask sampler;
		std::vector<int32_t> array_count;
		std::vector<SymbolMask> mate_data;
	};
	
	struct ImportProperty
	{
		std::u32string_view path;
	};

	struct ReferencesPath
	{
		SymbolMask property_reference;
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
		SymbolMask type;
		std::u32string_view name;
		Section section;
	};
	
	struct SnippetProperty
	{
		std::u32string_view name;
		std::vector<ReferencesPath> references;
		std::u32string_view code;
		std::vector<InoutParameter> parameters;
	};

	struct MaterialProperty
	{
		std::variant<std::u32string_view, ReferencesPath> compile_type;
		std::vector<SnippetProperty> snippets;
		std::vector<SymbolMask> propertys;
	};

	struct Content
	{
		std::vector<SymbolMask> property;
		std::vector<SymbolMask> statements;
	};

	struct C_PushData{  ValueMask mask;  };
	struct C_MarkAsArray{ size_t count = 0; };
	struct C_ConverType { SymbolMask mask; size_t count; };
	struct C_SetValue { SymbolMask to;  };

	namespace Exception
	{
		struct Interface{};

		struct UndefineSymbol { std::u32string name; };
	}

	template<typename Type>
	auto MakeException(Type&& type) { return Potato::Misc::create_exception_tuple<Exception::Interface>(type); }

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