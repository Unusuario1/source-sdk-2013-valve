//========== ------------------------------------------------- ============//
//
// Purpose: Converts .png, .jpg, etc.. to .psd for vtex compile
//
// $NoKeywords: $
//=============================================================================//
#include <io.h>
#include <windows.h>
#include "tier1/strtools.h"
#include "tier0/icommandline.h"
#include "tools_minidump.h"
#include "loadcmdline.h"
#include "cmdlib.h" //R
#include "filesystem_init.h" //R
#include "filesystem_tools.h" //R
#include "color.h" //R
#include "psdwriter.h"
#include "psdmain.h"

// disable annoying warning caused by xlocale(337): warning C4530: C++ exception handler used, but unwind semantics are not enabled. Specify /EHsc
#pragma warning(disable:4530)
#include <string>
#include <sstream>

// psd_sdk lib
#include "psd/Psd.h"
#include "psd/PsdPlatform.h"
#include "psd/PsdMallocAllocator.h"
#include "psd/PsdNativeFile.h"
#include "psd/PsdDocument.h"
#include "psd/PsdColorMode.h"
#include "psd/PsdLayer.h"
#include "psd/PsdChannel.h"
#include "psd/PsdChannelType.h"
#include "psd/PsdLayerMask.h"	//R
#include "psd/PsdVectorMask.h"	//R
#include "psd/PsdLayerMaskSection.h"	//R
#include "psd/PsdImageDataSection.h"
#include "psd/PsdImageResourcesSection.h"
#include "psd/PsdParseLayerMaskSection.h" // R
#include "psd/PsdParseImageDataSection.h"
#include "psd/PsdParseImageResourcesSection.h"
#include "psd/PsdLayerCanvasCopy.h"
#include "psd/PsdInterleave.h"
#include "psd/PsdPlanarImage.h"
#include "psd/PsdExport.h"
#include "psd/PsdExportDocument.h"


/*
Supported native bit-depths for g_rgpAssetConvertList[] formats:

Format     | 8-bit | 16-bit | 32-bit Float | Notes
---------------------------------------------------------------
.jpg       |  Yes  |   No   |      No      | Always 8-bit lossy RGB
.jpeg      |  Yes  |   No   |      No      | Same as .jpg
.png       |  Yes  |  Yes   |      No      | 8/16-bit per channel, no float
.tga       |  Yes  |   No   |      No      | 8-bit only, optional alpha
.bmp       |  Yes  |   No   |      No      | Integer format only (1, 4, 8, 24, 32-bit int)
.gif       |  Yes  |   No   |      No      | 8-bit paletted image (only firt frame)
.hdr       |   No  |   No   |     Yes      | Radiance HDR, native float (RGBE)
.pic       |  Yes  |   No   |     Yes      | Softimage PIC, supports float RGB
.ppm       |  Yes  |  Yes   |      No      | ASCII/binary PPM (RGB), 8/16-bit
.pgm       |  Yes  |  Yes   |      No      | Grayscale only, 8/16-bit

Note:
- stbi_loadf() returns float data for all formats, but only .hdr and .pic store native float.
*/

// TODO: these need to be dymanic alloc uint8_t, uin16_t, float32_t 
// TODO: add here the source image and its depth in SetMetaData
// TODO: inside the funtions add, pLayer1 for example albedo, _normal, _ao, etc...

// TEMP!! REMOVE THIS LATER!!!
const uint16_t IMAGE_WIDTH = 128;
const uint16_t IMAGE_HEIGHT = 128;

