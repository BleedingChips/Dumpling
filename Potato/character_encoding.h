#pragma once
#include <string>
#include <tuple>
#include <optional>
#include <assert.h>
namespace Potato::Encoding
{

#ifdef _WIN32
	size_t ansis_to_wchar_require_space(const char* input, size_t input_length, uint32_t code_page) noexcept;
	size_t ansis_to_wchar(const char* input, size_t input_length, wchar_t* output, size_t output_length, uint32_t code_page) noexcept;
	size_t wchar_to_ansis_require_space(const wchar_t* input, size_t input_length, uint32_t code_page) noexcept;
	size_t wchar_to_ansis(const wchar_t* input, size_t input_length, char* output, size_t output_length, uint32_t code_page) noexcept;
#endif

	namespace Implement {

#ifdef _WIN32
		using WCHAR_IMP_TYPE = char16_t;
#else
		using WCHAR_IMP_TYPE = char32_t;
#endif

		template<typename Type> struct char_imp {};
		template<> struct char_imp<char> {
			using type = char;
			static constexpr std::size_t max_space = 6;

			static std::size_t single_char_space(type first_char) noexcept;
			static std::optional<std::size_t> check_single_char(const type* input, std::size_t length) noexcept;
			static std::size_t to_single(const type* input, std::size_t length, char* output, std::size_t output_length) noexcept;
			static std::size_t to_single(const type* input, std::size_t length, char16_t* output, std::size_t output_length) noexcept;
			static std::size_t to_single(const type* input, std::size_t length, char32_t* output, std::size_t output_length) noexcept;
			static std::size_t to_single(const type* input, std::size_t length, wchar_t* output, std::size_t output_length) noexcept { return to_single(input, length, reinterpret_cast<WCHAR_IMP_TYPE*>(output), output_length); }
		};
		template<> struct char_imp<char16_t> { 
			using type = char16_t;
			static constexpr std::size_t max_space = 2;

			static std::size_t single_char_space(type first_char) noexcept;
			static std::optional<std::size_t> check_single_char(const type* input, std::size_t length) noexcept;
			static std::size_t to_single(const type* input, std::size_t length, char* output, std::size_t output_length) noexcept;
			static std::size_t to_single(const type* input, std::size_t length, char16_t* output, std::size_t output_length) noexcept;
			static std::size_t to_single(const type* input, std::size_t length, char32_t* output, std::size_t output_length) noexcept;
			static std::size_t to_single(const type* input, std::size_t length, wchar_t* output, std::size_t output_length) noexcept { return to_single(input, length, reinterpret_cast<WCHAR_IMP_TYPE*>(output), output_length); }
		};
		template<> struct char_imp<char32_t> { 
			using type = char32_t;
			static constexpr std::size_t max_space = 1;

			static std::size_t single_char_space(type first_char) noexcept { return 1; }
			static std::optional<std::size_t> check_single_char(const type* input, std::size_t length) noexcept;
			static std::size_t to_single(const type* input, std::size_t length, char* output, std::size_t output_length) noexcept;
			static std::size_t to_single(const type* input, std::size_t length, char16_t* output, std::size_t output_length) noexcept;
			static std::size_t to_single(const type* input, std::size_t length, char32_t* output, std::size_t output_length) noexcept;
			static std::size_t to_single(const type* input, std::size_t length, wchar_t* output, std::size_t output_length) noexcept { return to_single(input, length, reinterpret_cast<WCHAR_IMP_TYPE*>(output), output_length); }
		};
		template<> struct char_imp<wchar_t> { 
			using type = wchar_t;
			static constexpr std::size_t max_space = char_imp<WCHAR_IMP_TYPE>::max_space; 

			static std::size_t single_char_space(type first_char) noexcept { return char_imp<WCHAR_IMP_TYPE>::single_char_space(static_cast<WCHAR_IMP_TYPE>(first_char)); }
			static std::optional<std::size_t> check_single_char(const type* input, std::size_t length) noexcept { return char_imp<WCHAR_IMP_TYPE>::check_single_char(reinterpret_cast<const WCHAR_IMP_TYPE*>(input), length); }
			static std::size_t to_single(const type* input, std::size_t length, char* output, std::size_t output_length) noexcept {  return char_imp<WCHAR_IMP_TYPE>::to_single(reinterpret_cast<const WCHAR_IMP_TYPE*>(input), length, output, output_length); }
			static std::size_t to_single(const type* input, std::size_t length, char16_t* output, std::size_t output_length) noexcept { return char_imp<WCHAR_IMP_TYPE>::to_single(reinterpret_cast<const WCHAR_IMP_TYPE*>(input), length, output, output_length); }
			static std::size_t to_single(const type* input, std::size_t length, char32_t* output, std::size_t output_length) noexcept { return char_imp<WCHAR_IMP_TYPE>::to_single(reinterpret_cast<const WCHAR_IMP_TYPE*>(input), length, output, output_length); }
			static std::size_t to_single(const type* input, std::size_t length, wchar_t* output, std::size_t output_length) noexcept { return char_imp<WCHAR_IMP_TYPE>::to_single(reinterpret_cast<const WCHAR_IMP_TYPE*>(input), length, output, output_length); }
		};
	}

