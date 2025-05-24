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

extern const char*	g_rgpAssetConvertList[10];
extern const char*	g_rgpEndFileName[12];
extern bool			g_bIsSingleFile;
extern bool			g_bTempalteGeneration;

extern Color		header_color;
extern Color		successful_color;
extern Color		failed;

#endif // PSDMAIN_H