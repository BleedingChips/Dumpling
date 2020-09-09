#include "../Interface/CharEncode.h"
#include <assert.h>

#ifdef _WIN32
namespace PineApple::CharEncode
{

	std::size_t EncodingAnsiToWcharRequireSpace(char const* input, std::size_t input_length, uint32_t code_page) noexcept
	{
		return MultiByteToWideChar(code_page, 0, input, static_cast<int>(input_length), nullptr, 0);
	}

	std::size_t EncodingAnsiToWchar(char const* input, std::size_t input_length, wchar_t* output, std::size_t output_length, uint32_t code_page) noexcept
	{
		if (output_length != 0)
		{
			return MultiByteToWideChar(code_page, 0, input, static_cast<int>(input_length), reinterpret_cast<wchar_t*>(output), static_cast<int>(output_length));
		}
		return 0;
	}

	std::wstring EncodingAnsiToWchar(char* const input, std::size_t length, uint32_t code_page) noexcept
	{
		std::size_t Relength = EncodingAnsiToWcharRequireSpace(input, length, code_page);
		std::wstring Result;
		Result.resize(Relength);
		EncodingAnsiToWchar(input, length, Result.data(), Result.size(), code_page);
		return Result;
	}

	std::size_t EncodingWcharToAnsiRequireSpace(const wchar_t* input, std::size_t input_length, uint32_t code_page) noexcept
	{
		BOOL unchangleble = true;
		return WideCharToMultiByte(code_page, 0, reinterpret_cast<const wchar_t*>(input), static_cast<int>(input_length), nullptr, 0, "?", &unchangleble);
	}

	std::size_t EncodingWcharToAnsi(const wchar_t* input, std::size_t input_length, char* output, std::size_t output_length, uint32_t code_page) noexcept
	{
		if (output_length != 0)
		{
			BOOL unchangleble = true;
			return WideCharToMultiByte(code_page, 0, reinterpret_cast<const wchar_t*>(input), static_cast<int>(input_length), output, static_cast<int>(output_length), "?", &unchangleble);
		}
		return 0;
	}

	std::string EncodingWcharToAnsi(wchar_t* const input, std::size_t length, uint32_t code_page) noexcept
	{
		std::size_t Relength = EncodingWcharToAnsiRequireSpace(input, length, code_page);
		std::string Result;
		Result.resize(Relength);
		EncodingWcharToAnsi(input, length, Result.data(), Result.size(), code_page);
		return Result;
	}
}
#endif


namespace PineApple::CharEncode
{

	std::size_t UTF8RequireSpace(char first_char)
	{
		if ((first_char & 0xFE) == 0xFC)
			return 6;
		else if ((first_char & 0xFC) == 0xF8)
			return 5;
		else if ((first_char & 0xF8) == 0xF0)
			return 4;
		else if ((first_char & 0xF0) == 0xE0)
			return 3;
		else if ((first_char & 0xE0) == 0xC0)
			return 2;
		else if ((first_char & 0x80) == 0)
			return 1;
		else
			return 0;
	}

	std::size_t CheckUTF8String(char const* Input, std::size_t InputLength)
	{
		assert(Input != nullptr && InputLength != 0);
		auto Size = UTF8RequireSpace(Input[0]);
		if (InputLength >= Size)
		{
			for (std::size_t i = 1; i < Size; ++i)
			{
				if ((Input[i] & 0xC0) != 0x80)
					return 0;
			}
			return Size;
		}
		return 0;
	}

	Result Encoding<char, char>::RequireOutputSpace(From const* Input, std::size_t InputSpace, std::size_t MaxOutputSpace, std::size_t MaxCharLimited)
	{
		assert(Input != nullptr);
		Result Result;
		while (InputSpace > Result.InputUsedSpace && MaxOutputSpace > Result.OutputUsedSpace && Result.UsedChar < MaxCharLimited)
		{
			auto Size = CheckUTF8String(Input + Result.InputUsedSpace, InputSpace - Result.InputUsedSpace);
			if(Size != 0 && MaxOutputSpace >= Result.OutputUsedSpace + Size)
				Result.Add(Size, Size);
			else
				break;
		}
		return Result;
	}

	std::size_t Encoding<char, char>::EncodingNoCheck(From const* input, std::size_t InputSpace, To* Output)
	{
		std::memcpy(Output, input, sizeof(char) * InputSpace);
		return InputSpace;
	}

