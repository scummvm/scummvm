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
}

void EclipseEngine::loadAssetsC64FullGame() {
	Common::File file;
	file.open("totaleclipse.c64.data");
	loadMessagesFixedSize(&file, 0x1d82, 16, 30);
	loadFonts(&file, 0xc3e);
	load8bitBinary(&file, 0x9a3e, 16);

	for (auto &it : _areaMap) {
		it._value->addStructure(_areaMap[255]);

		for (int16 id = 183; id < 207; id++)
			it._value->addObjectFromArea(id, _areaMap[255]);
	}

	Graphics::Surface *surf = loadBundledImage("eclipse_border");
	surf->convertToInPlace(_gfx->_texturePixelFormat);
	_border = new Graphics::ManagedSurface();
	_border->copyFrom(*surf);
	surf->free();
	delete surf;
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
	Common::String encodedScoreStr = shiftStr(scoreStr, 'Z' - '0' + 1);
	drawStringInSurface(encodedScoreStr, 133, 11, back, gray, surface, 'Z' - '0' + 1);

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

	drawStringInSurface(shiftStr("0", 'Z' - '$' + 1 - _angleRotationIndex), 79, 141, back, yellow, surface);
	drawStringInSurface(shiftStr("3", 'Z' - '$' + 1 - _playerStepIndex), 63, 141, back, yellow, surface);
	drawStringInSurface(shiftStr("7", 'Z' - '$' + 1 - _playerHeightNumber), 240, 141, back, yellow, surface);

	if (_shootingFrames > 0) {
		drawStringInSurface(shiftStr("4", 'Z' - '$' + 1), 232, 141, back, yellow, surface);
		drawStringInSurface(shiftStr("<", 'Z' - '$' + 1) , 240, 141, back, yellow, surface);
	}
	drawAnalogClock(surface, 89, 172, back, back, gray);

	surface->fillRect(Common::Rect(227, 168, 235, 187), gray);
	drawCompass(surface, 231, 177, _yaw, 10, back);

	//drawIndicator(surface, 65, 7, 8);
	drawEclipseIndicator(surface, 215, 3, front, gray);
}

} // End of namespace Freescape