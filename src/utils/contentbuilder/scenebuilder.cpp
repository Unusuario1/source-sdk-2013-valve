#include <cstddef>
#include <windows.h>
#include "filesystem_init.h"

#include "soundbuilder.h"
#include "shared.h"


namespace SceneBuilder
{
	//Does audioprocess.exe exist?
	void AssetToolCheck(const char* gamebin)
	{
		Shared::AssetToolCheck(gamebin, NAME_SCENE_TOOL, "SceneBuilder");
	}

	void LoadGameInfoKv(char* tool_argv, std::size_t bufferSize)
	{
		//Load gameinfo.txt KeyValues
		char _argv2[2048] = "";

		Shared::LoadGameInfoKv(SCENEBUILDER_KV, _argv2, sizeof(_argv2));

		V_snprintf(tool_argv, bufferSize, " %s %s %s -game \"%s\"", DEFAULT_SCENE_COMMANDLINE, TOOL_VERBOSE_MODE, _argv2, g_gamedir);
	}

	void SceneCompile(const char* gamebin, std::size_t bufferSize, std::size_t &complete, std::size_t &error)
	{
		char tool_commands[4096] = "";

		SceneBuilder::LoadGameInfoKv(tool_commands, sizeof(tool_commands));

		//We need to read the keyvalues of SceneBuilder
		Shared::StartExe(gamebin, bufferSize, "Scenes", NAME_SCENE_TOOL, tool_commands, complete, error, false);
	}
}