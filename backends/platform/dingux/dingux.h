
#ifndef SDL_DINGUX_COMMON_H
#define SDL_DINGUX_COMMON_H

#include <SDL.h>

#include "backends/base-backend.h"
#include "backends/platform/sdl/sdl.h"

#if defined(DINGUX)

enum {
	GFX_HALF = 12
};

class OSystem_SDL_Dingux : public OSystem_SDL {
public:
	virtual bool hasFeature(Feature f);
	virtual void setFeatureState(Feature f, bool enable);
	virtual bool getFeatureState(Feature f);
	virtual int getDefaultGraphicsMode() const;
	void initSize(uint w, uint h);
	const OSystem::GraphicsMode *getSupportedGraphicsModes() const;
	bool setGraphicsMode(const char *name);
	bool setGraphicsMode(int mode);
	void setGraphicsModeIntern();
	void internUpdateScreen();
	void showOverlay();
	void hideOverlay();
	bool loadGFXMode();
	void drawMouse();
	void undrawMouse();	
	void warpMouse(int, int);
	void fillMouseEvent(Common::Event&, int, int);

protected:
	virtual bool remapKey(SDL_Event &ev, Common::Event &event);
};


#endif

#endif