	Result Encoding<char, char16_t>::RequireOutputSpace(From const* Input, std::size_t InputSpace, std::size_t MaxOutputSpace, std::size_t MaxCharLimited)
	{
		assert(Input != nullptr);
		Result Result;
		while (InputSpace > Result.InputUsedSpace && MaxOutputSpace > Result.OutputUsedSpace && Result.UsedChar < MaxCharLimited)
		{
			auto Size = CheckUTF8String(Input + Result.InputUsedSpace, InputSpace - Result.InputUsedSpace);
			if (Size != 0)
			{
				auto OutputSize = (Size <= 3) ? 1 : 2;
				if (MaxOutputSpace >= Result.OutputUsedSpace + Size)
					Result.Add(Size, OutputSize);
			}
			else
				break;
		}
		return Result;
	}


	std::size_t Encoding<char, char16_t>::EncodingNoCheck(From const* Input, std::size_t InputSpace, To* Output)
	{
		std::size_t Used = 0;
		while (InputSpace != 0)
		{
			char32_t Buffer;
			auto Size = UTF8RequireSpace(Input[0]);
			Encoding<char, char32_t>::EncodingNoCheck(Input, Size, &Buffer);
			Used += Encoding<char32_t, char16_t>::EncodingNoCheck(&Buffer, 1, Output + Used);
			Input += Size;
			InputSpace -= Size;
		}
		return Used;
	}

	Result Encoding<char, char32_t>::RequireOutputSpace(From const* Input, std::size_t InputSpace, std::size_t MaxOutputSpace, std::size_t MaxCharLimited)
	{
		assert(Input != nullptr);
		Result Result;
		while (InputSpace > Result.InputUsedSpace && MaxOutputSpace > Result.OutputUsedSpace && Result.UsedChar < MaxCharLimited)
		{
			auto Size = CheckUTF8String(Input + Result.InputUsedSpace, InputSpace - Result.InputUsedSpace);
			if (Size != 0 && MaxOutputSpace >= Result.OutputUsedSpace + 1)
				Result.Add(Size, 1);
			else
				break;
		}
		return Result;
	}


	std::size_t Encoding<char, char32_t>::EncodingNoCheck(From const* Input, std::size_t InputSpace, To* Output)
	{
		while (InputSpace != 0)
		{
			auto Size = UTF8RequireSpace(Input[0]);
			switch (Size)
			{
			case 1: Output[0] = Input[0]; break;
			case 2: Output[0] = ((Input[0] & 0x1F) << 6) | (Input[1] & 0x3F); break;
			case 3: Output[0] = ((Input[0] & 0x0F) << 12) | ((Input[1] & 0x3F) << 6) | (Input[2] & 0x3F); break;
			case 4: Output[0] = ((Input[0] & 0x07) << 18) | ((Input[1] & 0x3F) << 12) | ((Input[2] & 0x3F) << 6) | (Input[3] & 0x3F); break;
			case 5: Output[0] = ((Input[0] & 0x03) << 24) | ((Input[1] & 0x3F) << 18) | ((Input[2] & 0x3F) << 12) | ((Input[3] & 0x3F) << 6) | (Input[4] & 0x3F); break;
			case 6: Output[0] = ((Input[0] & 0x01) << 30) | ((Input[1] & 0x3F) << 24) | ((Input[2] & 0x3F) << 18) | ((Input[3] & 0x3F) << 12) | ((Input[4] & 0x3F) << 6) | (Input[5] & 0x3F); break;
			default: assert(false);
			}
			Input += Size;
			InputSpace -= Size;
			Output += 1;
		}
		return 1;
	}

	std::size_t CheckUTF16String(char16_t const* FirstChar, std::size_t Length)
	{
		assert(FirstChar != nullptr && Length != 0);
		if (((FirstChar[0] & 0xD800) == 0xD800) && Length >= 2 && (FirstChar[1] & 0xDC00) == 0xDC00)
			return 2;
		return 1;
	}

