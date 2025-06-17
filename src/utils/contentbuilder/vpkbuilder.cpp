//========= --------------------------------------------------- ============//
//
// Purpose: VpkBuilder – A ContentBuilder subsystem for vpk creation.
//
// $NoKeywords: $
//=============================================================================//
#include <cstddef>
#include <windows.h>
#include "filesystem_init.h"

#include "colorscheme.h"
#include "vpkbuilder.h"
#include "shared.h"


namespace VpkBuilder
{
	//-----------------------------------------------------------------------------
	// Purpose:	Check if vpk.exe exists
	//-----------------------------------------------------------------------------
	void AssetToolCheck(const char* pGameBin)
	{
		Shared::AssetToolCheck(pGameBin, NAME_VALVEPAKFILE_TOOL, VPKBUILDER_KV);
	}


	//-----------------------------------------------------------------------------
	// Purpose: Loads additional command-line parameters defined in the 'VpkBuilder'
	//          section of the GameInfo KeyValues file, and constructs the full
	//          command line for the tool (e.g -v -game "C:\Half Life 2\hl2")
	//-----------------------------------------------------------------------------
	static void LoadGameInfoKv(char* tool_command, const char* vpktemp, std::size_t uiBufferSize)
	{
		char szKvToolArgv[2048];

		Shared::LoadGameInfoKv(VPKBUILDER_KV, szKvToolArgv, sizeof(szKvToolArgv));

		V_snprintf(tool_command, uiBufferSize, " %s %s \"%s\"", DEFAULT_VALVEPAKFILE_COMMANDLINE, TOOL_VERBOSE_MODE, vpktemp);
	}


	//-----------------------------------------------------------------------------
	// Purpose:	Setup the enviroment for vpk.exe to start & compile
	//-----------------------------------------------------------------------------
	void VpkCompile()
	{
		char szToolArgv[4096] = "", szVpkTempPath[MAX_PATH] = "", szModVpkTempPath[MAX_PATH] = "";

		Shared::PrintHeaderCompileType("Valve Pack File (vpk)");

		// get the mod name, and set up the temp dir
		char *pModName = V_strrchr(gamedir, '\\');
		pModName++; // skip the first '\'

		V_snprintf(szVpkTempPath, sizeof(szVpkTempPath), "%s\\%s", g_contentbuilderPath, TEMP_VPK_DIR);
		V_snprintf(szModVpkTempPath, sizeof(szModVpkTempPath), "%s\\%s", szVpkTempPath, pModName);

		if (!Shared::CreateDirectoryRecursive(szModVpkTempPath))
		{
			DWORD err = GetLastError();
			if (err != ERROR_ALREADY_EXISTS) 
			{
				Shared::qError("AssetSystem -> Could not create temporary directory at: \"%s\" (Error code: %lu)\n", szModVpkTempPath, err);
				g_process_error++;
				return;
			}
		}

		// For building vpks files we COPY the the folders into _build/_tempvpk/modmane
		// We copy the next dirs: expressions, materials, models, particles, scenes, sounds, shaders
		float start = Plat_FloatTime();
		Msg("AssetSystem -> Creating temp files for vpk build (this may take a while)... ");
		
		const char* rgszFolderlist[] = { "expressions", MATERIALS_DIR, MODELS_DIR, "particles", SCENE_DIR, SOUNDS_DIR, "shaders" };
		for (const char* pFolder : rgszFolderlist)
		{
			char srcCopyDir[MAX_PATH] = "", outCopyDir[MAX_PATH] = "";

			V_snprintf(srcCopyDir, sizeof(srcCopyDir), "%s\\%s", gamedir, pFolder);
			V_snprintf(outCopyDir, sizeof(outCopyDir), "%s\\%s", szModVpkTempPath, pFolder);

			if (Shared::CheckIfPathOrFileExist(srcCopyDir)) 
			{
				Shared::CopyDirectoryContents(srcCopyDir, outCopyDir, pFolder == SCENE_DIR ? ".image" : "");
			}
			else
			{
				Shared::qWarning("AssetSystem -> Non existing path in gamedir: %s, excluding\n", pFolder);
			}
		}
		ColorSpewMessage(SPEW_MESSAGE, &done_color, "done(%.2f)\n", Plat_FloatTime() - start);

		LoadGameInfoKv(szToolArgv, szModVpkTempPath, sizeof(szToolArgv));

		Shared::StartExe("Valve Pack File (vpk)", NAME_VALVEPAKFILE_TOOL, szToolArgv);

		start = Plat_FloatTime();
		Msg("AssetSystem -> Coping contents from temp dir... ");

		// After the build we copy the generated pack file and delete the temp file
		if (!Shared::CopyDirectoryContents(szVpkTempPath, gamedir, VALVEPAKFILE_EXTENSION))
		{
			return;
		}
		ColorSpewMessage(SPEW_MESSAGE, &done_color, "done(%.2f)\n", Plat_FloatTime() - start);

		// detele _build/_tempvpk folder
		if (g_cleanuptempcontent) 
		{
			Shared::DeleteFolderWithContents(szVpkTempPath);
		}
	}
}