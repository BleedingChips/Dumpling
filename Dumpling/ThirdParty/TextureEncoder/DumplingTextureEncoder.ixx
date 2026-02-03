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
		UNKNOW
	};

	struct TextureInfo
	{
		std::size_t width = 0;
		std::size_t height = 0;
		
		operator bool() const { return width > 0 && height > 0; }
	};

	struct TextureBitMap
	{
		TextureBitMap() = default;
		~TextureBitMap();
		TextureBitMap(TextureBitMap&& wrapper);
		TextureBitMap& operator=(TextureBitMap&& wrapper);
		operator bool() const;
		TextureInfo GetTextureInfo() const;
		void Clear();
	protected:
		TextureBitMap(void* bitmap, EncodeFormat original_format = EncodeFormat::UNKNOW);
		void* bitmap = nullptr;
		EncodeFormat original_format = EncodeFormat::UNKNOW;

		friend struct TexEncoder;
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
		EncodeFormat GetTextureFormat(std::span<std::byte const> texture_binary);
		TextureBitMap LoadToBitMapTexture(std::span<std::byte const> texture_binary);


	protected:

		virtual ~TexEncoder() = default;

		virtual void AddTexEncoderRef() const = 0;
		virtual void SubTexEncoderRef() const = 0;
	};

	inline TexEncoder::Ptr CreateTexEncoder(std::pmr::memory_resource* resource = std::pmr::get_default_resource()) { return TexEncoder::Create(resource); }
}
