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
	_viewArea = Common::Rect(56, 28, 265, 132+6);
	_maxEnergy = 63;
	_maxShield = 63;
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
		loadMessagesFixedSize(&file, 0x2ac, 16, 23);
		loadMessagesFixedSize(&file, 0x56e6, 264, 1);
		loadFonts(&file, 0x5f7b);
		load8bitBinary(&file, 0x6173, 4);
	} else if (_variant & GF_ZX_DEMO_CRASH) {
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

	/*_indicators.push_back(loadBundledImage("dark_fallen_indicator"));
	_indicators.push_back(loadBundledImage("dark_crouch_indicator"));
	_indicators.push_back(loadBundledImage("dark_walk_indicator"));
	_indicators.push_back(loadBundledImage("dark_jet_indicator"));

	for (auto &it : _indicators)
		it->convertToInPlace(_gfx->_texturePixelFormat);*/
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

	_gfx->readFromPalette(2, r, g, b);
	uint32 red = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	int shield = _gameStateVars[k8bitVariableShield] * 100 / _maxShield;
	int score = _gameStateVars[k8bitVariableScore];

	if (!_currentAreaMessages.empty())
		drawStringInSurface(_currentAreaMessages[0], 102, 141, back, front, surface);

	Common::String scoreStr = Common::String::format("%07d", score);
	drawStringInSurface(scoreStr, 133, 11, back, gray, surface, 'Z' - '0' + 1);

	Common::String shieldStr = Common::String::format("%d", shield);

	int x = 171;
	if (shield < 10)
		x = 179;
	else if (shield < 100)
		x = 175;

	drawStringInSurface(shieldStr, x, 161, back, red, surface);

	drawStringInSurface(Common::String('0' + _angleRotationIndex - 3), 79, 141, back, front, surface, 'Z' - '$' + 1);
	drawStringInSurface(Common::String('3' - _playerStepIndex), 63, 141, back, front, surface, 'Z' - '$' + 1);
	drawStringInSurface(Common::String('7' - _playerHeightNumber), 240, 141, back, front, surface, 'Z' - '$' + 1);

	if (_shootingFrames > 0) {
		drawStringInSurface("4", 232, 141, back, front, surface, 'Z' - '$' + 1);
		drawStringInSurface("<", 240, 141, back, front, surface, 'Z' - '$' + 1);
	}
	drawAnalogClock(surface, 89, 172, back, back, gray);
}

} // End of namespace Freescape
