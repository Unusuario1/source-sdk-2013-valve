//========== ------------------------------------------------- ============//
//
// Purpose: Converts .png, .jpg, etc.. to .psd for vtex compile
//
// $NoKeywords: $
//=============================================================================//
#include <io.h>
#include <cstddef>
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

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

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


//-----------------------------------------------------------------------------
// Purpose: internal global vars, helpers for writing .psd files
//-----------------------------------------------------------------------------
int			g_iImageWidth;
int			g_iImageHeight;
int			g_iChannels;
uint8_t*	g_uiImageData8Bits;
uint16_t*	g_uiImageData16Bits;
float32_t*	g_uiImageData32Bits;


//-----------------------------------------------------------------------------
// Purpose: Check if the number is a power of two =)
//-----------------------------------------------------------------------------
FORCEINLINE static bool isPowerOfTwo(int n)
{
	return (n > 0) && ((n & (n - 1)) == 0);
}


//-----------------------------------------------------------------------------
// Purpose: Load the image
//-----------------------------------------------------------------------------
static void AllocVectorImageData(const char* pFileName, const uint uiBitType /*8,16,32*/)
{
	float start = Plat_FloatTime();
	char* pTemp = V_strdup(pFileName);
		
	qprintf("Allocating image vector data (%u)... %s", uiBitType, verbose ? "\n" : "");

	if (uiBitType == IMAGE8BITS_ID)
	{
		g_uiImageData8Bits = stbi_load(pFileName, &g_iImageWidth, &g_iImageHeight, &g_iChannels, 0);

		if (g_uiImageData8Bits)
		{
			if ((g_iImageWidth < MAX_WIGHT_RES_X) || (g_iImageHeight < MAX_WIGHT_RES_Y))
			{
				Msg("Loaded 8-bit image: %dx%d with %d channels.\n", g_iImageWidth, g_iImageHeight, g_iChannels);
			}
			else
			{
				Warning("Image: %s, exceeds the maximum resolution in source engine!!\n"
						"Maximun: x = %i, y = %i\n"
						"File: x = %i, y = %i\n", 
						pFileName, MAX_WIGHT_RES_X, MAX_WIGHT_RES_Y, g_iImageWidth, g_iImageHeight
					 );
			}
		}
		else
		{
			Error("Failed to 8-bits load image: %s\n", stbi_failure_reason());
		}
	}
	else if (uiBitType == IMAGE16BITS_ID)
	{
		g_uiImageData16Bits = stbi_load_16(pFileName, &g_iImageWidth, &g_iImageHeight, &g_iChannels, 0);

		if (g_uiImageData16Bits)
		{
			if ((g_iImageWidth < MAX_WIGHT_RES_X) || (g_iImageHeight < MAX_WIGHT_RES_Y))
			{
				Msg("Loaded 16-bit image: %dx%d with %d channels.\n", g_iImageWidth, g_iImageHeight, g_iChannels);
			}
			else
			{
				Warning("Image: %s, exceeds the maximum resolution in source engine!!\n"
						"Maximun: x = %i, y = %i\n"
						"File: x = %i, y = %i\n",
						pFileName, MAX_WIGHT_RES_X, MAX_WIGHT_RES_Y, g_iImageWidth, g_iImageHeight
				);
			}
		}
		else
		{
			Error("Failed to 16-bits load image: %s\n", stbi_failure_reason());
		}
	}
	else if (uiBitType == IMAGE32BITS_ID)
	{
		g_uiImageData32Bits = stbi_loadf(pFileName, &g_iImageWidth, &g_iImageHeight, &g_iChannels, 0);

		if (g_uiImageData32Bits)
		{
			if ((g_iImageWidth < MAX_WIGHT_RES_X) || (g_iImageHeight < MAX_WIGHT_RES_Y))
			{
				Msg("Loaded 32-bit image: %dx%d with %d channels.\n", g_iImageWidth, g_iImageHeight, g_iChannels);
			}
			else
			{
				Warning("Image: %s, exceeds the maximum resolution in source engine!!\n"
						"Maximun: x = %i, y = %i\n"
						"File: x = %i, y = %i\n",
						pFileName, MAX_WIGHT_RES_X, MAX_WIGHT_RES_Y, g_iImageWidth, g_iImageHeight
				);
			}
		}
		else
		{
			Error("Failed to 32-bits load image: %s\n", stbi_failure_reason());
		}
	}

	if(!isPowerOfTwo(g_iImageWidth) || !isPowerOfTwo(g_iImageHeight))
	{
		Warning("The file does not have a power-of-two resolution!\n"
				"This will cause an error in vtex.exe when compiling the texture!\n"
			   );
	}
	
	delete[] pTemp;

	qprintf("done(%.2fs)\n%s", Plat_FloatTime() - start, verbose ? "\n" : "");
}


