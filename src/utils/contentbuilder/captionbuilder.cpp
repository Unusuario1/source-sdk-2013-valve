//========= --------------------------------------------------- ============//
//
// Purpose: CaptionBuilder – A ContentBuilder subsystem for batch compiling 
//          and processing closed captions.
//
// $NoKeywords: $
//=============================================================================//
#include <cstddef>
#include <windows.h>
#include "filesystem_init.h"

#include "captionbuilder.h"
#include "colorscheme.h"
#include "shared.h"


namespace CaptionBuilder
{
	//-----------------------------------------------------------------------------
	// Purpose:	Check if captioncompiler.exe exists
	//-----------------------------------------------------------------------------
	void AssetToolCheck(const char* pGameBin)
	{
		Shared::AssetToolCheck(pGameBin, NAME_CAPTION_TOOL, CAPTIONBUILDER_KV);
	}


	//-----------------------------------------------------------------------------
	// Purpose: Loads additional command-line parameters defined in the 'CaptionBuilder'
	//          section of the GameInfo KeyValues file, and constructs the full
	//          command line for the tool (e.g -v -game "C:\Half Life 2\hl2")
	//-----------------------------------------------------------------------------
	static void LoadGameInfoKv(char* pToolArgv, std::size_t uiBufferSize)
	{
		char szKvToolArgv[2048] = "";

		Shared::LoadGameInfoKv(CAPTIONBUILDER_KV, szKvToolArgv, sizeof(szKvToolArgv));

		V_snprintf(pToolArgv, uiBufferSize, " %s %s %s -game \"%s\"", DEFAULT_CAPTION_COMMANDLINE, TOOL_VERBOSE_MODE, szKvToolArgv, gamedir);
	}


	//-----------------------------------------------------------------------------
	// Purpose:	Compile all the assets found in the given directory
	//-----------------------------------------------------------------------------
	static void CaptionProcessRec(const char* pDirectory, const char* szToolCommand, const char* pExtension)
	{
		char szSearchPath[MAX_PATH];
		V_snprintf(szSearchPath, sizeof(szSearchPath), "%s\\*", pDirectory);

		WIN32_FIND_DATAA findFileData;
		HANDLE hFind = FindFirstFileA(szSearchPath, &findFileData);
		if (hFind == INVALID_HANDLE_VALUE)
			return;

		do
		{
			const char* name = findFileData.cFileName;
			if (V_strcmp(name, ".") == 0 || V_strcmp(name, "..") == 0)
				continue;

			char szFullPath[MAX_PATH];
			V_snprintf(szFullPath, sizeof(szFullPath), "%s\\%s", pDirectory, name);

			if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				CaptionProcessRec(szFullPath, szToolCommand, pExtension);
			}
			else if (Shared::HasExtension(name, pExtension))
			{
				char szTemp[4096];

				if (!Shared::PartialBuildAsset(szFullPath, CAPTIONSRC_DIR, CAPTION_DIR))
					continue;

				// Exclude folder!
				if (Shared::ExcludeDirOrFile(szFullPath))
					continue;

				V_snprintf(szTemp, sizeof(szTemp), "%s \"%s\"", szToolCommand, szFullPath);
				Shared::StartExe("Captions", NAME_CAPTION_TOOL, szTemp);
			}
		} while (FindNextFileA(hFind, &findFileData));

		FindClose(hFind);
	}


	//-----------------------------------------------------------------------------
	// Purpose:	Setup the enviroment for captioncompiler.exe to start & compile
	//-----------------------------------------------------------------------------
	void CaptionCompile()
	{
		char szToolCommand[4096] = "", szCaptionSrcPath[MAX_PATH] = "";
		bool bContinue = true;

		Shared::PrintHeaderCompileType("Captions");

		V_snprintf(szCaptionSrcPath, sizeof(szCaptionSrcPath), "%s\\%s", gamedir, CAPTIONSRC_DIR); // (e.g: "C:\Half Life 2\hl2\resource")

		bContinue = Shared::DirectoryAssetTypeExist(szCaptionSrcPath, CAPTIONSRC_EXTENSION, "captions");
		if (!bContinue)
			return;

		Msg("%s", (g_spewallcommands) ? "Asset report:\n" : "");

		Shared::AssetInfoBuild(szCaptionSrcPath, CAPTIONSRC_EXTENSION);
		if (g_infocontent)
			return;

		CaptionBuilder::LoadGameInfoKv(szToolCommand, sizeof(szToolCommand));

		CaptionProcessRec(szCaptionSrcPath, szToolCommand, CAPTIONSRC_EXTENSION);
	}
}