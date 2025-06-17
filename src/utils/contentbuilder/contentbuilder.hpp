//== ContentBuilder -> Written by Unusuario2, https://github.com/Unusuario2  ===//
//
// Purpose: 
//
// $NoKeywords: $
//==============================================================================//
#ifndef CONTENTBUILDER_HPP
#define CONTENTBUILDER_HPP

#ifdef _WIN32
#pragma once
#endif // _WIN32

#include <platform.h>
#include <resourcecopy/cresourcecopy.hpp>
#include <consolelogger.hpp>


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
#define KV_BUILDER_MAIN            "ContentBuilder"
#define KV_EXCLUDE                 "Exclude"
#define KV_EXCLUDE_STRING          "ExcludeFileOrFolder"
#define KV_BUILDPARAMS             "BuildParams"

#define BUILDER_CONFIG_FILE        "scripts\\tools\\contentbuilder_settings.txt"
#define BUILDER_SPECIFIC_LOG	   "standart.log"
#define BUILDER_WARNING_LOG        "warning.log"
#define BUILDER_ERROR_LOG          "error.log"
#define BUILDER_ASSET_REPORT_SRC   "asset_report_source.contentlist"
#define BUILDER_ASSET_REPORT_COM   "asset_report_compiled.contentlist"

#define BUILDER_OUTDIR				"_build"


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
extern int  g_iThreads;
extern bool g_bForceBuildContent;
extern char g_szGameBin[MAX_PATH];
extern char g_szContentBuilderOutPath[MAX_PATH];
extern char g_szGameInfoFile[MAX_PATH];
extern char g_szSteamDir[MAX_PATH];
extern char g_szContentBuilderScriptFile[MAX_PATH];
extern CConsoleLogger*	g_pConsoleLogger;
extern SpewMode			g_eSpewMode;
extern CResourceCopy*	g_pResourceCopy;


#endif // CONTENTBUILDER_HPP

