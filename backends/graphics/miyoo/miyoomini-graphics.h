/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef BACKENDS_GRAPHICS_MIYOOMINI_H
#define BACKENDS_GRAPHICS_MIYOOMINI_H

#include "backends/graphics/surfacesdl/surfacesdl-graphics.h"

class MiyooMiniGraphicsManager : public SurfaceSdlGraphicsManager {
public:
	MiyooMiniGraphicsManager(SdlEventSource *sdlEventSource, SdlWindow *window) : SurfaceSdlGraphicsManager(sdlEventSource, window), _realHwScreen(nullptr) {}

	void initGraphicsSurface() override;
	void unloadGFXMode() override;
	void updateScreen(SDL_Rect *dirtyRectList, int actualDirtyRects) override;

private:
	SDL_Surface *_realHwScreen;
};

#endif /* BACKENDS_GRAPHICS_MIYOOMINI_H */
