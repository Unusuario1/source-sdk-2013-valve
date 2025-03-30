#ifndef VPKBUILDER_H
#define VPKBUILDER_H
#include <cstddef>


namespace VpkBuilder
{
	void AssetToolCheck(const char* gamebin);
	void VpkCompile(const char* gamebin, std::size_t bufferSize, std::size_t &complete, std::size_t &error);
}


#endif //VPKBUILDER_H