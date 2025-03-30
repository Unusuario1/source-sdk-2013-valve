#include <cstddef>

#include "captionbuilder.h"
#include "shared.h"


namespace CaptionBuilder
{
	//Does captioncompiler.exe exist?
	void AssetToolCheck(const char* gamebin)
	{
		Shared::AssetToolCheck(gamebin, NAME_CAPTION_TOOL, "CaptionBuilder");
	}

	void LoadGameInfoKv(const char* vtex_argv)
	{
		//by default we start vtex with: -mkdir -deducepath -crcvalidate

	}

	void CaptionCompile(const char* gamebin, std::size_t bufferSize, std::size_t &complete, std::size_t &error)
	{
		Shared::StartExe(gamebin, bufferSize, "Captions", NAME_CAPTION_TOOL, "", complete, error, false);
	}
}
