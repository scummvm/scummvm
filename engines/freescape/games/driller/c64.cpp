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

extern byte kC64Palette[16][3];

void DrillerEngine::initC64() {
	_viewArea = Common::Rect(32, 16, 288, 120);
}

void DrillerEngine::loadAssetsC64FullGame() {
	Common::File file;
	if (_targetName.hasPrefix("spacestationoblivion")) {
		file.open("spacestationoblivion.c64.data");
		loadMessagesFixedSize(&file, 0x167a, 14, 20);
		//loadFonts(&file, 0xae54);
		load8bitBinary(&file, 0x8e02, 4);
		loadGlobalObjects(&file, 0x1855, 8);
	} else if (_targetName.hasPrefix("driller")) {
		file.open("driller.c64.data");

		if (_variant) {
			loadMessagesFixedSize(&file, 0x167a, 14, 20);
			loadGlobalObjects(&file, 0x1855, 8);
			loadFonts(&file, 0x402);
			load8bitBinary(&file, 0x8b04, 16);
		/*} else if (_variant & GF_C64_BUDGET) {
			//loadFonts(&file, 0x402);
			load8bitBinary(&file, 0x7df7, 16);
			loadMessagesFixedSize(&file, 0x1399, 14, 20);
			loadGlobalObjects(&file, 0x150a, 8);*/
		} else
			error("Unknown C64 variant %x", _variant);

		// The color map from the data is not correct,
		// so we'll just hardcode the one that we found in the executable

		for (int i = 0; i < 15; i++) {
			_colorMap[i][0] = 0;
			_colorMap[i][1] = 0;
			_colorMap[i][2] = 0;
			_colorMap[i][3] = 0;
		}

		_colorMap[1][0] = 0x55;
		_colorMap[1][1] = 0x55;
		_colorMap[1][2] = 0x55;
		_colorMap[1][3] = 0x55;

		_colorMap[2][0] = 0xaa;
		_colorMap[2][1] = 0xaa;
		_colorMap[2][2] = 0xaa;
		_colorMap[2][3] = 0xaa;

		_colorMap[3][0] = 0xff;
		_colorMap[3][1] = 0xff;
		_colorMap[3][2] = 0xff;
		_colorMap[3][3] = 0xff;

		_colorMap[4][0] = 0x44;
		_colorMap[4][1] = 0x11;
		_colorMap[4][2] = 0x44;
		_colorMap[4][3] = 0x11;

		_colorMap[5][0] = 0x88;
		_colorMap[5][1] = 0x22;
		_colorMap[5][2] = 0x88;
		_colorMap[5][3] = 0x22;

		_colorMap[6][0] = 0xcc;
		_colorMap[6][1] = 0x33;
		_colorMap[6][2] = 0xcc;
		_colorMap[6][3] = 0x33;

		_colorMap[7][0] = 0x66;
		_colorMap[7][1] = 0x99;
		_colorMap[7][2] = 0x66;
		_colorMap[7][3] = 0x99;

		_colorMap[8][0] = 0x77;
		_colorMap[8][1] = 0xdd;
		_colorMap[8][2] = 0x77;
		_colorMap[8][3] = 0xdd;

		_colorMap[9][0] = 0xbb;
		_colorMap[9][1] = 0xee;
		_colorMap[9][2] = 0xbb;
		_colorMap[9][3] = 0xee;

		_colorMap[10][0] = 0x5a;
		_colorMap[10][1] = 0xa5;
		_colorMap[10][2] = 0x5a;
		_colorMap[10][3] = 0xa5;

		_colorMap[11][0] = 0xaf;
		_colorMap[11][1] = 0xfa;
		_colorMap[11][2] = 0xaf;
		_colorMap[11][3] = 0xfa;


		_colorMap[12][0] = 0x77;
		_colorMap[12][1] = 0xdd;
		_colorMap[12][2] = 0x77;
		_colorMap[12][3] = 0xdd;

		_colorMap[13][0] = 0xcc;
		_colorMap[13][1] = 0xcc;
		_colorMap[13][2] = 0xcc;
		_colorMap[13][3] = 0xcc;

		// TODO
		_colorMap[14][0] = 0x77;
		_colorMap[14][1] = 0xdd;
		_colorMap[14][2] = 0x77;
		_colorMap[14][3] = 0xdd;

		Graphics::Surface *surf = loadBundledImage("driller_border");
		surf->convertToInPlace(_gfx->_texturePixelFormat);
		_border = new Graphics::ManagedSurface();
		_border->copyFrom(*surf);
		surf->free();
		delete surf;

		file.close();
		file.open("driller.c64.title.bitmap");

		Common::File colorFile1;
		colorFile1.open("driller.c64.title.colors1");
		Common::File colorFile2;
		colorFile2.open("driller.c64.title.colors2");

		_title = loadAndConvertDoodleImage(&file, &colorFile1, &colorFile2, (byte *)&kC64Palette);
	} else
		error("Unknown C64 release");

	_playerSid = new DrillerSIDPlayer(_mixer);
}