//-----------------------------------------------------------------------------
// Purpose:		Free the alloc memory of the loaded image
//-----------------------------------------------------------------------------
static void DestroyAllocVectorImageData(const uint uiBitType /*8,16,32*/)
{
	if (uiBitType == IMAGE8BITS_ID)
	{
		stbi_image_free(g_uiImageData8Bits);

	}
	else if (uiBitType == IMAGE16BITS_ID)
	{
		stbi_image_free(g_uiImageData16Bits);
	}
	else if (uiBitType == IMAGE32BITS_ID)
	{
		stbi_image_free(g_uiImageData32Bits);
	}
}


//-----------------------------------------------------------------------------
// Purpose:		Checks if the image format is suported.
//-----------------------------------------------------------------------------
static void CheckExtensionImageFileConvert(const char* pFileName)
{
	bool bTemp = false;

	for (const char* pExt : g_rgpAssetConvertList)
	{
		if (V_strstr(pFileName, pExt))
		{
			bTemp = true;
		}
	}

	if (!bTemp) 
	{
		Warning("No match found for: \'%s\'\n", pFileName);
		Warning("Supported extension are:\n");

		for (const char* pExt2 : g_rgpAssetConvertList)
		{
			Warning("\t%s\n", pExt2);
		}

		exit(-1);
	}
}


//-----------------------------------------------------------------------------
// Purpose:		Generate name_(endname).txt for the .psd file  
//				e.g: foo_albedo.psd, foo_albedo.txt
//-----------------------------------------------------------------------------
static void GenerateVtexConfigFile(const char* pFileName)
{
	if (!g_bTempalteGeneration)
	{
		return;
	}

	float start = Plat_FloatTime();
	Msg("Generating config file for: %s... ", pFileName);

	// Sanity check!
	if (gamedir[0] == '\0')
	{
		Warning("\n"
				"Template generation cannot be executed!\n"
				"Set \'-game or -vproject\' path to enable this feature!\n"
			   );
		return;
	}

	bool bEndFileName = false;
	char szNameEnding[64]; // e.g: _normal, _detail, etc...
	const char* pImageFileName = V_strrchr(pFileName, '\\') + 1; // We skip the first char '\'
	
	if (pImageFileName)
	{
		for (const char* pEnding : g_rgpEndFileName)
		{
			if (V_strstr(pImageFileName, pEnding))
			{
				V_strcpy(szNameEnding, pEnding);
				bEndFileName = true;
				break;
			}
		}
	}
	else 
	{
		// Sanity check!
		Error( "\n"
			   "**INTERAL ERROR**: GenerateVtexConfigFile(); pImageFileName == nullptr!\n"
			   "Speak to a dev, this shouldnt happen!\n"
			 );
	}


	if (!bEndFileName)
	{
		Warning("\n"
				"File: %s does not have a valid end-name.\n"
				"Template config compile settings will not be applied!\n",
				pImageFileName);
		return;
	}


	qprintf("\n");

	// The path where we expect the template config files to be game/materialsrc/template_endname.txt
	char szSrcTemplateConfigFile[MAX_PATH];
	V_snprintf(szSrcTemplateConfigFile, sizeof(szSrcTemplateConfigFile), "%s\\%s%s.txt", g_szGameMaterialSrcDir, TEMAPLATE_COFIG_NAME_BASE, szNameEnding);
	qprintf("Source template config file:    %s\n", szSrcTemplateConfigFile);

	// Setup the template config to copy, TODO fix this shit!
	char* pImageFileConfig = V_strdup(pFileName);
	V_StripExtension(pImageFileConfig, pImageFileConfig, V_strlen(pImageFileConfig)); // the crash is most lilkey to happen here!
	V_snprintf(pImageFileConfig, V_strlen(pImageFileConfig), "%s.txt", pImageFileConfig); 
	qprintf("Template config .psd file :     %s\n", pImageFileConfig);


	// Sanity check!
	if(_access(szSrcTemplateConfigFile, 0))
	{
		Warning("\n"
				"Could not find a template config file in: %s\n"
				"Config file: %s\n"
				,g_szGameMaterialSrcDir, szSrcTemplateConfigFile
			   );
	}
	else 
	{
		// Copy the config file to the .psd dir!
		if (CopyFileA(pImageFileConfig, szSrcTemplateConfigFile, FALSE))
		{
			Msg("done(%.2fs)\n", Plat_FloatTime() - start);
		}
		else
		{
			DWORD errorCode = GetLastError();
			Warning("\n"
					"Could not generate the cofing file for the .psd file!\n"
					"Config file: %s\n"
					"CopyFileA(); Error code : % lu\n",
					errorCode, szSrcTemplateConfigFile
				   );
		}
	}

	delete[] pImageFileConfig;
}


