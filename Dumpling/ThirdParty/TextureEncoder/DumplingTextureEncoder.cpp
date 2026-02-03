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
		default:
			return EncodeFormat::UNKNOW;
		}
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

	TextureBitMap::TextureBitMap(void* bitmap, EncodeFormat original_format)
		: bitmap(bitmap), original_format(original_format)
	{

	}

	TextureBitMap& TextureBitMap::operator=(TextureBitMap&& wrapper)
	{
		Clear();
		bitmap = wrapper.bitmap;
		original_format = wrapper.original_format;
		wrapper.bitmap = nullptr;
		wrapper.original_format = EncodeFormat::UNKNOW;
		return *this;
	}

	void TextureBitMap::Clear()
	{
		if (bitmap != nullptr)
		{
			FreeImage_Unload(reinterpret_cast<FIBITMAP*>(bitmap));
			bitmap = nullptr;
		}
	}

	
	TextureBitMap::~TextureBitMap()
	{
		Clear();
	}

	TextureInfo TextureBitMap::GetTextureInfo() const
	{
		assert(*this);

		TextureInfo info;

		info.height = FreeImage_GetHeight(reinterpret_cast<FIBITMAP*>(bitmap));
		info.width = FreeImage_GetWidth(reinterpret_cast<FIBITMAP*>(bitmap));
		auto k = FreeImage_GetColorType(reinterpret_cast<FIBITMAP*>(bitmap));
		FreeImage_SaveToMemory(FREE_IMAGE_FORMAT::);

		return info;
	}

	EncodeFormat TexEncoder::GetTextureFormat(std::span<std::byte const> texture_binary)
	{
		auto memory = FreeImage_OpenMemory(
			const_cast<BYTE*>(reinterpret_cast<BYTE const*>(texture_binary.data())),
			texture_binary.size()
		);

		if (memory == nullptr)
			return EncodeFormat::UNKNOW;

		auto format_type = Translate(FreeImage_GetFileTypeFromMemory(memory));

		FreeImage_CloseMemory(memory);

		return format_type;

	}

	TextureBitMap TexEncoder::LoadToBitMapTexture(std::span<std::byte const> texture_binary)
	{
		auto memory = FreeImage_OpenMemory(
			const_cast<BYTE*>(reinterpret_cast<BYTE const*>(texture_binary.data())),
			texture_binary.size()
		);

		if (memory == nullptr)
			return {};

		auto freeimage_type = FreeImage_GetFileTypeFromMemory(memory);
		auto type = Translate(freeimage_type);

		if (type == EncodeFormat::UNKNOW)
		{
			FreeImage_CloseMemory(memory);
			return {};
		}
			
		auto bitmap = FreeImage_LoadFromMemory(freeimage_type, memory);

		if (bitmap == nullptr)
		{
			FreeImage_CloseMemory(memory);
			return {};
		}

		FreeImage_CloseMemory(memory);

		TextureBitMap ref{ reinterpret_cast<void*>(bitmap), type};
		return ref;
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
