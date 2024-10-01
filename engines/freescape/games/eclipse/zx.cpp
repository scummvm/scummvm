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

void EclipseEngine::initZX() {
	_viewArea = Common::Rect(56, 36, 265, 139);
	_maxEnergy = 63;
	_maxShield = 63;

	_soundIndexShoot = 5;
	_soundIndexCollide = -1;
	_soundIndexFall = 3;
	_soundIndexClimb = 4;
	_soundIndexMenu = -1;
	_soundIndexStart = 7;
	_soundIndexAreaChange = 7;

	_soundIndexStartFalling = 6;
	_soundIndexEndFalling = 5;

	_soundIndexNoShield = 8;
	_soundIndexNoEnergy = -1;
	_soundIndexFallen = 8;
	_soundIndexTimeout = 8;
	_soundIndexForceEndGame = 8;
	_soundIndexCrushed = 8;
	_soundIndexMissionComplete = 16;
}

void EclipseEngine::loadAssetsZXFullGame() {
	Common::File file;

	file.open("totaleclipse.zx.title");
	if (file.isOpen()) {
		_title = loadAndCenterScrImage(&file);
	} else
		error("Unable to find totaleclipse.zx.title");

	file.close();
	file.open("totaleclipse.zx.border");
	if (file.isOpen()) {
		_border = loadAndCenterScrImage(&file);
	} else
		error("Unable to find totaleclipse.zx.border");
	file.close();

	file.open("totaleclipse.zx.data");
	if (!file.isOpen())
		error("Failed to open totaleclipse.zx.data");

	if (isEclipse2()) {
		loadMessagesFixedSize(&file, 0x2ac, 16, 30);
		loadFonts(&file, 0x61c3);
		loadSpeakerFxZX(&file, 0x8c6, 0x91a);
		load8bitBinary(&file, 0x63bb, 4);
	} else {
		loadMessagesFixedSize(&file, 0x2ac, 16, 23);
		loadFonts(&file, 0x6163);
		loadSpeakerFxZX(&file, 0x816, 0x86a);
		load8bitBinary(&file, 0x635b, 4);

		// These paper colors are also invalid, but to signal the use of a special effect (only in zx release)
		_areaMap[42]->_paperColor = 0;
		_areaMap[42]->_underFireBackgroundColor = 0;
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

	_indicators.push_back(loadBundledImage("eclipse_ankh_indicator"));

	for (auto &it : _indicators)
		it->convertToInPlace(_gfx->_texturePixelFormat);
}

void EclipseEngine::loadAssetsZXDemo() {
	Common::File file;

	file.open("totaleclipse.zx.title");
	if (file.isOpen()) {
		_title = loadAndCenterScrImage(&file);
	} else
		error("Unable to find totaleclipse.zx.title");

	file.close();
	file.open("totaleclipse.zx.border");
	if (file.isOpen()) {
		_border = loadAndCenterScrImage(&file);
	} else
		error("Unable to find totaleclipse.zx.border");
	file.close();

	file.open("totaleclipse.zx.data");
	if (!file.isOpen())
		error("Failed to open totaleclipse.zx.data");

	if (_variant & GF_ZX_DEMO_MICROHOBBY) {
		loadSpeakerFxZX(&file, 0x798, 0x7ec);
		loadMessagesFixedSize(&file, 0x2ac, 16, 23);
		loadMessagesFixedSize(&file, 0x56e6, 264, 1);
		loadFonts(&file, 0x5f7b);
		load8bitBinary(&file, 0x6173, 4);
	} else if (_variant & GF_ZX_DEMO_CRASH) {
		loadSpeakerFxZX(&file, 0x65c, 0x6b0);
		loadMessagesFixedSize(&file, 0x364, 16, 9);
		loadMessagesFixedSize(&file, 0x5901, 264, 5);
		loadFonts(&file, 0x6589);
		load8bitBinary(&file, 0x6781, 4);
	} else
		error("Unknown ZX Spectrum demo variant");

	for (auto &it : _areaMap) {
		it._value->_name = "  NOW TRAINING  ";
		it._value->addStructure(_areaMap[255]);
		for (int16 id = 183; id < 207; id++)
			it._value->addObjectFromArea(id, _areaMap[255]);
	}

	_indicators.push_back(loadBundledImage("eclipse_ankh_indicator"));

	for (auto &it : _indicators)
		it->convertToInPlace(_gfx->_texturePixelFormat);
}

void EclipseEngine::drawZXUI(Graphics::Surface *surface) {
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

	_gfx->readFromPalette(7, r, g, b);
	uint32 gray = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	_gfx->readFromPalette(5, r, g, b);
	uint32 blue = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	_gfx->readFromPalette(2, r, g, b);
	uint32 red = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);


	int score = _gameStateVars[k8bitVariableScore];
	int shield = _gameStateVars[k8bitVariableShield] * 100 / _maxShield;
	int energy = _gameStateVars[k8bitVariableEnergy];
	shield = shield < 0 ? 0 : shield;

	uint32 yellow = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0xFF, 0xFF, 0);

	Common::String message;
	int deadline;
	getLatestMessages(message, deadline);
	if (deadline <= _countdown) {
		drawStringInSurface(message, 102, 141, back, yellow, surface);
		_temporaryMessages.push_back(message);
		_temporaryMessageDeadlines.push_back(deadline);
	} else if (!_currentAreaMessages.empty())
		drawStringInSurface(_currentArea->_name, 102, 141, back, yellow, surface);

	Common::String scoreStr = Common::String::format("%07d", score);
	drawStringInSurface(scoreStr, 133, 11, back, gray, surface, 'Z' - '0' + 1);

	Common::String shieldStr = Common::String::format("%d", shield);

	int x = 171;
	if (shield < 10)
		x = 179;
	else if (shield < 100)
		x = 175;

	if (energy < 0)
		energy = 0;

	drawStringInSurface(shieldStr, x, 161, back, red, surface);

	Common::Rect jarBackground(120, 162, 144, 192 - 4);
	surface->fillRect(jarBackground, back);

	Common::Rect jarWater(120, 192 - energy - 4, 144, 192 - 4);
	surface->fillRect(jarWater, blue);

	drawStringInSurface(Common::String('0' + _angleRotationIndex - 3), 79, 141, back, yellow, surface, 'Z' - '$' + 1);
	drawStringInSurface(Common::String('3' - _playerStepIndex), 63, 141, back, yellow, surface, 'Z' - '$' + 1);
	drawStringInSurface(Common::String('7' - _playerHeightNumber), 240, 141, back, yellow, surface, 'Z' - '$' + 1);

	if (_shootingFrames > 0) {
		drawStringInSurface("4", 232, 141, back, yellow, surface, 'Z' - '$' + 1);
		drawStringInSurface("<", 240, 141, back, yellow, surface, 'Z' - '$' + 1);
	}
	drawAnalogClock(surface, 89, 172, back, back, gray);

	surface->fillRect(Common::Rect(227, 168, 235, 187), gray);
	drawCompass(surface, 231, 177, _yaw, 10, back);

	drawIndicator(surface, 65, 7, 8);
	drawEclipseIndicator(surface, 215, 3, front, gray);
}

} // End of namespace Freescape
