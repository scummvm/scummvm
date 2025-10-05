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
#include "common/memstream.h"

#include "freescape/freescape.h"
#include "freescape/games/dark/dark.h"
#include "freescape/language/8bitDetokeniser.h"

namespace Freescape {

void DarkEngine::initC64() {
	_viewArea = Common::Rect(32, 24, 288, 127);
}

extern byte kC64Palette[16][3];

void DarkEngine::loadAssetsC64FullGame() {
	Common::File file;
	file.open("darkside.c64.data");

	if (_variant & GF_C64_TAPE) {
		int size = file.size();

		byte *buffer = (byte *)malloc(size * sizeof(byte));
		file.read(buffer, file.size());

		_extraBuffer = decompressC64RLE(buffer, &size, 0xdf);
		// size should be the size of the decompressed data
		Common::MemoryReadStream dfile(_extraBuffer, size, DisposeAfterUse::NO);

		loadMessagesFixedSize(&dfile, 0x1edf, 16, 27);
		loadFonts(&dfile, 0xc3e);
		loadGlobalObjects(&dfile, 0x20bd, 23);
		load8bitBinary(&dfile, 0x9b3e, 16);
	} else if (_variant & GF_C64_DISC) {
		loadMessagesFixedSize(&file, 0x16a3, 16, 27);
		loadFonts(&file, 0x402);

		// It is unclear why this C64 has this byte changed at 0x1881
		// Once the game is loaded, it will be set to 0x66
		// and the game will work
		file.seek(0x1881);
		_extraBuffer = (byte *)malloc(0x300 * sizeof(byte));
		file.read(_extraBuffer, 0x300);
		_extraBuffer[0] = 0x66;
		Common::MemoryReadStream stream(_extraBuffer, 0x300, DisposeAfterUse::NO);
		loadGlobalObjects(&stream, 0x0, 23);
		load8bitBinary(&file, 0x8914, 16);
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

	// TODO
	_colorMap[12][0] = 0x00;
	_colorMap[12][1] = 0x00;
	_colorMap[12][2] = 0x00;
	_colorMap[12][3] = 0x00;

	_colorMap[13][0] = 0x77;
	_colorMap[13][1] = 0xdd;
	_colorMap[13][2] = 0x77;
	_colorMap[13][3] = 0xdd;

	// TODO
	_colorMap[14][0] = 0xcc;
	_colorMap[14][1] = 0xcc;
	_colorMap[14][2] = 0xcc;
	_colorMap[14][3] = 0xcc;

	Graphics::Surface *surf = loadBundledImage("dark_border");
	surf->convertToInPlace(_gfx->_texturePixelFormat);
	_border = new Graphics::ManagedSurface();
	_border->copyFrom(*surf);
	surf->free();
	delete surf;

	file.close();
	file.open("darkside.c64.title.bitmap");

	Common::File colorFile1;
	colorFile1.open("darkside.c64.title.colors1");
	Common::File colorFile2;
	colorFile2.open("darkside.c64.title.colors2");

	_title = loadAndConvertDoodleImage(&file, &colorFile1, &colorFile2, (byte *)&kC64Palette);
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

	_gfx->readFromPalette(6, r, g, b); // Violet Blue
	uint32 outBarColor = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	_gfx->readFromPalette(14, r, g, b); // Violet
	uint32 inBarColor = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	_gfx->readFromPalette(3, r, g, b); // Light Blue
	uint32 lineColor = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	Common::Rect coverBar;
	coverBar = Common::Rect(64, 144, 135, 151);
	surface->fillRect(coverBar, back);

	if (shield >= 0) {
		Common::Rect shieldBar;

		shieldBar = Common::Rect(64, 144, 127 - (_maxShield - shield), 151);
		surface->fillRect(shieldBar, outBarColor);

		shieldBar = Common::Rect(64, 146, 127 - (_maxShield - shield), 149);
		surface->fillRect(shieldBar, inBarColor);
		if (shield >= 1)
			surface->drawLine(64, 147, 127 - (_maxShield - shield) - 1, 147, lineColor);
	}

	coverBar = Common::Rect(64, 144 + 8, 127, 159);
	surface->fillRect(coverBar, back);

	if (energy >= 0) {
		Common::Rect energyBar;
		energyBar = Common::Rect(64, 144 + 8, 127 - (_maxEnergy - energy), 159);
		surface->fillRect(energyBar, outBarColor);

		energyBar = Common::Rect(64, 146 + 8, 127 - (_maxEnergy - energy), 157);
		surface->fillRect(energyBar, inBarColor);
		if (energy >= 1)
			surface->drawLine(64, 147 + 8, 127 - (_maxEnergy - energy) - 1, 155, lineColor);
	}
	drawBinaryClock(surface, 304, 124, front, back);
	drawVerticalCompass(surface, 17, 77, _pitch, front);
}

} // End of namespace Freescape
