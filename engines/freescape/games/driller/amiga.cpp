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

void DrillerEngine::loadAssetsAmigaFullGame() {
	Common::File file;
	if (_variant & GF_AMIGA_RETAIL) {
		file.open("driller");

		if (!file.isOpen())
			error("Failed to open 'driller' executable for Amiga");

		_border = loadAndConvertNeoImage(&file, 0x137f4);
		byte *palette = (byte *)malloc(16 * 3);
		for (int i = 0; i < 16; i++) { // gray scale palette
			palette[i * 3 + 0] = i * (255 / 16);
			palette[i * 3 + 1] = i * (255 / 16);
			palette[i * 3 + 2] = i * (255 / 16);
		}
		_title = loadAndConvertNeoImage(&file, 0x10, palette);

		loadFonts(&file, 0x8940);
		loadMessagesFixedSize(&file, 0xc66e, 14, 20);
		loadGlobalObjects(&file, 0xbd62, 8);
		load8bitBinary(&file, 0x29c16, 16);
		loadPalettes(&file, 0x297d4);
		loadSoundsFx(&file, 0x30e80, 25);
	} else if (_variant & GF_AMIGA_BUDGET) {
		file.open("lift.neo");
		if (!file.isOpen())
			error("Failed to open 'lift.neo' file");

		_title = loadAndConvertNeoImage(&file, 0);

		file.close();
		file.open("console.neo");
		if (!file.isOpen())
			error("Failed to open 'console.neo' file");

		_border = loadAndConvertNeoImage(&file, 0);

		file.close();
		file.open("driller");
		if (!file.isOpen())
			error("Failed to open 'driller' executable for Amiga");

		loadFonts(&file, 0xa62);
		loadMessagesFixedSize(&file, 0x499a, 14, 20);
		loadGlobalObjects(&file, 0x4098, 8);
		load8bitBinary(&file, 0x21a3e, 16);
		loadPalettes(&file, 0x215fc);

		file.close();
		file.open("soundfx");
		if (!file.isOpen())
			error("Failed to open 'soundfx' executable for Amiga");

		loadSoundsFx(&file, 0, 25);
	} else
		error("Invalid or unknown Amiga release");
}

void DrillerEngine::loadAssetsAmigaDemo() {
	Common::File file;
	file.open("lift.neo");
	if (!file.isOpen())
		error("Failed to open 'lift.neo' file");

	_title = loadAndConvertNeoImage(&file, 0);

	file.close();
	file.open("console.neo");
	if (!file.isOpen())
		error("Failed to open 'console.neo' file");

	_border = loadAndConvertNeoImage(&file, 0);

	file.close();
	file.open("demo.cmd");
	if (!file.isOpen())
		error("Failed to open 'demo.cmd' file");

	loadDemoData(&file, 0, 0x1000);

	file.close();
	file.open("data");
	if (!file.isOpen())
		error("Failed to open 'data' file");

	load8bitBinary(&file, 0x442, 16);
	loadPalettes(&file, 0x0);

	file.close();
	file.open("driller");
	if (!file.isOpen())
		error("Failed to open 'driller' file");

	if (_variant & GF_AMIGA_MAGAZINE_DEMO) {
		loadFonts(&file, 0xa62);
		loadMessagesFixedSize(&file, 0x3df0, 14, 20);
		loadGlobalObjects(&file, 0x3ba6, 8);
		_demoMode = false;
	} else {
		loadFonts(&file, 0xa30);
		loadMessagesFixedSize(&file, 0x3960, 14, 20);
		loadGlobalObjects(&file, 0x3716, 8);
	}

	file.close();
	file.open("soundfx");
	if (!file.isOpen())
		error("Failed to open 'soundfx' executable for Amiga");

	loadSoundsFx(&file, 0, 25);
}

/*
The following function contains specific UI code for both Amiga and AtariST
*/

