#include "../Public/StrFormat.h"
#include "../Public/Nfa.h"
#include "../Public/CharEncode.h"
namespace PineApple::StrFormat
{

	static std::u32string_view PatternRex[] = {
		UR"([^\{\}]+)",
		UR"(\{\{)",
		UR"(\}\})",
		UR"(\{[^\{]*?\})",
	};

	static Nfa::Table table = Nfa::CreateTableFromRex(PatternRex, 4);


	PatternRef CreatePatternRef(std::u32string_view Ref)
	{
		try {
			auto Result = Nfa::Process(table, Ref);
			std::vector<PatternRef::Element> patterns;
			for (auto& ite : Result)
			{
				switch (ite.acception)
				{
				case 0: {
					patterns.push_back({ PatternType::NormalString, ite.capture });
				} break;
				case 1: {
					patterns.push_back({ PatternType::LeftBrace,  ite.capture });
				} break;
				case 2: {
					patterns.push_back({ PatternType::RightBrace, ite.capture });
				} break;
				case 3: {
					patterns.push_back({ PatternType::Parameter, {ite.capture.data() + 1, ite.capture.size() - 2} });
				} break;
				default: assert(false);
				}
			}
			return { std::move(patterns) };
		}
		catch (Nfa::Error::UnaccaptableString const& str)
		{
			throw Error::UnsupportPatternString{str.TotalString};
		}
	}

	namespace Implement
	{
		std::tuple<size_t, size_t> LocateParmeters(PatternRef const& pattern, size_t pattern_index)
		{
			size_t Sum = 0;
			for (; pattern_index < pattern.patterns.size(); ++pattern_index)
			{
				auto& [Type, str] = pattern.patterns[pattern_index];
				switch (Type)
				{
				case PatternType::NormalString: {Sum += str.size(); } break;
				case PatternType::RightBrace: {Sum += 1;} break;
				case PatternType::LeftBrace: {Sum += 1; } break;
				case PatternType::Parameter: {return { pattern_index, Sum}; } break;
				default:
					break;
				}
			}
			return { pattern_index, Sum};
		}

		size_t FormatImp(PatternRef const& pattern, size_t pattern_index, std::vector<std::u32string>& output)
		{
			auto [PIndex, Sum] = LocateParmeters(pattern, pattern_index);
			if (PIndex < pattern.patterns.size())
				throw Error::LackOfFormatParas{ ExceptionLink(pattern) };
			return Sum;
		}

		std::u32string Link(size_t require_space, PatternRef const& pattern, std::vector<std::u32string>& output)
		{
			std::u32string Result;
			Result.resize(require_space);
			size_t Sum = 0;
			size_t ParasUsed = 0;
			for (auto& ite : pattern.patterns)
			{
				auto [type, str] = ite;
				switch (type)
				{
				case PatternType::NormalString:
					assert(Sum + str.size()<= require_space);
					std::memcpy(Result.data() + Sum, str.data(), str.size() * sizeof(char32_t));
					Sum += str.size();
					break;
				case PatternType::Parameter:
					if (ParasUsed < output.size())
					{
						auto& Str = output[ParasUsed++];
						assert(Sum + Str.size() <= require_space);
						std::memcpy(Result.data() + Sum, Str.data(), Str.size() * sizeof(char32_t));
						Sum += Str.size();
					}
					else {
						assert(false);
					}
					break;
				case PatternType::LeftBrace:
					assert(Sum + 1 <= require_space);
					Result[Sum++] = U'{';
					break;
				case PatternType::RightBrace:
					assert(Sum + 1 <= require_space);
					Result[Sum++] = U'}';
					break;
				default:
					assert(false);
					break;
				}
			}
			return std::move(Result);
		}

