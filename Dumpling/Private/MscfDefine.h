#pragma once
#include <variant>
#include <vector>
#include "Potato/Public/Symbol.h"
namespace Dumpling::Mscf
{
	
	using Mask = Potato::Symbol::Table::Mask;
	using Section = Potato::Symbol::Section;
	using MemoryModel = Potato::Symbol::MemoryModel;
	using Table = Potato::Symbol::Table;

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
		std::variant<std::u32string_view, ReferencesPath> compile_type;
		std::vector<Mask> property;
		std::vector<Mask> snippets;
	};

	struct Content
	{
		std::vector<Mask> property;
		std::vector<Mask> statement;
	};

	struct Commands
	{
		
	private:
		
		struct ValueDescriptionC { Mask type; std::tuple<size_t, size_t> data; };
		struct MakeValueC { Mask type; size_t count; };
		struct EqualValueC { Mask type; size_t count; };
		struct MakeListC { Mask type; size_t count; };
		
	public:
		
		//void PushData(DataType command, Section section){ AllCommands.push_back({ ValueScriptionC{command}, section }); }
		Mask PushData(Table& table, bool value, Section section);
		Mask PushData(Table& table, std::u32string_view value, Section section);
		Mask PushData(Table& table, float value, Section section);
		Mask PushData(Table& table, int32_t value, Section section);
		Mask PushData(Mask mask, Table& table, std::byte const* data, size_t data_length, Section section);
		Mask MakeValue(Mask type, size_t parameter, Section section){ AllCommands.emplace_back(MakeValueC{type, parameter}, section); return type; }
		Mask MakeList(Mask type, size_t parameter, Section section) { AllCommands.emplace_back(MakeListC{type, parameter}, section ); return type; }
		Mask EqualValue(Mask type, size_t parameter, Section section) { AllCommands.emplace_back(EqualValueC{ type, parameter }, section); return type; }
		
	private:
		
		using CommandType = std::variant<ValueDescriptionC, MakeValueC, EqualValueC, MakeListC>;
		
		std::vector<std::byte> ConstDataTable;
		std::vector<std::tuple<CommandType, Section>> AllCommands;
	};

	

	std::tuple<Table, Commands> CreateDefaultContent();

	struct HlslStorageInfoLinker : PineApple::Symbol::MemoryModelMaker
	{
		using PineApple::Symbol::MemoryModelMaker::MemoryModelMaker;
		using MemoryModel = PineApple::Symbol::MemoryModel;
		virtual HandleResult Handle(MemoryModel cur, MemoryModel input) const override;
	};

	Content Parser(std::u32string_view code, Table& table, Commands& commands);

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