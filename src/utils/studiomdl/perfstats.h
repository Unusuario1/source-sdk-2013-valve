//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=====================================================================================//
#ifndef PERFSTATS_H
#define PERFSTATS_H

#ifdef _WIN32
#pragma once
#endif // _WIN32


#include "studio.h"
#include "optimize.h"


void SpewPerfStats( studiohdr_t *pStudioHdr, const char *pFilename );


#endif // PERFSTATS_H
