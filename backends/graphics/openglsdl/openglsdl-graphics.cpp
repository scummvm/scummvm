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

#include "backends/graphics/openglsdl/openglsdl-graphics.h"

OpenGLSdlGraphicsManager::OpenGLSdlGraphicsManager()
	:
	_hwscreen(0) {

}

OpenGLSdlGraphicsManager::~OpenGLSdlGraphicsManager() {

}

void OpenGLSdlGraphicsManager::init() {
	if (SDL_InitSubSystem(SDL_INIT_VIDEO) == -1) {
		error("Could not initialize SDL: %s", SDL_GetError());
	}

	SDL_ShowCursor(SDL_DISABLE);

	OpenGLGraphicsManager::init();
}

#ifdef USE_RGB_COLOR

const Graphics::PixelFormat RGBList[] = {
#if defined(ENABLE_32BIT)
	Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0), // RGBA8888
	Graphics::PixelFormat(3, 8, 8, 8, 0, 16, 8, 0, 0),  // RGB888
#endif
	Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0),  // RGB565
	Graphics::PixelFormat(2, 5, 5, 5, 0, 10, 5, 0, 0),  // RGB555
	Graphics::PixelFormat(2, 4, 4, 4, 4, 12, 8, 4, 0),  // RGBA4444
};

Common::List<Graphics::PixelFormat> OpenGLSdlGraphicsManager::getSupportedFormats() {
	static Common::List<Graphics::PixelFormat> list;
	static bool inited = false;

	if (inited)
		return list;

	int listLength = ARRAYSIZE(RGBList);

	Graphics::PixelFormat format = Graphics::PixelFormat::createFormatCLUT8();
	if (_hwscreen) {
		// Get our currently set hardware format
		format = Graphics::PixelFormat(_hwscreen->format->BytesPerPixel,
			8 - _hwscreen->format->Rloss, 8 - _hwscreen->format->Gloss,
			8 - _hwscreen->format->Bloss, 8 - _hwscreen->format->Aloss,
			_hwscreen->format->Rshift, _hwscreen->format->Gshift,
			_hwscreen->format->Bshift, _hwscreen->format->Ashift);

		// Workaround to MacOSX SDL not providing an accurate Aloss value.
		if (_hwscreen->format->Amask == 0)
			format.aLoss = 8;

		// Push it first, as the prefered format.
		for (int i = 0; i < listLength; i++) {
			if (RGBList[i] == format) {
				list.push_back(format);
				break;
			}
		}

		// Mark that we don't need to do this any more.
		inited = true;
	}

	for (int i = 0; i < listLength; i++) {
		if (inited && (RGBList[i].bytesPerPixel > format.bytesPerPixel))
			continue;
		if (RGBList[i] != format)
			list.push_back(RGBList[i]);
	}
	//list.push_back(Graphics::PixelFormat::createFormatCLUT8());
	return list;
}

#endif

void OpenGLSdlGraphicsManager::warpMouse(int x, int y) {
	if (_mouseCurState.x != x || _mouseCurState.y != y) {
		int y1 = y;

		/*if (_videoMode.aspectRatioCorrection && !_overlayVisible)
			y1 = real2Aspect(y);*/

		if (!_overlayVisible)
			SDL_WarpMouse(x * _videoMode.scaleFactor, y1 * _videoMode.scaleFactor);
		else
			SDL_WarpMouse(x, y1);

		setMousePos(x, y);
	}
}


void OpenGLSdlGraphicsManager::forceFullRedraw() {

}

bool OpenGLSdlGraphicsManager::handleScalerHotkeys(const SDL_KeyboardEvent &key) {
	return false;
}

bool OpenGLSdlGraphicsManager::isScalerHotkey(const Common::Event &event) {
	return false;
}

void OpenGLSdlGraphicsManager::adjustMouseEvent(Common::Event &event) {

}

void OpenGLSdlGraphicsManager::toggleFullScreen() {

}

bool OpenGLSdlGraphicsManager::saveScreenshot(const char *filename) {
	return false;
}

//
// Intern
//

bool OpenGLSdlGraphicsManager::loadGFXMode() {
	_videoMode.overlayWidth = _videoMode.screenWidth * _videoMode.scaleFactor;
	_videoMode.overlayHeight = _videoMode.screenHeight * _videoMode.scaleFactor;
	_videoMode.hardwareWidth = _videoMode.screenWidth * _videoMode.scaleFactor;
	_videoMode.hardwareHeight = _videoMode.screenHeight * _videoMode.scaleFactor;

	_hwscreen = SDL_SetVideoMode(_videoMode.hardwareWidth, _videoMode.hardwareHeight, 32,
		_videoMode.fullscreen ? (SDL_FULLSCREEN | SDL_OPENGL) : SDL_OPENGL
	);
	if (_hwscreen == NULL) {
		// DON'T use error(), as this tries to bring up the debug
		// console, which WON'T WORK now that _hwscreen is hosed.

		if (!_oldVideoMode.setup) {
			warning("SDL_SetVideoMode says we can't switch to that mode (%s)", SDL_GetError());
			g_system->quit();
		} else {
			return false;
		}
	}

	return OpenGLGraphicsManager::loadGFXMode();
}

void OpenGLSdlGraphicsManager::unloadGFXMode() {
	if (_hwscreen) {
		SDL_FreeSurface(_hwscreen);
		_hwscreen = NULL;
	}
}

bool OpenGLSdlGraphicsManager::hotswapGFXMode() {
	return false;
}

void OpenGLSdlGraphicsManager::internUpdateScreen() {
	
}
