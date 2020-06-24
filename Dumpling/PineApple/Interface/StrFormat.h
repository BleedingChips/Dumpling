#pragma once
#include <vector>
#include <variant>
#include <string>
#include <assert.h>
namespace PineApple::StrFormat
{

	//std::u32string operator()(std::u32string_view par, SourceType&&) { return {}; }
	template<typename SourceType>
	struct Formatter;

	enum class PatternType
	{
		NormalString,
		Parameter,
		LeftBrace,
		RightBrace,
	};

	struct PatternRef
	{
		struct Element
		{
			PatternType type;
			std::u32string_view string;
		};

		std::vector<Element> patterns;
	};

	PatternRef CreatePatternRef(std::u32string_view Ref);

	namespace Implement
	{
		// Index, Charactor Count
		std::tuple<size_t, size_t> LocateParmeters(PatternRef const& pattern, size_t pattern_index);
		size_t FormatImp(PatternRef const& pattern, size_t pattern_index, std::vector<std::u32string>& output);

		template<typename CurType, typename ...Type> 
		size_t FormatImp(PatternRef const& pattern, size_t pattern_index, std::vector<std::u32string>& output, CurType&& input1, Type&&... oinput)
		{
			auto [PIndex, Sum] = LocateParmeters(pattern, pattern_index);
			if (PIndex < pattern.patterns.size())
			{
				auto [type, pars] = pattern.patterns[PIndex];
				assert(type == PatternType::Parameter);
				std::u32string result = Formatter<std::remove_cv_t<std::remove_reference_t<CurType>>>{}(pars, std::forward<CurType>(input1));
				Sum += result.size();
				output.push_back(std::move(result));
				return Sum + FormatImp(pattern, PIndex + 1, output, std::forward<Type>(oinput)...);
			}
			return Sum + FormatImp(pattern, PIndex + 1, output);
		}

		std::u32string Link(size_t require_space, PatternRef const& pattern, std::vector<std::u32string>& output);
		std::u32string ExceptionLink(PatternRef const& pattern);
	}

	template<typename... Type>
	std::u32string Process(std::u32string_view Pattern, Type&&... args)
	{
		auto Patterns = CreatePatternRef(Pattern);
		return Process(Patterns, std::forward<Type>(args)...);
	}

	template<typename... Type>
	std::u32string Process(PatternRef const& Patterns, Type&&... args)
	{
		//auto Patterns = CreatePatternRef(Pattern);
		std::vector<std::u32string> Result;
		size_t require = Implement::FormatImp(Patterns, 0, Result, std::forward<Type>(args)...);
		return Implement::Link(require, Patterns, Result);
	}

	template<typename Type>
	std::u32string DirectProcess(std::u32string_view pars, Type&& args)
	{
		Formatter<std::remove_reference_t<std::remove_cv_t<Type>>>{}(pars, std::forward<Type>(args));
	}

	namespace Error
	{
		struct UnsupportPatternString {
			std::u32string PatternString;
		};

		struct LackOfFormatParas {
			std::u32string PatternString;
		};
	}
}

namespace PineApple::StrFormat
{
	template<>
	struct Formatter<char32_t*>
	{
		std::u32string operator()(std::u32string_view par, char32_t const* Input);
	};

	template<>
	struct Formatter<char32_t>
	{
		std::u32string operator()(std::u32string_view par, char32_t const* Input);
	};

	template<>
	struct Formatter<std::u32string>
	{
		std::u32string operator()(std::u32string_view par, std::u32string Input);
	};

	template<>
	struct Formatter<float>
	{
		std::u32string operator()(std::u32string_view par, float Input);
	};

	template<>
	struct Formatter<double>
	{
		std::u32string operator()(std::u32string_view par, double Input);
	};

	template<>
	struct Formatter<uint32_t>
	{
		std::u32string operator()(std::u32string_view par, uint32_t Input);
	};

	template<>
	struct Formatter<int32_t>
	{
		std::u32string operator()(std::u32string_view par, int32_t Input);
	};

	template<>
	struct Formatter<uint64_t>
	{
		std::u32string operator()(std::u32string_view par, uint64_t Input);
	};

	template<>
	struct Formatter<int64_t>
	{
		std::u32string operator()(std::u32string_view par, int64_t Input);
	};
}