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

	Potato::Document::BinaryStreamReader reader{u8R"(..\..\..\..\Test\Test.jpg)"};
	if (!reader)
		return 0;
	std::pmr::vector<std::byte> file_binary(reader.GetStreamSize());
	std::span<std::byte> file_binary_span = { file_binary.data(), file_binary .size()};
	reader.Read(file_binary_span);

	auto reference = instance->LoadToBitMapTexture(file_binary_span);
	auto format = reference.GetTextureInfo();

	return 0;
}