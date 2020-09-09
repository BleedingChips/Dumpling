#pragma once
#include <filesystem>
#include <optional>
#include <vector>
namespace Dumpling::Path
{
	std::optional<std::filesystem::path> UpSearch(std::u32string_view RequireName, std::filesystem::path const& StartPath = std::filesystem::current_path());
	std::vector<std::filesystem::path> UpSearchAll(std::u32string_view RequireName, std::filesystem::path const& StartPath = std::filesystem::current_path());
	std::optional<std::filesystem::path> Search(std::u32string_view RequireName, std::filesystem::path const& StartPath = std::filesystem::current_path());
	std::vector<std::filesystem::path> SearchAll(std::u32string_view RequireName, std::filesystem::path const& StartPath = std::filesystem::current_path());
	std::optional<std::vector<std::byte>> LoadEntireFile(std::filesystem::path const&);
}