#include <cstddef>

#include "modelbuilder.h"
#include "shared.h"


namespace ModelBuilder
{
	//Does studiomdl.exe exist?
	void AssetToolCheck(const char* gamebin)
	{
		Shared::AssetToolCheck(gamebin, NAME_MODEL_TOOL, "ModelBuilder");
	}

	void LoadGameInfoKv(const char* vtex_argv)
	{
		//Load gameinfo.txt KeyValues

	}

	//We need to read the keyvalues of Models
	void ModelCompile(const char* gamebin, std::size_t bufferSize, std::size_t &complete, std::size_t &error)
	{	
		Shared::StartExe(gamebin, bufferSize, "Models", NAME_MODEL_TOOL, "", complete, error);
	}
}