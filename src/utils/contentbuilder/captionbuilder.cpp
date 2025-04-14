#include <cstddef>
#include <windows.h>
#include "filesystem_init.h"

#include "captionbuilder.h"
#include "colorscheme.h"
#include "shared.h"


namespace CaptionBuilder
{
	//Does captioncompiler.exe exist?
	void AssetToolCheck(const char* gamebin)
	{
		Shared::AssetToolCheck(gamebin, NAME_CAPTION_TOOL, "CaptionBuilder");
	}


	void LoadGameInfoKv(char* tool_argv, std::size_t bufferSize)
	{
		char _argv2[2048] = "";

		Shared::LoadGameInfoKv(CAPTIONBUILDER_KV, _argv2, sizeof(_argv2));

		// Note we still need to add the closecaption.txt to the path! (we do this in CaptionCompile)
		V_snprintf(tool_argv, bufferSize, " %s %s %s -game \"%s\"", DEFAULT_CAPTION_COMMANDLINE, TOOL_VERBOSE_MODE, _argv2, g_gamedir);
	}


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

				V_snprintf(szTemp, sizeof(szTemp), "%s \"%s\"", tool_commands, fullPath);
				Shared::StartExe(gamebin, bufferSize, "Captions", NAME_CAPTION_TOOL, szTemp, complete, error, false);
			}
		} while (FindNextFileA(hFind, &findFileData));

		FindClose(hFind);
	}


	void CaptionCompile(const char* gamebin, std::size_t bufferSize, std::size_t &complete, std::size_t &error)
	{
		char tool_commands[4096] = "", searchPath[MAX_PATH] = "", captionsrcpath[MAX_PATH] = "";
		bool bContinue = true;

		ColorSpewMessage(SPEW_MESSAGE, &header_color, "\n====== Building %s ======\n", "Captions");

		V_snprintf(searchPath, sizeof(searchPath), "%s\\%s", g_gamedir, CAPTION_DIR);
		bContinue = Shared::DirectoryAssetTypeExist(searchPath, CAPTIONSRC_EXTENSION, "captions");
		if (!bContinue)
			return;

		Msg("Asset report:\n");
		Shared::AssetInfoBuild(searchPath, CAPTIONSRC_EXTENSION);
		if (g_infocontent)
			return;

		CaptionBuilder::LoadGameInfoKv(tool_commands, sizeof(tool_commands));
		
		CaptionProcessRec(gamebin, bufferSize, searchPath, tool_commands, complete, error, CAPTIONSRC_EXTENSION);
	}
}