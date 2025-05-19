//========== ------------------------------------------------- ============//
//
// Purpose: Converts .png, .jpg, etc.. to .psd for vtex compile
//
// $NoKeywords: $
//=============================================================================//
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
#include "psd/PsdParseDocument.h"	//R
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
const uint16_t IMAGE_WIDTH = 1024;
const uint16_t IMAGE_HEIGHT = 1024;


//-----------------------------------------------------------------------------
// Purpose: add metadata to the .psd file
//-----------------------------------------------------------------------------
static void SetMetaData(psd::ExportDocument* pDocument, psd::MallocAllocator allocator /*TODO: maybe fix this?*/, const char* pFilename)
{
	char szPsdWriterDateBuild[32];

	qprintf("Adding meta data to the .psd file... ");

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

	qprintf("done");
}


//-----------------------------------------------------------------------------
// Purpose:  Writes a 8 bit .psd file
//-----------------------------------------------------------------------------
void Write8BitsPsd(const char* pFilename, const char* pLayer1)
{
	uint8_t g_multiplyData[IMAGE_HEIGHT][IMAGE_WIDTH] = {};
	uint8_t g_xorData[IMAGE_HEIGHT][IMAGE_WIDTH] = {};
	uint8_t g_orData[IMAGE_HEIGHT][IMAGE_WIDTH] = {};
	uint8_t g_andData[IMAGE_HEIGHT][IMAGE_WIDTH] = {};
	uint8_t g_checkerBoardData[IMAGE_HEIGHT][IMAGE_WIDTH] = {};


	const std::wstring dstPath = L"SampleWrite_8.psd";

	psd::MallocAllocator allocator;
	psd::NativeFile file(&allocator);

	// try opening the file. if it fails, bail out.
	if (!file.OpenWrite(dstPath.c_str()))
	{
		Warning("Cannot open file.\n");
		return;
	}

	// write an RGB PSD file, 8-bit
	psd::ExportDocument* document = CreateExportDocument(&allocator, IMAGE_WIDTH, IMAGE_HEIGHT, 8u, psd::exportColorMode::RGB);

	SetMetaData(document, allocator, pFilename);

	// when adding a layer to the document, you first need to get a new index into the layer table.
	// with a valid index, layers can be updated in parallel, in any order.
	// this also allows you to only update the layer data that has changed, which is crucial when working with large data sets.
	const unsigned int layer1 = AddLayer(document, &allocator, pLayer1);

	// note that each layer has its own compression type. it is perfectly legal to compress different channels of different layers with different settings.
	// RAW is pretty much just a raw data dump. fastest to write, but large.
	// RLE stores run-length encoded data which can be good for 8-bit channels, but not so much for 16-bit or 32-bit data.
	// ZIP is a good compromise between speed and size.
	// ZIP_WITH_PREDICTION first delta encodes the data, and then zips it. slowest to write, but also smallest in size for most images.
	UpdateLayer(document, &allocator, layer1, psd::exportChannel::RED, 0, 0, IMAGE_WIDTH, IMAGE_HEIGHT, &g_multiplyData[0][0], psd::compressionType::RAW);
	UpdateLayer(document, &allocator, layer1, psd::exportChannel::GREEN, 0, 0, IMAGE_WIDTH, IMAGE_HEIGHT, &g_multiplyData[0][0], psd::compressionType::RAW);
	UpdateLayer(document, &allocator, layer1, psd::exportChannel::BLUE, 0, 0, IMAGE_WIDTH, IMAGE_HEIGHT, &g_multiplyData[0][0], psd::compressionType::RAW);


	// note that transparency information is always supported, regardless of the export color mode.
	// it is saved as true transparency, and not as separate alpha channel.
	UpdateLayer(document, &allocator, layer1, psd::exportChannel::ALPHA, 0, 0, IMAGE_WIDTH, IMAGE_HEIGHT, &g_multiplyData[0][0], psd::compressionType::RAW);


	// merged image data is optional. if none is provided, black channels will be exported instead.
	UpdateMergedImage(document, &allocator, &g_multiplyData[0][0], &g_xorData[0][0], &g_orData[0][0]);

	WriteDocument(document, &allocator, &file);

	DestroyExportDocument(document, &allocator);
	file.Close();
}


