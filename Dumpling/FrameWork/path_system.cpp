#include "path_system.h"
#include <map>
#include <fstream>
#include <assert.h>
#include "../Gui/../../Potato/character_encoding.h"
#include <shared_mutex>
namespace Dumpling::Path
{
	using namespace std::filesystem;
	std::optional<std::filesystem::path> UpSearch(std::u32string_view RequireName, std::filesystem::path const& StartPath)
	{
		std::filesystem::path SearchPath = StartPath;
		auto RootName = SearchPath.root_name();
		while (SearchPath != RootName)
		{
			for (auto ite : directory_iterator(SearchPath))
			{
				auto TemPath = ite.path().filename();
				if (RequireName == TemPath)
					return  ite.path();
			}
			SearchPath = SearchPath.parent_path();
		}
		return std::nullopt;
	}

	std::vector<std::filesystem::path> UpSearchAll(std::u32string_view RequireName, std::filesystem::path const& StartPath)
	{
		std::vector<std::filesystem::path> Result;
		std::filesystem::path SearchPath = StartPath;
		auto RootName = SearchPath.root_name();
		while (SearchPath != RootName)
		{
			for (auto ite : directory_iterator(SearchPath))
			{
				auto TemPath = ite.path().filename();
				if (RequireName == TemPath)
					Result.push_back(ite.path());
			}
			SearchPath = SearchPath.parent_path();
		}
		return Result;
	}

	std::optional<std::filesystem::path> Search(std::u32string_view RequireName, std::filesystem::path const& StartPath)
	{
		for (auto& ite : recursive_directory_iterator(StartPath))
		{
			auto TemPath = ite.path().filename();
			if (RequireName == TemPath)
				return ite.path();
		}
		return std::nullopt;
	}

	std::vector<std::filesystem::path> SearchAll(std::u32string_view RequireName, std::filesystem::path const& StartPath)
	{
		std::vector<std::filesystem::path> Result;
		for (auto& ite : recursive_directory_iterator(StartPath))
		{
			auto TemPath = ite.path().filename();
			if (RequireName == TemPath)
				Result.push_back(ite.path());
		}
		return Result;
	}

	std::optional<std::vector<std::byte>> LoadEntireFile(std::filesystem::path const& Path)
	{
		size_t Size = std::filesystem::file_size(Path);
		if (Size > 0)
		{
			std::ifstream InputFile(Path, std::ios::binary);
			if (InputFile.is_open())
			{
				std::vector<std::byte> Buffer;
				Buffer.resize(Size);
				InputFile.read(reinterpret_cast<char*>(Buffer.data()), Buffer.size());
				return std::move(Buffer);
			}
		}
		return std::nullopt;
	}

}