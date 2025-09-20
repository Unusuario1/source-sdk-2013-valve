//=== MaterialBuilder -> Written by Unusuario2, https://github.com/Unusuario2  ==//
//
// Purpose: MaterialBuilder – A ContentBuilder subsystem for texture batch compiling.
//
// $NoKeywords: $
//==============================================================================//
#ifndef ASSETBUILDER_MATERIAL_HPP
#define ASSETBUILDER_MATERIAL_HPP

#ifdef _WIN32
#pragma once
#endif // _WIN32

#pragma warning(disable : 4238)

#include <filesystem_init.h>
#include <colorschemetools.h>
#include <pipeline_shareddefs.h>
#include "corebuilder.hpp"

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
#define KV_BUILDER_MATERIAL        "MaterialBuilder"


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
class CMaterialBuilder : public CCoreBuilder
{
private:
	void CopySrcVmtToGameDir();

public:
	CMaterialBuilder();
	~CMaterialBuilder();

	void AssetBuilderCompile() override;
};


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CMaterialBuilder::CMaterialBuilder() :
	CCoreBuilder(
		KV_BUILDER_MATERIAL,
		DIR_MATERIALSRC,
		DIR_MATERIALS,
		false,
		g_iThreads,
		TOOL_MATERIAL,
		FileList{ FileString {EXT_TEXTURESRC_PFM},
				  FileString {EXT_TEXTURESRC_PSD},
				  FileString {EXT_TEXTURESRC_TGA}
				 },
		EXT_TEXTURE 
	)
{
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CMaterialBuilder::~CMaterialBuilder()
{
}


//-----------------------------------------------------------------------------
// Purpose: Copy all the .vmt of game/mod/materialsrc to game/mod/materials folder
//-----------------------------------------------------------------------------
void CMaterialBuilder::CopySrcVmtToGameDir()
{
	Msg("AssetSystemMaterialBuilder -> Copying .vmt files from "); ColorSpewMessage(SPEW_MESSAGE, &ColorPath, "%s", CCoreBuilder::m_szGameAssetSrcPath); Msg(" to "); ColorSpewMessage(SPEW_MESSAGE, &ColorPath, "%s\n", CCoreBuilder::m_szGameAssetDstPath);

	char szMaterialSrcDir[MAX_PATH];
	V_sprintf_safe(szMaterialSrcDir, "%s\\*%s", CCoreBuilder::m_szGameAssetSrcPath, EXT_MATERIAL);

	g_pResourceCopy->CopyDirTo(szMaterialSrcDir , CCoreBuilder::m_szGameAssetDstPath, true, true, &CCoreBuilder::GenerateBuildingListAssets(EXT_MATERIAL));
	Msg("\n");
}


//-----------------------------------------------------------------------------
// Purpose:	Generate & add a list of all the textures that need to be compiled.
//-----------------------------------------------------------------------------
void CMaterialBuilder::AssetBuilderCompile()
{
	CCoreBuilder::PrintHeaderCompileType();

	if (!CCoreBuilder::m_bRunAssetCompile)
		return;

	CMaterialBuilder::CopySrcVmtToGameDir();
	CCoreBuilder::AssetBuilderCompile();

	CCoreBuilder::GenerateGlobalOperationReport();
}


#endif // ASSETBUILDER_MATERIAL_HPP
