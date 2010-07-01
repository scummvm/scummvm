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

#ifdef GP2X

#include "backends/graphics/gp2xsdl/gp2xsdl-graphics.h"
#include "graphics/scaler/aspect.h"
#include <SDL_gp2x.h>

static const OSystem::GraphicsMode s_supportedGraphicsModes[] = {
	{"Fullscreen", "1x", GFX_NORMAL},
	{0, 0, 0}
};

GP2XSdlGraphicsManager::GP2XSdlGraphicsManager()
	:
	_adjustZoomOnMouse(false) {

}

const OSystem::GraphicsMode *GP2XSdlGraphicsManager::getSupportedGraphicsModes() const {
	return s_supportedGraphicsModes;
}

int GP2XSdlGraphicsManager::getDefaultGraphicsMode() const {
	return GFX_NORMAL;
}


bool GP2XSdlGraphicsManager::hasFeature(OSystem::Feature f) {
	if (f == OSystem::kFeatureIconifyWindow)
		return false;

	return SdlGraphicsManager::hasFeature(f);
}

void GP2XSdlGraphicsManager::setFeatureState(OSystem::Feature f, bool enable) {
	if (f != OSystem::kFeatureIconifyWindow)
		SdlGraphicsManager::setFeatureState(f, enable);
}

void GP2XSdlGraphicsManager::drawMouse() {
	if (!_mouseVisible || !_mouseSurface) {
		_mouseBackup.x = _mouseBackup.y = _mouseBackup.w = _mouseBackup.h = 0;
		return;
	}

	SDL_Rect zoomdst;
	SDL_Rect dst;
	int scale;
	int hotX, hotY;
	int tmpScreenWidth, tmpScreenHeight;

	// Temp vars to ensure we zoom to the LCD resolution or greater.
	tmpScreenWidth = _videoMode.screenWidth;
	tmpScreenHeight = _videoMode.screenHeight;

	if (_videoMode.screenHeight <= 240) {
		tmpScreenHeight = 240;
	}

	if (_videoMode.screenWidth <= 320) {
		tmpScreenWidth = 320;
	}

	dst.x = _mouseCurState.x;
	dst.y = _mouseCurState.y;

	if (!_overlayVisible) {
		scale = _videoMode.scaleFactor;
		dst.w = _mouseCurState.vW;
		dst.h = _mouseCurState.vH;
		hotX = _mouseCurState.vHotX;
		hotY = _mouseCurState.vHotY;
	} else {
		scale = 1;
		dst.w = _mouseCurState.rW;
		dst.h = _mouseCurState.rH;
		hotX = _mouseCurState.rHotX;
		hotY = _mouseCurState.rHotY;
	}

	// The mouse is undrawn using virtual coordinates, i.e. they may be
	// scaled and aspect-ratio corrected.

	_mouseBackup.x = dst.x - hotX;
	_mouseBackup.y = dst.y - hotY;
	_mouseBackup.w = dst.w;
	_mouseBackup.h = dst.h;

	// We draw the pre-scaled cursor image, so now we need to adjust for
	// scaling, shake position and aspect ratio correction manually.

	if (!_overlayVisible) {
		dst.y += _currentShakePos;
	}

	if (_videoMode.aspectRatioCorrection && !_overlayVisible)
		dst.y = real2Aspect(dst.y);

	dst.x = scale * dst.x - _mouseCurState.rHotX;
	dst.y = scale * dst.y - _mouseCurState.rHotY;
	dst.w = _mouseCurState.rW;
	dst.h = _mouseCurState.rH;

	// Hacking about with the zoom around mouse pointer stuff.
	if (_adjustZoomOnMouse){

		zoomdst.w = (tmpScreenWidth / 2);
		zoomdst.h = (tmpScreenHeight / 2);

		// Create a zoomed rect centered on the mouse pointer.
		// Will pan 1/4 of the screen.

		if (dst.x > ((tmpScreenWidth / 4) * 3)) {
			zoomdst.x = (tmpScreenWidth / 2);
		} else {
			zoomdst.x = (dst.x - (tmpScreenWidth / 4));
			if (zoomdst.x < 0) {
				zoomdst.x = 0;
			}
		}

		if (dst.y > ((tmpScreenHeight / 4) * 3)) {
			zoomdst.y = (tmpScreenHeight / 2);
		} else {
			zoomdst.y = (dst.y - (tmpScreenHeight / 4));
			if (zoomdst.y < 0) {
				zoomdst.y = 0;
			}
		}
		SDL_GP2X_Display(&zoomdst);
	} else {

		// Make sure we are looking at the whole screen otherwise.

		zoomdst.x = 0;
		zoomdst.y = 0;
		zoomdst.w = (tmpScreenWidth);
		zoomdst.h = (tmpScreenHeight);

		SDL_GP2X_Display(&zoomdst);
	};

	// Note that SDL_BlitSurface() and addDirtyRect() will both perform any
	// clipping necessary

	if (SDL_BlitSurface(_mouseSurface, NULL, _hwscreen, &dst) != 0)
		error("SDL_BlitSurface failed: %s", SDL_GetError());

	// The screen will be updated using real surface coordinates, i.e.
	// they will not be scaled or aspect-ratio corrected.

	addDirtyRect(dst.x, dst.y, dst.w, dst.h, true);
}

void GP2XSdlGraphicsManager::toggleZoomOnMouse() {
	_adjustZoomOnMouse = !_adjustZoomOnMouse;
}

#endif
