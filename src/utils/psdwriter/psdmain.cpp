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
#include "cmdlib.h"
#include "filesystem_init.h"
#include "filesystem_tools.h"
#include "color.h"
#include "psdwriter.h"


// TODO list:
//	-Add a exclusion command smth like -tga
//  -Fix the issue with reading saving the texture!!


#define MATERIALSRC_DIR		"materialsrc"


//-----------------------------------------------------------------------------
// Purpose: Global vars
//-----------------------------------------------------------------------------
bool			g_bQuiet = false;
bool            g_bSignature = false;
bool            g_bIsSingleFile = true;
bool            g_bTempalteGeneration = false;
bool            g_bDeleteSource = false;
bool            g_bForceImageBit = false;
float           g_fGlobalTimer;
uint8_t         g_uiCompressionType = 0;
uint8_t         g_uiForceImageBit;
Color			header_color(0, 255, 255, 255); //TODO: maybe move 'Color' in utils/common/colorscheme.cpp/.h
Color			successful_color(0, 255, 0, 255);
Color			failed(255, 0, 0, 255);
char			g_szGameMaterialSrcDir[MAX_PATH] = "";
char			g_szSingleInputFile[MAX_PATH] = "";
char			g_szExcludeExt[16][16] = { "" };
char            g_szSignature[1][128];
// Note: These extensions should be in sync with the suported in stb_image.h!!
const char*     g_rgpAssetConvertList[10] = {   ".jpg",         //0
                                                ".jpeg",        //1
                                                ".png",         //2
                                                ".tga",         //3
                                                ".bmp",         //4
                                                ".gif",         //5
                                                ".hdr",         //6
                                                ".pic",         //7
                                                ".ppm",         //9
                                                ".pgm"          //10
};
const char*     g_rgpEndFileName[13] = {        "_color",         //https://developer.valvesoftware.com/wiki/$basetexture
                                                "_albedo",         //https://developer.valvesoftware.com/wiki/$basetexture
                                                "_normal",        //https://developer.valvesoftware.com/wiki/$bumpmap
                                                "_ao",            //https://developer.valvesoftware.com/wiki/$ambientoccltexture
                                                "_detail",        //https://developer.valvesoftware.com/wiki/$detail
                                                "_envmap",        //https://developer.valvesoftware.com/wiki/$envmap
                                                "_envmapmask",    //https://developer.valvesoftware.com/wiki/$envmapmask
                                                "_trans",         //
                                                "_specular"       //https://developer.valvesoftware.com/wiki/$phongexponenttexture                             
                                                "_flowmap",       //https://developer.valvesoftware.com/wiki/Water_(shader)#Flowing_water
                                                "_selfilum",      //https://developer.valvesoftware.com/wiki/Glowing_textures_(Source)#$selfillum
                                                "_blendmodulate", //https://developer.valvesoftware.com/wiki/$blendmodulatetexture
                                                "_lightwarp",     //https://developer.valvesoftware.com/wiki/$lightwarptexture
                                                "_dudvmap"        //https://developer.valvesoftware.com/wiki/Water_(shader)
                                                "_ramp"           //https://developer.valvesoftware.com/wiki/$ramptexture
#if 0
                                                "_tintmask",      //this is only in post found in csgo branch
                                                "_flowmapnoise",  //https://developer.valvesoftware.com/wiki/Water_(shader)#Flowing_water
#endif     
};


//-----------------------------------------------------------------------------
// Purpose:   
//-----------------------------------------------------------------------------
static void ProcessDirAndConvertContents()
{
	char szWildCard[MAX_PATH] = "";

	for (const char* pExtension : g_rgpAssetConvertList)
	{
		for (uint8_t i = 0; i < 8; i++) //TODO: fix this!
		{
			if (pExtension != g_szExcludeExt[i]) 
			{
				V_snprintf(szWildCard, sizeof(szWildCard), "%s\\*%s", g_szGameMaterialSrcDir, pExtension);

				// TODO: make here the implementation 
			}
		}
	}
}


//-----------------------------------------------------------------------------
// Purpose:   Setup the search path
//-----------------------------------------------------------------------------
static void Init()
{
    g_fGlobalTimer = Plat_FloatTime();

    V_snprintf(g_szGameMaterialSrcDir, sizeof(g_szGameMaterialSrcDir), "%s\\%s", gamedir, MATERIALSRC_DIR);

    qprintf("Gamedir path:          %s\n", gamedir);
    qprintf("Material Source path:  %s\n", g_szGameMaterialSrcDir);
}


