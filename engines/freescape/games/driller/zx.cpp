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
#include "freescape/games/driller/driller.h"
#include "freescape/language/8bitDetokeniser.h"

namespace Freescape {

void DrillerEngine::initZX() {
	_viewArea = Common::Rect(56, 20, 264, 124);
	_soundIndexAreaChange = 10;
}

void DrillerEngine::loadAssetsZXFullGame() {
	Common::File file;
	file.open("driller.zx.title");
	if (file.isOpen()) {
		_title = loadAndCenterScrImage(&file);
	} else
		error("Unable to find driller.zx.title");

	file.close();

	file.open("driller.zx.border");
	if (file.isOpen()) {
		_border = loadAndCenterScrImage(&file);
	} else
		error("Unable to find driller.zx.border");
	file.close();

	file.open("driller.zx.data");

	if (!file.isOpen())
		error("Failed to open driller.zx.data");

	if (_variant & GF_ZX_DISC)
		loadMessagesFixedSize(&file, 0x2164, 14, 20);
	else
		loadMessagesFixedSize(&file, 0x20e4, 14, 20);

	if (_variant & GF_ZX_RETAIL)
		loadFonts(&file, 0x62ca);
	else if (_variant & GF_ZX_BUDGET)
		loadFonts(&file, 0x5aa8);
	else if (_variant & GF_ZX_DISC)
		loadFonts(&file, 0x63f0);

	if (_variant & GF_ZX_DISC)
		loadGlobalObjects(&file, 0x1d13, 8);
	else
		loadGlobalObjects(&file, 0x1c93, 8);

	if (_variant & GF_ZX_RETAIL)
		load8bitBinary(&file, 0x642c, 4);
	else if (_variant & GF_ZX_BUDGET)
		load8bitBinary(&file, 0x5c0a, 4);
	else if (_variant & GF_ZX_DISC)
		load8bitBinary(&file, 0x6552, 4);

	else
		error("Unknown ZX spectrum variant");
}

void DrillerEngine::drawZXUI(Graphics::Surface *surface) {
	uint32 color = 5;
	uint8 r, g, b;

	_gfx->readFromPalette(color, r, g, b);
	uint32 front = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	color = _currentArea->_usualBackgroundColor;
	if (_gfx->_colorRemaps && _gfx->_colorRemaps->contains(color)) {
		color = (*_gfx->_colorRemaps)[color];
	}

	_gfx->readFromPalette(color, r, g, b);
	uint32 back = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);
	uint32 white = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0xFF, 0xFF, 0xFF);

	int score = _gameStateVars[k8bitVariableScore];
	drawStringInSurface(_currentArea->_name, 174, 188, front, back, surface);
	drawStringInSurface(Common::String::format("%04d", int(2 * _position.x())), 151, 149, front, back, surface);
	drawStringInSurface(Common::String::format("%04d", int(2 * _position.z())), 151, 157, front, back, surface);
	drawStringInSurface(Common::String::format("%04d", int(2 * _position.y())), 151, 165, front, back, surface);
	if (_playerHeightNumber >= 0)
		drawStringInSurface(Common::String::format("%d", _playerHeightNumber), 72, 165, front, back, surface);
	else
		drawStringInSurface(Common::String::format("%s", "J"), 72, 165, front, back, surface);

	drawStringInSurface(Common::String::format("%02d", int(_angleRotations[_angleRotationIndex])), 63, 149, front, back, surface);
	drawStringInSurface(Common::String::format("%3d", _playerSteps[_playerStepIndex]), 63, 157, front, back, surface);
	drawStringInSurface(Common::String::format("%07d", score), 215, 133, white, back, surface);

	int seconds, minutes, hours;
	getTimeFromCountdown(seconds, minutes, hours);
	drawStringInSurface(Common::String::format("%02d", hours), 185, 12, front, back, surface);
	drawStringInSurface(Common::String::format("%02d", minutes), 207, 12, front, back, surface);
	drawStringInSurface(Common::String::format("%02d", seconds), 231, 12, front, back, surface);

	Common::String message;
	int deadline;
	getLatestMessages(message, deadline);
	if (deadline <= _countdown) {
		drawStringInSurface(message, 168, 181, back, front, surface);
		_temporaryMessages.push_back(message);
		_temporaryMessageDeadlines.push_back(deadline);
	} else {
		if (_currentArea->_gasPocketRadius == 0)
			message = _messagesList[2];
		else if (_drillStatusByArea[_currentArea->getAreaID()])
			message = _messagesList[0];
		else
			message = _messagesList[1];

		drawStringInSurface(message, 168, 181, front, back, surface);
	}

	int energy = _gameStateVars[k8bitVariableEnergy];
	int shield = _gameStateVars[k8bitVariableShield];

	if (energy >= 0) {
		Common::Rect backBar(43, 188, 107 - energy, 194);
		surface->fillRect(backBar, back);
		Common::Rect energyBar(106 - energy, 188, 106, 194);
		surface->fillRect(energyBar, front);
	}

	if (shield >= 0) {
		Common::Rect backBar(43, 181, 107 - shield, 187);
		surface->fillRect(backBar, back);

		Common::Rect shieldBar(106 - shield, 181, 106, 187);
		surface->fillRect(shieldBar, front);
	}

	drawCompass(surface, 103, 160, _yaw, 10, front);
	drawCompass(surface, 220 - 3, 160, _pitch - 30, 10, front);
}

} // End of namespace Freescape