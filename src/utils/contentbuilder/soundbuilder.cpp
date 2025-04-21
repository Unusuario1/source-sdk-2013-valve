#include <cstddef>
#include <windows.h>
#include "filesystem_init.h"

#include "soundbuilder.h"
#include "colorscheme.h"
#include "shared.h"

/*
-r                  : recurse subdirectories
-dump               : dump wave info
-genericformat2wav file.*  : convert .mp3 file to .wav file
-stretch <timescale> file.wav  : Rescales the time of file.wav by the scale factor, output as file_stretch.wav
-volume             : sets the volume of the file
*/

namespace SoundBuilder
{
	//Does audioprocess.exe exist?
	void AssetToolCheck(const char* gamebin)
	{
		Shared::AssetToolCheck(gamebin, NAME_SOUND_TOOL, "SoundBuilder");
	}

	void LoadGameInfoKv(const char* vtex_argv)
	{
		//Load gameinfo.txt KeyValues

	}

	void SoundCompile()
	{
		Shared::PrintHeaderCompileType("Sounds");

		// TODO: make this work!

		//audioprocess.exe

		//We need to read the keyvalues of audioprocess
		Shared::StartExe("Sounds", NAME_SOUND_TOOL, "");
	}
}