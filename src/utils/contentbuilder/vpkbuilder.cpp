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
	void AssetToolCheck(const char* gamebin)
	{
		Shared::AssetToolCheck(gamebin, NAME_VALVEPAKFILE_TOOL, "VpkBuilder");
	}


	//-----------------------------------------------------------------------------
	// Purpose: Loads additional command-line parameters defined in the 'VpkBuilder'
	//          section of the GameInfo KeyValues file, and constructs the full
	//          command line for the tool (e.g -v -game "C:\Half Life 2\hl2")
	//-----------------------------------------------------------------------------
	void LoadGameInfoKv(char* tool_command, const char* vpktemp, std::size_t bufferSize)
	{
		char _argv2[2048];

		Shared::LoadGameInfoKv(VPKBUILDER_KV, _argv2, sizeof(_argv2));

		V_snprintf(tool_command, bufferSize, " %s %s \"%s\"", DEFAULT_VALVEPAKFILE_COMMANDLINE, TOOL_VERBOSE_MODE, vpktemp);
	}


	//-----------------------------------------------------------------------------
	// Purpose:	Setup the enviroment for vpk.exe to start & compile
	//-----------------------------------------------------------------------------
	void VpkCompile()
	{
		char tool_args[4096] = "", VpkTempPath[MAX_PATH] = "", modVpkTempPath[MAX_PATH] = "";

		Shared::PrintHeaderCompileType("Valve Pack File (vpk)");

		// get the mod name, and set up the temp dir
		char *modname = V_strrchr(gamedir, '\\');
		modname++; // skip the first '\'

		V_snprintf(VpkTempPath, sizeof(VpkTempPath), "%s\\_tempvpk", g_contentbuilderPath);
		V_snprintf(modVpkTempPath, sizeof(modVpkTempPath), "%s\\%s", VpkTempPath, modname);

		if (!Shared::CreateDirectoryRecursive(modVpkTempPath))
		{
			DWORD err = GetLastError();
			if (err != ERROR_ALREADY_EXISTS) 
			{
				Shared::qError("AssetSystem -> Could not create temporary directory at: \"%s\" (Error code: %lu)\n", modVpkTempPath, err);
			}
		}

		// For building vpks files we COPY the the folders into _build/_tempvpk/modname
		// We copy the next dirs: expressions, materials, models, particles, scenes, sounds, shaders
		float start = Plat_FloatTime();
		Msg("AssetSystem -> Creating temp files for vpk build (this may take a while)... ");
		
		const char* folderlist[] = { "expressions", MATERIALS_DIR, MODELS_DIR, "particles", SCENE_DIR, SOUNDS_DIR, "shaders" };
		for (const char* folder : folderlist) 
		{
			char srcCopyDir[MAX_PATH] = "", outCopyDir[MAX_PATH] = "";

			V_snprintf(srcCopyDir, sizeof(srcCopyDir), "%s\\%s", gamedir, folder);
			V_snprintf(outCopyDir, sizeof(outCopyDir), "%s\\%s", modVpkTempPath, folder);

			if (Shared::CheckIfFileExist(srcCopyDir)) 
			{
				Shared::CopyDirectoryContents(srcCopyDir, outCopyDir, folder == SCENE_DIR ? ".image" : "");
			}
			else
			{
				Msg("AssetSystem -> Non existing path in gamedir: %s, excluding\n", folder);
			}
		}
		ColorSpewMessage(SPEW_MESSAGE, &done_color, "done(%.2f)\n", Plat_FloatTime() - start);

		LoadGameInfoKv(tool_args, modVpkTempPath, sizeof(tool_args));

		Shared::StartExe("Valve Pack File (vpk)", NAME_VALVEPAKFILE_TOOL, tool_args);

		start = Plat_FloatTime();
		Msg("AssetSystem -> Coping contents from temp dir... ");

		// After the build we copy the generated pack file and delete the temp file
		if (!Shared::CopyDirectoryContents(VpkTempPath, gamedir, VALVEPAKFILE_EXTENSION))
		{
			return;
		}
		ColorSpewMessage(SPEW_MESSAGE, &done_color, "done(%.2f)\n", Plat_FloatTime() - start);

		// detele _build/_tempvpk folder
		if (g_cleanuptempcontent) 
		{
			Shared::DeleteFolderWithContents(VpkTempPath);
		}
	}
}