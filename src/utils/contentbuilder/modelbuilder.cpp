#include <cstddef>
#include <windows.h>
#include "filesystem_init.h"

#include "modelbuilder.h"
#include "shared.h"


namespace ModelBuilder
{
	//Does studiomdl.exe exist?
	void AssetToolCheck(const char* gamebin)
	{
		Shared::AssetToolCheck(gamebin, NAME_MODEL_TOOL, "ModelBuilder");
	}

	void LoadGameInfoKv(char* tool_argv, std::size_t bufferSize)
	{
		char _argv2[2048] = "";

		Shared::LoadGameInfoKv(MODELBUILDER_KV, _argv2, sizeof(_argv2));

		V_snprintf(tool_argv, bufferSize, " %s %s %s -game \"%s\"", DEFAULT_MODEL_COMMANDLINE, TOOL_VERBOSE_MODE, _argv2, g_gamedir);
	}

	void ModelCompile(const char* gamebin, std::size_t bufferSize, std::size_t &complete, std::size_t &error)
	{	
		//Since studiomdl doesnt support batch compiles, we have to scan all the .qc files inside game/mod/modelsrc
		// get the .qc file and pass to studiomdl that file. This is done manually inside ModelCompile

		char tool_commands[4096] = "", searchPath[MAX_PATH] = "";
		float start, end;
		start = Plat_FloatTime();

		Msg("\n====== Building %s ======\n", "Models");

		ModelBuilder::LoadGameInfoKv(tool_commands, sizeof(tool_commands)); 

		V_snprintf(searchPath, MAX_PATH, "%s\\%s\\*%s", g_gamedir, MODELSRC_DIR, MODELSRC_EXTENSION);

		//We add the model name before the command line (e.g: "C:\Half Life 2\hl2\modelsrc\test.qc" -game "C:\Half Life 2\hl2")
		V_snprintf(tool_commands, sizeof(tool_commands), "%s %s", searchPath, tool_commands);

		WIN32_FIND_DATAA findFileData;
		HANDLE hFind = FindFirstFileA(searchPath, &findFileData);

		if (hFind == INVALID_HANDLE_VALUE) 
		{
			Warning("AssetsSystem -> No files found in \"%s\"\n"
					"AssetsSystem -> Skipping model compile!\n", 
					searchPath);
			return;
		}

		do 
		{
			if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) 
			{
				char filePath[MAX_PATH];
				V_snprintf(filePath, sizeof(filePath), "%s\\%s\\%s", g_gamedir, MODELSRC_DIR, findFileData.cFileName);
				Shared::StartExe(gamebin, bufferSize, "Models", NAME_MODEL_TOOL, tool_commands, complete, error, true);
			}
		} while (FindNextFileA(hFind, &findFileData));

		FindClose(hFind);
		
		end = Plat_FloatTime();

		Msg("\nAssetTools -> Done building %s in %f seconds.\n", "Models", end - start);
	}
}