	Result Encoding<char16_t, char>::RequireOutputSpace(From const* Input, std::size_t InputSpace, std::size_t MaxOutputSpace, std::size_t MaxCharLimited)
	{
		assert(Input != nullptr);
		Result Result;
		while (InputSpace > Result.InputUsedSpace && MaxOutputSpace > Result.OutputUsedSpace && Result.UsedChar < MaxCharLimited)
		{
			auto Size = CheckUTF16String(Input + Result.InputUsedSpace, InputSpace - Result.InputUsedSpace);
			std::size_t RequireSize = 0;
			switch (Size)
			{
			case 2: RequireSize = 4; break;
			case 1:
				if (Input[0] <= 0x7F) RequireSize = 1;
				else if (Input[0] <= 0x7FF) RequireSize = 2;
				else if (Input[0] <= 0xFFFF) RequireSize = 3;
				else assert(false);
				break;
			}
			if (MaxOutputSpace >= Result.OutputUsedSpace + RequireSize)
				Result.Add(Size, RequireSize);
			else
				break;
		}
		return Result;
	}


	std::size_t Encoding<char16_t, char>::EncodingNoCheck(From const* Input, std::size_t InputSpace, To* Output)
	{
		std::size_t Used = 0;
		while (InputSpace != 0)
		{
			char32_t Buffer;
			auto Size = CheckUTF16String(Input, InputSpace);
			Encoding<char16_t, char32_t>::EncodingNoCheck(Input, Size, &Buffer);
			Used += Encoding<char32_t, char>::EncodingNoCheck(&Buffer, 1, Output + Used);
			Input += Size;
			InputSpace -= Size;
		}
		return Used;
	}

	Result Encoding<char16_t, char16_t>::RequireOutputSpace(From const* Input, std::size_t InputSpace, std::size_t MaxOutputSpace, std::size_t MaxCharLimited)
	{
		assert(Input != nullptr);
		Result Result;
		while (InputSpace > Result.InputUsedSpace && MaxOutputSpace > Result.OutputUsedSpace && Result.UsedChar < MaxCharLimited)
		{
			auto Size = CheckUTF16String(Input + Result.InputUsedSpace, InputSpace - Result.InputUsedSpace);
			if (MaxOutputSpace >= Result.OutputUsedSpace + Size)
				Result.Add(Size, Size);
			else
				break;
		}
		return Result;
	}


	std::size_t Encoding<char16_t, char16_t>::EncodingNoCheck(From const* Input, std::size_t InputSpace, To* Output)
	{
		std::memcpy(Output, Input, sizeof(char16_t) * InputSpace);
		return InputSpace;
	}

	Result Encoding<char16_t, char32_t>::RequireOutputSpace(From const* Input, std::size_t InputSpace, std::size_t MaxOutputSpace, std::size_t MaxCharLimited)
	{
		assert(Input != nullptr);
		Result Result;
		while (InputSpace > Result.InputUsedSpace && MaxOutputSpace > Result.OutputUsedSpace && Result.UsedChar < MaxCharLimited)
		{
			auto Size = CheckUTF16String(Input + Result.InputUsedSpace, InputSpace - Result.InputUsedSpace);
			if (MaxOutputSpace >= Result.OutputUsedSpace + 1)
				Result.Add(Size, 1);
			else
				break;
		}
		return Result;
	}


	std::size_t Encoding<char16_t, char32_t>::EncodingNoCheck(From const* Input, std::size_t InputSpace, To* Output)
	{
		std::size_t Used = 0;
		while (InputSpace > 0)
		{
			auto Size = CheckUTF16String(Input, InputSpace);
			switch (Size)
			{
			case 1:
				Output[0] = Input[0];
				break;
			case 2:
				Output[0] = ((Input[0] & 0x3FF) << 10) | ((Input[1] & 0x3FF));
				break;
			default: assert(false); break;
			}
			Input += Size;
			InputSpace -= Size;
			Output++;
			++Used;
		}
		return Used;
	}


	Result Encoding<char32_t, char>::RequireOutputSpace(From const* Input, std::size_t InputSpace, std::size_t MaxOutputSpace, std::size_t MaxCharLimited)
	{
		assert(Input != nullptr);
		Result Result;
		while (InputSpace > Result.InputUsedSpace && MaxOutputSpace > Result.OutputUsedSpace && Result.UsedChar < MaxCharLimited)
		{
			std::size_t RequireSize = 0;
			if (Input[0] <= 0x7F) RequireSize = 1;
			else if (Input[0] <= 0x7FF) RequireSize = 2;
			else if (Input[0] <= 0xFFFF) RequireSize = 3;
			else if (Input[0] <= 0x1FFFFF) RequireSize = 4;
			else assert(false);
			if (MaxOutputSpace >= Result.OutputUsedSpace + RequireSize)
				Result.Add(1, RequireSize);
			else
				break;
		}
		return Result;
	}