void DrillerEngine::drawC64UI(Graphics::Surface *surface) {

	uint8 r, g, b;
	uint32 front = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0xAA, 0xAA, 0xAA);

	Common::Rect cover;

	uint32 color = 0;
	if (_gfx->_colorRemaps && _gfx->_colorRemaps->contains(color)) {
		color = (*_gfx->_colorRemaps)[color];
	}

	_gfx->readFromPalette(color, r, g, b);
	uint32 back = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	int score = _gameStateVars[k8bitVariableScore];
	drawStringInSurface(_currentArea->_name, 200, 184, front, back, surface);
	cover = Common::Rect(150, 143, 183, 167);

	surface->fillRect(cover, back);
	drawStringInSurface(Common::String::format("%04d", int(2 * _position.x())), 150, 148 - 4, front, back, surface);
	drawStringInSurface(Common::String::format("%04d", int(2 * _position.z())), 150, 156 - 4, front, back, surface);
	drawStringInSurface(Common::String::format("%04d", int(2 * _position.y())), 150, 164 - 4, front, back, surface);
	if (_playerHeightNumber >= 0)
		drawStringInSurface(Common::String::format("%d", _playerHeightNumber), 54 + 6, 164 - 3, front, back, surface);
	else
		drawStringInSurface(Common::String::format("%s", "J"), 54 + 6, 164 - 3, front, back, surface);

	drawStringInSurface(Common::String::format("%02d", int(_angleRotations[_angleRotationIndex])), 46, 148 - 3, front, back, surface);
	drawStringInSurface(Common::String::format("%3d", _playerSteps[_playerStepIndex]), 46, 156 - 3, front, back, surface);
	drawStringInSurface(Common::String::format("%07d", score), 239, 128, front, back, surface);

	int seconds, minutes, hours;
	getTimeFromCountdown(seconds, minutes, hours);
	drawStringInSurface(Common::String::format("%02d", hours), 207, 8, front, back, surface);
	drawStringInSurface(Common::String::format("%02d", minutes), 230, 8, front, back, surface);
	drawStringInSurface(Common::String::format("%02d", seconds), 254, 8, front, back, surface);

	Common::String message;
	int deadline;
	getLatestMessages(message, deadline);
	if (deadline <= _countdown) {
		drawStringInSurface(message, 191, 176, back, front, surface);
		_temporaryMessages.push_back(message);
		_temporaryMessageDeadlines.push_back(deadline);
	} else {
		if (_currentArea->_gasPocketRadius == 0)
			message = _messagesList[2];
		else if (_drillStatusByArea[_currentArea->getAreaID()])
			message = _messagesList[0];
		else
			message = _messagesList[1];

		drawStringInSurface(message, 191, 176, front, back, surface);
	}

	uint32 green = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0x68, 0xa9, 0x41);
	int energy = _gameStateVars[k8bitVariableEnergy];
	int shield = _gameStateVars[k8bitVariableShield];

	if (energy >= 0) {
		Common::Rect backBar(21, 183, 85 - energy, 190);
		surface->fillRect(backBar, back);
		Common::Rect energyBar(84 - energy, 184, 84, 190);
		surface->fillRect(energyBar, green);
	}

	if (shield >= 0) {
		Common::Rect backBar(25 - 4, 180 - 4, 89 - shield - 4, 186 - 4);
		surface->fillRect(backBar, back);

		Common::Rect shieldBar(88 - 4  - shield, 180 - 4, 88 - 4, 186 - 4);
		surface->fillRect(shieldBar, green);
	}

	_gfx->readFromPalette(7, r, g, b);
	uint32 yellow = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	surface->fillRect(Common::Rect(87, 156, 104, 166), back);
	drawCompass(surface, 94, 156, _yaw - 30, 11, 75, yellow);
	surface->fillRect(Common::Rect(224, 151, 235, 160), back);
	drawCompass(surface, 223, 156, _pitch - 30, 12, 60, yellow);
}

} // End of namespace Freescape
