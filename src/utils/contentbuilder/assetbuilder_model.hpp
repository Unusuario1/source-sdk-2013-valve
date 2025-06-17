//==== ModelBuilder -> Written by Unusuario2, https://github.com/Unusuario2  ===//
//
// Purpose: ModelBuilder � A ContentBuilder subsystem for model batch compiling.
//
// $NoKeywords: $
//==============================================================================//
#ifndef ASSETBUILDER_MODEL_HPP
#define ASSETBUILDER_MODEL_HPP

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
#define KV_BUILDER_MODEL	"ModelBuilder"


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
class CModelBuilder : public CCoreBuilder
{
public:
	CModelBuilder();
	~CModelBuilder();

	void AssetBuilderCompile() override;
};


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CModelBuilder::CModelBuilder() :
	CCoreBuilder(KV_BUILDER_MODEL,
				 DIR_MODELSRC,
				 DIR_MODELS,
				 false,
				 g_iThreads, 
				 TOOL_MODEL,
		         FileList{ FileString { EXT_MODELSRC}},
				 EXT_MODEL
	)
{
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CModelBuilder::~CModelBuilder()
{
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CModelBuilder::AssetBuilderCompile()
{
	CCoreBuilder::PrintHeaderCompileType();

	if (!CCoreBuilder::m_bRunAssetCompile)
		return;

	CCoreBuilder::AssetBuilderCompile();

	CCoreBuilder::GenerateGlobalOperationReport();
}



#endif // ASSETBUILDER_MODEL_HPP