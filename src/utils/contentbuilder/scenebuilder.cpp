#include <cstddef>

#include "soundbuilder.h"
#include "shared.h"


namespace SceneBuilder
{
	//Does audioprocess.exe exist?
	void AssetToolCheck(const char* gamebin)
	{
		Shared::AssetToolCheck(gamebin, NAME_SCENE_TOOL, "SceneBuilder");
	}

	void LoadGameInfoKv(const char* vtex_argv)
	{
		//Load gameinfo.txt KeyValues

	}

	void SceneCompile(const char* gamebin, std::size_t bufferSize, std::size_t &complete, std::size_t &error)
	{
		//We need to read the keyvalues of SceneBuilder
		Shared::StartExe(gamebin, bufferSize, "Scenes", NAME_SCENE_TOOL, "", complete, error);
	}
}