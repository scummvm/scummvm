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

#include "freescape/freescape.h"
#include "freescape/language/8bitDetokeniser.h"

/*
This file contains specific code for both Ammiga and AtariST implementations of Driller
*/

namespace Freescape {

void DrillerEngine::drawAmigaAtariSTUI(Graphics::Surface *surface) {
	uint32 white = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0xFF, 0xFF, 0xFF);
	uint32 yellow = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0xFF, 0xFF, 0x55);
	uint32 brownish = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0x9E, 0x80, 0x20);
	uint32 brown = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0x7E, 0x60, 0x19);
	uint32 black = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0x00, 0x00, 0x00);
	uint32 transparent = _gfx->_texturePixelFormat.ARGBToColor(0x00, 0x00, 0x00, 0x00);

	int score = _gameStateVars[k8bitVariableScore];
	Common::String coords;

	if (!isDemo()) { // It seems demos will not include the complete font?
		drawStringInSurface("x", 37, 18, white, transparent, surface, 82);
		coords = Common::String::format("%04d", 2 * int(_position.x()));
		for (int i = 0; i < 4; i++)
			drawStringInSurface(Common::String(coords[i]), 47 + 6*i, 18, white, transparent, surface, 112);

		drawStringInSurface("y", 37, 26, white, transparent, surface, 82);
		coords = Common::String::format("%04d", 2 * int(_position.z())); // Coords y and z are swapped!
		for (int i = 0; i < 4; i++)
			drawStringInSurface(Common::String(coords[i]), 47 + 6*i, 26, white, transparent, surface, 112);

		drawStringInSurface("z", 37, 34, white, transparent, surface, 82);
		coords = Common::String::format("%04d", 2 * int(_position.y())); // Coords y and z are swapped!
		for (int i = 0; i < 4; i++)
			drawStringInSurface(Common::String(coords[i]), 47 + 6*i, 34, white, transparent, surface, 112);
	}

	drawStringInSurface(_currentArea->_name, 188, 185, yellow, black, surface);
	drawStringInSurface(Common::String::format("%07d", score), 240, 129, yellow, black, surface);

	int seconds, minutes, hours;
	getTimeFromCountdown(seconds, minutes, hours);
	drawStringInSurface(Common::String::format("%02d:", hours), 208, 7, yellow, black, surface);
	drawStringInSurface(Common::String::format("%02d:", minutes), 230, 7, yellow, black, surface);
	drawStringInSurface(Common::String::format("%02d", seconds), 254, 7, yellow, black, surface);

	Common::String message;
	int deadline;
	getLatestMessages(message, deadline);
	if (deadline <= _countdown) {
		drawStringInSurface(message, 188, 177, black, yellow, surface);
		_temporaryMessages.push_back(message);
		_temporaryMessageDeadlines.push_back(deadline);
	} else {
		if (_currentArea->_gasPocketRadius == 0)
			message = _messagesList[2];
		else if (_drillStatusByArea[_currentArea->getAreaID()])
			message = _messagesList[0];
		else
			message = _messagesList[1];

		drawStringInSurface(message, 188, 177, yellow, black, surface);
	}

	int energy = _gameStateVars[k8bitVariableEnergy];
	int shield = _gameStateVars[k8bitVariableShield];

	if (shield >= 0) {
		Common::Rect shieldBar;
		shieldBar = Common::Rect(11, 178, 76 - (k8bitMaxShield - shield), 184);
		surface->fillRect(shieldBar, brown);

		shieldBar = Common::Rect(11, 179, 76 - (k8bitMaxShield - shield), 183);
		surface->fillRect(shieldBar, brownish);

		shieldBar = Common::Rect(11, 180, 76 - (k8bitMaxShield - shield), 182);
		surface->fillRect(shieldBar, yellow);
	}

	if (energy >= 0) {
		Common::Rect energyBar;
		energyBar = Common::Rect(11, 186, 75 - (k8bitMaxEnergy - energy), 192);
		surface->fillRect(energyBar, brown);

		energyBar = Common::Rect(11, 187, 75 - (k8bitMaxEnergy - energy), 191);
		surface->fillRect(energyBar, brownish);

		energyBar = Common::Rect(11, 188, 75 - (k8bitMaxEnergy - energy), 190);
		surface->fillRect(energyBar, yellow);
	}
}

} // End of namespace Freescape