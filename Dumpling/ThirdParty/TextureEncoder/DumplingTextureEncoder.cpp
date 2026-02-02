module;


module DumplingTextureEncoder;

namespace Dumpling::TextureEncoder
{

	TextureInfo TexEncoder::DetectTextureInfo(std::span<std::byte const> texture_resource, TextureEncodeFormat suggest_format)
	{
		
		switch (suggest_format)
		{
		case TextureEncodeFormat::TGA:
		{
			auto info = GetTGATextureInfo(texture_resource);
			if (info)
				return info;
		}
			break;
		}

		auto info = GetTGATextureInfo(texture_resource);

		return info;
	}

	TextureInfo TexEncoder::GetTGATextureInfo(std::span<std::byte const> file_binary)
	{
		return {};
	}

	struct TexEncoderImplement : public TexEncoder
	{
		TexEncoderImplement()
		{
			
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