//-----------------------------------------------------------------------------
// Purpose:  Writes a 16 bit .psd file
//-----------------------------------------------------------------------------
void Write16BitsPsd(const char* pFilename)
{
	uint16_t g_multiplyData16[IMAGE_HEIGHT][IMAGE_WIDTH] = {};
	uint16_t g_xorData16[IMAGE_HEIGHT][IMAGE_WIDTH] = {};
	uint16_t g_orData16[IMAGE_HEIGHT][IMAGE_WIDTH] = {};
	uint16_t g_andData16[IMAGE_HEIGHT][IMAGE_WIDTH] = {};
	uint16_t g_checkerBoardData16[IMAGE_HEIGHT][IMAGE_WIDTH] = {};

	const std::wstring dstPath = L"SampleWrite_16.psd";

	psd::MallocAllocator allocator;
	psd::NativeFile file(&allocator);

	// try opening the file. if it fails, bail out.
	if (!file.OpenWrite(dstPath.c_str()))
	{
		Warning("Cannot open file.\n");
		return;
	}

	// write a Grayscale PSD file, 16-bit.
	// Grayscale works similar to RGB, only the types of export channels change.
	psd::ExportDocument* document = CreateExportDocument(&allocator, IMAGE_WIDTH, IMAGE_HEIGHT, 16u, psd::exportColorMode::GRAYSCALE);

	const unsigned int layer1 = AddLayer(document, &allocator, "MUL pattern");
	UpdateLayer(document, &allocator, layer1, psd::exportChannel::RED, 0, 0, IMAGE_WIDTH, IMAGE_HEIGHT, &g_multiplyData16[0][0], psd::compressionType::RAW);
	UpdateLayer(document, &allocator, layer1, psd::exportChannel::BLUE, 0, 0, IMAGE_WIDTH, IMAGE_HEIGHT, &g_multiplyData16[0][0], psd::compressionType::RAW);
	UpdateLayer(document, &allocator, layer1, psd::exportChannel::GREEN, 0, 0, IMAGE_WIDTH, IMAGE_HEIGHT, &g_multiplyData16[0][0], psd::compressionType::RAW);

	UpdateMergedImage(document, &allocator, &g_multiplyData16[0][0], &g_xorData16[0][0], &g_andData16[0][0]);

	WriteDocument(document, &allocator, &file);
	
	DestroyExportDocument(document, &allocator);
	file.Close();
}


//-----------------------------------------------------------------------------
// Purpose:  Writes a 32 bit .psd file
//-----------------------------------------------------------------------------
void Write32bitsPsd(const char* pFilename)
{
	float32_t g_multiplyData32[IMAGE_HEIGHT][IMAGE_WIDTH] = {};
	float32_t g_xorData32[IMAGE_HEIGHT][IMAGE_WIDTH] = {};
	float32_t g_orData32[IMAGE_HEIGHT][IMAGE_WIDTH] = {};
	float32_t g_andData32[IMAGE_HEIGHT][IMAGE_WIDTH] = {};
	float32_t g_checkerBoardData32[IMAGE_HEIGHT][IMAGE_WIDTH] = {};

	const std::wstring dstPath = L"SampleWrite_32.psd";

	psd::MallocAllocator allocator;
	psd::NativeFile file(&allocator);

	//TODO: remove this!
	// try opening the file. if it fails, bail out.
	if (!file.OpenWrite(dstPath.c_str()))
	{
		Warning("Cannot open file.\n");
		return;
	}

	// write an RGB PSD file, 32-bit
	psd::ExportDocument* document = CreateExportDocument(&allocator, IMAGE_WIDTH, IMAGE_HEIGHT, 32u, psd::exportColorMode::RGB);

	// TODO: let the user set the compresion type!! 
	// TODO: add here if albedo, normal, envmap, ambientoclusion, etc..
	const uint32_t layer1 = AddLayer(document, &allocator, "");
	UpdateLayer(document, &allocator, layer1, psd::exportChannel::RED, 0, 0, IMAGE_WIDTH, IMAGE_HEIGHT, &g_multiplyData32[0][0], psd::compressionType::RAW);
	UpdateLayer(document, &allocator, layer1, psd::exportChannel::GREEN, 0, 0, IMAGE_WIDTH, IMAGE_HEIGHT, &g_multiplyData32[0][0], psd::compressionType::RAW);
	UpdateLayer(document, &allocator, layer1, psd::exportChannel::BLUE, 0, 0, IMAGE_WIDTH, IMAGE_HEIGHT, &g_multiplyData32[0][0], psd::compressionType::RAW);
	UpdateLayer(document, &allocator, layer1, psd::exportChannel::ALPHA, 0, 0, IMAGE_WIDTH, IMAGE_HEIGHT, &g_checkerBoardData32[0][0], psd::compressionType::RAW);

	const uint32_t layer2 = AddLayer(document, &allocator, "Mixed pattern with transparency");
	UpdateLayer(document, &allocator, layer2, psd::exportChannel::RED, 0, 0, IMAGE_WIDTH, IMAGE_HEIGHT, &g_multiplyData32[0][0], psd::compressionType::RLE);
	UpdateLayer(document, &allocator, layer2, psd::exportChannel::GREEN, 0, 0, IMAGE_WIDTH, IMAGE_HEIGHT, &g_xorData32[0][0], psd::compressionType::ZIP);
	UpdateLayer(document, &allocator, layer2, psd::exportChannel::BLUE, 0, 0, IMAGE_WIDTH, IMAGE_HEIGHT, &g_orData32[0][0], psd::compressionType::ZIP_WITH_PREDICTION);
	UpdateLayer(document, &allocator, layer2, psd::exportChannel::ALPHA, 0, 0, IMAGE_WIDTH, IMAGE_HEIGHT, &g_checkerBoardData32[0][0], psd::compressionType::RAW);

	UpdateMergedImage(document, &allocator, &g_multiplyData32[0][0], &g_xorData32[0][0], &g_checkerBoardData32[0][0]);

	WriteDocument(document, &allocator, &file);

	DestroyExportDocument(document, &allocator);
	file.Close();
}