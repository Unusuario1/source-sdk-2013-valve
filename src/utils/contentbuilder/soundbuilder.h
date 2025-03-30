#ifndef SOUNDBUILDER_H
#define SOUNDBUILDER_H
#include <cstddef>


namespace SoundBuilder
{
	void AssetToolCheck(const char* gamebin);
	void SoundCompile(const char* gamebin, std::size_t bufferSize, std::size_t &complete, std::size_t &error);
}


#endif //SOUNDBUILDER_H