	template<typename Type> struct string_encoding
	{
		using type = Type;
		static constexpr std::size_t max_space = Implement::char_imp<Type>::max_space;
		string_encoding(const Type* input, std::size_t input_length) noexcept : input(input), length(input_length), bad_string(false) {}
		operator bool() noexcept { return !bad_string && input != nullptr && length != 0; }
		bool is_bad_string() const noexcept { return bad_string; }
		template<typename OutputType>
		std::tuple<std::size_t, std::size_t> to_string(OutputType* output, std::size_t output_length) noexcept 
		{
			if (input != nullptr)
			{
				const char* cur = input;
				std::size_t s_used = 0;
				std::size_t t_used = 0;
				std::size_t cur_length = length;
				while (!bad_string && cur_length != 0)
				{
					auto P = Implement::char_imp<Type>::check_single_char(cur, cur_length);
					if (P)
					{
						if (*P != 0)
						{
							auto result = Implement::char_imp<Type>::to_single(cur, *P, output, output_length);
							if (result == 0)
								return { s_used, t_used };
							else {
								cur += *P;
								assert(cur_length >= *P);
								cur_length -= *P;
								s_used += *P;
								t_used += result;
								output += result;
								assert(output_length >= result);
								output_length -= result;
							}
						}
						else
							return { s_used, t_used };
					}
					else {
						bad_string = true;
						return { s_used, t_used };
					}
				}
			}
			return { 0, 0 };
		}
		template<typename OutputType>
		std::basic_string<OutputType> to_string() noexcept
		{
			OutputType buffer[Implement::char_imp<OutputType>::max_space];
			std::basic_string<OutputType> result;
			while (*this)
			{
				auto [u_size, t_size] = to_string(buffer, Implement::char_imp<OutputType>::max_space);
				if (t_size != 0)
				{
					result.insert(result.end(), buffer, buffer + t_size);
				}
				else
					break;
				this->shift(u_size);
			}
			return std::move(result);
		}
		void shift(std::size_t len) {
			assert(length >= len);
			length -= len;
			input += len;
		}
	private:
		bool bad_string = false;
		const Type* input;
		std::size_t length;
	};

}

namespace Potato::Encoding
{

	enum class BomType
	{
		None,
		UTF8,
		UTF16LE,
		UTF16BE,
		UTF32LE,
		UTF32BE
	};

	static constexpr size_t bom_length = 4;

	// bom type, bom length
	std::tuple<BomType, size_t> translate_binary_to_bomtype(const std::byte* bom, size_t bom_length) noexcept;

	std::tuple<const std::byte*, size_t> translate_bomtype_to_binary(BomType format) noexcept;
}