//-----------------------------------------------------------------------------
// Purpose:   Prints the header
//-----------------------------------------------------------------------------
static void PrintHeader()
{
	ColorSpewMessage(SPEW_MESSAGE, &header_color, "\n----- -------- - psdwriter.exe (Build: %s %s)\n", __DATE__, __TIME__);
}


//-----------------------------------------------------------------------------
// Purpose:   Print psdwriter usage
//-----------------------------------------------------------------------------
static void PrintUsage(int argc, char* argv[])
{
    char szTemp[256] = "";
    char szTemp2[256] = "";

    for(const char* pExt: g_rgpAssetConvertList)
    {
        V_snprintf(szTemp, sizeof(szTemp), "%s %s ", szTemp == "" ? "" : szTemp, pExt);
    }

    // TODO: fix this
    for(const char* pEndName : g_rgpEndFileName)
    {
        V_snprintf(szTemp2, sizeof(szTemp2), "                                   %s %s\n", szTemp2 == "" ? "" : szTemp2, pEndName);
    }

    Msg("Usage: psdwriter.exe [options] -game <path>\n\n");
    ColorSpewMessage(SPEW_MESSAGE, &header_color, " General Options:\n");
    Msg("   -i <file>:                      \n"
        "   -dir <path>:                    \n"
        "   -recc:                          batch dir recursive\n"  
        "   -psdtemplategeneration:         Suported end-names\n"
        "%s"
        "   -exclusionformat <extension>:   Suported formats: %s\n"
		"   -game <path>:                   Specify the folder of the gameinfo.txt file.\n"
        "   -vproject <path>:               Same as \'-game\'.\n"
        "\n", szTemp2, szTemp);
    ColorSpewMessage(SPEW_MESSAGE, &header_color, " Spew Options:\n");
    Msg("   -v or -verbose:                 Enables verbose.\n"
        "   -quiet:                         Prints minimal text. (Note: Disables \'-verbose\')\n"
        "\n");
    ColorSpewMessage(SPEW_MESSAGE, &header_color, " Other Options:\n");
    Msg("   -FullMinidumps:                 Write large minidumps on crash.\n"
        "   -forcebittype <n>:              8/16/32 applied to all .psd files\n" 
        "   -psdcompresion <n>:             Manages the compresion of the .psd file when importing a image (default 0).\n"
        "                                   0 = Raw data (very big filesize)\n"
        "                                   1 = RLE-compressed data (using the PackBits algorithm).\n"
        "                                   2 = ZIP-compressed data.\n"
        "                                   3 =	ZIP-compressed data with prediction (delta-encoding).\n"
        "   -signature <studio> <mod>:      (e.g: -signature myfirststudio myfirstsourcemod)\n"
        "   -deletesource:                  Deletes source (%s) files if the .psd conversion is generated\n"    
        "\n", szTemp);

    DeleteCmdLine(argc, argv);
    CmdLib_Cleanup();
    CmdLib_Exit(1);
}


