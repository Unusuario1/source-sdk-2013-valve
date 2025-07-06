//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Look up table, used for ambient cubes/detail props and 
//			test trace sky in VRAD.
//
//=============================================================================//
#if !defined(_STATIC_LINKED) || defined(_SHARED_LIB)
#include "mathlib/vector.h"
#include "mathlib/mathlib.h"
#include "mathlib/anorms.h"
#include "tier1/strtools.h"
#include "../utils/common/cmdlib.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


Vector* g_anorms;


//-----------------------------------------------------------------------------
// Legacy lookup table for unit sphere normals.
// Note: This approximation is based on recursive subdivision of an icosahedron
//       to simulate a spherical distribution of normals.
//-----------------------------------------------------------------------------
#define NUMVERTEXNORMALS	162
#define VERTEXNORMAL_CONE_INNER_ANGLE	DEG2RAD(7.275) // The angle between consecutive g_anorms[] vectors is ~14.55 degrees

Vector g_anormsLegacyLUT[NUMVERTEXNORMALS] =
{
	Vector(-0.525731, 0.000000, 0.850651), 
	Vector(-0.442863, 0.238856, 0.864188), 
	Vector(-0.295242, 0.000000, 0.955423), 
	Vector(-0.309017, 0.500000, 0.809017), 
	Vector(-0.162460, 0.262866, 0.951056), 
	Vector(0.000000, 0.000000, 1.000000), 
	Vector(0.000000, 0.850651, 0.525731), 
	Vector(-0.147621, 0.716567, 0.681718), 
	Vector(0.147621, 0.716567, 0.681718), 
	Vector(0.000000, 0.525731, 0.850651), 
	Vector(0.309017, 0.500000, 0.809017), 
	Vector(0.525731, 0.000000, 0.850651), 
	Vector(0.295242, 0.000000, 0.955423), 
	Vector(0.442863, 0.238856, 0.864188), 
	Vector(0.162460, 0.262866, 0.951056), 
	Vector(-0.681718, 0.147621, 0.716567), 
	Vector(-0.809017, 0.309017, 0.500000), 
	Vector(-0.587785, 0.425325, 0.688191), 
	Vector(-0.850651, 0.525731, 0.000000), 
	Vector(-0.864188, 0.442863, 0.238856), 
	Vector(-0.716567, 0.681718, 0.147621), 
	Vector(-0.688191, 0.587785, 0.425325), 
	Vector(-0.500000, 0.809017, 0.309017), 
	Vector(-0.238856, 0.864188, 0.442863), 
	Vector(-0.425325, 0.688191, 0.587785), 
	Vector(-0.716567, 0.681718, -0.147621), 
	Vector(-0.500000, 0.809017, -0.309017), 
	Vector(-0.525731, 0.850651, 0.000000), 
	Vector(0.000000, 0.850651, -0.525731), 
	Vector(-0.238856, 0.864188, -0.442863), 
	Vector(0.000000, 0.955423, -0.295242), 
	Vector(-0.262866, 0.951056, -0.162460), 
	Vector(0.000000, 1.000000, 0.000000), 
	Vector(0.000000, 0.955423, 0.295242), 
	Vector(-0.262866, 0.951056, 0.162460), 
	Vector(0.238856, 0.864188, 0.442863), 
	Vector(0.262866, 0.951056, 0.162460), 
	Vector(0.500000, 0.809017, 0.309017), 
	Vector(0.238856, 0.864188, -0.442863), 
	Vector(0.262866, 0.951056, -0.162460), 
	Vector(0.500000, 0.809017, -0.309017), 
	Vector(0.850651, 0.525731, 0.000000), 
	Vector(0.716567, 0.681718, 0.147621), 
	Vector(0.716567, 0.681718, -0.147621), 
	Vector(0.525731, 0.850651, 0.000000), 
	Vector(0.425325, 0.688191, 0.587785), 
	Vector(0.864188, 0.442863, 0.238856), 
	Vector(0.688191, 0.587785, 0.425325), 
	Vector(0.809017, 0.309017, 0.500000), 
	Vector(0.681718, 0.147621, 0.716567), 
	Vector(0.587785, 0.425325, 0.688191), 
	Vector(0.955423, 0.295242, 0.000000), 
	Vector(1.000000, 0.000000, 0.000000), 
	Vector(0.951056, 0.162460, 0.262866), 
	Vector(0.850651, -0.525731, 0.000000), 
	Vector(0.955423, -0.295242, 0.000000), 
	Vector(0.864188, -0.442863, 0.238856), 
	Vector(0.951056, -0.162460, 0.262866), 
	Vector(0.809017, -0.309017, 0.500000), 
	Vector(0.681718, -0.147621, 0.716567), 
	Vector(0.850651, 0.000000, 0.525731), 
	Vector(0.864188, 0.442863, -0.238856), 
	Vector(0.809017, 0.309017, -0.500000), 
	Vector(0.951056, 0.162460, -0.262866), 
	Vector(0.525731, 0.000000, -0.850651), 
	Vector(0.681718, 0.147621, -0.716567), 
	Vector(0.681718, -0.147621, -0.716567), 
	Vector(0.850651, 0.000000, -0.525731), 
	Vector(0.809017, -0.309017, -0.500000), 
	Vector(0.864188, -0.442863, -0.238856), 
	Vector(0.951056, -0.162460, -0.262866), 
	Vector(0.147621, 0.716567, -0.681718), 
	Vector(0.309017, 0.500000, -0.809017), 
	Vector(0.425325, 0.688191, -0.587785), 
	Vector(0.442863, 0.238856, -0.864188), 
	Vector(0.587785, 0.425325, -0.688191), 
	Vector(0.688191, 0.587785, -0.425325), 
	Vector(-0.147621, 0.716567, -0.681718), 
	Vector(-0.309017, 0.500000, -0.809017), 
	Vector(0.000000, 0.525731, -0.850651), 
	Vector(-0.525731, 0.000000, -0.850651), 
	Vector(-0.442863, 0.238856, -0.864188), 
	Vector(-0.295242, 0.000000, -0.955423), 
	Vector(-0.162460, 0.262866, -0.951056), 
	Vector(0.000000, 0.000000, -1.000000), 
	Vector(0.295242, 0.000000, -0.955423), 
	Vector(0.162460, 0.262866, -0.951056), 
	Vector(-0.442863, -0.238856, -0.864188), 
	Vector(-0.309017, -0.500000, -0.809017), 
	Vector(-0.162460, -0.262866, -0.951056), 
	Vector(0.000000, -0.850651, -0.525731), 
	Vector(-0.147621, -0.716567, -0.681718), 
	Vector(0.147621, -0.716567, -0.681718), 
	Vector(0.000000, -0.525731, -0.850651), 
	Vector(0.309017, -0.500000, -0.809017), 
	Vector(0.442863, -0.238856, -0.864188), 
	Vector(0.162460, -0.262866, -0.951056), 
	Vector(0.238856, -0.864188, -0.442863), 
	Vector(0.500000, -0.809017, -0.309017), 
	Vector(0.425325, -0.688191, -0.587785), 
	Vector(0.716567, -0.681718, -0.147621), 
	Vector(0.688191, -0.587785, -0.425325), 
	Vector(0.587785, -0.425325, -0.688191), 
	Vector(0.000000, -0.955423, -0.295242), 
	Vector(0.000000, -1.000000, 0.000000), 
	Vector(0.262866, -0.951056, -0.162460), 
	Vector(0.000000, -0.850651, 0.525731), 
	Vector(0.000000, -0.955423, 0.295242), 
	Vector(0.238856, -0.864188, 0.442863), 
	Vector(0.262866, -0.951056, 0.162460), 
	Vector(0.500000, -0.809017, 0.309017), 
	Vector(0.716567, -0.681718, 0.147621), 
	Vector(0.525731, -0.850651, 0.000000), 
	Vector(-0.238856, -0.864188, -0.442863), 
	Vector(-0.500000, -0.809017, -0.309017), 
	Vector(-0.262866, -0.951056, -0.162460), 
	Vector(-0.850651, -0.525731, 0.000000), 
	Vector(-0.716567, -0.681718, -0.147621), 
	Vector(-0.716567, -0.681718, 0.147621), 
	Vector(-0.525731, -0.850651, 0.000000), 
	Vector(-0.500000, -0.809017, 0.309017), 
	Vector(-0.238856, -0.864188, 0.442863), 
	Vector(-0.262866, -0.951056, 0.162460), 
	Vector(-0.864188, -0.442863, 0.238856), 
	Vector(-0.809017, -0.309017, 0.500000), 
	Vector(-0.688191, -0.587785, 0.425325), 
	Vector(-0.681718, -0.147621, 0.716567), 
	Vector(-0.442863, -0.238856, 0.864188), 
	Vector(-0.587785, -0.425325, 0.688191), 
	Vector(-0.309017, -0.500000, 0.809017), 
	Vector(-0.147621, -0.716567, 0.681718), 
	Vector(-0.425325, -0.688191, 0.587785), 
	Vector(-0.162460, -0.262866, 0.951056), 
	Vector(0.442863, -0.238856, 0.864188), 
	Vector(0.162460, -0.262866, 0.951056), 
	Vector(0.309017, -0.500000, 0.809017), 
	Vector(0.147621, -0.716567, 0.681718), 
	Vector(0.000000, -0.525731, 0.850651), 
	Vector(0.425325, -0.688191, 0.587785), 
	Vector(0.587785, -0.425325, 0.688191), 
	Vector(0.688191, -0.587785, 0.425325), 
	Vector(-0.955423, 0.295242, 0.000000), 
	Vector(-0.951056, 0.162460, 0.262866), 
	Vector(-1.000000, 0.000000, 0.000000), 
	Vector(-0.850651, 0.000000, 0.525731), 
	Vector(-0.955423, -0.295242, 0.000000), 
	Vector(-0.951056, -0.162460, 0.262866), 
	Vector(-0.864188, 0.442863, -0.238856), 
	Vector(-0.951056, 0.162460, -0.262866), 
	Vector(-0.809017, 0.309017, -0.500000), 
	Vector(-0.864188, -0.442863, -0.238856), 
	Vector(-0.951056, -0.162460, -0.262866), 
	Vector(-0.809017, -0.309017, -0.500000), 
	Vector(-0.681718, 0.147621, -0.716567), 
	Vector(-0.681718, -0.147621, -0.716567), 
	Vector(-0.850651, 0.000000, -0.525731), 
	Vector(-0.688191, 0.587785, -0.425325), 
	Vector(-0.587785, 0.425325, -0.688191), 
	Vector(-0.425325, 0.688191, -0.587785), 
	Vector(-0.425325, -0.688191, -0.587785), 
	Vector(-0.587785, -0.425325, -0.688191), 
	Vector(-0.688191, -0.587785, -0.425325)
};


