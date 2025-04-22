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
#include "freescape/games/dark/dark.h"
#include "freescape/language/8bitDetokeniser.h"

namespace Freescape {

void DarkEngine::initC64() {
	_viewArea = Common::Rect(32, 24, 288, 127);
}

void DarkEngine::loadAssetsC64FullGame() {
	Common::File file;
    file.open("darkside.c64.data");
    loadMessagesFixedSize(&file, 0x1edf, 16, 27);
    loadFonts(&file, 0xc3e);
    loadGlobalObjects(&file, 0x20bd, 23);
    load8bitBinary(&file, 0x9b3e, 16);

    Graphics::Surface *surf = loadBundledImage("dark_border");
    surf->convertToInPlace(_gfx->_texturePixelFormat);
    _border = new Graphics::ManagedSurface();
    _border->copyFrom(*surf);
	surf->free();
	delete surf;
}


void DarkEngine::drawC64UI(Graphics::Surface *surface) {
	uint8 r, g, b;
	uint32 front = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0xAA, 0xAA, 0xAA);

	uint32 color = _currentArea->_usualBackgroundColor;
	if (_gfx->_colorRemaps && _gfx->_colorRemaps->contains(color)) {
		color = (*_gfx->_colorRemaps)[color];
	}

	_gfx->readFromPalette(color, r, g, b);
	uint32 back = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	int score = _gameStateVars[k8bitVariableScore];
	int ecds = _gameStateVars[kVariableActiveECDs];
	drawStringInSurface(Common::String::format("%04d", int(2 * _position.x())), 206, 137 + 8, front, back, surface);
	drawStringInSurface(Common::String::format("%04d", int(2 * _position.z())), 206, 145 + 8, front, back, surface);
	drawStringInSurface(Common::String::format("%04d", int(2 * _position.y())), 206, 153 + 8, front, back, surface);

	drawStringInSurface(Common::String::format("%02d", int(_angleRotations[_angleRotationIndex])), 68 + 5 + 5, 168 + 9, front, back, surface);
	drawStringInSurface(Common::String::format("%3d", _playerSteps[_playerStepIndex]), 70, 177 + 8, front, back, surface);
	drawStringInSurface(Common::String::format("%07d", score), 86, 8, front, back, surface);
	drawStringInSurface(Common::String::format("%3d%%", ecds), 198, 8, front, back, surface);

	int seconds, minutes, hours;
	getTimeFromCountdown(seconds, minutes, hours);

	Common::String message;
	int deadline;
	getLatestMessages(message, deadline);
	if (deadline <= _countdown) {
		drawStringInSurface(message, 120, 185, back, front, surface);
		_temporaryMessages.push_back(message);
		_temporaryMessageDeadlines.push_back(deadline);
	} else
		drawStringInSurface(_currentArea->_name, 120, 185, front, back, surface);

	int energy = _gameStateVars[k8bitVariableEnergy]; // called fuel in this game
	int shield = _gameStateVars[k8bitVariableShield];

	_gfx->readFromPalette(_gfx->_inkColor, r, g, b);
	uint32 inkColor = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	if (shield >= 0) {
		Common::Rect shieldBar;
		shieldBar = Common::Rect(72, 141 - 1, 143 - (_maxShield - shield), 146);
		surface->fillRect(shieldBar, inkColor);

		shieldBar = Common::Rect(72, 143 - 1, 143 - (_maxShield - shield), 144);
		surface->fillRect(shieldBar, front);
	}

	if (energy >= 0) {
		Common::Rect energyBar;
		energyBar = Common::Rect(72, 147 + 1, 143 - (_maxEnergy - energy), 155 - 1);
		surface->fillRect(energyBar, inkColor);

		energyBar = Common::Rect(72, 148 + 2, 143 - (_maxEnergy - energy), 154 - 2);
		surface->fillRect(energyBar, front);
	}
	drawBinaryClock(surface, 304, 124, front, back);
}

} // End of namespace Freescape