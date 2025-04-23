#ifndef SHARED_H
#define SHARED_H
#include <stdio.h>
#include <cstddef>
#include <windows.h>
#include "basetypes.h"

#include "contentbuilder.h"

//Here is a quick explanation of the KeyValues system in contentbuilder.exe: 
//  contentbuilder.exe builds all of the posible types of assets in the engine.
//  Since contentbuilder is a buildsystem it relies on other tools to compile 
//  those assets, the managment about all the paths and assets if not done by the 
//  native tools is hadled by the buildsystem (contentbuilder). 
//  
//  Every tool has a KeyValue asociated to them and before calling Shared::StartExe 
//  we pass everything inside the KeyValue (what is inside the KV are commandline arg):
//  e.g: (MaterialBuilder is what handles compile of textures and copying of .vmt files to game dir)
//  (NOTE: every tool KV needs to be inside ContentBuilder)
//  
//  ContenBuilder
//  {    
//      MaterialBuilder
//      {
//          //put what evert command line you want that vtex.exe supports
//      }
// 
//      //Here it goes other KV of other compile tools
//  }
//  
//  Here are all the asset types of source engine that needs to be compiled in order to load into the engine
//      - Materials: .vmt     ,These even though these files dont
//      - Textures: *.tga -> *.vtf
//      - Models:   *.qc -> *.mdl
//      - Sounds:   *.mp3 (and other formats) -> *.wav
//      - Scene:    *.vcd -> scene.image
//      - Caption:  *.txt -> *.dat
//      - Maps:     *.vmf or *.vmn -> .bsp
// 
//  Example of a complete set of KeyValues inside "gameinfo.txt" (This is needed for the buildsystem to work)
// 
//  ContentBuilder
//	{
//      //Inside this KV the user passes the commandline args to vtex.exe
//		MaterialBuilder
//		{
//          
//		}
// 
//      //Inside this KV the user passes the commandline args to studiomdl.exe
//		ModelBuilder
//		{
//
//		}
// 
//      //Inside this KV the user passes the commandline args to audioprocess.exe
//		SoundBuilder
//		{
//
//		}
//
//      //Inside this KV the user passes the commandline args to sceneimagebuilder.exe
//		SceneBuilder
//		{
//
//		}
//
//      //Inside this KV the user passes the commandline args to captioncompiler.exe
//		CaptionBuilder
//		{
//
//		}
//      
//      //Since compiling maps is a complex process everything inside "MapBuilder" control the building
//		MapBuilder
//		{
//          //This is relative to mapsrc!
//          Exclude
//          {
//              instances
//              prefab
//              dev
//          }
// 
//          //Inside this KV the user passes the commandline args to vbsp.exe
//			Vbsp
//			{
//              -low
//              -FullMinidumps
//			}
// 
//          //Inside this KV the user passes the commandline args to vvis.exe
//			Vvis
//			{
//              -low
//              -FullMinidumps
//			}
//      
//          //Inside this KV the user passes the commandline args to vrad.exe
//			Vrad
//			{
//              -final
//              -low
//              -FullMinidumps
//			}
//
//          
//          //After the bsp compile do we build nav meshes? if no set this value to 0 
//			NavBuilder 1
//          //After the bsp compile do we build node graph? if no set this value to 0 
//			NodeGraphBuilder 1
//		}
// 
//      //Inside this KV the user passes the commandline args to vpk.exe
//		VpkBuilder
//		{
//
//		}
//	}
// 

// TODO: change this to enum!

//content builder stuff
#define CONTENTBUILDER_OUTPATH      "_build"

//Common paths
#define MATERIALS_DIR               "materials"
#define MATERIALSRC_DIR             "materialsrc"
#define MODELS_DIR                  "models"
#define MODELSRC_DIR                "modelsrc"
#define SOUNDS_DIR                  "sounds"
#define SOUNDSRC_DIR                "soundsrc"
#define SCENE_DIR                   "scenes"
#define SCENESRC_DIR                "scenes"
#define CAPTION_DIR                 "resource"
#define CAPTIONSRC_DIR              "resource"
#define MAPS_DIR                    "maps" 
#define MAPSRC_DIR                  "mapsrc"

//Bin tools paths
#define TOOLS_PATH_64BITS           "bin\\x64"
#define TOOLS_PATH_32BITS           "bin"

//Common tool names
#define NAME_MATERIAL_TOOL          "vtex.exe"
#define NAME_MODEL_TOOL             "studiomdl.exe"
#define NAME_SOUND_TOOL             "audioprocess.exe"
#define NAME_SCENE_TOOL             "sceneimagebuilder.exe"
#define NAME_CAPTION_TOOL           "captioncompiler.exe"
#define NAME_MAP_GEOMETRY_TOOL      "vbsp.exe"
#define NAME_MAP_VISIBILITY_TOOL    "vvis.exe"
#define NAME_MAP_RADIOSITY_TOOL     "vrad.exe"
#define NAME_MAP_BPSINFO_TOOL       "vbspinfo.exe"
#define NAME_MAP_ZIP_TOOL           "bspzip.exe"
#define NAME_VALVEPAKFILE_TOOL      "vpk.exe"

