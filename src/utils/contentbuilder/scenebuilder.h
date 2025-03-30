#ifndef SCENEBUILDER_H
#define SCENEBUILDER_H
#include <cstddef>


namespace SceneBuilder
{
	void AssetToolCheck(const char* gamebin);
	void SceneCompile(const char* gamebin, std::size_t bufferSize, std::size_t &complete, std::size_t &error);
}

#endif //SCENEBUILDER_H