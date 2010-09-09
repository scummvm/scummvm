/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
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
 * $URL$
 * $Id$
 *
 */

#ifndef GPH_SDL_H
#define GPH_SDL_H

#include "backends/platform/sdl/sdl.h"

// FIXME: For now keep hacks in this header to save polluting the SDL backend.
enum {
    GFX_HALF = 12
};

#define __GP2XWIZ__
#define MIXER_DOUBLE_BUFFERING 1

#ifndef PATH_MAX
	#define PATH_MAX 255
#endif

class OSystem_GPH : public OSystem_SDL {
public:
	OSystem_GPH() {}

	/* Graphics */
    void initSize(uint w, uint h);
    void setGraphicsModeIntern();
    bool setGraphicsMode(int mode);
    void internUpdateScreen();
    const OSystem::GraphicsMode *getSupportedGraphicsModes() const;
	bool setGraphicsMode(const char *name);
    int getDefaultGraphicsMode() const;
    bool loadGFXMode();
    void drawMouse();
    void undrawMouse();
    void showOverlay();
    void hideOverlay();

	/* Event Stuff */
	void moveStick();
	void fillMouseEvent(Common::Event&, int, int);
	void warpMouse(int, int);
	bool remapKey(SDL_Event&, Common::Event&);

	/* Platform Setup Stuff */
	void addSysArchivesToSearchSet(Common::SearchSet &s, int priority);
	void initBackend();
	void quit();

protected:
	bool _stickBtn[32];

	bool handleMouseButtonDown(SDL_Event &ev, Common::Event &event);
	bool handleMouseButtonUp(SDL_Event &ev, Common::Event &event);
	bool handleJoyButtonDown(SDL_Event &ev, Common::Event &event);
	bool handleJoyButtonUp(SDL_Event &ev, Common::Event &event);
};

#endif //GPH_SDL_H
