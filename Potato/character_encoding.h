#pragma once
#include <string>
#include <tuple>
#include <optional>
#include <assert.h>
#include <string_view>

#ifdef _WIN32
#include <Windows.h>
namespace Potato::Encoding
{
	std::size_t EncodingAnsiToWcharRequireSpace(char const* input, std::size_t length, uint32_t code_page = CP_ACP) noexcept;
	std::size_t EncodingAnsiToWchar(char const* input, std::size_t length, wchar_t* output, std::size_t OutputLength, uint32_t code_page = CP_ACP) noexcept;
	std::wstring EncodingAnsiToWchar(char const* input, std::size_t length, uint32_t code_page = CP_ACP) noexcept;

	std::size_t EncodingWcharToAnsiRequireSpace(wchar_t const*  input, std::size_t length, uint32_t code_page = CP_ACP) noexcept;
	std::size_t EncodingWcharToAnsi(wchar_t const* input, std::size_t length, char* output, std::size_t OutputLength, uint32_t code_page = CP_ACP) noexcept;
	std::string EncodingWcharToAnsi(wchar_t const* input, std::size_t length, uint32_t code_page = CP_ACP) noexcept;
}
#endif

#undef max
namespace Potato::Encoding
{

#ifdef _WIN32
	using WCHAR_IMP_TYPE = char16_t;
#else
	using WCHAR_IMP_TYPE = char32_t;
#endif


	struct EncodeResult
	{
		std::size_t UsedChar = 0;
		std::size_t InputUsedSpace = 0;
		std::size_t OutputUsedSpace = 0;
		EncodeResult& Add(std::size_t InputUsed, std::size_t OutputUsed) {
			UsedChar += 1;
			InputUsedSpace += InputUsed;
			OutputUsedSpace += OutputUsed;
			return *this;
		}
	};

	template<typename From, typename To> struct Encoding;
	template<> struct Encoding<char, char> {
		using From = char;
		using To = char;

		static EncodeResult RequireOutputSpace(From const* Input, std::size_t InputSpace, std::size_t MaxOutputSpace, std::size_t MaxCharLimited);
		static std::size_t EncodingNoCheck(From const* input, std::size_t InputSpace, To* Output);
	};

	template<> struct Encoding<char, char16_t> {
		using From = char;
		using To = char16_t;

		static EncodeResult RequireOutputSpace(From const* Input, std::size_t InputSpace, std::size_t MaxOutputSpace, std::size_t MaxCharLimited);
		static std::size_t EncodingNoCheck(From const* input, std::size_t InputSpace, To* Output);
	};

	template<> struct Encoding<char, char32_t> {
		using From = char;
		using To = char32_t;

		static EncodeResult RequireOutputSpace(From const* Input, std::size_t InputSpace, std::size_t MaxOutputSpace, std::size_t MaxCharLimited);
		static std::size_t EncodingNoCheck(From const* input, std::size_t InputSpace, To* Output);
	};

	template<> struct Encoding<char, wchar_t> {
		using From = char;
		using To = wchar_t;

		static EncodeResult RequireOutputSpace(From const* Input, std::size_t InputSpace, std::size_t MaxOutputSpace, std::size_t MaxCharLimited) { return Encoding<char, WCHAR_IMP_TYPE>::RequireOutputSpace(Input, InputSpace, MaxOutputSpace, MaxCharLimited); }
		static std::size_t EncodingNoCheck(From const* input, std::size_t InputSpace, To* Output) { Encoding<char, WCHAR_IMP_TYPE>::EncodingNoCheck(input, InputSpace, reinterpret_cast<WCHAR_IMP_TYPE*>(Output)); }
	};

	template<> struct Encoding<char16_t, char> {
		using From = char16_t;
		using To = char;

		static EncodeResult RequireOutputSpace(From const* Input, std::size_t InputSpace, std::size_t MaxOutputSpace, std::size_t MaxCharLimited);
		static std::size_t EncodingNoCheck(From const* input, std::size_t InputSpace, To* Output);
	};

	template<> struct Encoding<char16_t, char16_t> {
		using From = char16_t;
		using To = char16_t;

		static EncodeResult RequireOutputSpace(From const* Input, std::size_t InputSpace, std::size_t MaxOutputSpace, std::size_t MaxCharLimited);
		static std::size_t EncodingNoCheck(From const* input, std::size_t InputSpace, To* Output);
	};