//-----------------------------------------------------------------------------
// Purpose:		Check if the psd was writen
//-----------------------------------------------------------------------------
static bool IsPsdOK(const char* pFileName)
{
	char szFile[MAX_PATH];
	V_snprintf(szFile, sizeof(szFile), "%s.psd", pFileName);

	if (_access(szFile, 0))
	{
		ColorSpewMessage(SPEW_MESSAGE, &failed, "FAILED - %s\n", szFile);
		return true;
	}
	else 
	{
		ColorSpewMessage(SPEW_MESSAGE, &successful_color, "OK"); Msg(" - %s (%.2fs)\n", szFile, Plat_FloatTime() - g_fGlobalTimer);
		return false;
	}
}


//-----------------------------------------------------------------------------
// Purpose: add metadata to the .psd file
//-----------------------------------------------------------------------------
static void SetMetaData(psd::ExportDocument* pDocument, psd::MallocAllocator allocator /*TODO: maybe fix this?*/, const char* pFileName)
{
	float start = Plat_FloatTime();
	char szPsdWriterDateBuild[64];

	Msg("Adding meta data to the .psd file... ");

	V_snprintf(szPsdWriterDateBuild, sizeof(szPsdWriterDateBuild), "Build date: %s %s", __DATE__, __TIME__);

	AddMetaData(pDocument, &allocator, "Auto - generated with psdwriter.exe", szPsdWriterDateBuild);
	AddMetaData(pDocument, &allocator, "Resolution [x, y]:", ""); // put here the resoluction of the image

	if(g_bSignature)
	{
		AddMetaData(pDocument, &allocator, g_szSignature[0], g_szSignature[1]);
	}

	Msg("done(%.2fs)\n", Plat_FloatTime() - start);
}


