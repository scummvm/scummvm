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

void DrillerEngine::initC64() {
	_viewArea = Common::Rect(32, 16, 288, 119);
}

void DrillerEngine::loadAssetsC64FullGame() {
	Common::File file;
	if (_targetName.hasPrefix("spacestationoblivion")) {
		loadBundledImages();
		file.open("spacestationoblivion.c64.data");
		loadMessagesFixedSize(&file, 0x167a, 14, 20);
		//loadFonts(&file, 0xae54);
		load8bitBinary(&file, 0x8e02, 4);
		loadGlobalObjects(&file, 0x1855, 8);
	} else if (_targetName.hasPrefix("driller")) {
		file.open("driller.c64.data");
		loadMessagesFixedSize(&file, 0x167a - 0x400, 14, 20);
		//loadFonts(&file, 0xae54);
		load8bitBinary(&file, 0x8e02 - 0x400, 4);
		loadGlobalObjects(&file, 0x1855 - 0x400, 8);
	} else
		error("Unknown C64 release");
}


void DrillerEngine::drawC64UI(Graphics::Surface *surface) {
	uint32 color = 1;
	uint8 r, g, b;

	_gfx->selectColorFromFourColorPalette(color, r, g, b);
	uint32 front = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	color = 0;
	if (_gfx->_colorRemaps && _gfx->_colorRemaps->contains(color)) {
		color = (*_gfx->_colorRemaps)[color];
	}

	_gfx->readFromPalette(color, r, g, b);
	uint32 back = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	int score = _gameStateVars[k8bitVariableScore];
	drawStringInSurface(_currentArea->_name, 200, 188, front, back, surface);
	drawStringInSurface(Common::String::format("%04d", int(2 * _position.x())), 149, 148, front, back, surface);
	drawStringInSurface(Common::String::format("%04d", int(2 * _position.z())), 149, 156, front, back, surface);
	drawStringInSurface(Common::String::format("%04d", int(2 * _position.y())), 149, 164, front, back, surface);
	if (_playerHeightNumber >= 0)
		drawStringInSurface(Common::String::format("%d", _playerHeightNumber), 54, 164, front, back, surface);
	else
		drawStringInSurface(Common::String::format("%s", "J"), 54, 164, front, back, surface);

	drawStringInSurface(Common::String::format("%02d", int(_angleRotations[_angleRotationIndex])), 46, 148, front, back, surface);
	drawStringInSurface(Common::String::format("%3d", _playerSteps[_playerStepIndex]), 44, 156, front, back, surface);
	drawStringInSurface(Common::String::format("%07d", score), 240, 128, front, back, surface);

	int seconds, minutes, hours;
	getTimeFromCountdown(seconds, minutes, hours);
	drawStringInSurface(Common::String::format("%02d", hours), 209, 11, front, back, surface);
	drawStringInSurface(Common::String::format("%02d", minutes), 232, 11, front, back, surface);
	drawStringInSurface(Common::String::format("%02d", seconds), 254, 11, front, back, surface);

	Common::String message;
	int deadline;
	getLatestMessages(message, deadline);
	if (deadline <= _countdown) {
		drawStringInSurface(message, 191, 180, back, front, surface);
		_temporaryMessages.push_back(message);
		_temporaryMessageDeadlines.push_back(deadline);
	} else {
		if (_currentArea->_gasPocketRadius == 0)
			message = _messagesList[2];
		else if (_drillStatusByArea[_currentArea->getAreaID()])
			message = _messagesList[0];
		else
			message = _messagesList[1];

		drawStringInSurface(message, 191, 180, front, back, surface);
	}

	int energy = _gameStateVars[k8bitVariableEnergy];
	int shield = _gameStateVars[k8bitVariableShield];

	if (energy >= 0) {
		Common::Rect backBar(25, 187, 89 - energy, 194);
		surface->fillRect(backBar, back);
		Common::Rect energyBar(88 - energy, 187, 88, 194);
		surface->fillRect(energyBar, front);
	}

	if (shield >= 0) {
		Common::Rect backBar(25, 180, 89 - shield, 186);
		surface->fillRect(backBar, back);

		Common::Rect shieldBar(88 - shield, 180, 88, 186);
		surface->fillRect(shieldBar, front);
	}
}

} // End of namespace Freescape