//Common extension
#define MATERIALS_EXTENSION         ".vmt"
#define TEXTURE_EXTENSION           ".vtf"
#define TEXTURESRC_EXTENSION1       ".tga"
#define TEXTURESRC_EXTENSION2       ".pfm"
#define TEXTURESRC_EXTENSION3       ".psd"
#define MODELS_EXTENSION            ".mdl"
#define MODELSRC_EXTENSION          ".qc"
#define SOUNDS_EXTENSION            ".wav"
#define SOUNDSRC_EXTENSION          ".mp3"      //add more type of sounds
#define SCENE_EXTENSION             ".image"
#define SCENESRC_EXTENSION          ".vcd"
#define CAPTION_EXTENSION           ".dat"
#define CAPTIONSRC_EXTENSION        ".txt"
#define MAPS_EXTENSION              ".bsp"
#define MAPSRC_EXTENSION            ".vmf"      
#define MAPSRC_EXTENSION2           ".vmn" 
#define VALVEPAKFILE_EXTENSION      ".vpk"

//Common commandline strings
#define TOOL_VERBOSE_OR_QUIET_MODE          verbose ? "-v" : (g_quiet ? "-quiet" : "")
#define TOOL_VERBOSE_MODE                   verbose ? "-v" : ""
#define DEFAULT_TEXTURE_COMMANDLINE         "-nopause -deducepath -nop4"
#define DEFAULT_MODEL_COMMANDLINE           "-nop4"
#define DEFAULT_SOUND_COMMANDLINE           ""
#define DEFAULT_SCENE_COMMANDLINE           "-nopause"
#define DEFAULT_CAPTION_COMMANDLINE         ""
#define DEFAULT_GEOMETRY_COMMANDLINE        ""
#define DEFAULT_VISIBILITY_COMMANDLINE      ""
#define DEFAULT_RADIOSITY_COMMANDLINE       ""
#define DEFAULT_ZIP_COMMANDLINE             ""
#define DEFAULT_VALVEPAKFILE_COMMANDLINE    "-M"

//Extras
#define GAMEINFO                    "gameinfo.txt"
#define ADDONINFO                   "addoninfo.txt"
#define CONTENTBUILDER_KV           "ContentBuilder"
#define MATERIALBUILDER_KV          "MaterialBuilder"
#define MODELBUILDER_KV             "ModelBuilder"
#define SOUNDBUILDER_KV             "SoundBuilder"
#define SCENEBUILDER_KV             "SceneBuilder"
#define CAPTIONBUILDER_KV           "CaptionBuilder"
#define MAPBUILDER_KV               "MapBuilder"
#define MAP_GEOMETRY_KV             "Vbsp"
#define MAP_VISIBILITY_KV           "Vvis"
#define MAP_RADIOSITY_KV            "Vrad"
#define MAP_BSPINFO_KV              "VbspInfo"
#define VPKBUILDER_KV               "VpkBuilder"
#define EXCLUDE_KV                  "Exclude"       // Exclude folder and files   


namespace Shared
{
    void qError(const char* format, ...);
    void StartExe(const char* type_asset, const char* tool_name, const char* Keyvalues);
    std::size_t CountAssets(const char* directory, const char* asset_type);
    void CreateAssetSystemGamePath(const char* gamedir, const char* asset_dir);
    void LoadGameInfoKv(const char* ToolKeyValue, char* output_argv, std::size_t bufferSize);
    void AssetToolCheck(const char* gamebin, const char* tool_name, const char* sub_system);
    void CopyFilesRecursivelyGame(const char* srcDir, const char* srcfolder, const char* gamefolder, const char* extension);
    wchar_t* CtoWc(char* input, wchar_t* output, std::size_t size);
    bool TargetPlatform();
    char* ReplaceSubstring(const char* str, const char* old_sub, const char* new_sub);
    bool DirectoryAssetTypeExist(const char* directoryPath, const char* extension, const char* asset_type);
    bool CheckIfFileExist(const char* path);
    bool DirectoryExists(const char* path);
    void SetUpBinDir(char* string, size_t bufferSize);
    const char* TimeStamp();
    void AssetInfoBuild(const char* folder, const char* extension);
    void AssetInfoBuild(WIN32_FIND_DATAA findData);
    bool HasExtension(const char* filename, const char* extension);
    void PrintHeaderCompileType(const char* compile_type);
    bool PartialBuildAsset(const char* asset_path, const char* asset_src_folder, const char* asset_folder);
    bool CopyDirectoryContents(const char* srcPath, const char* dstPath, const char* extension);
    void DeleteFolderWithContents(const char* folderPath);
    bool CreateDirectoryRecursive(const char* path);
    bool ExcludeDirOrFile(const char* assetpath, const char* AssetSystem_KV);
}


#endif //SHARED_H