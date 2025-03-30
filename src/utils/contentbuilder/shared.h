#ifndef SHARED_H
#define SHARED_H
#include <stdio.h>
#include <cstddef>
#include <windows.h>


//Common paths
#define MATERIALS_DIR               "materials"
#define MATERIALSRC_DIR             "materialsrc"
#define MODELS_DIR                  "models"
#define MODELSRC_DIR                "modelsrc"
#define SOUNDS_DIR                  "sounds"
#define SOUNDSRC_DIR                "soundsrc"
#define SCENE_DIR                   "scene"
#define SCENESRC_DIR                "scene"
#define CAPTION_DIR                 "resource"
#define CAPTIONSRC_DIR              "resource"
#define MAPS_DIR                    "maps" 
#define MAPSRC_DIR                  "mapsrc"

//Common tool names
#define NAME_MATERIAL_TOOL          "vtex.exe"
#define NAME_MODEL_TOOL             "studiomdl.exe"
#define NAME_SOUND_TOOL             "audioprocess.exe"
#define NAME_SCENE_TOOL             "vcdgen.exe"
#define NAME_CAPTION_TOOL           "captioncompiler.exe"
#define NAME_MAP_GEOMETRY_TOOL      "vbsp.exe"
#define NAME_MAP_VISIBILITY_TOOL    "vvis.exe"
#define NAME_MAP_RADIOSITY_TOOL     "vrad.exe"
#define NAME_MAP_ZIP_TOOL           "bspzip.exe"
#define NAME_VALVEPAKFILE_TOOL      "vpk.exe"

//Common extension
#define MATERIALS_EXTENSION         ".vtf"
#define MATERIALSRC_EXTENSION       ".tga"
#define MODELS_EXTENSION            ".mdl"
#define MODELSRC_EXTENSION          ".qc"
#define SOUNDS_EXTENSION            ".wav"
#define SOUNDSRC_EXTENSION          ".mp3"      //add more type of sounds
#define SCENE_EXTENSION             ".image"
#define SCENESRC_EXTENSION          ".vcd"
#define CAPTION_EXTENSION           ".dat"
#define CAPTIONSRC_EXTENSION        ".txt"
#define MAPS_EXTENSION              ".bsp"
#define MAPSRC_EXTENSION            ".vmf"      //add manifiest

//Extras
#define GAMEINFO                    "gameinfo.txt"
#define CONTENTBUILDER_KV           "ContentBuilder"
#define MATERIALBUILDER_KV          "MaterialBuilder"
#define MODELBUILDER_KV             "ModelBuilder"
#define SOUNDBUILDER_KV             "SoundBuilder"
#define SCENEBUILDER_KV             "SceneBuilder"
#define CAPTIONBUILDER_KV           "CaptionBuilder"
#define MAPBUILDER_KV               "MapBuilder"
#define VPKBUILDER_KV               "VpkBuilder"


namespace Shared
{
    void qError(const char* format, ...);
    void StartExe(const char* gamebin, std::size_t bufferSize, const char* type_asset, const char* tool_name,
                                                const char* Keyvalues, std::size_t &complete, std::size_t &error);
    std::size_t CountAssets(const char* directory, const char* asset_type);
    void CreateAssetSystemGamePath(const char* gamedir, const char* asset_dir);
    void LoadGameInfoKv(const char* ToolKeyValue, char* output_argv, std::size_t bufferSize);
    void AssetToolCheck(const char* gamebin, const char* tool_name, const char* sub_system);
    wchar_t* CtoWc(char* input, wchar_t* output, std::size_t size);
    bool TargetPlatform();
    bool CheckIfFileExist(const char* path);
    bool DirectoryExists(const char* path);
    void SetUpBinDir(char* string, size_t bufferSize);
}


#endif //SHARED_H