//========== ------------------------------------------------- ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//
#ifndef PSDWRITER_H
#define PSDWRITER_H

#ifdef _WIN32
#pragma once
#endif // _WIN32


void Write8BitsPsd(const char* pFilename, const char* pLayer1);
void Write16BitsPsd(const char* pFilename, const char* pLayer1);
void Write32BitsPsd(const char* pFilename, const char* pLayer1);


#endif // PSDWRITER_H