//-----------------------------------------------------------------------------
// Purpose:   Parse command line
//-----------------------------------------------------------------------------
static void ParseCommandline(int argc, char* argv[])
{
    if (argc == 1)
    {
        PrintUsage(argc, argv);
    }

    for (int i = 1; i < argc; ++i)
    {
        if (!V_stricmp(argv[i], "-?") || !V_stricmp(argv[i], "-help") || argc == 1)
        {
            PrintUsage(argc, argv);
        }
        else if (!V_stricmp(argv[i], "-v") || !V_stricmp(argv[i], "-verbose"))
        {
            verbose = true;
			g_bQuiet = false;
        }
        else if (!V_stricmp(argv[i], "-quiet"))
        {
			verbose	 = false;
            g_bQuiet = true;
        }
        else if (!V_stricmp(argv[i], "-FullMinidumps"))
        {
			EnableFullMinidumps(true);
		}
        else if (!V_stricmp(argv[i], "-psdtemplategeneration"))
        {
            g_bTempalteGeneration = true;
		}
        else if (!V_stricmp(argv[i], "-deletesource"))
        {
            g_bDeleteSource = true;
		}
        else if (!V_stricmp(argv[i], "-forcebittype"))
        {
            uint8_t uiTempBitImage = atoi(argv[i + 1]);

            if(uiTempBitImage == IMAGE8BITS_ID)
            {
                g_uiForceImageBit = IMAGE8BITS_ID;
            }
            else if(uiTempBitImage == IMAGE16BITS_ID)
            {
                g_uiForceImageBit = IMAGE16BITS_ID;
            }
            else if(uiTempBitImage == IMAGE32BITS_ID)
            {
                g_uiForceImageBit = IMAGE32BITS_ID;
            }
            else 
            {
                Error("Unknow images bit type! Supported are only: %u, %u, %u\n", IMAGE8BITS_ID, IMAGE16BITS_ID, IMAGE32BITS_ID);
            }

            g_bForceImageBit = true;
		}
        else if (!V_stricmp(argv[i], "-psdcompresion"))
        {
            const int iValue = atoi(argv[i]);

            // Note: This values need to be in sync with psd::compressionType in PsdCompressionType.h =)
            if ((iValue >= 0) && (iValue <= 3))
            {
                g_uiCompressionType = (uint8_t)iValue;
            }
            else
            {
                Error("Error: \'-psdcompresion\' requires a valid integer value, expected range: [0,3], value: %u\n", iValue);
            }
		}
        else if (!V_stricmp(argv[i], "-signature"))
        {
            const char* pStrTemp1 = argv[i + 1]; //i++ replace
            const char* pStrTemp2 = argv[i + 2];

            // Note: This values need to be in sync with psd::compressionType in PsdCompressionType.h
            if (pStrTemp1 && pStrTemp2)
            {
                V_strcpy(g_szSignature[0], pStrTemp1);
                V_strcpy(g_szSignature[1], pStrTemp2);
            }
            else
            {
                Error("Error: \'-signature\' requires a TWO strings, (e.g: -signature blah blah2)\n");
            }
		}
        else if (!V_stricmp(argv[i], "-i"))
        {
            if (++i < argc && argv[i][0] != '-')
            {
                char* pInputPath = argv[i];

                if (!pInputPath)
                {
                    Error("Error: \'-i\' requires a valid path argument. NULL path\n");
                }

                g_bIsSingleFile = true;
                V_strcpy(g_szSingleInputFile, pInputPath);
            }
            else
            {
                Error("Error: \'-i\' requires a valid path argument.\n");
            }
        }
        else if (!V_stricmp(argv[i], "-dir"))
        {
            if (++i < argc && argv[i][0] != '-')
            {
                char* pInputPath = argv[i];

                if (!pInputPath)
                {
                    Error("Error: \'-dir\' requires a valid path argument. NULL path\n");
                }

                V_strcpy(g_szGameMaterialSrcDir, pInputPath);
            }
            else
            {
                Error("Error: \'-dir\' requires a valid path argument.\n");
            }
        }
        else if (!V_stricmp(argv[i], "-game") || !V_stricmp(argv[i], "-vproject"))
        {
            if (++i < argc && argv[i][0] != '-')
            {
                char* pGamePath = argv[i];

                if (!pGamePath)
                {
                    Error("Error: \'-game\' requires a valid path argument. NULL path\n");
                }

                V_strcpy(gamedir, pGamePath);
            }
            else
            {
                Error("Error: \'-game\' requires a valid path argument.\n");
            }
        }
        else
        {
            Warning("\nUnknown option \'%s\'\n", argv[i]);
            PrintUsage(argc, argv);
        }
    }
}


//-----------------------------------------------------------------------------
// Purpose:   Main funtion
//-----------------------------------------------------------------------------
int main(int argc, char* argv[])
{
	SetupDefaultToolsMinidumpHandler();
	CommandLine()->CreateCmdLine(argc, argv);
	InstallSpewFunction();
	PrintHeader();
	ParseCommandline(argc, argv);
	Init();
	
    if (g_bIsSingleFile)
    {
        CheckAndExportToPsd(g_szSingleInputFile);
    }
    else 
    {
        ProcessDirAndConvertContents();
    }

	DeleteCmdLine(argc, argv);
	CmdLib_Cleanup();
	CmdLib_Exit(1);
	return 0;
}
