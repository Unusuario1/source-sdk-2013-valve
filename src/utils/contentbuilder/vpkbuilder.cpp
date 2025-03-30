#include <cstddef>

#include "vpkbuilder.h"
#include "shared.h"


namespace VpkBuilder
{
	//Does vpk.exe exist?
	void AssetToolCheck(const char* gamebin)
	{
		Shared::AssetToolCheck(gamebin, NAME_VALVEPAKFILE_TOOL, "VpkBuilder");
	}

	void LoadGameInfoKv(const char* vtex_argv)
	{
		//by default we start vtex with: -mkdir -deducepath -crcvalidate

	}

	void VpkCompile(const char* gamebin, std::size_t bufferSize, std::size_t &complete, std::size_t &error)
	{
		//We need to read the keyvalues of vpk
		Shared::StartExe(gamebin, bufferSize, "Valve Pack File (vpk)", NAME_VALVEPAKFILE_TOOL, "", complete, error, false);
	}
}