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

#include "backends/graphics/miyoo/miyoomini-graphics.h"

void MiyooMiniGraphicsManager::initGraphicsSurface() {
	_hwScreen = nullptr;
	_realHwScreen = SDL_SetVideoMode(_videoMode.hardwareWidth, _videoMode.hardwareHeight, 32,
					 SDL_HWSURFACE);
	if (!_realHwScreen)
		return;
	_hwScreen = SDL_CreateRGBSurface(SDL_HWSURFACE, _videoMode.hardwareWidth, _videoMode.hardwareHeight,
					 _realHwScreen->format->BitsPerPixel,
					 _realHwScreen->format->Rmask,
					 _realHwScreen->format->Gmask,
					 _realHwScreen->format->Bmask,
					 _realHwScreen->format->Amask);
	_isDoubleBuf = false;
	_isHwPalette = false;
}

void MiyooMiniGraphicsManager::unloadGFXMode() {
	if (_realHwScreen) {
		SDL_FreeSurface(_realHwScreen);
		_realHwScreen = nullptr;
	}
	SurfaceSdlGraphicsManager::unloadGFXMode();
}

void MiyooMiniGraphicsManager::updateScreen(SDL_Rect *dirtyRectList, int actualDirtyRects) {
	SDL_BlitSurface(_hwScreen, nullptr, _realHwScreen, nullptr);
	SDL_UpdateRects(_realHwScreen, actualDirtyRects, _dirtyRectList);
}

void MiyooMiniGraphicsManager::getDefaultResolution(uint &w, uint &h) {
	w = 640;
	h = 480;
}
