//========= --------------------------------------------------- ============//
//
// Purpose: ModelBuilder – A ContentBuilder subsystem for model batch compiling.
//
// $NoKeywords: $
//=============================================================================//
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
	void AssetToolCheck(const char* pGameBin)
	{
		Shared::AssetToolCheck(pGameBin, NAME_MODEL_TOOL, MODELBUILDER_KV);
	}


	//-----------------------------------------------------------------------------
	// Purpose: Loads additional command-line parameters defined in the 'ModelBuilder'
	//          section of the GameInfo KeyValues file, and constructs the full
	//          command line for the tool (e.g -v -game "C:\Half Life 2\hl2")
	//-----------------------------------------------------------------------------
	static void LoadGameInfoKv(char* pToolArgv, std::size_t uiBufferSize)
	{
		char szKvToolArgv[2048] = "";

		Shared::LoadGameInfoKv(MODELBUILDER_KV, szKvToolArgv, sizeof(szKvToolArgv));

		V_snprintf(pToolArgv, uiBufferSize, " %s %s %s -game \"%s\"", DEFAULT_MODEL_COMMANDLINE, TOOL_VERBOSE_OR_QUIET_MODE, szKvToolArgv, gamedir);
	}


	//-----------------------------------------------------------------------------
	// Purpose:	Compile all the assets found in the given directory
	//-----------------------------------------------------------------------------
	static void ModelProcessRec(const char* pDirectory, const char* szToolCommand, const char* pExtension)
	{
		char szSearchPath[MAX_PATH];
		V_snprintf(szSearchPath, sizeof(szSearchPath), "%s\\*", pDirectory);

		WIN32_FIND_DATAA findFileData;
		HANDLE hFind = FindFirstFileA(szSearchPath, &findFileData);
		if (hFind == INVALID_HANDLE_VALUE)
			return;

		do
		{
			const char* pName = findFileData.cFileName;
			if (V_strcmp(pName, ".") == 0 || V_strcmp(pName, "..") == 0)
				continue;

			char szFullPath[MAX_PATH];
			V_snprintf(szFullPath, sizeof(szFullPath), "%s\\%s", pDirectory, pName);

			if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				ModelProcessRec(szFullPath, szToolCommand, pExtension);
			}
			else if (Shared::HasExtension(pName, pExtension))
			{
				char szTemp[4096];

				if (!Shared::PartialBuildAsset(szFullPath, MODELSRC_DIR, MODELS_DIR))
					continue;

				// Exclude folder!
				if (Shared::ExcludeDirOrFile(szFullPath))
					continue;

				V_snprintf(szTemp, sizeof(szTemp), "%s \"%s\"", szToolCommand, szFullPath);
				Shared::StartExe("Models", NAME_MODEL_TOOL, szTemp);
			}
		} while (FindNextFileA(hFind, &findFileData));

		FindClose(hFind);
	}


	//-----------------------------------------------------------------------------
	// Purpose:	Setup the enviroment for studiomdl.exe to start & compile
	//-----------------------------------------------------------------------------
	void ModelCompile()
	{	
		char szToolCommand[4096] = "", szModelSrcPath[MAX_PATH] = "";
		bool bContinue = true;

		Shared::PrintHeaderCompileType("Models");

		V_snprintf(szModelSrcPath, sizeof(szModelSrcPath), "%s\\%s", gamedir, MODELSRC_DIR); // (e.g: "C:\Half Life 2\hl2\modelsrc")

		bContinue = Shared::DirectoryAssetTypeExist(szModelSrcPath, MODELSRC_EXTENSION, "models");
		if (!bContinue)
			return;

		Msg("%s", (g_spewallcommands) ? "Asset report:\n" : "");
		Shared::AssetInfoBuild(szModelSrcPath, MODELSRC_EXTENSION);
		if (g_infocontent)
			return;

		ModelBuilder::LoadGameInfoKv(szToolCommand, sizeof(szToolCommand));
		ModelProcessRec(szModelSrcPath, szToolCommand, MODELSRC_EXTENSION);
	}
}