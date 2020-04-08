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

#include "common/scummsys.h"

#if defined(GPH_DEVICE)

#include "backends/graphics/gph/gph-graphics.h"

GPHGraphicsManager::GPHGraphicsManager(SdlEventSource *sdlEventSource, SdlWindow *window)
	: DownscaleSdlGraphicsManager(sdlEventSource, window) {
}

void GPHGraphicsManager::initSize(uint w, uint h, const Graphics::PixelFormat *format) {
	DownscaleSdlGraphicsManager::initSize(w, h, format);

	_videoMode.overlayWidth = 320;
	_videoMode.overlayHeight = 240;
}

void GPHGraphicsManager::setupHardwareSize() {
	// Set the hardware stats to match the LCD.
	_videoMode.hardwareWidth = 320;
	_videoMode.hardwareHeight = 240;

	if (_videoMode.screenHeight != 200)
		_videoMode.aspectRatioCorrection = false;

	if (_videoMode.screenWidth > 320 || _videoMode.screenHeight > 240) {
		_videoMode.aspectRatioCorrection = false;
		setGraphicsMode(GFX_HALF);
	} else {
		setGraphicsMode(GFX_NORMAL);
	}

	if ((_videoMode.mode == GFX_HALF) && !_overlayVisible) {
		_videoMode.overlayWidth = _videoMode.screenWidth / 2;
		_videoMode.overlayHeight = _videoMode.screenHeight / 2;
	} else {
		_videoMode.overlayWidth = _videoMode.screenWidth * _videoMode.scaleFactor;
		_videoMode.overlayHeight = _videoMode.screenHeight * _videoMode.scaleFactor;

		if (_videoMode.aspectRatioCorrection)
			_videoMode.overlayHeight = real2Aspect(_videoMode.overlayHeight);
	}
}

bool GPHGraphicsManager::loadGFXMode() {
	bool success = DownscaleSdlGraphicsManager::loadGFXMode();

	// The old GP2X hacked SDL needs this after any call to SDL_SetVideoMode
	// and it does not hurt other devices.
	SDL_ShowCursor(SDL_DISABLE);

	return success;
}

#endif
