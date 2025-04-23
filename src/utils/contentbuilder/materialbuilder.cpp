#include <cstddef>
#include <windows.h>
#include "filesystem_init.h"

#include "materialbuilder.h"
#include "colorscheme.h"
#include "shared.h"


namespace MaterialBuilder 
{
	//-----------------------------------------------------------------------------
	// Purpose: Copy all the .vmt of game/mod/materialsrc to game/mod/materials folder
	//-----------------------------------------------------------------------------
	void CopySrcVmtToGameDir(const char* materialsrc_path)
	{
		float start = Plat_FloatTime();
		Msg("AssetTools -> Materials: Starting copy of src (%s) files to gamedir... ", MATERIALS_EXTENSION);
		Shared::CopyFilesRecursivelyGame(materialsrc_path, MATERIALSRC_DIR, MATERIALS_DIR, MATERIALS_EXTENSION);
		ColorSpewMessage(SPEW_MESSAGE, &done_color, "done(%.2f)\n", Plat_FloatTime() - start);
	}


	void AssetToolCheck(const char* gamebin)
	{
		Shared::AssetToolCheck(gamebin, NAME_MATERIAL_TOOL, "MaterialBuilder");
	}
	

	//-----------------------------------------------------------------------------
	// Purpose: Loads additional command-line parameters defined in the 'MaterialBuilder'
	//          section of the GameInfo KeyValues file, and constructs the full
	//          command line for the tool (e.g -v -game "C:\Half Life 2\hl2")
	//-----------------------------------------------------------------------------
	void LoadGameInfoKv(char *tool_argv, std::size_t bufferSize)
	{
		char _argv2[2048] = "";

		Shared::LoadGameInfoKv(MATERIALBUILDER_KV, _argv2, sizeof(_argv2));

		// Note: vtex.exe does not support -verbose or -v 
		V_snprintf(tool_argv, bufferSize, " %s %s %s -game \"%s\"", DEFAULT_TEXTURE_COMMANDLINE, g_quiet ? "-quiet" : "", _argv2, gamedir);
	}


	//-----------------------------------------------------------------------------
	// Purpose:	Compile all the assets found in the given directory
	//-----------------------------------------------------------------------------
	void MaterialProcessRec(const char* directory, const char* tool_commands, const char* extension)
	{
		char searchPath[MAX_PATH];
		V_snprintf(searchPath, sizeof(searchPath), "%s\\*", directory);

		WIN32_FIND_DATAA findFileData;
		HANDLE hFind = FindFirstFileA(searchPath, &findFileData);
		if (hFind == INVALID_HANDLE_VALUE)
			return;

		do
		{
			const char* name = findFileData.cFileName;
			if (V_strcmp(name, ".") == 0 || V_strcmp(name, "..") == 0)
				continue;

			char fullPath[MAX_PATH];
			V_snprintf(fullPath, sizeof(fullPath), "%s\\%s", directory, name);

			if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				MaterialProcessRec(fullPath, tool_commands, extension);
			}
			else if (Shared::HasExtension(name, extension))
			{
				char szTemp[4096];

				if (!Shared::PartialBuildAsset(fullPath, MATERIALSRC_DIR, MATERIALS_DIR))
					continue;

				V_snprintf(szTemp, sizeof(szTemp), "%s \"%s\"", tool_commands, fullPath);
				Shared::StartExe("Materials", NAME_MATERIAL_TOOL, szTemp);
			}
		} while (FindNextFileA(hFind, &findFileData));

		FindClose(hFind);
	}


	//-----------------------------------------------------------------------------
	// Purpose:	Setup the enviroment for vtex.exe to start & compile
	//-----------------------------------------------------------------------------
	void MaterialCompile()
	{
		char tool_commands[4096] = "", matsrcdir[MAX_PATH] = "", searchPath[MAX_PATH] = "";
		bool bContinueTga = true, bContinuePfm = true, bContinuePsd = true, bContinueVmt = true;

		Shared::PrintHeaderCompileType("Materials");

		// Check if there is any file to compile!
		V_snprintf(matsrcdir, sizeof(matsrcdir), "%s\\%s", gamedir, MATERIALSRC_DIR);
		bContinueTga = Shared::DirectoryAssetTypeExist(matsrcdir, TEXTURESRC_EXTENSION1, "texure");
		bContinuePfm = Shared::DirectoryAssetTypeExist(matsrcdir, TEXTURESRC_EXTENSION2, "texure");
		bContinuePsd = Shared::DirectoryAssetTypeExist(matsrcdir, TEXTURESRC_EXTENSION3, "texure");
		bContinueVmt = Shared::DirectoryAssetTypeExist(matsrcdir, MATERIALS_EXTENSION, "materials");

		if (!bContinueTga && !bContinuePfm && !bContinuePsd && !bContinueVmt)
			return;

		Msg("%s", (g_quiet || !g_spewallcommands) ? "Asset report:\n" : "");

		// Copy the vmt files from materialsrc to materials
		if (bContinueVmt)
		{
			Shared::AssetInfoBuild(matsrcdir, MATERIALS_EXTENSION);
			if (!g_infocontent) 
			{
				MaterialBuilder::CopySrcVmtToGameDir(matsrcdir);
				Msg("\n");
			}
		}

		if (bContinueTga || bContinuePfm || bContinuePsd)
		{
			bContinueTga ? Shared::AssetInfoBuild(matsrcdir, TEXTURESRC_EXTENSION1) : void();
			bContinuePfm ? Shared::AssetInfoBuild(matsrcdir, TEXTURESRC_EXTENSION2) : void();
			bContinuePsd ? Shared::AssetInfoBuild(matsrcdir, TEXTURESRC_EXTENSION3) : void();

			if (g_infocontent)
				return;

			MaterialBuilder::LoadGameInfoKv(tool_commands, sizeof(tool_commands));

			if (bContinueTga)
			{
				MaterialProcessRec(matsrcdir, tool_commands, TEXTURESRC_EXTENSION1);
			}
			if (bContinuePfm) 
			{
				MaterialProcessRec(matsrcdir, tool_commands, TEXTURESRC_EXTENSION2);
			}
			if (bContinuePsd)
			{
				MaterialProcessRec(matsrcdir, tool_commands, TEXTURESRC_EXTENSION3);
			}
		}
	}
}