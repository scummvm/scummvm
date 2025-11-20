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
#include "freescape/games/eclipse/eclipse.h"
#include "freescape/language/8bitDetokeniser.h"

namespace Freescape {

void EclipseEngine::initC64() {
	_viewArea = Common::Rect(32, 32, 288, 136);

	_maxEnergy = 35;
}

extern byte kC64Palette[16][3];

void EclipseEngine::loadAssetsC64FullGame() {
	Common::File file;
	file.open(isEclipse2() ? "totaleclipse2.c64.data" : "totaleclipse.c64.data");

	if (_variant & GF_C64_TAPE) {
		int size = file.size();

		byte *buffer = (byte *)malloc(size * sizeof(byte));
		file.read(buffer, file.size());

		_extraBuffer = decompressC64RLE(buffer, &size, isEclipse2() ? 0xd2 : 0xe1);
		// size should be the size of the decompressed data
		Common::MemoryReadStream dfile(_extraBuffer, size, DisposeAfterUse::NO);

		loadMessagesFixedSize(&dfile, 0x1d84, 16, 30);
		loadFonts(&dfile, 0xc3e);
		load8bitBinary(&dfile, 0x9a3e, 16);
	} else if (_variant & GF_C64_DISC) {
		loadMessagesFixedSize(&file, 0x1534, 16, 30);
		loadFonts(&file, 0x3f2);
		if (isEclipse2())
			load8bitBinary(&file, 0x7ac4, 16);
		else
			load8bitBinary(&file, 0x7ab4, 16);
	} else
		error("Unknown C64 variant %x", _variant);

	Graphics::Surface *surf = loadBundledImage("eclipse_border");
	surf->convertToInPlace(_gfx->_texturePixelFormat);
	_border = new Graphics::ManagedSurface();
	_border->copyFrom(*surf);
	surf->free();
	delete surf;

	file.close();
	file.open(isEclipse2() ? "totaleclipse2.c64.title.bitmap" : "totaleclipse.c64.title.bitmap");

	Common::File colorFile1;
	colorFile1.open(isEclipse2() ? "totaleclipse2.c64.title.colors1" : "totaleclipse.c64.title.colors1");
	Common::File colorFile2;
	colorFile2.open(isEclipse2() ? "totaleclipse2.c64.title.colors2" : "totaleclipse.c64.title.colors2");

	_title = loadAndConvertDoodleImage(&file, &colorFile1, &colorFile2, (byte *)&kC64Palette);

	_indicators.push_back(loadBundledImage("eclipse_ankh_indicator"));

	for (auto &it : _indicators)
		it->convertToInPlace(_gfx->_texturePixelFormat);
}


void EclipseEngine::drawC64UI(Graphics::Surface *surface) {
	uint32 color = _currentArea->_underFireBackgroundColor;
	uint8 r, g, b;

	_gfx->readFromPalette(color, r, g, b);
	uint32 front = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	color = _currentArea->_usualBackgroundColor;
	if (_gfx->_colorRemaps && _gfx->_colorRemaps->contains(color)) {
		color = (*_gfx->_colorRemaps)[color];
	}

	_gfx->readFromPalette(color, r, g, b);
	uint32 back = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	_gfx->readFromPalette(13, r, g, b);
	uint32 green = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	_gfx->readFromPalette(3, r, g, b);
	uint32 blue = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	_gfx->readFromPalette(2, r, g, b);
	uint32 red = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	_gfx->readFromPalette(0, r, g, b);
	uint32 black = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	_gfx->readFromPalette(1, r, g, b);
	uint32 white = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	int score = _gameStateVars[k8bitVariableScore];
	int shield = _gameStateVars[k8bitVariableShield] * 100 / _maxShield;
	int energy = _gameStateVars[k8bitVariableEnergy];
	shield = shield < 0 ? 0 : shield;

	_gfx->readFromPalette(7, r, g, b);
	uint32 yellow = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	Common::String message;
	int deadline;
	getLatestMessages(message, deadline);
	if (deadline <= _countdown) {
		drawStringInSurface(message, 104, 138, back, yellow, surface);
		_temporaryMessages.push_back(message);
		_temporaryMessageDeadlines.push_back(deadline);
	} else if (!_currentAreaMessages.empty())
		drawStringInSurface(_currentArea->_name, 104, 138, back, yellow, surface);

	drawScoreString(score, 128, 7, black, white, surface);

	Common::String shieldStr = Common::String::format("%d", shield);

	int x = 174;
	if (shield < 10)
		x = 182;
	else if (shield < 100)
		x = 179;

	if (energy < 0)
		energy = 0;

	drawStringInSurface(shieldStr, x, 161, back, red, surface);

	Common::Rect jarBackground(112, 170, 144, 196);
	surface->fillRect(jarBackground, back);

	Common::Rect jarWater(112, 196 - energy, 144, 196);
	surface->fillRect(jarWater, blue);

	// TODO
	/*drawStringInSurface(shiftStr("0", 'Z' - '$' + 1 - _angleRotationIndex), 79, 138, back, yellow, surface);
	drawStringInSurface(shiftStr("3", 'Z' - '$' + 1 - _playerStepIndex), 63, 138, back, yellow, surface);
	drawStringInSurface(shiftStr("7", 'Z' - '$' + 1 - _playerHeightNumber), 240, 138, back, yellow, surface);

	if (_shootingFrames > 0) {
		drawStringInSurface(shiftStr("4", 'Z' - '$' + 1), 232, 138, back, yellow, surface);
		drawStringInSurface(shiftStr("<", 'Z' - '$' + 1) , 240, 138, back, yellow, surface);
	}*/

	drawAnalogClockHand(surface, 72, 172, 38 * 6 - 90, 11, white);
	drawAnalogClockHand(surface, 72, 172, 37 * 6 - 90, 11, white);
	drawAnalogClockHand(surface, 72, 172, 36 * 6 - 90, 11, white);
	drawAnalogClock(surface, 72, 172, back, red, white);

	surface->fillRect(Common::Rect(236, 170, 258, 187), white);
	drawCompass(surface, 247, 177, _yaw, 13, back);

	drawIndicator(surface, 56, 4, 8);
	drawEclipseIndicator(surface, 224, 0, front, green);
}

} // End of namespace Freescape
