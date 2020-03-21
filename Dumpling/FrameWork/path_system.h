#pragma once
#include <filesystem>
namespace Dumpling
{
	std::filesystem::path trans_to_local_path(std::filesystem::path P) { return P; }
}