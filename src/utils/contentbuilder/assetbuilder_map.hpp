//===== MapBuilder -> Written by Unusuario2, https://github.com/Unusuario2  ====//
//
// Purpose: MapBuilder – A ContentBuilder subsystem for map batch compiling.
//
// $NoKeywords: $
//==============================================================================//
#ifndef ASSETBUILDER_MAP_HPP
#define ASSETBUILDER_MAP_HPP

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
#define KV_BUILDER_MAP             "MapBuilder"
#define KV_MAP_GEOMETRY            "Vbsp"
#define KV_MAP_VISIBILITY          "Vvis"
#define KV_MAP_RADIOSITY           "Vrad"
#define KV_MAP_INFO                "VbspInfo"


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
class CMapBuilder : public CCoreBuilder
{
public:
	CMapBuilder();
	~CMapBuilder();

	void AssetBuilderCompile() override;
};


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CMapBuilder::CMapBuilder() :
	CCoreBuilder(KV_BUILDER_MAP,
				DIR_MAPSRC,
				DIR_MAPS,
				true,
				1, // For mapbuilder we dont use multithreading becouse the tools like, vvis and vrad uses multrithreading.
				TOOL_MAPBUILDER,
				FileList{ FileString { EXT_MAPSRC_VMN },
					      FileString { EXT_MAPSRC_VMF} },
				EXT_MAP
	)
{
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CMapBuilder::~CMapBuilder()
{
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CMapBuilder::AssetBuilderCompile()
{
	CCoreBuilder::PrintHeaderCompileType();

	if (!CCoreBuilder::m_bRunAssetCompile)
		return;

	CCoreBuilder::AssetBuilderCompile();
	CCoreBuilder::GenerateGlobalOperationReport();
}


#endif // ASSETBUILDER_MAP_HPP
