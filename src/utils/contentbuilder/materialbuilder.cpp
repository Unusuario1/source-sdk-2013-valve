#include <cstddef>
#include <windows.h>
#include "filesystem_init.h"

#include "materialbuilder.h"
#include "shared.h"


namespace MaterialBuilder 
{
	//We copy all the .vmt of game/mod/materialsrc to game/mod/materials folder
	void CopySrcVmtToGameDir(const char* gamedir, const char* srcdir)
	{
		char _srcmat[MAX_PATH] = "";
		char _gamemat[MAX_PATH] = "";

		float start = Plat_FloatTime();
		Msg("AssetTools -> Materials: Starting copy of src (%s) files to gamedir... ", MATERIALS_EXTENSION);
		
		//This is the base path, in this case (e.g: C:\Half Life 2\hl2\materialsrc)
		V_snprintf(_srcmat, sizeof(_srcmat), "%s\\%s", g_gamedir, MATERIALSRC_DIR);
		V_snprintf(_gamemat, sizeof(_gamemat), "%s\\%s", g_gamedir, MATERIALS_DIR);

		//We scan recursivbly the _srcmat path to gamemat
		Shared::CopyFilesRecursively(_srcmat, _gamemat, MATERIALS_EXTENSION);

		float end = Plat_FloatTime();
		Msg("Done(%f)\n", end - start);
	}


	void AssetToolCheck(const char* gamebin)
	{
		Shared::AssetToolCheck(gamebin, NAME_MATERIAL_TOOL, "MaterialBuilder");
	}


	void LoadGameInfoKv(char *tool_argv, std::size_t bufferSize)
	{
		char _argv2[2048] = "";

		Shared::LoadGameInfoKv(MATERIALBUILDER_KV, _argv2, sizeof(_argv2));
		
		V_snprintf(tool_argv, bufferSize, " %s %s %s -game \"%s\"", DEFAULT_TEXTURE_COMMANDLINE, TOOL_VERBOSE_MODE, _argv2, g_gamedir);
	}


	void MaterialCompile(const char* gamebin, std::size_t bufferSize, std::size_t &complete, std::size_t &error)
	{
		char tool_commands[4096] = "", gamesrcmat[MAX_PATH] = "";

		MaterialBuilder::LoadGameInfoKv(tool_commands, sizeof(tool_commands));
		
		Shared::StartExe(gamebin, bufferSize, "Materials", NAME_MATERIAL_TOOL, tool_commands, complete, error, false);

		//Copy the vmt files from materialsrc to materials
		V_snprintf(gamesrcmat, sizeof(gamesrcmat), "%s\\%s", g_gamedir, MATERIALSRC_DIR);

		MaterialBuilder::CopySrcVmtToGameDir(g_gamedir, gamesrcmat);
	}
}

