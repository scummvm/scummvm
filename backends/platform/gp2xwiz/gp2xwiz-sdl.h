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

#ifndef GP2XWIZ_SDL_H
#define GP2XWIZ_SDL_H

#include "backends/platform/sdl/sdl.h"
#include "backends/platform/gp2xwiz/gp2xwiz-scaler.h"

#include <SDL.h>

#define __GP2XWIZ__
#define MIXER_DOUBLE_BUFFERING 1

#ifndef PATH_MAX
	#define PATH_MAX 255
#endif

class OSystem_GP2XWIZ : public OSystem_SDL {
public:
	OSystem_GP2XWIZ() {}

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
	bool pollEvent(Common::Event &event);
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
};

#endif
