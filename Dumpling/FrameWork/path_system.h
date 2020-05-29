#pragma once
#include <filesystem>
#include <optional>
namespace Dumpling::Path
{
	struct Path
	{
		Path(std::u32string_view Input);
		Path(Path const&);
		Path(Path&&);
		Path& operator=(const Path&);
		bool operator==(Path const&) const noexcept;
		bool operator<(Path const&) const noexcept;
		Path RelateTo(Path const& input) const;
	private:
		std::u32string_view RootName;
		std::u32string_view FileName;
		std::u32string mStorage;
	};

















	//std::filesystem::path const& SearchAndResetNamedDirectory(std::u32string_view Name, std::filesystem::path const& Path);
	std::optional<std::filesystem::path> SearchAndResetRootDirectory(std::u32string_view RootDirectoryName);
	std::optional<std::vector<std::byte>> LoadEntireBinaryFile(std::filesystem::path const& path);
	std::optional<std::u32string> LoadEntireStringFile(std::filesystem::path const& path);
	std::optional<std::filesystem::path> SearchFile(std::u32string_view RootDirectoryName);
}