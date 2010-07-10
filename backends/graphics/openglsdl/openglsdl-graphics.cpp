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

OpenGLSDLGraphicsManager::OpenGLSDLGraphicsManager()
	:
	_hwscreen(0) {

}

OpenGLSDLGraphicsManager::~OpenGLSDLGraphicsManager() {

}

void OpenGLSDLGraphicsManager::init() {
	if (SDL_InitSubSystem(SDL_INIT_VIDEO) == -1) {
		error("Could not initialize SDL: %s", SDL_GetError());
	}

	SDL_ShowCursor(SDL_DISABLE);

	OpenGLGraphicsManager::init();
}

void OpenGLSDLGraphicsManager::forceFullRedraw() {

}

bool OpenGLSDLGraphicsManager::handleScalerHotkeys(const SDL_KeyboardEvent &key) {
	return false;
}

bool OpenGLSDLGraphicsManager::isScalerHotkey(const Common::Event &event) {
	return false;
}

void OpenGLSDLGraphicsManager::adjustMouseEvent(Common::Event &event) {

}

void OpenGLSDLGraphicsManager::setMousePos(int x, int y) {

}

void OpenGLSDLGraphicsManager::toggleFullScreen() {

}

bool OpenGLSDLGraphicsManager::saveScreenshot(const char *filename) {
	return false;
}

//
// Protected
//

bool OpenGLSDLGraphicsManager::loadGFXMode() {
	return false;
}

void OpenGLSDLGraphicsManager::unloadGFXMode() {

}

bool OpenGLSDLGraphicsManager::hotswapGFXMode() {
	return false;
}

void OpenGLSDLGraphicsManager::internUpdateScreen() {
	
}
