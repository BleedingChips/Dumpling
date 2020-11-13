#pragma once
#include <variant>
#include <vector>
#include "../../PineApple/Public/Symbol.h"
namespace Dumpling::Mscf
{
	
	using Mask = PineApple::Symbol::Table::Mask;

	struct ValueProperty
	{
		Mask type;
		std::vector<int64_t> array_count;
	};

	struct TypeProperty
	{
		struct Member
		{
			std::u32string_view name;
			ValueProperty property;
		};
		std::vector<Member> values;
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
		Mask read_type;
	};

	struct SamplerProperty {};

	struct Commands
	{
		using DataType = std::variant<int64_t, float, std::u32string_view, bool>;
	private:
		struct Data { DataType datas; };
		struct CoverType { Mask type; size_t parameter; };
		struct EqualToData { Mask value; };
	public:
		void PushData(DataType command){ AllCommands.push_back(Data{command}); }
		void CoverToType(Mask type, size_t parameter){ AllCommands.push_back(CoverType{type, parameter}); }
		void EqualData(Mask type) { AllCommands.push_back(EqualToData{ type }); }
	private:
		using CommandType = std::variant<Data, CoverType, EqualToData>;
		std::vector<CommandType> AllCommands;
	};

	using Table = PineApple::Symbol::Table;

	std::tuple<Table, Commands> CreateDefaultContent();

	struct HlslStorageInfoLinker : PineApple::Symbol::StorageInfoLinker
	{
		using PineApple::Symbol::StorageInfoLinker::StorageInfoLinker;
		using StorageInfo = PineApple::Symbol::StorageInfo;
		virtual HandleResult Handle(StorageInfo cur, StorageInfo input) const override;
	};

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