	template<> struct Encoding<char16_t, char32_t> {
		using From = char16_t;
		using To = char32_t;

		static EncodeResult RequireOutputSpace(From const* Input, std::size_t InputSpace, std::size_t MaxOutputSpace, std::size_t MaxCharLimited);
		static std::size_t EncodingNoCheck(From const* input, std::size_t InputSpace, To* Output);
	};

	template<> struct Encoding<char16_t, wchar_t> {
		using From = char16_t;
		using To = wchar_t;

		static EncodeResult RequireOutputSpace(From const* Input, std::size_t InputSpace, std::size_t MaxOutputSpace, std::size_t MaxCharLimited) { return Encoding<char16_t, WCHAR_IMP_TYPE>::RequireOutputSpace(Input, InputSpace, MaxOutputSpace, MaxCharLimited); }
		static std::size_t EncodingNoCheck(From const* input, std::size_t InputSpace, To* Output) { Encoding<char16_t, WCHAR_IMP_TYPE>::EncodingNoCheck(input, InputSpace, reinterpret_cast<WCHAR_IMP_TYPE*>(Output)); }
	};

	template<> struct Encoding<char32_t, char> {
		using From = char32_t;
		using To = char;

		static EncodeResult RequireOutputSpace(From const* Input, std::size_t InputSpace, std::size_t MaxOutputSpace, std::size_t MaxCharLimited);
		static std::size_t EncodingNoCheck(From const* input, std::size_t InputSpace, To* Output);
	};

	template<> struct Encoding<char32_t, char16_t> {
		using From = char32_t;
		using To = char16_t;

		static EncodeResult RequireOutputSpace(From const* Input, std::size_t InputSpace, std::size_t MaxOutputSpace, std::size_t MaxCharLimited);
		static std::size_t EncodingNoCheck(From const* input, std::size_t InputSpace, To* Output);
	};

	template<> struct Encoding<char32_t, char32_t> {
		using From = char32_t;
		using To = char32_t;

		static EncodeResult RequireOutputSpace(From const* Input, std::size_t InputSpace, std::size_t MaxOutputSpace, std::size_t MaxCharLimited);
		static std::size_t EncodingNoCheck(From const* input, std::size_t InputSpace, To* Output);
	};

	template<> struct Encoding<char32_t, wchar_t> {
		using From = char32_t;
		using To = wchar_t;

		static EncodeResult RequireOutputSpace(From const* Input, std::size_t InputSpace, std::size_t MaxOutputSpace, std::size_t MaxCharLimited) {
			return Encoding<char32_t, WCHAR_IMP_TYPE>::RequireOutputSpace(Input, InputSpace, MaxOutputSpace, MaxCharLimited);
		}
		static std::size_t EncodingNoCheck(From const* input, std::size_t InputSpace, To* Output) {
			return Encoding<char32_t, WCHAR_IMP_TYPE>::EncodingNoCheck(input, InputSpace, reinterpret_cast<WCHAR_IMP_TYPE*>(Output));
		}
	};

	template<> struct Encoding<wchar_t, char> {
		using From = wchar_t;
		using To = char;

		static EncodeResult RequireOutputSpace(From const* Input, std::size_t InputSpace, std::size_t MaxOutputSpace, std::size_t MaxCharLimited) {
			return Encoding<WCHAR_IMP_TYPE, To>::RequireOutputSpace(reinterpret_cast<WCHAR_IMP_TYPE const *>(Input), InputSpace, MaxOutputSpace, MaxCharLimited);
		}
		static std::size_t EncodingNoCheck(From const* input, std::size_t InputSpace, To* Output) {
			return Encoding<WCHAR_IMP_TYPE, To>::EncodingNoCheck(reinterpret_cast<WCHAR_IMP_TYPE const*>(input), InputSpace, Output);
		}
	};

