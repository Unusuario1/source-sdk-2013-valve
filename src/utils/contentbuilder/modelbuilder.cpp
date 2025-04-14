#include <cstddef>
#include <windows.h>
#include "filesystem_init.h"

#include "modelbuilder.h"
#include "colorscheme.h"
#include "shared.h"


namespace ModelBuilder
{
	//-----------------------------------------------------------------------------
	// Purpose:	Check if studiomdl.exe exists
	//-----------------------------------------------------------------------------
	void AssetToolCheck(const char* gamebin)
	{
		Shared::AssetToolCheck(gamebin, NAME_MODEL_TOOL, "ModelBuilder");
	}


	//-----------------------------------------------------------------------------
	// Purpose: Loads additional command-line parameters defined in the 'ModelBuilder'
	//          section of the GameInfo KeyValues file, and constructs the full
	//          command line for the tool (e.g -v -game "C:\Half Life 2\hl2")
	//-----------------------------------------------------------------------------
	void LoadGameInfoKv(char* tool_argv, std::size_t bufferSize)
	{
		char _argv2[2048] = "";

		Shared::LoadGameInfoKv(MODELBUILDER_KV, _argv2, sizeof(_argv2));

		V_snprintf(tool_argv, bufferSize, " %s %s %s -game \"%s\"", DEFAULT_MODEL_COMMANDLINE, TOOL_VERBOSE_OR_QUIET_MODE, _argv2, gamedir);
	}


	//-----------------------------------------------------------------------------
	// Purpose:	Compile all the assets found in the given directory
	//-----------------------------------------------------------------------------
	void ModelProcessRec(const char* gamebin, std::size_t bufferSize, const char* directory,
		const char* tool_commands, std::size_t& complete, std::size_t& error, const char* extension)
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
				ModelProcessRec(gamebin, bufferSize, fullPath, tool_commands, complete, error, extension);
			}
			else if (Shared::HasExtension(name, extension))
			{
				char szTemp[4096];

				if (!Shared::PartialBuildAsset(fullPath, MODELSRC_DIR, MODELS_DIR))
					continue;

				V_snprintf(szTemp, sizeof(szTemp), "%s \"%s\"", tool_commands, fullPath);
				Shared::StartExe(gamebin, bufferSize, "Models", NAME_MODEL_TOOL, szTemp, complete, error, false);
			}
		} while (FindNextFileA(hFind, &findFileData));

		FindClose(hFind);
	}


	//-----------------------------------------------------------------------------
	// Purpose:	Setup the enviroment for studiomdl.exe to start & compile
	//-----------------------------------------------------------------------------
	void ModelCompile(const char* gamebin, std::size_t bufferSize, std::size_t &complete, std::size_t &error)
	{	
		char tool_commands[4096] = "", modelSrcPath[MAX_PATH] = "";
		bool bContinue = true;

		Shared::PrintHeaderCompileType("Models");

		V_snprintf(modelSrcPath, sizeof(modelSrcPath), "%s\\%s", gamedir, MODELSRC_DIR); // (e.g: "C:\Half Life 2\hl2\modelsrc")

		bContinue = Shared::DirectoryAssetTypeExist(modelSrcPath, MODELSRC_EXTENSION, "models");
		if (!bContinue)
			return;

		Msg("Asset report:\n");
		Shared::AssetInfoBuild(modelSrcPath, MODELSRC_EXTENSION);
		if (g_infocontent)
			return;

		ModelBuilder::LoadGameInfoKv(tool_commands, sizeof(tool_commands));
		ModelProcessRec(gamebin, bufferSize, modelSrcPath, tool_commands, complete, error, MODELSRC_EXTENSION);
	}
}