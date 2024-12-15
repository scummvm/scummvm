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

void DarkEngine::initCPC() {
	_viewArea = Common::Rect(36, 24, 284, 125);
	_soundIndexShoot = 0xa;
	_soundIndexStart = 0x17;
	_soundIndexAreaChange = 0x1c;
	_soundIndexDestroyECD = 0x1b;
	_soundIndexRestoreECD = 8;
}

extern byte kCPCPaletteTitleData[4][3];
extern byte kCPCPaletteBorderData[4][3];

byte kCPCPaletteDarkTitle[16][3] = {
	{0x00, 0x00, 0x00}, // 0: X
	{0xff, 0xff, 0xff}, // 1: ?
	{0x80, 0x80, 0x80}, // 2: X
	{0xff, 0x00, 0xff}, // 3: X
	{0x80, 0x80, 0x80}, // 4: X
	{0xff, 0xff, 0x00}, // 5: X
	{0x80, 0x00, 0x00}, // 6: X
	{0xff, 0x00, 0x00}, // 7: X
	{0x00, 0x80, 0x80}, // 8: X
	{0xff, 0x00, 0x80}, // 9: X
	{0xff, 0x80, 0x00}, // 10: X
	{0xff, 0x80, 0x80}, // 11: X
	{0x00, 0xff, 0x00}, // 12: X
	{0x00, 0x00, 0x80}, // 13: X
	{0x00, 0x00, 0xff}, // 14: X
	{0x00, 0x80, 0x00}, // 15: X
};

extern Graphics::ManagedSurface *readCPCImage(Common::SeekableReadStream *file, bool mode0);

void DarkEngine::loadAssetsCPCFullGame() {
	Common::File file;

	file.open("DARK1.SCR");
	if (!file.isOpen())
		error("Failed to open DARK1.SCR");

	_title = readCPCImage(&file, false);
	_title->setPalette((byte*)&kCPCPaletteDarkTitle, 0, 16);

	file.close();
	file.open("DARK2.SCR");
	if (!file.isOpen())
		error("Failed to open DARK2.SCR");

	_border = readCPCImage(&file, true);
	_border->setPalette((byte*)&kCPCPaletteBorderData, 0, 4);

	file.close();
	file.open("DARKCODE.BIN");

	if (!file.isOpen())
		error("Failed to open DARKCODE.BIN");

	loadMessagesFixedSize(&file, 0x5d9, 16, 27);
	loadFonts(&file, 0x60f3);
	loadGlobalObjects(&file, 0x9a, 23);
	load8bitBinary(&file, 0x6255, 16);
	_indicators.push_back(loadBundledImage("dark_fallen_indicator"));
	_indicators.push_back(loadBundledImage("dark_crouch_indicator"));
	_indicators.push_back(loadBundledImage("dark_walk_indicator"));
	_indicators.push_back(loadBundledImage("dark_jet_indicator"));

	for (auto &it : _indicators)
		it->convertToInPlace(_gfx->_texturePixelFormat);
}

void DarkEngine::drawCPCUI(Graphics::Surface *surface) {
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

	int score = _gameStateVars[k8bitVariableScore];
	int ecds = _gameStateVars[kVariableActiveECDs];
	drawStringInSurface(Common::String::format("%04d", int(2 * _position.x())), 200, 137, front, back, surface);
	drawStringInSurface(Common::String::format("%04d", int(2 * _position.z())), 200, 145, front, back, surface);
	drawStringInSurface(Common::String::format("%04d", int(2 * _position.y())), 200, 153, front, back, surface);

	drawStringInSurface(Common::String::format("%02d", int(_angleRotations[_angleRotationIndex])), 72, 168, front, back, surface);
	drawStringInSurface(Common::String::format("%3d", _playerSteps[_playerStepIndex]), 72, 177, front, back, surface);
	drawStringInSurface(Common::String::format("%07d", score), 95, 8, front, back, surface);
	drawStringInSurface(Common::String::format("%3d%%", ecds), 191, 8, front, back, surface);

	int seconds, minutes, hours;
	getTimeFromCountdown(seconds, minutes, hours);

	Common::String message;
	int deadline;
	getLatestMessages(message, deadline);
	if (deadline <= _countdown) {
		drawStringInSurface(message, 111, 173 + 4, back, front, surface);
		_temporaryMessages.push_back(message);
		_temporaryMessageDeadlines.push_back(deadline);
	} else
		drawStringInSurface(_currentArea->_name, 111, 173 + 4, front, back, surface);

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
	drawBinaryClock(surface, 300, 124, front, back);
	drawIndicator(surface, 160, 136);
}

} // End of namespace Freescape
