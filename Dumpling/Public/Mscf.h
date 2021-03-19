#pragma once
#include <string>
#include <array>
#include <string>
#include <map>
#include <set>
#include <array>

#include "Potato/Public/Ebnf.h"
#include "Potato/Public/Misc.h"
#include "ParserDefine.h"

namespace Dumpling::Mscf
{
	
	using Section = Potato::Ebnf::Section;
	using BuildInType = Dumpling::Parser::BuildInType;

	enum class StorageType : uint8_t
	{
		Import,
		Code,
		Snippet,
		Property,
		Material,
	};

	struct MscfDocument
	{
		struct Type
		{
			std::u32string name;
			struct Member
			{
				std::optional<BuildInType> type;
				size_t reference_type;
				std::u32string name;
				size_t offset;
				size_t length;
			};
			std::vector<Member> members;
			size_t align;
			size_t length;
		};
		std::vector<Type> type_define;


		//std::vector<>
	};

	/*
	struct MscfDocumenet
	{
		using IndexSpan = Potato::IndexSpan<>;

		struct Type
		{
			struct Member
			{
				std::u32string name;
				BuildInType type;
				size_t reference_type;
				size_t offset;
				size_t array_count;
			};
		};

		std::vector<std::byte> data_table;

		struct MappingElement
		{
			StorageType type;
			size_t index;
		};

		std::vector<MappingElement> export_list;
		std::vector<IndexSpan> import_path;
		std::vector<IndexSpan> sub_referenc;

		struct Reference
		{
			MappingElement root_ref;
			IndexSpan sub_ref;
		};

		std::vector<Reference> references;

		struct CodeElement
		{
			IndexSpan ref;
			IndexSpan code;
		};

		std::vector<CodeElement> codes;

		struct InoutElement
		{
			bool is_input;
			IndexSpan type_name;
			IndexSpan name;
		};

		std::vector<InoutElement> inouts;

		struct SnippetElement
		{
			IndexSpan input;
			IndexSpan ref;
			IndexSpan code;
		};

		std::vector<SnippetElement> snippet;

		struct DefineElement {
			IndexSpan id;
			IndexSpan define_str;
		};

		std::vector<DefineElement> defines;

		struct MateData
		{
			IndexSpan name;
			IndexSpan data;
			BuildInType type;
		};

		std::vector<MateData> matedatas;

		struct MaterialVariableElement
		{
			IndexSpan type_name;
			IndexSpan name;
			IndexSpan data;
			BuildInType type;
		};

		std::vector<MaterialVariableElement> material_datas;

		struct MaterialElement
		{
			IndexSpan shading_mode;
			IndexSpan matedatas;
			IndexSpan defines;
			IndexSpan datas;
			IndexSpan snippets;
		};
		
		std::vector<MaterialElement> materials;

		struct VariableElement
		{
			IndexSpan name;
			IndexSpan type_name;
			BuildInType type;
			IndexSpan matedatas;
			IndexSpan datas;
		};

		std::vector<VariableElement> export_variable;
		std::vector<VariableElement> instance_variable;
		std::vector<uint32_t> buffer;

		struct TypeVariant
		{
			IndexSpan name;
			int32_t size;
			int32_t alig;
			IndexSpan type_define;
		};
	};
	*/

	struct Mscf
	{
		
	};

	MscfDocument Translate(std::u32string_view code);

}

namespace Dumpling::Exception::Mscf
{
	using Section = Potato::Ebnf::Section;

	struct Interface {};

	using BaseDefineInterface = Potato::Exception::DefineInterface<Interface>;


	struct UndefineSymbol {
		using ExceptionInterface = BaseDefineInterface;
		std::u32string name;
	};

	struct UndefineType
	{
		using ExceptionInterface = BaseDefineInterface;
		std::u32string type_name;
		Section section;
	};

	struct RequireTypeDonotSupportSample
	{
		using ExceptionInterface = BaseDefineInterface;
		std::u32string type_name;
		Section section;
	};

	struct RedefineProperty
	{
		using ExceptionInterface = BaseDefineInterface;
		std::u32string redefine_name;
		Section redefine_section;
		Section predefine_section;
	};

	struct UndefineImport
	{
		using ExceptionInterface = BaseDefineInterface;
		std::u32string name;
		Section section;
	};
}