//-----------------------------------------------------------------------------
// Purpose:  Writes a 8, 16 or 32 bit .psd file
//-----------------------------------------------------------------------------
static void WriteNBitsPsd(const char* pFileName, const char* pLayer1, const uint uiBitType)
{
	float start = Plat_FloatTime();

	char pPsdFileName[MAX_PATH];
	V_StripExtension(pFileName, pPsdFileName, MAX_PATH);

	wchar_t szWideName[MAX_PATH];
	mbstowcs(szWideName, pPsdFileName, MAX_PATH);

	wchar_t szBuffer[MAX_PATH];
	swprintf(szBuffer, MAX_PATH, L"%s.psd", szWideName);

	// Load the image 
	AllocVectorImageData(pFileName, uiBitType);

	psd::MallocAllocator allocator;
	psd::NativeFile file(&allocator);

	// try opening the file. if it fails, bail out.
	if (!file.OpenWrite(szBuffer))
	{
		Warning("Cannot create .psd file!\n");
		if(g_bIsSingleFile)
		{
			exit(-1);
		}
		else
		{
			Warning("Skipping the file!!\n");
			return;
		}
	}

	Msg("Exporting %s as .psd\n", V_strrchr(pFileName, '\\') + 1);

	psd::ExportDocument* document = CreateExportDocument(&allocator, g_iImageWidth, g_iImageHeight, uiBitType, psd::exportColorMode::RGB);

	SetMetaData(document, allocator, pFileName);

	// when adding a layer to the document, you first need to get a new index into the layer table.
	// with a valid index, layers can be updated in parallel, in any order.
	// this also allows you to only update the layer data that has changed, which is crucial when working with large data sets.
	start = Plat_FloatTime();
	Msg("Creating \'%s\' layer... ", pLayer1);
	const uint layer1 = AddLayer(document, &allocator, pLayer1);
	Msg("done(%.2fs)\n", Plat_FloatTime() - start);


	// note that each layer has its own compression type. it is perfectly legal to compress different channels of different layers with different settings.
	// RAW is pretty much just a raw data dump. fastest to write, but large.
	// RLE stores run-length encoded data which can be good for 8-bit channels, but not so much for 16-bit or 32-bit data.
	// ZIP is a good compromise between speed and size.
	// ZIP_WITH_PREDICTION first delta encodes the data, and then zips it. slowest to write, but also smallest in size for most images.
	// Note: This values need to be in sync with psd::compressionType in PsdCompressionType.h
	psd::compressionType::Enum compression = psd::compressionType::RAW;
	if		(g_uiCompressionType == 0) { compression = psd::compressionType::RAW; }
	else if (g_uiCompressionType == 1) { compression = psd::compressionType::RLE; }
	else if (g_uiCompressionType == 2) { compression = psd::compressionType::ZIP; }
	else if (g_uiCompressionType == 3) { compression = psd::compressionType::ZIP_WITH_PREDICTION; }

	start = Plat_FloatTime();
	Msg("Creating RGBA layers: ");
	if		(uiBitType == IMAGE8BITS_ID)  { UpdateLayer(document, &allocator, layer1, psd::exportChannel::RED, 0, 0, g_iImageWidth, g_iImageHeight, g_uiImageData8Bits, compression); }
	else if (uiBitType == IMAGE16BITS_ID) { UpdateLayer(document, &allocator, layer1, psd::exportChannel::RED, 0, 0, g_iImageWidth, g_iImageHeight, g_uiImageData16Bits, compression); }
	else if (uiBitType == IMAGE32BITS_ID) { UpdateLayer(document, &allocator, layer1, psd::exportChannel::RED, 0, 0, g_iImageWidth, g_iImageHeight, g_uiImageData32Bits, compression); }
	Msg("red, ");
	if		(uiBitType == IMAGE8BITS_ID)  { UpdateLayer(document, &allocator, layer1, psd::exportChannel::GREEN, 0, 0, g_iImageWidth, g_iImageHeight, g_uiImageData8Bits, compression); }
	else if (uiBitType == IMAGE16BITS_ID) { UpdateLayer(document, &allocator, layer1, psd::exportChannel::GREEN, 0, 0, g_iImageWidth, g_iImageHeight, g_uiImageData16Bits, compression); }
	else if (uiBitType == IMAGE32BITS_ID) { UpdateLayer(document, &allocator, layer1, psd::exportChannel::GREEN, 0, 0, g_iImageWidth, g_iImageHeight, g_uiImageData32Bits, compression); }
	Msg("green, ");
	if		(uiBitType == IMAGE8BITS_ID)  { UpdateLayer(document, &allocator, layer1, psd::exportChannel::BLUE, 0, 0, g_iImageWidth, g_iImageHeight, g_uiImageData8Bits, compression); }
	else if (uiBitType == IMAGE16BITS_ID) { UpdateLayer(document, &allocator, layer1, psd::exportChannel::BLUE, 0, 0, g_iImageWidth, g_iImageHeight, g_uiImageData16Bits, compression); }
	else if (uiBitType == IMAGE32BITS_ID) { UpdateLayer(document, &allocator, layer1, psd::exportChannel::BLUE, 0, 0, g_iImageWidth, g_iImageHeight, g_uiImageData32Bits, compression); }
	Msg("blue%s ", g_iChannels == 4 ? "," : "");
	if (g_iChannels == 4)
	{
		if		(uiBitType == IMAGE8BITS_ID) { UpdateLayer(document, &allocator, layer1, psd::exportChannel::ALPHA, 0, 0, g_iImageWidth, g_iImageHeight, g_uiImageData8Bits, compression); }
		else if (uiBitType == IMAGE16BITS_ID) { UpdateLayer(document, &allocator, layer1, psd::exportChannel::ALPHA, 0, 0, g_iImageWidth, g_iImageHeight, g_uiImageData16Bits, compression); }
		else if (uiBitType == IMAGE32BITS_ID) { UpdateLayer(document, &allocator, layer1, psd::exportChannel::ALPHA, 0, 0, g_iImageWidth, g_iImageHeight, g_uiImageData32Bits, compression); }
		Msg("alpha");
	}
	Msg("... done(%.2fs)\n", Plat_FloatTime() - start);

	start = Plat_FloatTime();
	Msg("Merging RGBA layers... ");
	// merged image data is optional. if none is provided, black channels will be exported instead.
	if		(uiBitType == IMAGE8BITS_ID)  { UpdateMergedImage(document, &allocator, g_uiImageData8Bits, g_uiImageData8Bits, g_uiImageData8Bits); }
	else if (uiBitType == IMAGE16BITS_ID) { UpdateMergedImage(document, &allocator, g_uiImageData16Bits, g_uiImageData16Bits, g_uiImageData16Bits); }
	else if (uiBitType == IMAGE32BITS_ID) { UpdateMergedImage(document, &allocator, g_uiImageData32Bits, g_uiImageData32Bits, g_uiImageData32Bits); }
	Msg("done(%.2fs)\n", Plat_FloatTime() - start);

	start = Plat_FloatTime();
	Msg("Writting %u-bits .psd file... ", uiBitType);
	WriteDocument(document, &allocator, &file);
	Msg("done(%.2fs)\n", Plat_FloatTime() - start);

	DestroyExportDocument(document, &allocator);
	file.Close();

	DestroyAllocVectorImageData(uiBitType);

	//Check if the psd exists!
	IsPsdOK(pPsdFileName);
}


