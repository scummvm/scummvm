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
	if (_renderMode == Common::kRenderEGA) {
		loadBundledImages();
		file.open("DSIDEE.EXE");

		if (!file.isOpen())
			error("Failed to open DSIDEE.EXE");

		loadFonts(&file, 0xa113);
		load8bitBinary(&file, 0xa280, 16);
	} else if (_renderMode == Common::kRenderCGA) {
		loadBundledImages();
		file.open("DSIDEC.EXE");

		if (!file.isOpen())
			error("Failed to open DSIDEC.EXE");
		load8bitBinary(&file, 0x7bb0, 4); // TODO
	} else
		error("Invalid or unsupported render mode %s for Dark Side", Common::getRenderModeDescription(_renderMode));
}

void DarkEngine::gotoArea(uint16 areaID, int entranceID) {
	debugC(1, kFreescapeDebugMove, "Jumping to area: %d, entrance: %d", areaID, entranceID);
	if (!_gameStateBits.contains(areaID))
		_gameStateBits[areaID] = 0;

	assert(_areaMap.contains(areaID));
	_currentArea = _areaMap[areaID];
	_currentArea->show();

	_currentAreaMessages.clear();
	_currentAreaMessages.push_back(_currentArea->_name);

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
	uint32 gray = _gfx->_texturePixelFormat.ARGBToColor(0x00, 0xA0, 0xA0, 0xA0);
	uint32 yellow = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0xFF, 0xFF, 0x55);
	uint32 black = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0x00, 0x00, 0x00);

	Graphics::Surface *surface = nullptr;
	if (_border) {
		surface = new Graphics::Surface();
		surface->create(_screenW, _screenH, _gfx->_texturePixelFormat);
		surface->fillRect(_fullscreenViewArea, gray);
	}

	if (_currentAreaMessages.size() == 1) {
		int score = _gameStateVars[k8bitVariableScore];
		drawStringInSurface(_currentAreaMessages[0], 112, 177, yellow, black, surface);
		drawStringInSurface(Common::String::format("%04d", 2 * int(_position.x())), 201, 137, yellow, black, surface);
		drawStringInSurface(Common::String::format("%04d", 2 * int(_position.z())), 201, 145, yellow, black, surface);
		drawStringInSurface(Common::String::format("%04d", 2 * int(_position.y())), 201, 153, yellow, black, surface);
		drawStringInSurface(Common::String::format("%07d", score), 95, 8, yellow, black, surface);
	}

	if (surface) {
		if (!_uiTexture)
			_uiTexture = _gfx->createTexture(surface);
		else
			_uiTexture->update(surface);

		_gfx->setViewport(_fullscreenViewArea);
		_gfx->drawTexturedRect2D(_fullscreenViewArea, _fullscreenViewArea, _uiTexture);
		_gfx->setViewport(_viewArea);

		surface->free();
		delete surface;
	}
}

} // End of namespace Freescape
