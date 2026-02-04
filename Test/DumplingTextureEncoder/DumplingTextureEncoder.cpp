#include <imgui.h>

import Dumpling;
import DumplingTextureEncoder;
import Potato;
import std;

using namespace Dumpling;
using namespace Dumpling::TextureEncoder;


int main()
{
	auto instance = CreateTexEncoder();

	auto cur = std::filesystem::current_path();

	auto RootPath = Potato::Path::ReverseRecursiveSearchDirectory(
		u8"Test"
	);

	auto target_path = RootPath;
	target_path.append(u8"Test.jpg");

	Potato::Document::BinaryStreamReader reader{ target_path };
	if (!reader)
		return 0;

	std::pmr::vector<std::byte> file_binary(reader.GetStreamSize());
	std::span<std::byte> file_binary_span = { file_binary.data(), file_binary .size()};
	reader.Read(file_binary_span);

	auto wrapper = instance->CreateWrapper(file_binary_span);
	auto bit_map = wrapper.CoverToBitMap();
	auto info = bit_map.GetTextureInfo();
	auto span = bit_map.GetByte();
	auto new_wrapper = bit_map.CoverToEncodedTexture(EncodeFormat::PNG);

	{
		Potato::Document::BinaryStreamWriter writer{ u8R"(text.png)", Potato::Document::BinaryStreamWriter::OpenMode::CREATE_OR_EMPTY };
		writer.Write(new_wrapper.GetByteData());
		writer.Close();
	}

	return 0;
}