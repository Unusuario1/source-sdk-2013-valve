//========= --------------------------------------------------- ============//
//
// Purpose: SceneBuilder – A ContentBuilder subsystem for scene compile.
//
// $NoKeywords: $
//=============================================================================//
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
	void AssetToolCheck(const char* pGameBin)
	{
		Shared::AssetToolCheck(pGameBin, NAME_SCENE_TOOL, "SceneBuilder");
	}


	//-----------------------------------------------------------------------------
	// Purpose: Loads additional command-line parameters defined in the 'SceneBuilder'
	//          section of the GameInfo KeyValues file, and constructs the full
	//          command line for the tool (e.g -v -game "C:\Half Life 2\hl2")
	//-----------------------------------------------------------------------------
	static void LoadGameInfoKv(char* pToolArgv, std::size_t uiBufferSize)
	{
		char szKvToolArgv[2048] = "";

		Shared::LoadGameInfoKv(SCENEBUILDER_KV, szKvToolArgv, sizeof(szKvToolArgv));

		V_snprintf(pToolArgv, uiBufferSize, " %s %s %s -game \"%s\"", DEFAULT_SCENE_COMMANDLINE, TOOL_VERBOSE_OR_QUIET_MODE, szKvToolArgv, gamedir);
	}


	//-----------------------------------------------------------------------------
	// Purpose:	Setup the enviroment for sceneimagebuilder.exe to start & compile
	//-----------------------------------------------------------------------------
	void SceneCompile()
	{
		char szToolCommand[4096] = "", szSceneSrcPath[MAX_PATH] = "";
		bool bContinue = true;
		
		Shared::PrintHeaderCompileType("Scenes");

		V_snprintf(szSceneSrcPath, sizeof(szSceneSrcPath), "%s\\%s", gamedir, SCENESRC_DIR); // (e.g: C:\Half Life 2\hl2\scenes)

		bContinue = Shared::DirectoryAssetTypeExist(szSceneSrcPath, SCENESRC_EXTENSION, "scenes");
		if (!bContinue)
			return;

		Msg("%s", (g_spewallcommands) ? "Asset report:\n" : "");
		Shared::AssetInfoBuild(szSceneSrcPath, SCENESRC_EXTENSION);
		if (g_infocontent)
			return;

		// Note: scenebuilder indepently if -lb is enabled, always is going to make a full build, 
		// we cannot control this in contentbuilder, but is not an issue becouse the compile is very fast
		SceneBuilder::LoadGameInfoKv(szToolCommand, sizeof(szToolCommand));

		Shared::StartExe("Scenes", NAME_SCENE_TOOL, szToolCommand);
	}
}