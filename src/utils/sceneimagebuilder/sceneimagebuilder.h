#ifndef VCDGEN_H
#define VCDGEN_H

#include "appframework/tier3app.h"
#include "../game/shared/sceneimage.h"
#include "../game/shared/choreoevent.h"
#include "../game/shared/sceneimage.cpp"


class CSceneImageBuilderApp : public CTier3SteamApp, public ISceneImage
{
private:
	CSceneImage sceneImage;
	typedef CTier3SteamApp BaseClass;

	virtual bool SetupSearchPaths();
public:
	virtual bool CreateSceneImageFile(CUtlBuffer& targetBuffer, char const* pchModPath, bool bLittleEndian, bool bQuiet, ISceneCompileStatus* Status) override;
	virtual bool PreInit();
	virtual void PostShutdown();
	virtual bool Create();
	virtual void Destroy();
	virtual int	 Main();
    virtual void SceneBuild();
};

#endif //VCDGEN_H