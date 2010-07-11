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

void OpenGLSdlGraphicsManager::setMousePos(int x, int y) {

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

	return true;
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