	std::size_t Encoding<char32_t, char>::EncodingNoCheck(From const* Input, std::size_t InputSpace, To* Output)
	{
		std::size_t Used = 0;
		while (InputSpace > 0)
		{
			if (Input[0] <= 0x7F)
			{
				Output[Used] = static_cast<char>(Input[0] & 0x0000007F);
				Used += 1;
			}
			else if (Input[0] <= 0x7FF)
			{
				Output[Used] = 0xC0 | static_cast<char>((Input[0] & 0x07C0) >> 6);
				Output[Used + 1] = 0x80 | static_cast<char>((Input[0] & 0x3F));
				Used += 2;
			}
			else if (Input[0] <= 0xFFFF)
			{
				Output[Used] = 0xE0 | static_cast<char>((Input[0] & 0xF000) >> 12);
				Output[Used+ 1] = 0x80 | static_cast<char>((Input[0] & 0xFC0) >> 6);
				Output[Used + 2] = 0x80 | static_cast<char>((Input[0] & 0x3F));
				Used += 3;
			}
			else if (Input[0] <= 0x1FFFFF)
			{
				Output[Used] = 0x1E | static_cast<char>((Input[0] & 0x1C0000) >> 18);
				Output[Used + 1] = 0x80 | static_cast<char>((Input[0] & 0x3F000) >> 12);
				Output[Used + 2] = 0x80 | static_cast<char>((Input[0] & 0xFC0) >> 6);
				Output[Used + 3] = 0x80 | static_cast<char>((Input[0] & 0x3F));
			}
			else
				assert(false);
			Input += 1;
			InputSpace--;
		}
		return Used;
	}

	Result Encoding<char32_t, char16_t>::RequireOutputSpace(From const* Input, std::size_t InputSpace, std::size_t MaxOutputSpace, std::size_t MaxCharLimited)
	{
		assert(Input != nullptr);
		Result Result;
		while (InputSpace > Result.InputUsedSpace && MaxOutputSpace > Result.OutputUsedSpace && Result.UsedChar < MaxCharLimited)
		{
			std::size_t RequireSize = 0;
			if (Input[0] <= 0xFFFF) RequireSize = 1;
			else RequireSize = 2;
			if (MaxOutputSpace >= Result.OutputUsedSpace + RequireSize)
				Result.Add(1, RequireSize);
			else
				break;
		}
		return Result;
	}

	std::size_t Encoding<char32_t, char16_t>::EncodingNoCheck(From const* Input, std::size_t InputSpace, To* Output)
	{
		std::size_t Used;
		while (InputSpace > 0)
		{
			if (Input[0] <= 0xFFFF)
			{
				Output[Used] = static_cast<char16_t>(Input[0]);
				Used += 1;
			}
			else if (Input[0] <= 0x7FF)
			{
				Output[Used] = 0xDB00 | static_cast<char16_t>((Input[0] & 0xFFC00) >> 10);
				Output[Used + 1] = 0xDC00 | static_cast<char16_t>((Input[0] & 0x3FF));
				Used += 2;
			}
			Input += 1;
			InputSpace--;
		}
		return Used;
	}


	Result Encoding<char32_t, char32_t>::RequireOutputSpace(From const* Input, std::size_t InputSpace, std::size_t MaxOutputSpace, std::size_t MaxCharLimited)
	{
		assert(Input != nullptr);
		std::size_t Min = InputSpace < MaxOutputSpace ? InputSpace : MaxOutputSpace;
		Min = Min < MaxCharLimited ? MaxCharLimited : MaxCharLimited;
		return { Min , Min , Min };
	}

	std::size_t Encoding<char32_t, char32_t>::EncodingNoCheck(From const* Input, std::size_t InputSpace, To* Output)
	{
		std::memcpy(Output, Input, sizeof(char32_t) * InputSpace);
		return InputSpace;
	}



	Ending DetectEnding() {
		constexpr uint16_t Detect = 0x0001;
		return (*reinterpret_cast<uint8_t const*>(&Detect) == 0x01) ? Ending::Less : Ending::Big;
	};

	const unsigned char utf8_bom[] = { 0xEF, 0xBB, 0xBF };
	const unsigned char utf16_le_bom[] = { 0xFF, 0xFE };
	const unsigned char utf16_be_bom[] = { 0xFE, 0xFF };
	const unsigned char utf32_le_bom[] = { 0x00, 0x00, 0xFE, 0xFF };
	const unsigned char utf32_be_bom[] = { 0xFF, 0xFe, 0x00, 0x00 };

