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

void DarkEngine::initZX() {
	_viewArea = Common::Rect(56, 28, 265, 132);
}

void DarkEngine::loadAssetsZXDemo() {
	Common::File file;

	file.open("darkside.zx.title");
	if (file.isOpen()) {
		_title = loadAndCenterScrImage(&file);
	} else
		error("Unable to find darkside.zx.title");

	file.close();
	file.open("darkside.zx.border");
	if (file.isOpen()) {
		_border = loadAndCenterScrImage(&file);
	} else
		error("Unable to find driller.zx.border");
	file.close();

	file.open("darkside.zx.data");
	if (!file.isOpen())
		error("Failed to open darksize.zx.data");

	loadMessagesFixedSize(&file, 0x56b, 16, 27);
	loadMessagesFixedSize(&file, 0x5761, 264, 5);

	loadFonts(&file, 0x6164);
	loadGlobalObjects(&file, 0x20, 23);
	load8bitBinary(&file, 0x62c6, 4);
	for (auto &it : _areaMap) {
		addWalls(it._value);
		addECDs(it._value);
	}
}

void DarkEngine::drawZXUI(Graphics::Surface *surface) {
	uint32 color = 7;
	uint8 r, g, b;

	_gfx->readFromPalette(color, r, g, b);
	uint32 front = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	color = _currentArea->_usualBackgroundColor;
	if (_gfx->_colorRemaps && _gfx->_colorRemaps->contains(color)) {
		color = (*_gfx->_colorRemaps)[color];
	}

	_gfx->readFromPalette(color, r, g, b);
	uint32 back = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	int score = _gameStateVars[k8bitVariableScore];
	drawStringInSurface(Common::String::format("%04d", int(2 * _position.x())), 191, 141, front, back, surface);
	drawStringInSurface(Common::String::format("%04d", int(2 * _position.z())), 191, 149, front, back, surface);
	drawStringInSurface(Common::String::format("%04d", int(2 * _position.y())), 191, 157, front, back, surface);

	drawStringInSurface(Common::String::format("%02d", int(_angleRotations[_angleRotationIndex])), 78, 165, front, back, surface);
	drawStringInSurface(Common::String::format("%3d", _playerSteps[_playerStepIndex]), 78, 173, front, back, surface);
	drawStringInSurface(Common::String::format("%07d", score), 94, 13, front, back, surface);

	int seconds, minutes, hours;
	getTimeFromCountdown(seconds, minutes, hours);
	// TODO: implement binary clock

	Common::String message;
	int deadline;
	getLatestMessages(message, deadline);
	if (deadline <= _countdown) {
		drawStringInSurface(message, 112, 173, back, front, surface);
		_temporaryMessages.push_back(message);
		_temporaryMessageDeadlines.push_back(deadline);
	} else
		drawStringInSurface(_currentArea->_name, 112, 173, front, back, surface);

	int energy = _gameStateVars[k8bitVariableEnergy]; // called fuel in this game
	int shield = _gameStateVars[k8bitVariableShield];

	if (shield >= 0) {
		Common::Rect shieldBar;
		shieldBar = Common::Rect(80, 141, 151 - (k8bitMaxShield - shield), 147);
		surface->fillRect(shieldBar, back);

		shieldBar = Common::Rect(80, 142, 151 - (k8bitMaxShield - shield), 146);
		surface->fillRect(shieldBar, front);
	}

	if (energy >= 0) {
		Common::Rect energyBar;
		energyBar = Common::Rect(72, 147, 151 - (k8bitMaxEnergy - energy), 154);
		surface->fillRect(energyBar, back);

		energyBar = Common::Rect(72, 148, 151 - (k8bitMaxEnergy - energy), 153);
		surface->fillRect(energyBar, front);
	}
}

} // End of namespace Freescape