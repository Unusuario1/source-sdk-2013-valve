//=== CaptionBuilder -> Written by Unusuario2, https://github.com/Unusuario2  ==//
//
// Purpose: CaptionBuilder – A ContentBuilder subsystem for batch compiling 
//          and processing closed captions.
//
// $NoKeywords: $
//==============================================================================//
#ifndef ASSETBUILDER_CAPTION_HPP
#define ASSETBUILDER_CAPTION_HPP

#ifdef _WIN32
#pragma once
#endif // _WIN32

#pragma warning(disable : 4238)

#include <filesystem_init.h>
#include <colorschemetools.h>
#include <resourcecopy/cresourcecopy.hpp>
#include "corebuilder.hpp"


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
#define KV_BUILDER_CAPTION         "CaptionBuilder"


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
class CCaptionBuilder : public CCoreBuilder
{
public:
	CCaptionBuilder();
	~CCaptionBuilder();

	void AssetBuilderCompile() override;
	void DeleteCompiledContents();
};


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CCaptionBuilder::CCaptionBuilder() :
	CCoreBuilder(
		KV_BUILDER_CAPTION,
		DIR_CAPTIONSRC,
		DIR_CAPTIONS,
		false,
		g_iThreads,
		TOOL_CAPTION,
		FileList{ FileString { EXT_CAPTIONSRC }},
		EXT_CAPTION
	)
{
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CCaptionBuilder::~CCaptionBuilder()
{
}


//-----------------------------------------------------------------------------
// Purpose:	Setup the enviroment for captioncompiler.exe to start & compile
//-----------------------------------------------------------------------------
void CCaptionBuilder::AssetBuilderCompile()
{
	CCoreBuilder::PrintHeaderCompileType();

	if (!CCoreBuilder::m_bRunAssetCompile)
		return;

	CCoreBuilder::AssetBuilderCompile();

	// Copy all the .dat files to /resources
	{
		char szWildCard[MAX_PATH];
		V_sprintf_safe(szWildCard, "%s\\*%s", m_szGameAssetSrcPath, EXT_CAPTION);
		g_pResourceCopy->TransferDirTo(szWildCard, m_szGameAssetDstPath, false);
	}

	CCoreBuilder::GenerateGlobalOperationReport();
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CCaptionBuilder::DeleteCompiledContents()
{
	Msg("AssetSystem%s -> Deleting Caption compiled files\n", m_szKeyValue);
	
	char szWildCard[MAX_PATH];
	V_sprintf_safe(szWildCard, "%s\\*%s", CCoreBuilder::m_szGameAssetDstPath, CCoreBuilder::m_szCompiledExtension);
	g_pResourceCopy->DeleteDirRecursive(szWildCard);
}


#endif // ASSETBUILDER_CAPTION_HPP