//-----------------------------------------------------------------------------
// Purpose:  
//-----------------------------------------------------------------------------
void CheckAndExportToPsd(const char* pFileName)
{
	char szLayerName[64];

	// Sanity check!
	if(_access(pFileName, 0))
	{
		Error("The file: %s, doesnt exist!\n", pFileName);
	}

	const char* pImageFileName = V_strrchr(pFileName, '\\') + 1; // We skip the first char '\'

	// Sanity check!
	if (pImageFileName == nullptr)
	{
		Error(	"**INTERAL ERROR**: pImageFileName == nullptr!\n"
				"Speak to a dev, this shouldnt happen!\n"
			 );
	}

	V_strcpy(szLayerName, pImageFileName); // Default layer name


	// one more Sanity check, i swear..
	CheckExtensionImageFileConvert(pFileName);

	// Force the image to be in N bits, if the user wants it. 
	if(g_bForceImageBit)
	{
		
		WriteNBitsPsd(pFileName, szLayerName, g_uiForceImageBit);
	}
	else
	{
		if		(V_strstr(pFileName, g_rgpAssetConvertList[0])) { WriteNBitsPsd(pFileName, szLayerName, IMAGE8BITS_ID);  /*.jpg	- 8bits*/ }
		else if (V_strstr(pFileName, g_rgpAssetConvertList[1])) { WriteNBitsPsd(pFileName, szLayerName, IMAGE8BITS_ID);  /*.jpeg- 8bits*/ }
		else if (V_strstr(pFileName, g_rgpAssetConvertList[2])) { WriteNBitsPsd(pFileName, szLayerName, IMAGE16BITS_ID); /*.png	- 16bits*/ }
		else if (V_strstr(pFileName, g_rgpAssetConvertList[3])) { WriteNBitsPsd(pFileName, szLayerName, IMAGE8BITS_ID);  /*.tga	- 8bits*/ }
		else if (V_strstr(pFileName, g_rgpAssetConvertList[4])) { WriteNBitsPsd(pFileName, szLayerName, IMAGE8BITS_ID);  /*.bmp	- 8bits*/ }
		else if (V_strstr(pFileName, g_rgpAssetConvertList[5])) { WriteNBitsPsd(pFileName, szLayerName, IMAGE8BITS_ID);  /*.gif	- 8bits*/ }
		else if (V_strstr(pFileName, g_rgpAssetConvertList[6])) { WriteNBitsPsd(pFileName, szLayerName, IMAGE32BITS_ID); /*.hdr	- 32bits*/ }
		else if (V_strstr(pFileName, g_rgpAssetConvertList[7])) { WriteNBitsPsd(pFileName, szLayerName, IMAGE32BITS_ID); /*.pic	- 32bits*/ }
		else if (V_strstr(pFileName, g_rgpAssetConvertList[8])) { WriteNBitsPsd(pFileName, szLayerName, IMAGE16BITS_ID); /*.ppm	- 16bits*/ }
		else if (V_strstr(pFileName, g_rgpAssetConvertList[9])) { WriteNBitsPsd(pFileName, szLayerName, IMAGE16BITS_ID); /*.pgm	- 16bits*/ }
	}

	// Remove the source file
	if (g_bDeleteSource)
	{
		if (remove(pFileName) != 0)
		{
			Warning("Could not delete source file: %s!\n", pFileName);
		}
		else
		{
			Msg("Deleting source file at: %s\n", pFileName);
		}
	}

	GenerateVtexConfigFile(pFileName);
}