void DrillerEngine::drawAmigaAtariSTUI(Graphics::Surface *surface) {
	uint32 white = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0xFF, 0xFF, 0xFF);
	uint32 yellow = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0xFF, 0xFF, 0x55);
	uint32 brownish = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0x9E, 0x80, 0x20);
	uint32 brown = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0x7E, 0x60, 0x19);
	uint32 primaryFontColor = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0xA0, 0x80, 0x00);
	uint32 secondaryFontColor = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0x60, 0x40, 0x00);
	uint32 black = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0x00, 0x00, 0x00);
	uint32 transparent = _gfx->_texturePixelFormat.ARGBToColor(0x00, 0x00, 0x00, 0x00);

	int score = _gameStateVars[k8bitVariableScore];
	Common::String coords;

	// It seems that some demos will not include the complete font
	if (!isDemo() || (_variant & GF_AMIGA_MAGAZINE_DEMO) || (_variant & GF_ATARI_MAGAZINE_DEMO)) {
		drawStringInSurface("x", 37, 18, white, transparent, transparent, surface, 82);
		coords = Common::String::format("%04d", 2 * int(_position.x()));
		for (int i = 0; i < 4; i++)
			drawStringInSurface(Common::String(coords[i]), 47 + 6*i, 18, white, transparent, transparent, surface, 112);

		drawStringInSurface("y", 37, 26, white, transparent, transparent, surface, 82);
		coords = Common::String::format("%04d", 2 * int(_position.z())); // Coords y and z are swapped!
		for (int i = 0; i < 4; i++)
			drawStringInSurface(Common::String(coords[i]), 47 + 6*i, 26, white, transparent, transparent, surface, 112);

		drawStringInSurface("z", 37, 34, white, transparent, transparent, surface, 82);
		coords = Common::String::format("%04d", 2 * int(_position.y())); // Coords y and z are swapped!
		for (int i = 0; i < 4; i++)
			drawStringInSurface(Common::String(coords[i]), 47 + 6*i, 34, white, transparent, transparent, surface, 112);
	}

	drawStringInSurface(_currentArea->_name, 188, 185, primaryFontColor, secondaryFontColor, black, surface);
	drawStringInSurface(Common::String::format("%07d", score), 241, 129, primaryFontColor, secondaryFontColor, black, surface);

	int seconds, minutes, hours;
	getTimeFromCountdown(seconds, minutes, hours);
	drawStringInSurface(Common::String::format("%02d:", hours), 210, 7, primaryFontColor, secondaryFontColor, black, surface);
	drawStringInSurface(Common::String::format("%02d:", minutes), 230, 7, primaryFontColor, secondaryFontColor, black, surface);
	drawStringInSurface(Common::String::format("%02d", seconds), 254, 7, primaryFontColor, secondaryFontColor, black, surface);

	Common::String message;
	int deadline;
	getLatestMessages(message, deadline);
	if (deadline <= _countdown) {
		drawStringInSurface(message, 188, 177, yellow, secondaryFontColor, black, surface);
		_temporaryMessages.push_back(message);
		_temporaryMessageDeadlines.push_back(deadline);
	} else {
		if (_currentArea->_gasPocketRadius == 0)
			message = _messagesList[2];
		else if (_drillStatusByArea[_currentArea->getAreaID()])
			message = _messagesList[0];
		else
			message = _messagesList[1];

		drawStringInSurface(message, 188, 177, primaryFontColor, secondaryFontColor, black, surface);
	}

	int energy = _gameStateVars[k8bitVariableEnergy];
	int shield = _gameStateVars[k8bitVariableShield];

	if (shield >= 0) {
		Common::Rect shieldBar;
		shieldBar = Common::Rect(11, 178, 76 - (_maxShield - shield), 184);
		surface->fillRect(shieldBar, brown);

		shieldBar = Common::Rect(11, 179, 76 - (_maxShield - shield), 183);
		surface->fillRect(shieldBar, brownish);

		shieldBar = Common::Rect(11, 180, 76 - (_maxShield - shield), 182);
		surface->fillRect(shieldBar, yellow);
	}

	if (energy >= 0) {
		Common::Rect energyBar;
		energyBar = Common::Rect(11, 186, 75 - (_maxEnergy - energy), 192);
		surface->fillRect(energyBar, brown);

		energyBar = Common::Rect(11, 187, 75 - (_maxEnergy - energy), 191);
		surface->fillRect(energyBar, brownish);

		energyBar = Common::Rect(11, 188, 75 - (_maxEnergy - energy), 190);
		surface->fillRect(energyBar, yellow);
	}
}

void DrillerEngine::initAmigaAtari() {
	_viewArea = Common::Rect(36, 16, 284, 118);

	_moveFowardArea = Common::Rect(184, 125, 199, 144);
	_moveLeftArea = Common::Rect(161, 145, 174, 164);
	_moveRightArea = Common::Rect(207, 145, 222, 164);
	_moveBackArea = Common::Rect(184, 152, 199, 171);
	_moveUpArea = Common::Rect(231, 145, 246, 164);
	_moveDownArea = Common::Rect(254, 145, 269, 164);
	_deployDrillArea = Common::Rect(284, 145, 299, 166);
	_infoScreenArea = Common::Rect(125, 172, 152, 197);
	_saveGameArea = Common::Rect(9, 145, 39, 154);
	_loadGameArea = Common::Rect(9, 156, 39, 164);
}

} // End of namespace Freescape