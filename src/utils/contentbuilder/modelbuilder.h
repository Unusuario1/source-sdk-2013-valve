#ifndef MODELBUILDER_H
#define MODELBUILDER_H
#include <cstddef>

namespace ModelBuilder
{
	void AssetToolCheck(const char* gamebin);
	void ModelCompile(const char* gamebin, std::size_t bufferSize, std::size_t &complete, std::size_t &error);
}


#endif //MODELBUILDER_H