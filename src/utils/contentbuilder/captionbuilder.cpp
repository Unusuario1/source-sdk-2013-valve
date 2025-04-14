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
	void AssetToolCheck(const char* gamebin)
	{
		Shared::AssetToolCheck(gamebin, NAME_CAPTION_TOOL, "CaptionBuilder");
	}


	//-----------------------------------------------------------------------------
	// Purpose: Loads additional command-line parameters defined in the 'CaptionBuilder'
	//          section of the GameInfo KeyValues file, and constructs the full
	//          command line for the tool (e.g -v -game "C:\Half Life 2\hl2")
	//-----------------------------------------------------------------------------
	void LoadGameInfoKv(char* tool_argv, std::size_t bufferSize)
	{
		char _argv2[2048] = "";

		Shared::LoadGameInfoKv(CAPTIONBUILDER_KV, _argv2, sizeof(_argv2));

		// Note we still need to add the closecaption.txt to the path! (we do this in CaptionCompile)
		V_snprintf(tool_argv, bufferSize, " %s %s %s -game \"%s\"", DEFAULT_CAPTION_COMMANDLINE, TOOL_VERBOSE_MODE, _argv2, gamedir);
	}


	//-----------------------------------------------------------------------------
	// Purpose:	Compile all the assets found in the given directory
	//-----------------------------------------------------------------------------
	void CaptionProcessRec(const char* gamebin, std::size_t bufferSize, const char* directory,
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
				CaptionProcessRec(gamebin, bufferSize, fullPath, tool_commands, complete, error, extension);
			}
			else if (Shared::HasExtension(name, extension))
			{
				char szTemp[4096];

				if (!Shared::PartialBuildAsset(fullPath, CAPTIONSRC_DIR, CAPTION_DIR))
					continue;

				V_snprintf(szTemp, sizeof(szTemp), "%s \"%s\"", tool_commands, fullPath);
				Shared::StartExe(gamebin, bufferSize, "Captions", NAME_CAPTION_TOOL, szTemp, complete, error, false);
			}
		} while (FindNextFileA(hFind, &findFileData));

		FindClose(hFind);
	}


	//-----------------------------------------------------------------------------
	// Purpose:	Setup the enviroment for captioncompiler.exe to start & compile
	//-----------------------------------------------------------------------------
	void CaptionCompile(const char* gamebin, std::size_t bufferSize, std::size_t &complete, std::size_t &error)
	{
		char tool_commands[4096] = "", captionSrcPath[MAX_PATH] = "";
		bool bContinue = true;

		Shared::PrintHeaderCompileType("Captions");

		V_snprintf(captionSrcPath, sizeof(captionSrcPath), "%s\\%s", gamedir, CAPTIONSRC_DIR); // (e.g: "C:\Half Life 2\hl2\resource")

		bContinue = Shared::DirectoryAssetTypeExist(captionSrcPath, CAPTIONSRC_EXTENSION, "captions");
		if (!bContinue)
			return;

		Msg("Asset report:\n");
		Shared::AssetInfoBuild(captionSrcPath, CAPTIONSRC_EXTENSION);
		if (g_infocontent)
			return;

		CaptionBuilder::LoadGameInfoKv(tool_commands, sizeof(tool_commands));
		CaptionProcessRec(gamebin, bufferSize, captionSrcPath, tool_commands, complete, error, CAPTIONSRC_EXTENSION);
	}
}