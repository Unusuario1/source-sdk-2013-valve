#include <cstddef>
#include <windows.h>
#include "filesystem_init.h"

#include "materialbuilder.h"
#include "colorscheme.h"
#include "shared.h"


namespace MaterialBuilder 
{
	//-----------------------------------------------------------------------------
	// Purpose: We copy all the .vmt of game/mod/materialsrc to game/mod/materials folder
	//-----------------------------------------------------------------------------
	void CopySrcVmtToGameDir(const char* gamedir, const char* srcdir)
	{
		char _srcmat[MAX_PATH] = "";
		float start = Plat_FloatTime();

		Msg("AssetTools -> Materials: Starting copy of src (%s) files to gamedir... ", MATERIALS_EXTENSION);
		
		// (e.g: C:\Half Life 2\hl2\materialsrc)
		V_snprintf(_srcmat, sizeof(_srcmat), "%s\\%s", g_gamedir, MATERIALSRC_DIR);

		Shared::CopyFilesRecursively(_srcmat, MATERIALSRC_DIR, MATERIALS_DIR, MATERIALS_EXTENSION);

		ColorSpewMessage(SPEW_MESSAGE, &done_color, "done(%.2f)\n", Plat_FloatTime() - start);
	}


	void AssetToolCheck(const char* gamebin)
	{
		Shared::AssetToolCheck(gamebin, NAME_MATERIAL_TOOL, "MaterialBuilder");
	}


	void LoadGameInfoKv(char *tool_argv, std::size_t bufferSize)
	{
		char _argv2[2048] = "";

		Shared::LoadGameInfoKv(MATERIALBUILDER_KV, _argv2, sizeof(_argv2));

		// Note: vtex.exe does not support -verbose or -v 
		V_snprintf(tool_argv, bufferSize, " %s %s %s -game \"%s\"", DEFAULT_TEXTURE_COMMANDLINE, /*TOOL_VERBOSE_MODE*/ "", _argv2, g_gamedir);
	}


	void MaterialProcessRec(const char* gamebin, std::size_t bufferSize, const char* directory,
		const char* tool_commands, std::size_t& complete, std::size_t& error, const char* extension)
	{
		char searchPath[MAX_PATH];
		V_snprintf(searchPath, MAX_PATH, "%s\\*", directory); 

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
			V_snprintf(fullPath, MAX_PATH, "%s\\%s", directory, name);

			if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				MaterialProcessRec(gamebin, bufferSize, fullPath, tool_commands, complete, error, extension);
			}
			else if (Shared::HasExtension(name, extension))
			{
				char szTemp[4096];

				V_snprintf(szTemp, sizeof(szTemp), "%s \"%s\"", tool_commands, fullPath);
				Shared::StartExe(gamebin, bufferSize, "Materials", NAME_MATERIAL_TOOL, szTemp, complete, error, false);
			}
		} while (FindNextFileA(hFind, &findFileData));

		FindClose(hFind);
	}


	void MaterialCompile(const char* gamebin, std::size_t bufferSize, std::size_t& complete, std::size_t& error)
	{
		char tool_commands[4096] = "", gamesrcmat[MAX_PATH] = "", matsrcdir[MAX_PATH] = "", searchPath[MAX_PATH] = "";
		bool bContinueTga = true, bContinuePfm = true, bContinuePsd = true, bContinueVmt = true;

		ColorSpewMessage(SPEW_MESSAGE, &header_color, "\n====== Building %s ======\n", "Materials");

		// Check if there is any file to compile!
		V_snprintf(matsrcdir, sizeof(matsrcdir), "%s\\%s", g_gamedir, MATERIALSRC_DIR);
		bContinueTga = Shared::DirectoryAssetTypeExist(matsrcdir, TEXTURESRC_EXTENSION1, "texure");
		V_snprintf(matsrcdir, sizeof(matsrcdir), "%s\\%s", g_gamedir, MATERIALSRC_DIR);
		bContinuePfm = Shared::DirectoryAssetTypeExist(matsrcdir, TEXTURESRC_EXTENSION2, "texure");
		V_snprintf(matsrcdir, sizeof(matsrcdir), "%s\\%s", g_gamedir, MATERIALSRC_DIR);
		bContinuePsd = Shared::DirectoryAssetTypeExist(matsrcdir, TEXTURESRC_EXTENSION3, "texure");
		V_snprintf(matsrcdir, sizeof(matsrcdir), "%s\\%s", g_gamedir, MATERIALSRC_DIR);
		bContinueVmt = Shared::DirectoryAssetTypeExist(matsrcdir, MATERIALS_EXTENSION, "materials");

		if (!bContinueTga && !bContinuePfm && !bContinuePsd && !bContinueVmt)
			return;

		// Copy the vmt files from materialsrc to materials
		if (bContinueVmt)
		{
			Msg("Asset report:\n");
			V_snprintf(gamesrcmat, sizeof(gamesrcmat), "%s\\%s", g_gamedir, MATERIALSRC_DIR);
			Shared::AssetInfoBuild(gamesrcmat, MATERIALS_EXTENSION);
			if (!g_infocontent) 
			{
				MaterialBuilder::CopySrcVmtToGameDir(g_gamedir, gamesrcmat);
				Msg("\n");
			}
		}

		if (bContinueTga || bContinuePfm || bContinuePsd)
		{
			bContinueTga ? Shared::AssetInfoBuild(gamesrcmat, TEXTURESRC_EXTENSION1) : void();
			bContinuePfm ? Shared::AssetInfoBuild(gamesrcmat, TEXTURESRC_EXTENSION2) : void();
			bContinuePsd ? Shared::AssetInfoBuild(gamesrcmat, TEXTURESRC_EXTENSION3) : void();

			if (g_infocontent)
				return;

			MaterialBuilder::LoadGameInfoKv(tool_commands, sizeof(tool_commands)); //TODO, fix the rec path issue!

			V_snprintf(searchPath, sizeof(searchPath), "%s\\%s", g_gamedir, MATERIALSRC_DIR);

			if (bContinueTga)
			{
				MaterialProcessRec(gamebin, bufferSize, searchPath, tool_commands, complete, error, TEXTURESRC_EXTENSION1);
			}
			if (bContinuePfm) 
			{
				MaterialProcessRec(gamebin, bufferSize, searchPath, tool_commands, complete, error, TEXTURESRC_EXTENSION2);
			}
			if (bContinuePsd)
			{
				MaterialProcessRec(gamebin, bufferSize, searchPath, tool_commands, complete, error, TEXTURESRC_EXTENSION3);
			}
		}
	}
}