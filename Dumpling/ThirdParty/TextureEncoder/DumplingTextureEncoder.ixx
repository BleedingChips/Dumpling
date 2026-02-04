module;

export module DumplingTextureEncoder;

import std;
import Potato;

export namespace Dumpling::TextureEncoder
{
	enum class EncodeFormat
	{
		TARGA,
		JPEG,
		BMP,
		HDR,
		DDS,
		PNG,
		UNKNOW
	};

	struct TextureInfo
	{
		std::size_t width = 0;
		std::size_t height = 0;
		std::size_t pixel_size = 0;
		std::size_t total_size = 0;
		
		operator bool() const { return width > 0 && height > 0; }
	};

	struct TextureBitMap;

	struct TextureWrapper
	{
		TextureWrapper() = default;
		~TextureWrapper();
		TextureWrapper(TextureWrapper&& wrapper);
		TextureWrapper& operator=(TextureWrapper&& wrapper);
		operator bool() const;
		void Reset();
		std::span<std::byte const> GetByteData() const;
		TextureBitMap CoverToBitMap();
	protected:
		TextureWrapper(void* wrapper, EncodeFormat encode_format) : wrapper(wrapper), encode_format(encode_format) {}
		void* wrapper = nullptr;
		EncodeFormat encode_format = EncodeFormat::UNKNOW;

		friend struct TexEncoder;
		friend struct TextureBitMap;
	};

	struct TextureBitMap
	{
		TextureBitMap() = default;
		~TextureBitMap();
		TextureBitMap(TextureBitMap&& wrapper);
		TextureBitMap& operator=(TextureBitMap&& wrapper);
		operator bool() const;
		TextureInfo GetTextureInfo() const;
		void Reset();
		std::span<std::byte const> GetByte() const;
		TextureWrapper CoverToEncodedTexture(EncodeFormat target_format);
	protected:
		TextureBitMap(void* bitmap, EncodeFormat original_format = EncodeFormat::UNKNOW) : bitmap(bitmap), original_format(original_format) {}
		void* bitmap = nullptr;
		EncodeFormat original_format = EncodeFormat::UNKNOW;

		friend struct TexEncoder;
		friend struct TextureWrapper;
	};

	struct TexEncoder
	{
		struct Wrapper
		{
			void AddRef(TexEncoder const* ptr) { ptr->AddTexEncoderRef(); }
			void SubRef(TexEncoder const* ptr) { ptr->SubTexEncoderRef(); }
		};


		using Ptr = Potato::Pointer::IntrusivePtr<TexEncoder, Wrapper>;

		static Ptr Create(std::pmr::memory_resource* resource = std::pmr::get_default_resource());
		TextureWrapper CreateWrapper(std::span<std::byte const> encoded_texture);

	protected:

		virtual ~TexEncoder() = default;

		virtual void AddTexEncoderRef() const = 0;
		virtual void SubTexEncoderRef() const = 0;
	};

	inline TexEncoder::Ptr CreateTexEncoder(std::pmr::memory_resource* resource = std::pmr::get_default_resource()) { return TexEncoder::Create(resource); }
}
