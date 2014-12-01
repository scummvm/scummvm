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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef BACKENDS_GRAPHICS_SURFACESDL20_GRAPHICS_H
#define BACKENDS_GRAPHICS_SURFACESDL20_GRAPHICS_H

#include "backends/graphics/surfacesdl/surfacesdl-graphics.h"

class SurfaceSdl20GraphicsManager : public SurfaceSdlGraphicsManager {
public:
	SurfaceSdl20GraphicsManager(SdlEventSource *sdlEventSource)
	:
	SurfaceSdlGraphicsManager(sdlEventSource), _windowTitle("ScummVM"),
	_hwwindow(0), _hwrenderer(0), _hwtexture(0) {}

	virtual void setWindowCaption(const char *title, const char *icon);

protected:
	SDL_Window *_hwwindow;
	SDL_Renderer *_hwrenderer;
	SDL_Texture *_hwtexture;

	Common::String _windowTitle;

	virtual void warpMouse(int x, int y);
	virtual void iconifyWindow();
	virtual void setColors(SDL_Surface *surface, SDL_Color *colors, int firstcolor, int ncolors);
	virtual void setAlpha(SDL_Surface *surface,  Uint32 flag, Uint8 alpha);
	virtual void setColorKey(SDL_Surface *surface,  int flag, Uint32 key);
	virtual void createHwScreen();
	virtual void destroyHwScreen();
	virtual void blitToHwScreen();
};
#endif
