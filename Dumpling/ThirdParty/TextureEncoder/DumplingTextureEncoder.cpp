module;
#include <cassert>
#include "FreeImage.h"


module DumplingTextureEncoder;

namespace Dumpling::TextureEncoder
{
	
	EncodeFormat Translate(FREE_IMAGE_FORMAT format)
	{
		switch (format)
		{
		case FREE_IMAGE_FORMAT::FIF_BMP:
			return EncodeFormat::BMP;
		case FREE_IMAGE_FORMAT::FIF_JPEG:
			return EncodeFormat::JPEG;
		case FREE_IMAGE_FORMAT::FIF_HDR:
			return EncodeFormat::HDR;
		case FREE_IMAGE_FORMAT::FIF_TARGA:
			return EncodeFormat::TARGA;
		case FREE_IMAGE_FORMAT::FIF_DDS:
			return EncodeFormat::DDS;
		case FREE_IMAGE_FORMAT::FIF_PNG:
			return EncodeFormat::PNG;
		default:
			return EncodeFormat::UNKNOW;
		}
	}

	FREE_IMAGE_FORMAT Translate(EncodeFormat format)
	{
		switch (format)
		{
		case EncodeFormat::BMP:
			return FREE_IMAGE_FORMAT::FIF_BMP;
		case EncodeFormat::JPEG:
			return FREE_IMAGE_FORMAT::FIF_JPEG;
		case EncodeFormat::HDR:
			return FREE_IMAGE_FORMAT::FIF_HDR;
		case EncodeFormat::TARGA:
			return FREE_IMAGE_FORMAT::FIF_TARGA;
		case EncodeFormat::DDS:
			return FREE_IMAGE_FORMAT::FIF_DDS;
		case EncodeFormat::PNG :
			return FREE_IMAGE_FORMAT::FIF_PNG;
		default:
			return FREE_IMAGE_FORMAT::FIF_UNKNOWN;
		}
	}

	TextureWrapper::TextureWrapper(TextureWrapper&& wrapper)
		: wrapper(wrapper.wrapper), encode_format(wrapper.encode_format)
	{
		wrapper.wrapper = nullptr;
		wrapper.encode_format = EncodeFormat::UNKNOW;
	}

	TextureWrapper::~TextureWrapper()
	{
		Reset();
	}

	void TextureWrapper::Reset()
	{
		if (wrapper != nullptr)
		{
			FreeImage_CloseMemory(reinterpret_cast<FIMEMORY*>(wrapper));
			wrapper = nullptr;
		}
		encode_format = EncodeFormat::UNKNOW;
	}
	
	TextureWrapper::operator bool() const
	{
		return wrapper != nullptr && encode_format != EncodeFormat::UNKNOW;
	}

	std::span<std::byte const> TextureWrapper::GetByteData() const
	{
		BYTE* byte = nullptr;
		DWORD size = 0;
		if (FreeImage_AcquireMemory(reinterpret_cast<FIMEMORY*>(wrapper), &byte, &size))
		{
			return {reinterpret_cast<std::byte const*>(byte), size};
		}
		return {};
	}

	TextureBitMap TextureWrapper::CoverToBitMap()
	{
		auto bitmap = FreeImage_LoadFromMemory(
			Translate(encode_format), 
			reinterpret_cast<FIMEMORY*>(wrapper)
		);
		if (bitmap != nullptr)
		{
			FreeImage_SeekMemory(
				reinterpret_cast<FIMEMORY*>(wrapper),
				0,
				SEEK_SET
			);
			return TextureBitMap{bitmap, encode_format };
		}
		return {};
	}

	TextureWrapper& TextureWrapper::operator=(TextureWrapper&& wrapper)
	{
		Reset();
		this->wrapper = wrapper.wrapper;
		wrapper.wrapper = nullptr;
		this->encode_format = wrapper.encode_format;
		wrapper.encode_format = EncodeFormat::UNKNOW;
		return *this;
	}

	TextureBitMap::operator bool() const
	{
		return bitmap != nullptr;
	}
	
	TextureBitMap::TextureBitMap(TextureBitMap&& wrapper)
		: bitmap(wrapper.bitmap), original_format(wrapper.original_format)
	{
		wrapper.bitmap = nullptr;
		wrapper.original_format = EncodeFormat::UNKNOW;
	}

	TextureBitMap& TextureBitMap::operator=(TextureBitMap&& wrapper)
	{
		Reset();
		bitmap = wrapper.bitmap;
		original_format = wrapper.original_format;
		wrapper.bitmap = nullptr;
		wrapper.original_format = EncodeFormat::UNKNOW;
		return *this;
	}

	void TextureBitMap::Reset()
	{
		if (bitmap != nullptr)
		{
			FreeImage_Unload(reinterpret_cast<FIBITMAP*>(bitmap));
			bitmap = nullptr;
		}
	}

	
	TextureBitMap::~TextureBitMap()
	{
		Reset();
	}

	TextureInfo TextureBitMap::GetTextureInfo() const
	{
		assert(*this);

		TextureInfo info;

		auto header = FreeImage_GetInfoHeader(
			reinterpret_cast<FIBITMAP*>(bitmap)
		);

		if (header != nullptr)
		{
			info.height = header->biHeight;
			info.width = header->biWidth;
			info.pixel_size = header->biBitCount;
			info.total_size = info.height * info.width * info.pixel_size;
		}

		return info;
	}

	std::span<std::byte const> TextureBitMap::GetByte() const
	{
		auto info = GetTextureInfo();
		if (info)
		{
			auto byte = FreeImage_GetBits(
				reinterpret_cast<FIBITMAP*>(bitmap)
			);
			if (byte != nullptr)
			{
				return {
					reinterpret_cast<std::byte const*>(byte),
					info.total_size
				};
			}
		}
		return {};
	}

	TextureWrapper TextureBitMap::CoverToEncodedTexture(EncodeFormat target_format)
	{
		if (target_format != EncodeFormat::UNKNOW)
		{
			auto mem = FreeImage_OpenMemory();
			if (mem == nullptr)
			{
				return {};
			}
			if (FreeImage_SaveToMemory(
				Translate(target_format),
				reinterpret_cast<FIBITMAP*>(bitmap),
				mem
			))
			{
				FreeImage_SeekMemory(mem, 0, SEEK_SET);
				return TextureWrapper{ mem, target_format };
			}
			FreeImage_CloseMemory(mem);
		}
		return {};
	}

	TextureWrapper TexEncoder::CreateWrapper(std::span<std::byte const> encoded_texture)
	{
		auto memory = FreeImage_OpenMemory(
			const_cast<BYTE*>(reinterpret_cast<BYTE const*>(encoded_texture.data())),
			encoded_texture.size()
		);

		if (memory != nullptr)
		{
			auto encode_format = Translate(
				FreeImage_GetFileTypeFromMemory(memory)
			);

			if (encode_format != EncodeFormat::UNKNOW)
			{
				return TextureWrapper{memory, encode_format };
			}
		}
		return {};
	}

	

	

	struct TexEncoderImplement : public TexEncoder
	{
		TexEncoderImplement()
		{
			FreeImage_Initialise();
		}

		~TexEncoderImplement()
		{
			FreeImage_DeInitialise();
		}

		operator bool() const { return true; }

	protected:

		virtual void AddTexEncoderRef() const override {  }
		virtual void SubTexEncoderRef() const override {  }
	};

	auto TexEncoder::Create(std::pmr::memory_resource* resource) -> Ptr
	{
		static TexEncoderImplement imp;
		if(imp)
			return &imp;
		return {};
	}
}
