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

#include "common/config-manager.h"
#include "common/file.h"

#include "freescape/freescape.h"
#include "freescape/language/8bitDetokeniser.h"

namespace Freescape {

DarkEngine::DarkEngine(OSystem *syst, const ADGameDescription *gd) : FreescapeEngine(syst, gd) {
	_viewArea = Common::Rect(40, 24, 279, 124);
	_playerHeightNumber = 1;
	_playerHeights.push_back(16);
	_playerHeights.push_back(48);

	_playerHeight = _playerHeights[_playerHeightNumber];
	_playerHeight = 64;
	_playerWidth = 12;
	_playerDepth = 32;
}

void DarkEngine::loadAssets() {
	Common::File file;
	if (_renderMode == "ega") {
		loadBundledImages();
		file.open("DSIDEE.EXE");

		if (!file.isOpen())
			error("Failed to open DSIDEE.EXE");

		loadFonts(&file, 0xa113);
		load8bitBinary(&file, 0xa280, 16);
	} else if (_renderMode == "cga") {
		loadBundledImages();
		file.open("DSIDEC.EXE");

		if (!file.isOpen())
			error("Failed to open DSIDEC.EXE");
		load8bitBinary(&file, 0x7bb0, 4); // TODO
	} else
		error("Invalid render mode %s for Dark Side", _renderMode.c_str());
}

void DarkEngine::gotoArea(uint16 areaID, int entranceID) {
	debugC(1, kFreescapeDebugMove, "Jumping to area: %d, entrance: %d", areaID, entranceID);
	if (!_gameStateBits.contains(areaID))
		_gameStateBits[areaID] = 0;

	assert(_areaMap.contains(areaID));
	_currentArea = _areaMap[areaID];
	_currentArea->show();

	_currentAreaMessages.clear();
	_currentAreaMessages.push_back(_currentArea->name);

	int scale = _currentArea->getScale();
	assert(scale > 0);

	if (entranceID > 0 || areaID == 127) {
		traverseEntrance(entranceID);
	} else if (entranceID == 0) {
		Math::Vector3d diff = _lastPosition - _position;
		// debug("dif: %f %f %f", diff.x(), diff.y(), diff.z());
		//  diff should be used to determinate which entrance to use
		int newPos = -1;
		if (ABS(diff.x()) < ABS(diff.z())) {
			if (diff.z() > 0)
				newPos = 4000;
			else
				newPos = 100;
			_position.setValue(2, newPos);
		} else {
			if (diff.x() > 0)
				newPos = 4000;
			else
				newPos = 100;
			_position.setValue(0, newPos);
		}
		assert(newPos != -1);
	}

	debugC(1, kFreescapeDebugMove, "starting player position: %f, %f, %f", _position.x(), _position.y(), _position.z());
	playSound(5, false);
	_lastPosition = _position;

	// Ignore sky/ground fields
	if (_currentArea->getAreaFlags() == 1)
		_gfx->_keyColor = 0;
	else
		_gfx->_keyColor = 255;
}

void DarkEngine::drawUI() {
	_gfx->renderCrossair(0, _crossairPosition);

	if (_currentAreaMessages.size() == 1) {
		_gfx->setViewport(_fullscreenViewArea);

		Graphics::Surface *surface = new Graphics::Surface();
		surface->create(_screenW, _screenH, _gfx->_currentPixelFormat);
		surface->fillRect(_fullscreenViewArea, 0xA0A0A0FF);

		int score = _gameStateVars[k8bitVariableScore];

		uint32 yellow = 0xFFFF55FF;
		uint32 black = 0x000000FF;

		drawStringInSurface(_currentAreaMessages[0], 112, 177, yellow, black, surface);
		drawStringInSurface(Common::String::format("%04d", 2 * int(_position.x())), 201, 137, yellow, black, surface);
		drawStringInSurface(Common::String::format("%04d", 2 * int(_position.z())), 201, 145, yellow, black, surface);
		drawStringInSurface(Common::String::format("%04d", 2 * int(_position.y())), 201, 153, yellow, black, surface);

		drawStringInSurface(Common::String::format("%07d", score), 95, 8, yellow, black, surface);

		if (!_uiTexture)
			_uiTexture = _gfx->createTexture(surface);
		else
			_uiTexture->update(surface);

		_gfx->drawTexturedRect2D(_fullscreenViewArea, _fullscreenViewArea, _uiTexture);
		surface->free();
		delete surface;
	}

	_gfx->setViewport(_viewArea);
}

} // End of namespace Freescape