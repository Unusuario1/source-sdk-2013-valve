//==== VpkBuilder -> Written by Unusuario2, https://github.com/Unusuario2  =====//
//
// Purpose: VpkBuilder – A ContentBuilder subsystem for vpk creation.
//
// $NoKeywords: $
//==============================================================================//
#ifndef ASSETBUILDER_VPK_HPP
#define ASSETBUILDER_VPK_HPP

#ifdef _WIN32
#pragma once
#endif // _WIN32

#include <filesystem_init.h>
#include <colorschemetools.h>
#include <pipeline_shareddefs.h>
#include <KeyValues.h>
#include "corebuilder.hpp"

#pragma warning(disable : 4238)


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
#define KV_BUILDER_VPK             "VpkBuilder"
#define KV_VPK_EXCLUDE_ROOT        "Exclude"
#define KV_VPK_EXCLUDE_CHILD       "ExcludeFileOrFolder"
#define KV_VPK_ADD_CONTENT         "AddFileOrFolder"
#define BUILDER_VPK_TEMPDIR        "_temp_vpk"


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
class CVpkBuilder : public CCoreBuilder
{
private:
	FileList CVpkBuilder::LoadAdditionalGameInfoKv();

public:
	CVpkBuilder();
	~CVpkBuilder();

	void AssetBuilderCompile() override;
	void GenerateAssetReport() override;
};


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CVpkBuilder::CVpkBuilder() :
	CCoreBuilder(
		KV_BUILDER_VPK,
		true,
		1,
		TOOL_VPK
	)
{
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CVpkBuilder::~CVpkBuilder()
{
}


//-----------------------------------------------------------------------------
// Purpose: Load 'Exclude' Kv inside VpkBuilder
//-----------------------------------------------------------------------------
FileList CVpkBuilder::LoadAdditionalGameInfoKv()
{
	FileList VTemp;

	KeyValues* pContentBuilderRead = new KeyValues("");
	if (!pContentBuilderRead->LoadFromFile(g_pFullFileSystem, g_szContentBuilderScriptFile))
	{
		g_pConsoleLogger->Warning("\nAssetSystem%s -> Failed to read file: %s\n"
			"AssetSystem%s -> Using default values for asset compile, this might not be ideal!\n",
			m_szKeyValue, g_szContentBuilderScriptFile, m_szKeyValue);
		return VTemp;
	}

	KeyValues* pKv = pContentBuilderRead->FindKey(m_szKeyValue, false);
	if (!pKv)
	{
		g_pConsoleLogger->Warning("\nAssetSystem%s -> Could not get \'%s\' KeyValues from \"%s\"!\n"
			"AssetSystem%s -> Using default values for asset compile, this might not be ideal!\n",
			m_szKeyValue, g_szContentBuilderScriptFile, m_szKeyValue);
		return VTemp;
	}

	KeyValues* pExcludeKvRoot = pKv->FindKey(KV_VPK_EXCLUDE_ROOT, false);
	if (pExcludeKvRoot)
	{
		for(KeyValues* pKv = pExcludeKvRoot->GetFirstSubKey(); pKv; pKv = pKv->GetNextKey())
		{
			const char* pExcludeString = pKv->GetString();
			if(pExcludeString)
			{
				char* pTemp = V_strdup(pExcludeString);
				V_FixSlashes(pTemp);
				FileString Temp;
				V_strncpy(Temp.data(), pTemp, Temp.size());
				VTemp.push_back(Temp);
				delete[] pTemp;
			}
		}
	}
	else
	{
		g_pConsoleLogger->Warning("\nAssetSystem%s -> Could not get \'%s\' KeyValues from \"%s\"!\n"
			"AssetSystem%s -> Using default values for asset compile, this might not be ideal!\n",
			m_szKeyValue, KV_VPK_EXCLUDE_ROOT, m_szKeyValue, m_szKeyValue);
		return VTemp;
	}

	return VTemp;
}


// TODO: overwrite GenerateAssetReport!
//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CVpkBuilder::AssetBuilderCompile()
{
	char szVpkTempPath[MAX_PATH];
	char szModVpkTempPath[MAX_PATH];
	char szFullCommandLine[MAX_PATH];

	CCoreBuilder::PrintHeaderCompileType();
	if (!CCoreBuilder::m_bRunAssetCompile)
		return;

	FileList VFileList;
	{
		Msg("AssetSystem%s -> Creating list of files to pack in the vpk... \n", m_szKeyValue);

		{
			char szWildCard[MAX_PATH];
			V_sprintf_safe(szWildCard, "%s\\*", m_szGameDirPath);
			VFileList = g_pResourceCopy->ScanDirectoryRecursive(szWildCard);
		}

		if (VFileList.empty())
		{
			g_pConsoleLogger->Warning("\nVFileList empty!! THIS SHOULD NOT HAPPEN IN ANY WAY!!\n"
				"Make an issue in the github repo!\n");
			return;
		}

		// Now compare all the strings with the strings we want to exclude
		const FileList VFileOrFolderToExclude = LoadAdditionalGameInfoKv();
		for (const FileString& ExcludeString : VFileOrFolderToExclude)
		{
			for (int j = 0; j < VFileList.size(); j++)
			{
				if (V_strstr(VFileList[j].data(), ExcludeString.data()))
				{
					VFileList.erase(VFileList.begin() + j);
					j--;
				}
			}
		}
	}

	// get the mod name, and set up the temp dir
	{
		Msg("AssetSystem%s -> Creating temp files for vpk build (this may take a while)... \n", m_szKeyValue);
		V_sprintf_safe(szVpkTempPath, "%s\\%s", g_szContentBuilderOutPath, BUILDER_VPK_TEMPDIR);
		V_sprintf_safe(szModVpkTempPath, "%s\\%s", szVpkTempPath, V_strrchr(m_szGameDirPath, '\\') + 1 /*Mod Name*/);
		g_pResourceCopy->CreateDir(szModVpkTempPath);
		g_pResourceCopy->CopyDirTo(m_szGameDirPath, szModVpkTempPath, true, true, &VFileList);

		char szToolArgv[MAX_CMD_BUFFER_SIZE];
		CCoreBuilder::LoadGameInfoKv(szToolArgv);
		V_sprintf_safe(szFullCommandLine, "%s\\%s %s %s", m_szGameDirToolsPath, m_szToolName, szToolArgv, szModVpkTempPath);
	}

	Msg("\n");
	CCoreBuilder::StartExe(szFullCommandLine);
	Msg("\n");

	Msg("AssetSystem%s -> Deleting temp files for vpk build (this may take a while)... \n\n", m_szKeyValue);
	{
		char szWildCard[MAX_PATH];
		V_sprintf_safe(szWildCard, "%s\\*", szModVpkTempPath);
		g_pResourceCopy->DeleteDirRecursive(szWildCard, true, true);
	}
	Msg("AssetSystem%s -> Transfering vpk files... \n\n", m_szKeyValue);
	g_pResourceCopy->TransferDirTo(szVpkTempPath, m_szGameDirPath, true, true);

	CCoreBuilder::GenerateGlobalOperationReport();
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CVpkBuilder::GenerateAssetReport()
{
	// TODO: Change this! we run this two times!
	FileList VFileList;
	{
		char szWildCard[MAX_PATH];
		V_sprintf_safe(szWildCard, "%s\\*", m_szGameDirPath);
		VFileList = GenerateBuildingListAssets(szWildCard);
	}

	char szBuilderSubpath[MAX_PATH];
	{
		char* pTemp = V_strdup(m_szKeyValue);
		V_sprintf_safe(szBuilderSubpath, "%s\\%s\\assetreport%s.txt", m_szGameDirPath, BUILDER_OUTDIR, strlwr(pTemp));
		delete[] pTemp;
	}

	FILE* fp = fopen(szBuilderSubpath, "w");
	if (!fp)
	{
		g_pConsoleLogger->Warning("AssetSystem%s -> Could not create: %s!\n"
			"AssetSystem%s -> Skipping saving the output of the console!\n",
			m_szKeyValue, szBuilderSubpath, m_szKeyValue);
		return;
	}

	for (auto File : VFileList)
	{
		fprintf(fp, "%s\n", File.data());
	}

	fclose(fp);
}

#endif // ASSETBUILDER_VPK_HPP

