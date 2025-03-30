#ifndef CAPTIONCOMPILER_H
#define CAPTIONCOMPILER_H
#include <cstddef>


namespace CaptionBuilder
{
	void AssetToolCheck(const char* gamebin);
	void CaptionCompile(const char* gamebin, std::size_t bufferSize, std::size_t &complete, std::size_t &error);
}


#endif // CAPTIONCOMPILER_H