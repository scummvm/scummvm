/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef BACKENDS_GRAPHICS_SDL_H
#define BACKENDS_GRAPHICS_SDL_H

#ifdef USE_OPENGL
#include <SDL_opengl.h>
#endif
#undef ARRAYSIZE

#include "backends/graphics/graphics.h"
#include "graphics/pixelformat.h"
#include "graphics/scaler.h"
#include "common/events.h"
#include "common/system.h"

#include "backends/events/sdl/sdl-events.h"

#include "backends/platform/sdl/sdl-sys.h"


/**
 * SDL graphics manager
 */
class SurfaceSdlGraphicsManager : public GraphicsManager, public Common::EventObserver {
public:
	SurfaceSdlGraphicsManager(SdlEventSource *sdlEventSource);
	virtual ~SurfaceSdlGraphicsManager();

	virtual void initEventObserver();

	virtual bool hasFeature(OSystem::Feature f);
	virtual void setFeatureState(OSystem::Feature f, bool enable);
	virtual bool getFeatureState(OSystem::Feature f);

	virtual void launcherInitSize(uint w, uint h);
	byte *setupScreen(int screenW, int screenH, bool fullscreen, bool accel3d);
	virtual int getScreenChangeID() const { return _screenChangeCount; }
	virtual int16 getHeight();
	virtual int16 getWidth();

public:
	virtual void updateScreen();

	virtual void showOverlay();
	virtual void hideOverlay();
	virtual Graphics::PixelFormat getOverlayFormat() const { return _overlayFormat; }
	virtual void clearOverlay();
	virtual void grabOverlay(OverlayColor *buf, int pitch);
	virtual void copyRectToOverlay(const OverlayColor *buf, int pitch, int x, int y, int w, int h);
	virtual int16 getOverlayHeight() { return _overlayHeight; }
	virtual int16 getOverlayWidth() { return _overlayWidth; }
	void closeOverlay();

	virtual bool showMouse(bool visible);
	virtual void warpMouse(int x, int y);
	void setMouseCursor(const byte *buf, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor, int cursorTargetScale = 1, const Graphics::PixelFormat *format = NULL) {}
	
#ifdef USE_OSD
	virtual void displayMessageOnOSD(const char *msg);
#endif

	// Override from Common::EventObserver
	bool notifyEvent(const Common::Event &event);

protected:
	SdlEventSource *_sdlEventSource;

	SDL_Surface *_screen;

#ifdef USE_OPENGL
	bool _opengl;
#endif
	bool _fullscreen;

	// overlay
	SDL_Surface *_overlayscreen;
	bool _overlayVisible;
	Graphics::PixelFormat _overlayFormat;
	int _overlayWidth, _overlayHeight;
	bool _overlayDirty;
#ifdef USE_OPENGL
	int _overlayNumTex;
	GLuint *_overlayTexIds;
#endif

	/** Force full redraw on next updateScreen */
	bool _forceFull;

	int _screenChangeCount;
};

#endif
