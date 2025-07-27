//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef ANORMS_H
#define ANORMS_H

#ifdef _WIN32
#pragma once
#endif

#include "mathlib/vector.h"

extern Vector *g_anorms;

void GenerateUnitSphereLookUpTable(const bool g_bDumpGeneratedUnitSphereVectors, const char* cSourceDir,
								   const int g_iUnitSpherePoints, float& g_fUnitSphereVectorVertexInnerAngle);


#endif // ANORMS_H
