module;

export module DumplingTextureEncoder;

import std;
import Potato;

export namespace Dumpling::TextureEncoder
{
	enum class TextureEncodeFormat
	{
		TGA,
		UNKNOW,
	};

	struct TextureInfo
	{
		TextureEncodeFormat format = TextureEncodeFormat::UNKNOW;
		
		operator bool() const { return format != TextureEncodeFormat::UNKNOW; }
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
		
		TextureInfo DetectTextureInfo(std::span<std::byte const> file_binary, TextureEncodeFormat suggest_format = TextureEncodeFormat::UNKNOW);
		TextureInfo GetTGATextureInfo(std::span<std::byte const> file_binary);


	protected:

		virtual ~TexEncoder() = default;

		virtual void AddTexEncoderRef() const = 0;
		virtual void SubTexEncoderRef() const = 0;
	};

	inline TexEncoder::Ptr CreateTexEncoder(std::pmr::memory_resource* resource = std::pmr::get_default_resource()) { return TexEncoder::Create(resource); }
}
