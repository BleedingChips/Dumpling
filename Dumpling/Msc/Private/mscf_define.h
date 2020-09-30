#pragma once
#include <variant>
#include <vector>
#include "../../PineApple/Public/Symbol.h"
namespace Dumpling::Mscf
{
	
	using Mask = PineApple::Symbol::Table::Mask;

	struct DataStorage
	{
		struct Mask{ size_t offset = 0; size_t length = 0; };
		std::tuple<std::byte const*, size_t> Read(Mask mask) const;
		Mask Push(std::byte const* data, size_t length);
	private:
		std::vector<std::byte> datas;
	};

	using DataMask = DataStorage::Mask;

	struct ValueProperty
	{
		size_t read_wrapper;
		bool is_array;
		std::vector<size_t> array_count;
		bool unmark_array;
	};

	struct Value
	{
		Mask type;
		std::u32string_view name;
		ValueProperty property;
		DataMask value;
	};

	struct Record
	{
		size_t element_count;
	};

	struct HlslStorageInfoLinker : PineApple::Symbol::StorageInfoLinker
	{
		using PineApple::Symbol::StorageInfoLinker::StorageInfoLinker;
		virtual HandleResult Handle(StorageInfo cur, StorageInfo input) const override;
	};

	struct TypeProperty
	{
		struct Value {
			Mask type;
			std::u32string_view name;
			size_t offset;
		};
		DataMask default_value;
		PineApple::Symbol::StorageInfo info;
		std::vector<TypeProperty> values;
	};

	struct SymbolTable
	{
		PineApple::Symbol::Table table;
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