//-----------------------------------------------------------------------------
// Purpose: Generates an N amount of point in the sphere surface and then 
//			creates a Look up table for VRAD to use.
// Note: This funtion uses the Fibonacci Sphere Algorithm: https://arxiv.org/pdf/0912.4540 
//-----------------------------------------------------------------------------
void GenerateUnitSphereLookUpTable(const bool g_bGenerateUnitSphereVector, const bool g_bDumpGeneratedUnitSphereVectors, 
								  const char* cSourceDir, int &g_iUnitSpherePoints, float &g_fUnitSphereVectorVertexInnerAngle)
{
	float start = Plat_FloatTime();

	if (!g_bGenerateUnitSphereVector)
	{
		g_anorms = g_anormsLegacyLUT;
		g_iUnitSpherePoints = NUMVERTEXNORMALS;
		g_fUnitSphereVectorVertexInnerAngle = VERTEXNORMAL_CONE_INNER_ANGLE;
		return;
	}

	Msg("Generating unit sphere vector lookup table (LUT)... ");
	
	const int nVector = g_iUnitSpherePoints;
	float fGoldenAngle = M_PI * (3 - sqrt(5));
	Vector* UnitaryLUT = new Vector[nVector];

	for (int i = 0; i < nVector; ++i)
	{
		float z = 1 - (2.0 * i + 1.0) / static_cast<float>(nVector);
		float fRadius = sqrt(1 - z * z);
		float ftheta = fGoldenAngle * i;

		float x = fRadius * cos(ftheta);
		float y = fRadius * sin(ftheta);

		UnitaryLUT[i] = Vector(x, y, z).Normalized();
	}

	g_anorms = UnitaryLUT;
	Msg("done(%.2fs)\n", Plat_FloatTime() - start);

	if (g_bDumpGeneratedUnitSphereVectors)
	{
		start = Plat_FloatTime();
		char szTemp[MAX_PATH];

		V_strcpy(szTemp, cSourceDir);
		V_StripLastDir(szTemp, MAX_PATH);
		V_snprintf(szTemp, MAX_PATH, "%sdumpsphereLUT.txt", szTemp);
		remove(szTemp);

		Msg("Dumping unit sphere vector data... ");

		FILE* fp = fopen(szTemp, "a");
		fprintf(fp, "// THIS FILE IS A AUTO-GENERATED UNIT VECTOR DUMP.\n");

		for (int i = 0; i < nVector; ++i) 
		{
			fprintf(fp, "%f, %f, %f\n", UnitaryLUT[i].x, UnitaryLUT[i].y, UnitaryLUT[i].z);
		}

		fclose(fp);
		Msg("done(%.2fs)\n", Plat_FloatTime() - start);
	}

	// We want to measure how tightly the points are packed on the surface of the unit sphere.
	// For each normal, find its nearest neighbor (i.e., the one with the smallest angular distance).
	// Then compute the average of these smallest angles across all normals.
	// This gives an estimate of the "inner cone angle" � the minimum angle required to ensure
	// that every direction on the sphere is within reach of at least one sampled normal.
	start = Plat_FloatTime();
	Msg("Finding the smallest angle across the unit sphere normals... ");

	const float fAngle = 1.0 / nVector;
	float fAngleSum = 0;
	for (int i = 0; i < nVector; ++i)
	{
		float maxDot = -1.0f; 
		for (int j = 0; j < nVector; ++j)
		{
			if (i == j)
			{
				continue;
			}
			float dot = DOT_PRODUCT(UnitaryLUT[i], UnitaryLUT[j]);
			if (dot > maxDot)
			{
				maxDot = dot; // find largest dot product (closest neighbor)
			}
		}
		float angle = acos(maxDot);
		fAngleSum += angle;
	}
	float averageAngle = fAngleSum / nVector;
	g_fUnitSphereVectorVertexInnerAngle = averageAngle;

	Msg("done(%.2fs)\n", Plat_FloatTime() - start);
}

#endif // !_STATIC_LINKED || _SHARED_LIB
