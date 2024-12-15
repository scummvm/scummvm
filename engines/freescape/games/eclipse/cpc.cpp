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
#include "freescape/games/eclipse/eclipse.h"
#include "freescape/language/8bitDetokeniser.h"

namespace Freescape {

void EclipseEngine::initCPC() {
	_viewArea = Common::Rect(36 + 3, 24 + 8, 284, 130 + 3);
}

byte kCPCPaletteEclipseTitleData[4][3] = {
	{0x00, 0x00, 0x00},
	{0xff, 0xff, 0x00},
	{0xff, 0x00, 0xff},
	{0xff, 0x80, 0x00},
};

byte kCPCPaletteEclipseBorderData[4][3] = {
	{0x00, 0x00, 0x00},
	{0xff, 0x80, 0x00},
	{0x80, 0xff, 0xff},
	{0x00, 0x80, 0x00},
};


extern Graphics::ManagedSurface *readCPCImage(Common::SeekableReadStream *file, bool mode0);

void EclipseEngine::loadAssetsCPCFullGame() {
	Common::File file;

	if (isEclipse2())
		file.open("TE2.BI1");
	else
		file.open("TESCR.SCR");

	if (!file.isOpen())
		error("Failed to open TESCR.SCR/TE2.BI1");

	_title = readCPCImage(&file, true);
	_title->setPalette((byte*)&kCPCPaletteEclipseTitleData, 0, 4);

	file.close();
	if (isEclipse2())
		file.open("TE2.BI3");
	else
		file.open("TECON.SCR");

	if (!file.isOpen())
		error("Failed to open TECON.SCR/TE2.BI3");

	_border = readCPCImage(&file, true);
	_border->setPalette((byte*)&kCPCPaletteEclipseBorderData, 0, 4);

	file.close();
	if (isEclipse2())
		file.open("TE2.BI2");
	else
		file.open("TECODE.BIN");

	if (!file.isOpen())
		error("Failed to open TECODE.BIN/TE2.BI2");

	if (isEclipse2()) {
		loadFonts(&file, 0x60bc);
		loadMessagesFixedSize(&file, 0x326, 16, 30);
		load8bitBinary(&file, 0x62b4, 16);
	} else {
		loadFonts(&file, 0x6076);
		loadMessagesFixedSize(&file, 0x326, 16, 30);
		load8bitBinary(&file, 0x626e, 16);
	}

	for (auto &it : _areaMap) {
		it._value->addStructure(_areaMap[255]);

		if (isEclipse2() && it._value->getAreaID() == 1)
			continue;

		if (isEclipse2() && it._value->getAreaID() == _startArea)
			continue;

		for (int16 id = 183; id < 207; id++)
			it._value->addObjectFromArea(id, _areaMap[255]);
	}
	loadColorPalette();
	swapPalette(1);

	_indicators.push_back(loadBundledImage("eclipse_ankh_indicator"));

	for (auto &it : _indicators)
		it->convertToInPlace(_gfx->_texturePixelFormat);
}

void EclipseEngine::loadAssetsCPCDemo() {
	Common::File file;

	file.open("TECON.BIN");
	if (!file.isOpen())
		error("Failed to open TECON.BIN");

	_border = readCPCImage(&file, true);
	_border->setPalette((byte*)&kCPCPaletteEclipseTitleData, 0, 4);

	file.close();
	file.open("TEPROG.BIN");

	if (!file.isOpen())
		error("Failed to open TEPROG.BIN");

	loadFonts(&file, 0x63ce);
	loadMessagesFixedSize(&file, 0x362, 16, 23);
	loadMessagesFixedSize(&file, 0x570b, 264, 5);
	load8bitBinary(&file, 0x65c6, 16);
	for (auto &it : _areaMap) {
		it._value->_name = "  NOW TRAINING  ";
		it._value->addStructure(_areaMap[255]);
		for (int16 id = 183; id < 207; id++)
			it._value->addObjectFromArea(id, _areaMap[255]);
	}
	loadColorPalette();
	swapPalette(1);

	_indicators.push_back(loadBundledImage("eclipse_ankh_indicator"));

	for (auto &it : _indicators)
		it->convertToInPlace(_gfx->_texturePixelFormat);
}

void EclipseEngine::drawCPCUI(Graphics::Surface *surface) {
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

	color = _currentArea->_inkColor;

	_gfx->readFromPalette(color, r, g, b);
	uint32 other = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	int score = _gameStateVars[k8bitVariableScore];
	int shield = _gameStateVars[k8bitVariableShield] * 100 / _maxShield;
	shield = shield < 0 ? 0 : shield;

	Common::String message;
	int deadline;
	getLatestMessages(message, deadline);
	if (deadline <= _countdown) {
		drawStringInSurface(message, 102, 135, back, front, surface);
		_temporaryMessages.push_back(message);
		_temporaryMessageDeadlines.push_back(deadline);
	} else if (!_currentAreaMessages.empty())
		drawStringInSurface(_currentArea->_name, 102, 135, back, front, surface);

	Common::String scoreStr = Common::String::format("%07d", score);
	drawStringInSurface(scoreStr, 136, 6, back, other, surface, 'Z' - '0' + 1);

	int x = 171;
	if (shield < 10)
		x = 179;
	else if (shield < 100)
		x = 175;

	Common::String shieldStr = Common::String::format("%d", shield);
	drawStringInSurface(shieldStr, x, 162, back, other, surface);

	drawStringInSurface(Common::String('0' + _angleRotationIndex - 3), 79, 135, back, front, surface, 'Z' - '$' + 1);
	drawStringInSurface(Common::String('3' - _playerStepIndex), 63, 135, back, front, surface, 'Z' - '$' + 1);
	drawStringInSurface(Common::String('7' - _playerHeightNumber), 240, 135, back, front, surface, 'Z' - '$' + 1);

	if (_shootingFrames > 0) {
		drawStringInSurface("4", 232, 135, back, front, surface, 'Z' - '$' + 1);
		drawStringInSurface("<", 240, 135, back, front, surface, 'Z' - '$' + 1);
	}
	drawAnalogClock(surface, 90, 172, back, other, front);
	drawIndicator(surface, 45, 4, 12);
	drawEclipseIndicator(surface, 228, 0, front, other);

	uint32 blue = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0x55, 0x55, 0xFF);

	Common::Rect jarBackground(124, 165, 148, 192);
	surface->fillRect(jarBackground, back);

	Common::Rect jarWater(124, 192 - _gameStateVars[k8bitVariableEnergy], 148, 192);
	surface->fillRect(jarWater, blue);

	surface->fillRect(Common::Rect(225, 168, 235, 187), front);
	drawCompass(surface, 229, 177, _yaw, 10, back);

}

} // End of namespace Freescape