// 8 - bits image data
uint8_t** g_multiplyData;
uint8_t** g_xorData;
uint8_t** g_orData;


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
static void AllocVectorImageData(const uint uiBitType /*8,16,32*/)
{
	if (uiBitType == 8)
	{

	}
	else if (uiBitType == 16)
	{

	}
	else if (uiBitType == 32)
	{
		
	}
	else
	{
		Error(	"****Internal ERROR****: \'static void AllocVectorImageData()\' only supports 8, 16, 32 image bits!!\n"
				"Unknow type of image bit: %s\n", 
				uiBitType
		     );
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
static void DestroyAllocVectorImageData(const uint uiBitType /*8,16,32*/)
{
	if (uiBitType == 8)
	{

	}
	else if (uiBitType == 16)
	{

	}
	else if (uiBitType == 32)
	{
		
	}
	else
	{
		Error(	"****Internal ERROR****: \'static void DestroyAllocVectorImageData()\' only supports 8, 16, 32 image bits!!\n"
				"Unknow type of image bit: %s\n", 
				uiBitType
		     );
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
static void IsPsdOK(const char* pFilename, const float startTime)
{
	char szFile[MAX_PATH];
	V_snprintf(szFile, sizeof(szFile), "%s.psd", pFilename);

	if (_access(szFile, 0))
	{
		ColorSpewMessage(SPEW_MESSAGE, &failed, "FAILED - %s\n", szFile);
	}
	else 
	{
		ColorSpewMessage(SPEW_MESSAGE, &successful_color, "OK"); Msg(" - %s (%.2f)\n", szFile, Plat_FloatTime() - startTime);
	}
}


//-----------------------------------------------------------------------------
// Purpose: add metadata to the .psd file
//-----------------------------------------------------------------------------
static void SetMetaData(psd::ExportDocument* pDocument, psd::MallocAllocator allocator /*TODO: maybe fix this?*/, const char* pFilename)
{
	char szPsdWriterDateBuild[32];

	Msg("Adding meta data to the .psd file... ");

	V_snprintf(szPsdWriterDateBuild, sizeof(szPsdWriterDateBuild), "Build date: %s %s", __DATE__, __TIME__);

	AddMetaData(pDocument, &allocator, "---------- software", "--------");
	AddMetaData(pDocument, &allocator, "Auto - generated with psdwriter.exe", szPsdWriterDateBuild);
	AddMetaData(pDocument, &allocator, "Resolution [x, y]:", ""); // put here the resoluction of the image

	//TODO: fix this!
	/*
	if(g_bSignature)
	{
		AddMetaData(pDocument, &allocator, "blah, blah", "");
	}
	*/

	Msg("done\n");
}


//-----------------------------------------------------------------------------
// Purpose:  Writes a 8, 16 or 32 bit .psd file
//-----------------------------------------------------------------------------
static void WriteNBitsPsd(const char* pFilename, const char* pLayer1, const uint uiBitType)
{
	float start = Plat_FloatTime();

	char pPsdFileName[MAX_PATH];
	V_StripExtension(pFilename, pPsdFileName, MAX_PATH);

	wchar_t szBuffer[256];
	swprintf(szBuffer, sizeof(szBuffer), L"%s.psd", pPsdFileName);


	psd::MallocAllocator allocator;
	psd::NativeFile file(&allocator);

	// try opening the file. if it fails, bail out.
	if (!file.OpenWrite(szBuffer))
	{
		Warning("Cannot open file.\n");
		return;
	}

	Msg("Exporting %s as .psd\n", V_strrchr(pFilename, '\\') + 1);

	// write an RGB PSD file, 8-bit
	psd::ExportDocument* document = CreateExportDocument(&allocator, IMAGE_WIDTH, IMAGE_HEIGHT, uiBitType /*8u, 16u, 32u ?*/, psd::exportColorMode::RGB);

	SetMetaData(document, allocator, pFilename);

	// when adding a layer to the document, you first need to get a new index into the layer table.
	// with a valid index, layers can be updated in parallel, in any order.
	// this also allows you to only update the layer data that has changed, which is crucial when working with large data sets.
	Msg("Creating \'%s\' layer... ", pLayer1);
	const uint layer1 = AddLayer(document, &allocator, pLayer1);
	Msg("done\n");
	
	// note that each layer has its own compression type. it is perfectly legal to compress different channels of different layers with different settings.
	// RAW is pretty much just a raw data dump. fastest to write, but large.
	// RLE stores run-length encoded data which can be good for 8-bit channels, but not so much for 16-bit or 32-bit data.
	// ZIP is a good compromise between speed and size.
	// ZIP_WITH_PREDICTION first delta encodes the data, and then zips it. slowest to write, but also smallest in size for most images.
	Msg("Creating RGBA layers: ");
	UpdateLayer(document, &allocator, layer1, psd::exportChannel::RED, 0, 0, IMAGE_WIDTH, IMAGE_HEIGHT, &g_multiplyData[0][0], psd::compressionType::RAW);
	Msg("red, ");
	UpdateLayer(document, &allocator, layer1, psd::exportChannel::GREEN, 0, 0, IMAGE_WIDTH, IMAGE_HEIGHT, &g_multiplyData[0][0], psd::compressionType::RAW);
	Msg("green, ");
	UpdateLayer(document, &allocator, layer1, psd::exportChannel::BLUE, 0, 0, IMAGE_WIDTH, IMAGE_HEIGHT, &g_multiplyData[0][0], psd::compressionType::RAW);
	Msg("blue, ");
	UpdateLayer(document, &allocator, layer1, psd::exportChannel::ALPHA, 0, 0, IMAGE_WIDTH, IMAGE_HEIGHT, &g_multiplyData[0][0], psd::compressionType::RAW);
	Msg("alpha");
	Msg("... done\n");

	Msg("Merging RGBA layers... ");
	// merged image data is optional. if none is provided, black channels will be exported instead.
	UpdateMergedImage(document, &allocator, &g_multiplyData[0][0], &g_xorData[0][0], &g_orData[0][0]);
	Msg("done\n");

	Msg("Writting %u bits .psd file at: %s... ", szBuffer, uiBitType);
	WriteDocument(document, &allocator, &file);
	Msg("done\n");

	DestroyExportDocument(document, &allocator);
	file.Close();

	//Check if the psd exists!
	IsPsdOK(pPsdFileName, start);
}

#if 0
//-----------------------------------------------------------------------------
// Purpose:  Writes a 16 bit .psd file
//-----------------------------------------------------------------------------
static void Write16BitsPsd(const char* pFilename, const char* pLayer1)
{
	uint16_t g_multiplyData16[IMAGE_HEIGHT][IMAGE_WIDTH] = {};
	uint16_t g_xorData16[IMAGE_HEIGHT][IMAGE_WIDTH] = {};
	uint16_t g_orData16[IMAGE_HEIGHT][IMAGE_WIDTH] = {};
	uint16_t g_andData16[IMAGE_HEIGHT][IMAGE_WIDTH] = {};
	uint16_t g_checkerBoardData16[IMAGE_HEIGHT][IMAGE_WIDTH] = {};

	char pPsdFileName[MAX_PATH];
	V_StripExtension(pFilename, pPsdFileName, MAX_PATH);

	wchar_t szBuffer[256];
	swprintf(szBuffer, sizeof(szBuffer), L"%s.psd", pPsdFileName);

	psd::MallocAllocator allocator;
	psd::NativeFile file(&allocator);

	// try opening the file. if it fails, bail out.
	if (!file.OpenWrite(szBuffer))
	{
		Warning("Cannot open file.\n");
		return;
	}

	Msg("Exporting %s as .psd... ", V_strrchr(pFilename, '\\'));

	// write a rgb PSD file, 16-bit.
	psd::ExportDocument* document = CreateExportDocument(&allocator, IMAGE_WIDTH, IMAGE_HEIGHT, 16u, psd::exportColorMode::GRAYSCALE);

	const unsigned int layer1 = AddLayer(document, &allocator, pLayer1);
	UpdateLayer(document, &allocator, layer1, psd::exportChannel::RED, 0, 0, IMAGE_WIDTH, IMAGE_HEIGHT, &g_multiplyData16[0][0], psd::compressionType::RAW);
	UpdateLayer(document, &allocator, layer1, psd::exportChannel::BLUE, 0, 0, IMAGE_WIDTH, IMAGE_HEIGHT, &g_multiplyData16[0][0], psd::compressionType::RAW);
	UpdateLayer(document, &allocator, layer1, psd::exportChannel::GREEN, 0, 0, IMAGE_WIDTH, IMAGE_HEIGHT, &g_multiplyData16[0][0], psd::compressionType::RAW);

	UpdateMergedImage(document, &allocator, &g_multiplyData16[0][0], &g_xorData16[0][0], &g_andData16[0][0]);

	WriteDocument(document, &allocator, &file);
	
	DestroyExportDocument(document, &allocator);
	file.Close();

	Msg("done\n");

	//Check if the psd exists!
	IsPsdOK(pPsdFileName);
}


//-----------------------------------------------------------------------------
// Purpose:  Writes a 32 bit .psd file
//-----------------------------------------------------------------------------
static void Write32BitsPsd(const char* pFilename, const char* pLayer1)
{
	float32_t g_multiplyData32[IMAGE_HEIGHT][IMAGE_WIDTH] = {};
	float32_t g_xorData32[IMAGE_HEIGHT][IMAGE_WIDTH] = {};
	float32_t g_orData32[IMAGE_HEIGHT][IMAGE_WIDTH] = {};
	float32_t g_andData32[IMAGE_HEIGHT][IMAGE_WIDTH] = {};
	float32_t g_checkerBoardData32[IMAGE_HEIGHT][IMAGE_WIDTH] = {};

	char pPsdFileName[MAX_PATH];
	V_StripExtension(pFilename, pPsdFileName, MAX_PATH);

	wchar_t szBuffer[256];
	swprintf(szBuffer, 1024, L"%s.psd", pPsdFileName);

	psd::MallocAllocator allocator;
	psd::NativeFile file(&allocator);

	// try opening the file. if it fails, bail out.
	if (!file.OpenWrite(szBuffer))
	{
		Warning("Cannot open file.\n");
		return;
	}

	Msg("Exporting %s as .psd... ", V_strrchr(pFilename, '\\'));

	// write an RGB PSD file, 32-bit
	psd::ExportDocument* document = CreateExportDocument(&allocator, IMAGE_WIDTH, IMAGE_HEIGHT, 32u, psd::exportColorMode::RGB);

	// TODO: let the user set the compresion type!! 
	// TODO: add here if albedo, normal, envmap, ambientoclusion, etc..
	const uint32_t layer1 = AddLayer(document, &allocator, pLayer1);
	UpdateLayer(document, &allocator, layer1, psd::exportChannel::RED, 0, 0, IMAGE_WIDTH, IMAGE_HEIGHT, &g_multiplyData32[0][0], psd::compressionType::RAW);
	UpdateLayer(document, &allocator, layer1, psd::exportChannel::GREEN, 0, 0, IMAGE_WIDTH, IMAGE_HEIGHT, &g_multiplyData32[0][0], psd::compressionType::RAW);
	UpdateLayer(document, &allocator, layer1, psd::exportChannel::BLUE, 0, 0, IMAGE_WIDTH, IMAGE_HEIGHT, &g_multiplyData32[0][0], psd::compressionType::RAW);
	UpdateLayer(document, &allocator, layer1, psd::exportChannel::ALPHA, 0, 0, IMAGE_WIDTH, IMAGE_HEIGHT, &g_checkerBoardData32[0][0], psd::compressionType::RAW);

	UpdateMergedImage(document, &allocator, &g_multiplyData32[0][0], &g_xorData32[0][0], &g_checkerBoardData32[0][0]);

	WriteDocument(document, &allocator, &file);

	DestroyExportDocument(document, &allocator);
	file.Close();

	Msg("done\n");

	//Check if the psd exists!
	IsPsdOK(pPsdFileName);
}
#endif // 0

//TODO, DO a sanity check of the slashes!!!!!
//-----------------------------------------------------------------------------
// Purpose:  
//-----------------------------------------------------------------------------
void CheckAndExportToPsd(const char* pFilename)
{
	char szLayerName[64]; bool bEndFileName = false;

	// Sanity check!
	if(_access(pFilename, 0))
	{
		Error("The file: %s, doesnt exist!\n", pFilename);
	}

	const char* pImageFileName = V_strrchr(pFilename, '\\') + 1; // We skip the first char '\'

	V_strcpy(szLayerName, pImageFileName); // Default layer name

	//Check if the Image file has any of these endings
	if (g_bTempalteGeneration)
	{
		if (pImageFileName)
		{
			for (const char* pEnding : g_rgpEndFileName)
			{
				if (V_strstr(pImageFileName, pEnding))
				{
					V_strcpy(szLayerName, pEnding + 1); // we skip the first '_' in pEnding
					bEndFileName = true;
					break;
				}
			}
		}
		else if (!bEndFileName)
		{
			Warning("File: %s does not have a end-name.\n"
					"Template compile settings will not be applied!\n", 
					pImageFileName);
		}
	}


	if		(V_strstr(pFilename, g_rgpAssetConvertList[0]))	{ WriteNBitsPsd(pFilename, szLayerName, 8u);  /*.jpg	- 8bits*/ }
	else if (V_strstr(pFilename, g_rgpAssetConvertList[1]))	{ WriteNBitsPsd(pFilename, szLayerName, 8u);  /*.jpeg	- 8bits*/ }
	else if (V_strstr(pFilename, g_rgpAssetConvertList[2]))	{ WriteNBitsPsd(pFilename, szLayerName, 16u); /*.png	- 16bits*/}
	else if (V_strstr(pFilename, g_rgpAssetConvertList[3]))	{ WriteNBitsPsd(pFilename, szLayerName, 8u);  /*.tga	- 8bits*/ }
	else if (V_strstr(pFilename, g_rgpAssetConvertList[4]))	{ WriteNBitsPsd(pFilename, szLayerName, 8u);  /*.bmp	- 8bits*/ }
	else if (V_strstr(pFilename, g_rgpAssetConvertList[5]))	{ WriteNBitsPsd(pFilename, szLayerName, 8u);  /*.gif	- 8bits*/ }
	else if (V_strstr(pFilename, g_rgpAssetConvertList[6]))	{ WriteNBitsPsd(pFilename, szLayerName, 32u); /*.hdr	- 32bits*/}
	else if (V_strstr(pFilename, g_rgpAssetConvertList[7]))	{ WriteNBitsPsd(pFilename, szLayerName, 32u); /*.pic	- 32bits*/}
	else if (V_strstr(pFilename, g_rgpAssetConvertList[8]))	{ WriteNBitsPsd(pFilename, szLayerName, 16u); /*.ppm	- 16bits*/}
	else if (V_strstr(pFilename, g_rgpAssetConvertList[9]))	{ WriteNBitsPsd(pFilename, szLayerName, 16u); /*.pgm	- 16bits*/}
	else
	{
		// Sanity check
		Warning("No match found for: \'%s\'\n", pFilename);
		Warning("Supported extension are:\n");
		
		for (const char* pExt: g_rgpAssetConvertList)
		{
			Warning("\t%s\n", pExt);
		}

		exit(-1);
	}
}