	template<> struct Encoding<wchar_t, char16_t> {
		using From = wchar_t;
		using To = char16_t;

		static EncodeResult RequireOutputSpace(From const* Input, std::size_t InputSpace, std::size_t MaxOutputSpace, std::size_t MaxCharLimited) {
			return Encoding<WCHAR_IMP_TYPE, To>::RequireOutputSpace(reinterpret_cast<WCHAR_IMP_TYPE const*>(Input), InputSpace, MaxOutputSpace, MaxCharLimited);
		}
		static EncodeResult RequireOutputSpace(From const* Input, std::size_t InputSpace) noexcept { return RequireOutputSpace(Input, InputSpace, std::numeric_limits<std::size_t>::max(), std::numeric_limits<std::size_t>::max()); }
		static EncodeResult RequireOutputSpaceMaxOutput(From const* Input, std::size_t InputSpace, std::size_t MaxOutputSpace) { return RequireOutputSpace(Input, InputSpace, MaxOutputSpace, std::numeric_limits<std::size_t>::max()); }
		static EncodeResult RequireOutputSpaceMaxRequire(From const* Input, std::size_t InputSpace, std::size_t MaxRequire) { return RequireOutputSpace(Input, InputSpace, std::numeric_limits<std::size_t>::max(), MaxRequire); }

		static std::size_t EncodingNoCheck(From const* input, std::size_t InputSpace, To* Output) {
			return Encoding<WCHAR_IMP_TYPE, To>::EncodingNoCheck(reinterpret_cast<WCHAR_IMP_TYPE const*>(input), InputSpace, Output);
		}
	};

	template<> struct Encoding<wchar_t, char32_t> {
		using From = wchar_t;
		using To = char32_t;

		static EncodeResult RequireOutputSpace(From const* Input, std::size_t InputSpace, std::size_t MaxOutputSpace, std::size_t MaxCharLimited) {
			return Encoding<WCHAR_IMP_TYPE, To>::RequireOutputSpace(reinterpret_cast<WCHAR_IMP_TYPE const*>(Input), InputSpace, MaxOutputSpace, MaxCharLimited);
		}
		static std::size_t EncodingNoCheck(From const* input, std::size_t InputSpace, To* Output) {
			return Encoding<WCHAR_IMP_TYPE, To>::EncodingNoCheck(reinterpret_cast<WCHAR_IMP_TYPE const*>(input), InputSpace, Output);
		}
	};

	template<> struct Encoding<wchar_t, wchar_t> {
		using From = wchar_t;
		using To = wchar_t;

		static EncodeResult RequireOutputSpace(From const* Input, std::size_t InputSpace, std::size_t MaxOutputSpace, std::size_t MaxCharLimited) {
			return Encoding<WCHAR_IMP_TYPE, To>::RequireOutputSpace(reinterpret_cast<WCHAR_IMP_TYPE const*>(Input), InputSpace, MaxOutputSpace, MaxCharLimited);
		}
		static std::size_t EncodingNoCheck(From const* input, std::size_t InputSpace, To* Output) {
			return Encoding<WCHAR_IMP_TYPE, To>::EncodingNoCheck(reinterpret_cast<WCHAR_IMP_TYPE const*>(input), InputSpace, Output);
		}
	};

	template<typename Type> struct EncodingWrapper
	{
		EncodingWrapper(Type const* Input, size_t Length) : mStorage(Input, Length) {}
		EncodingWrapper(std::basic_string_view<Type> Input) : mStorage(Input) {}
		template<typename ToType> std::basic_string<ToType> To() {
			EncodeResult Re = Encoding<Type, ToType>::RequireOutputSpace(mStorage.data(), mStorage.size(), std::numeric_limits<size_t>::max(), std::numeric_limits<size_t>::max());
			std::basic_string<ToType> Result;
			Result.resize(Re.OutputUsedSpace);
			Encoding<Type, ToType>::EncodingNoCheck(mStorage.data(), mStorage.size(), Result.data());
			return Result;
		}
	private:
		std::basic_string_view<Type> mStorage;
	};

	enum class BomType
	{
		None,
		UTF8,
		UTF16LE,
		UTF16BE,
		UTF32LE,
		UTF32BE
	};

	enum Ending {
		Less,
		Big
	};

	Ending DetectEnding();

	static constexpr std::size_t bom_length = 4;

	// bom type, bom length
	std::tuple<BomType, std::size_t> BinaryToBom(const std::byte* bom, std::size_t bom_length) noexcept;
	std::tuple<const std::byte*, std::size_t> BomToBinary(BomType format) noexcept;
	std::tuple<BomType, std::byte*, size_t> FixBinaryWithBom(std::byte* Bom, std::size_t BomLength) noexcept;
	void FixBinary(std::byte* Bom, std::size_t BomLength, BomType Type) noexcept;
}