	std::tuple<BomType, std::size_t> BinaryToBom(const std::byte* bom, std::size_t bom_length) noexcept
	{
		assert(bom_length >= 4);
		if (std::memcmp(bom, utf8_bom, 3) == 0)
			return { BomType::UTF8, 3 };
		else if (std::memcmp(bom, utf32_le_bom, 4) == 0)
			return { BomType::UTF32LE, 4 };
		else if (std::memcmp(bom, utf32_be_bom, 4) == 0)
			return { BomType::UTF32BE, 4 };
		else if (std::memcmp(bom, utf16_le_bom, 2) == 0)
			return { BomType::UTF16LE, 2 };
		else if (std::memcmp(bom, utf16_be_bom, 2) == 0)
			return { BomType::UTF16BE, 2 };
		else
			return { BomType::None, 0 };
	}

	std::tuple<const std::byte*, std::size_t> BomToBinary(BomType format) noexcept
	{
		switch (format)
		{
		case BomType::UTF8: return { reinterpret_cast<const std::byte*>(utf8_bom), 3 };
		case BomType::UTF16LE: return { reinterpret_cast<const std::byte*>(utf16_le_bom), 2 };
		case BomType::UTF16BE: return { reinterpret_cast<const std::byte*>(utf16_be_bom), 2 };
		case BomType::UTF32LE: return { reinterpret_cast<const std::byte*>(utf32_le_bom), 4 };
		case BomType::UTF32BE: return { reinterpret_cast<const std::byte*>(utf32_be_bom), 4 };
		default: return { nullptr, 0 };
		}
	}

	std::tuple<BomType, std::byte*, size_t> FixBinaryWithBom(std::byte* Bom, std::size_t BomLength) noexcept
	{
		auto [Type, size] = BinaryToBom(Bom, BomLength);
		BomLength -= size;
		Bom += size;
		FixBinary(Bom, BomLength, Type);
		return { Type, Bom, BomLength };
	}
	void FixBinary(std::byte* Bom, std::size_t BomLength, BomType Type) noexcept
	{
		if (Type == BomType::UTF16LE && DetectEnding() != Ending::Less || Type == BomType::UTF16BE && DetectEnding() != Ending::Big)
		{
			for (size_t i = 0; i + 4 < BomLength; i += 4)
			{
				std::swap(Bom[i], Bom[i + 3]);
				std::swap(Bom[i + 1], Bom[i + 2]);
			}
		}
		else if (Type == BomType::UTF32LE && DetectEnding() != Ending::Less || Type == BomType::UTF32BE && DetectEnding() != Ending::Big)
		{
			for (size_t i = 0; i + 8 < BomLength; i += 8)
			{
				for(size_t k =0; k < 4; ++k)
					std::swap(Bom[i + k], Bom[i + 7 - k]);
			}
		}
	}

	namespace Implement
	{
		std::u32string Imp<char32_t>::operator()(std::byte* datas, size_t length)
		{
			auto [t, b, s] = FixBinaryWithBom(datas, length);
			switch (t)
			{
			case BomType::None:
			case BomType::UTF8:
				return Wrapper(reinterpret_cast<char const*>(b), s).To<char32_t>();
			case BomType::UTF16BE:
			case BomType::UTF16LE:
				return Wrapper(reinterpret_cast<char16_t const*>(b), s).To<char32_t>();
			case BomType::UTF32BE:
			case BomType::UTF32LE:
				return std::u32string(reinterpret_cast<char32_t const*>(b), s);
			default: assert(false);
				return {};
			}
		}

		std::string Imp<char>::operator()(std::byte* datas, size_t length)
		{
			auto [t, b, s] = FixBinaryWithBom(datas, length);
			switch (t)
			{
			case BomType::None:
			case BomType::UTF8:
				return std::string(reinterpret_cast<char const*>(b), s);
			case BomType::UTF16BE:
			case BomType::UTF16LE:
				return Wrapper(reinterpret_cast<char16_t const*>(b), s).To<char>();
			case BomType::UTF32BE:
			case BomType::UTF32LE:
				return Wrapper(reinterpret_cast<char32_t const*>(b), s).To<char>();
			default: assert(false);
				return {};
			}
		}
	}
}