#include <cstddef>

namespace MapBuilder
{
	void AssetToolCheck(const char* gamebin);
	void MapCompile(const char* gamebin, std::size_t bufferSize, std::size_t& complete, std::size_t& error);
}