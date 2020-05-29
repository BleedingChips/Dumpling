#include "path_system.h"
#include <map>
#include <fstream>
#include <assert.h>
#include "../Gui/../../Potato/character_encoding.h"
#include <shared_mutex>
namespace Dumpling::Path
{

	std::shared_mutex RootNameMutex;
	std::vector<std::u32string> RootPath;
	std::map<std::u32string, size_t> RootPathMapping;


	Path::Path(std::u32string const& ref)
	{

	}






	using namespace std::filesystem;

	path DefaultRootDirectoty;

	std::optional<std::vector<std::byte>> LoadEntireBinaryFile(std::filesystem::path const& path)
	{
		auto TargetPath = DefaultRootDirectoty;
		TargetPath += path;
		std::ifstream input(TargetPath, std::ios::binary);
		if (input.is_open())
		{
			size_t file_size = std::filesystem::file_size(TargetPath);
			std::vector<std::byte> buffer;
			buffer.resize(file_size);
			input.read(reinterpret_cast<char*>(buffer.data()), buffer.size());
			return std::move(buffer);
		}
		return std::nullopt;
	}

	std::optional<std::u32string> LoadEntireStringFile(std::filesystem::path const& path)
	{
		using namespace Potato::Encoding;
		auto Data = LoadEntireBinaryFile(path);
		if (Data)
		{
			auto& ref = *Data;
			auto [Type, size] = translate_binary_to_bomtype(ref.data(), ref.size());
			switch (Type)
			{
			case BomType::UTF8:
			case BomType::None: {
				string_encoding<char> se(reinterpret_cast<char const*>(ref.data() + size), ref.size() - size);
				return se.to_string<char32_t>();
			}	break;
			default: assert(false);
				break;
			}
		}
		return std::nullopt;
	}

	std::optional<std::filesystem::path> SearchFile(std::u32string_view RootDirectoryName)
	{
		for (auto& ite : recursive_directory_iterator(DefaultRootDirectoty))
		{
			if (ite.is_regular_file())
			{
				auto TemPath = ite.path().filename();
				if (TemPath == RootDirectoryName)
					return ite.path();
			}
		}
		return std::nullopt;
	}


	std::optional<std::filesystem::path> SearchAndResetRootDirectory(std::u32string_view RootDirectoryName)
	{
		std::filesystem::path SearchPath = current_path();
		while (SearchPath != SearchPath.root_name())
		{
			for (auto ite : directory_iterator(SearchPath))
			{
				if (ite.is_directory())
				{
					auto TemPath = ite.path().filename();
					if (RootDirectoryName == TemPath)
					{
						DefaultRootDirectoty = ite.path();
						return DefaultRootDirectoty;
					}
				}
			}
			SearchPath = SearchPath.parent_path();
		}
		return {};
	}

}