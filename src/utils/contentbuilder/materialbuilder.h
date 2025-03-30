#ifndef MATERIALBUILDER_H
#define MATERIALBUILDER_H
#include <cstddef>


namespace MaterialBuilder
{
	void AssetToolCheck(const char* gamebin);
	void MaterialCompile(const char* gamebin, std::size_t bufferSize, std::size_t &complete, std::size_t &error);
}


#endif // MATERIALBUILDER_H
