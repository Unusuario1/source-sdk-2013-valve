#include <cstddef>
#include <windows.h>
#include "filesystem_init.h"

#include "soundbuilder.h"
#include "colorscheme.h"
#include "shared.h"


namespace SceneBuilder
{
	//-----------------------------------------------------------------------------
	// Purpose:	Check if sceneimagebuilder.exe exists
	//-----------------------------------------------------------------------------
	void AssetToolCheck(const char* gamebin)
	{
		Shared::AssetToolCheck(gamebin, NAME_SCENE_TOOL, "SceneBuilder");
	}


	//-----------------------------------------------------------------------------
	// Purpose: Loads additional command-line parameters defined in the 'SceneBuilder'
	//          section of the GameInfo KeyValues file, and constructs the full
	//          command line for the tool (e.g -v -game "C:\Half Life 2\hl2")
	//-----------------------------------------------------------------------------
	void LoadGameInfoKv(char* tool_argv, std::size_t bufferSize)
	{
		char _argv2[2048] = "";

		Shared::LoadGameInfoKv(SCENEBUILDER_KV, _argv2, sizeof(_argv2));

		V_snprintf(tool_argv, bufferSize, " %s %s %s -game \"%s\"", DEFAULT_SCENE_COMMANDLINE, TOOL_VERBOSE_OR_QUIET_MODE, _argv2, gamedir);
	}


	//-----------------------------------------------------------------------------
	// Purpose:	Setup the enviroment for sceneimagebuilder.exe to start & compile
	//-----------------------------------------------------------------------------
	void SceneCompile()
	{
		char tool_commands[4096] = "", sceneSrcPath[MAX_PATH] = "";
		bool bContinue = true;
		
		Shared::PrintHeaderCompileType("Scenes");

		V_snprintf(sceneSrcPath, sizeof(sceneSrcPath), "%s\\%s", gamedir, SCENESRC_DIR); // (e.g: C:\Half Life 2\hl2\scenes)

		bContinue = Shared::DirectoryAssetTypeExist(sceneSrcPath, SCENESRC_EXTENSION, "scenes");
		if (!bContinue)
			return;

		Msg("Asset report:\n");
		Shared::AssetInfoBuild(sceneSrcPath, SCENESRC_EXTENSION);
		if (g_infocontent)
			return;

		// Note: scenebuilder indepently if -lb is enabled, always is going to make a full build, 
		// we cannot control this in contentbuilder, but is not an issue becouse the compile is very fast

		SceneBuilder::LoadGameInfoKv(tool_commands, sizeof(tool_commands));
		Shared::StartExe("Scenes", NAME_SCENE_TOOL, tool_commands);
	}
}