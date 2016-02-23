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

#include "titanic/screen_manager.h"
#include "titanic/video_surface.h"

namespace Titanic {

CScreenManagerRec::CScreenManagerRec() {
	_field0 = 0;
	_field4 = 0;
	_field8 = 0;
	_fieldC = 0;
}

/*------------------------------------------------------------------------*/

CScreenManager *CScreenManager::_screenManagerPtr;
CScreenManager *CScreenManager::_currentScreenManagerPtr;

CScreenManager::CScreenManager(TitanicEngine *vm): _vm(vm) {
	_screenManagerPtr = nullptr;
	_currentScreenManagerPtr = nullptr;

	_frontRenderSurface = nullptr;
	_mouseCursor = nullptr;
	_textCursor = nullptr;
	_fontNumber = 0;
}

CScreenManager::~CScreenManager() {
	_screenManagerPtr = nullptr;
}

void CScreenManager::setWindowHandle(int v) {
	// Not needed
}

bool CScreenManager::resetWindowHandle(int v) {
	proc27();
	return true;
}

CScreenManager *CScreenManager::setCurrent() {
	if (!_currentScreenManagerPtr)
		_currentScreenManagerPtr = _screenManagerPtr;

	return _currentScreenManagerPtr;
}

/*------------------------------------------------------------------------*/

OSScreenManager::OSScreenManager(TitanicEngine *vm): CScreenManager(vm),
		_directDrawManager(vm, 0) {
	_field48 = 0;
	_field4C = 0;
	_field50 = 0;
	_field54 = 0;
}

OSScreenManager::~OSScreenManager() {
	destroyFrontAndBackBuffers();
}

void OSScreenManager::setMode(int width, int height, int bpp, uint numBackSurfaces, bool flag2) {
	destroyFrontAndBackBuffers();
	_directDrawManager.initVideo(width, height, bpp, numBackSurfaces);

	_frontRenderSurface = new OSVideoSurface(this, nullptr);
	_frontRenderSurface->setSurface(this, _directDrawManager._mainSurface);

	for (uint idx = 0; idx < numBackSurfaces; ++idx) {
		OSVideoSurface videoSurface(this, nullptr);
		videoSurface.setSurface(this, _directDrawManager._backSurfaces[idx]);
	}

	// Load fonts
	_fonts[0].load(149);
	_fonts[1].load(151);
	_fonts[2].load(152);
	_fonts[3].load(153);

	// Load the cursors
	loadCursors();
}

void OSScreenManager::proc5() {}
void OSScreenManager::proc6() {}
void OSScreenManager::proc7() {}
void OSScreenManager::proc8() {}
void OSScreenManager::proc9() {}
void OSScreenManager::proc10() {}
void OSScreenManager::proc11() {}
void OSScreenManager::proc12() {}
void OSScreenManager::proc13() {}
void OSScreenManager::proc14() {}
void OSScreenManager::proc15() {}
void OSScreenManager::proc16() {}
void OSScreenManager::getFont() {}
void OSScreenManager::proc18() {}
void OSScreenManager::proc19() {}
void OSScreenManager::proc20() {}
void OSScreenManager::proc21() {}
void OSScreenManager::proc22() {}
void OSScreenManager::proc23() {}
void OSScreenManager::proc24() {}
void OSScreenManager::proc25() {}
void OSScreenManager::showCursor() {}
void OSScreenManager::proc27() {}

void OSScreenManager::destroyFrontAndBackBuffers() {
	delete _frontRenderSurface;
	_frontRenderSurface = nullptr;

	for (uint idx = 0; idx < _backSurfaces.size(); ++idx)
		delete _backSurfaces[idx];
	_backSurfaces.clear();
}

void OSScreenManager::loadCursors() {
	// TODO
}

} // End of namespace Titanic