		std::u32string ExceptionLink(PatternRef const& pattern)
		{
			size_t size = 0;
			for (auto [type, str] : pattern.patterns)
			{
				switch (type)
				{
				case PatternType::NormalString:
					size += str.size();
					break;
				case PatternType::LeftBrace:
				case PatternType::Parameter:
				case PatternType::RightBrace:
					size += 2;
					break;
				default:
					assert(false);
					break;
				}
			}
			std::u32string Resource;
			Resource.reserve(size);
			for (auto [type, str] : pattern.patterns)
			{
				switch (type)
				{
				case PatternType::NormalString:
					Resource += str;
					break;
				case PatternType::Parameter:
					Resource += U"{}";
					break;
				case PatternType::LeftBrace:
					Resource += U"{{";
					break;
				case PatternType::RightBrace:
					Resource += U"}}";
					break;
				default:
					assert(false);
					break;
				}
			}
			return Resource;
		}
	}
}
namespace PineApple::StrFormat
{

	static std::u32string_view FormatRex[] = {
		UR"(\s)",
		UR"(-hex)",
		//UR"(-e)"
	};

	static Nfa::Table FormatTable = Nfa::CreateTableFromRexReversal(FormatRex, 2);

	struct Paras
	{
		bool IsHex = false;
	};

	Paras ParasTranslate(std::u32string_view Code)
	{
		Paras Result;
		//std::optional<std::string> size_require;
		auto Re = Nfa::Process(FormatTable, Code);
		for (auto& ite : Re)
		{
			switch (ite.acception)
			{
			case 0: {} break;
			case 1: {Result.IsHex = true; } break;
			//case 2: {IsE = true; } break;
			//case 3: {size_require = Encoding::EncodingWrapper<char32_t>({ ite.capture.data() + 1, ite.capture.size() - 1 }).To<char>();} break;
			default:
				break;
			}
		}
		return Result;
	}


	std::u32string Formatter<char32_t*>::operator()(std::u32string_view par, char32_t const* Input) { return std::u32string(Input); }
	std::u32string Formatter<std::u32string>::operator()(std::u32string_view par, std::u32string Input) { return std::move(Input); }
	std::u32string Formatter<float>::operator()(std::u32string_view par, float input) { 
		char Buffer[100];
		size_t t = sprintf_s(Buffer, 100, "%f", input);
		return CharEncode::Wrapper<char>(std::string_view{Buffer, t}).To<char32_t>();
	}
	std::u32string Formatter<double>::operator()(std::u32string_view par, double input) {
		char Buffer[100];
		size_t t = sprintf_s(Buffer, 100, "%lf", input);
		return CharEncode::Wrapper<char>(std::string_view{ Buffer, t }).To<char32_t>();
	}
	std::u32string Formatter<uint32_t>::operator()(std::u32string_view par, uint32_t input) {
		Paras state = ParasTranslate(par);
		char const* TarString = nullptr;
		if (state.IsHex)
			TarString = "%I32x";
		else
			TarString = "%I32u";
		char Buffer[100];
		size_t t = sprintf_s(Buffer, 100, TarString, input);
		return CharEncode::Wrapper<char>(std::string_view{ Buffer, t }).To<char32_t>();
	}
	std::u32string Formatter<int32_t>::operator()(std::u32string_view par, int32_t input) {
		//Paras state = ParasTranslate(par);
		char const* TarString = nullptr;
		TarString = "%I32d";
		char Buffer[100];
		size_t t = sprintf_s(Buffer, 100, TarString, input);
		return CharEncode::Wrapper<char>(std::string_view{ Buffer, t }).To<char32_t>();
	}
	std::u32string Formatter<uint64_t>::operator()(std::u32string_view par, uint64_t input) {
		Paras state = ParasTranslate(par);
		char const* TarString = nullptr;
		if (state.IsHex)
			TarString = "%I64x";
		else
			TarString = "%I64u";
		char Buffer[100];
		size_t t = sprintf_s(Buffer, 100, TarString, input);
		return CharEncode::Wrapper<char>(std::string_view{ Buffer, t }).To<char32_t>();
	}
	std::u32string Formatter<int64_t>::operator()(std::u32string_view par, int64_t input) {
		//Paras state = ParasTranslate(par);
		char const* TarString = nullptr;
		TarString = "%I64d";
		char Buffer[100];
		size_t t = sprintf_s(Buffer, 100, TarString, input);
		return CharEncode::Wrapper<char>(std::string_view{ Buffer, t }).To<char32_t>();
	}
}