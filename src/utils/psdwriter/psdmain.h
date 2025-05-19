//========== ------------------------------------------------- ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//
#ifndef PSDMAIN_H
#define PSDMAIN_H


#ifdef _WIN32
#pragma once
#endif // _WIN32


extern bool			g_bIsSingleFile;
extern bool			g_bSignature;
extern bool			g_bTempalteGeneration;
extern bool			g_bDeleteSource;
extern bool			g_bForceImageBit;
extern float		g_fGlobalTimer;
extern uint8_t		g_uiCompressionType;
extern uint8_t      g_uiForceImageBit;
extern Color		header_color;
extern Color		successful_color;
extern Color		failed;
extern char			g_szSignature[1][128];
extern char			g_szGameMaterialSrcDir[MAX_PATH];
extern const char*	g_rgpAssetConvertList[10];
extern const char*	g_rgpEndFileName[13];


#endif // PSDMAIN_H