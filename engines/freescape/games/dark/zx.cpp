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
#include "freescape/games/dark/dark.h"
#include "freescape/language/8bitDetokeniser.h"

namespace Freescape {

void DarkEngine::initZX() {
	_viewArea = Common::Rect(56, 28, 265, 132);
	_maxEnergy = 63;
	_maxShield = 63;

	_soundIndexShoot = 5;
	_soundIndexCollide = -1; // Scripted
	_soundIndexFall = 3;
	_soundIndexClimb = 4;
	_soundIndexMenu = 25;
	_soundIndexStart = 11;
	_soundIndexAreaChange = 0x1c;
	_soundIndexRestoreECD = 30;

	_soundIndexNoShield = 14;
	_soundIndexNoEnergy = 14;
	_soundIndexFallen = 7;
	_soundIndexTimeout = 14;
	_soundIndexForceEndGame = 14;
	_soundIndexCrushed = 25;
	_soundIndexMissionComplete = 8;
}

void DarkEngine::loadAssetsZXFullGame() {
	Common::File file;

	file.open("darkside.zx.title");
	if (file.isOpen()) {
		_title = loadAndCenterScrImage(&file);
	} else
		error("Unable to find darkside.zx.title");

	file.close();
	file.open("darkside.zx.border");
	if (file.isOpen()) {
		_border = loadAndCenterScrImage(&file);
	} else
		error("Unable to find driller.zx.border");
	file.close();

	file.open("darkside.zx.data");
	if (!file.isOpen())
		error("Failed to open darksize.zx.data");

	loadMessagesFixedSize(&file, 0x56b - 6, 16, 27);

	loadFonts(&file, 0x5d60 - 6);
	loadGlobalObjects(&file, 0x1a, 23);
	load8bitBinary(&file, 0x5ec0 - 4, 4);
	loadSpeakerFxZX(&file, 0x9c1, 0xa55);

	_indicators.push_back(loadBundledImage("dark_fallen_indicator"));
	_indicators.push_back(loadBundledImage("dark_crouch_indicator"));
	_indicators.push_back(loadBundledImage("dark_walk_indicator"));
	_indicators.push_back(loadBundledImage("dark_jet_indicator"));

	for (auto &it : _indicators)
		it->convertToInPlace(_gfx->_texturePixelFormat);
}

void DarkEngine::loadAssetsZXDemo() {
	Common::File file;

	file.open("darkside.zx.title");
	if (file.isOpen()) {
		_title = loadAndCenterScrImage(&file);
	} else
		error("Unable to find darkside.zx.title");

	file.close();
	file.open("darkside.zx.border");
	if (file.isOpen()) {
		_border = loadAndCenterScrImage(&file);
	} else
		error("Unable to find driller.zx.border");
	file.close();

	file.open("darkside.zx.data");
	if (!file.isOpen())
		error("Failed to open darksize.zx.data");

	loadMessagesFixedSize(&file, 0x56b, 16, 27);
	loadMessagesFixedSize(&file, 0x5761, 264, 5);
	loadSpeakerFxZX(&file, 0x9c7, 0xa5b);

	loadFonts(&file, 0x6164);
	loadGlobalObjects(&file, 0x20, 23);
	load8bitBinary(&file, 0x62c6, 4);
	_indicators.push_back(loadBundledImage("dark_fallen_indicator"));
	_indicators.push_back(loadBundledImage("dark_crouch_indicator"));
	_indicators.push_back(loadBundledImage("dark_walk_indicator"));
	_indicators.push_back(loadBundledImage("dark_jet_indicator"));

	for (auto &it : _indicators)
		it->convertToInPlace(_gfx->_texturePixelFormat);
}

void DarkEngine::drawZXUI(Graphics::Surface *surface) {
	uint32 color = 7;
	uint8 r, g, b;

	_gfx->readFromPalette(color, r, g, b);
	uint32 front = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	color = _currentArea->_usualBackgroundColor;
	if (_gfx->_colorRemaps && _gfx->_colorRemaps->contains(color)) {
		color = (*_gfx->_colorRemaps)[color];
	}

	_gfx->readFromPalette(color, r, g, b);
	uint32 back = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);
	uint32 transparent = _gfx->_texturePixelFormat.ARGBToColor(0x00, 0x00, 0x00, 0x00);

	int score = _gameStateVars[k8bitVariableScore];
	int ecds = _gameStateVars[kVariableActiveECDs];
	surface->fillRect(Common::Rect(193, 140, 223, 163), back);
	drawStringInSurface(Common::String::format("%04d", int(2 * _position.x())), 191, 141, front, transparent, surface);
	drawStringInSurface(Common::String::format("%04d", int(2 * _position.z())), 191, 149, front, transparent, surface);
	drawStringInSurface(Common::String::format("%04d", int(2 * _position.y())), 191, 157, front, transparent, surface);

	surface->fillRect(Common::Rect(80, 165, 95, 171), back);
	surface->fillRect(Common::Rect(80, 172, 102, 178), back);
	drawStringInSurface(Common::String::format("%02d", int(_angleRotations[_angleRotationIndex])), 79, 165, front, transparent, surface);
	drawStringInSurface(Common::String::format("%3d", _playerSteps[_playerStepIndex]), 79, 173, front, transparent, surface);
	surface->fillRect(Common::Rect(96, 12, 151, 18), back);
	drawStringInSurface(Common::String::format("%07d", score), 95, 13, front, transparent, surface);
	drawStringInSurface(Common::String::format("%3d%%", ecds), 191, 13, front, back, surface);

	int seconds, minutes, hours;
	getTimeFromCountdown(seconds, minutes, hours);

	Common::String message;
	int deadline;
	getLatestMessages(message, deadline);
	if (deadline <= _countdown) {
		drawStringInSurface(message, 112, 173, back, front, surface);
		_temporaryMessages.push_back(message);
		_temporaryMessageDeadlines.push_back(deadline);
	} else
		drawStringInSurface(_currentArea->_name, 112, 173, front, back, surface);

	int energy = _gameStateVars[k8bitVariableEnergy]; // called fuel in this game
	int shield = _gameStateVars[k8bitVariableShield];

	if (shield >= 0) {
		Common::Rect shieldBar;
		shieldBar = Common::Rect(80, 140, 143 - (_maxShield - shield), 148);
		surface->fillRect(shieldBar, back);

		shieldBar = Common::Rect(80, 141, 143 - (_maxShield - shield), 147);
		surface->fillRect(shieldBar, front);
	}

	if (energy >= 0) {
		Common::Rect energyBar;
		energyBar = Common::Rect(80, 147, 143 - (_maxEnergy - energy), 155);
		surface->fillRect(energyBar, back);

		energyBar = Common::Rect(80, 148, 143 - (_maxEnergy - energy), 154);
		surface->fillRect(energyBar, front);
	}
	uint32 clockColor = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0xFF, 0x00, 0x00);
	drawBinaryClock(surface, 273, 128, clockColor, back);
	drawIndicator(surface, 152, 140);
}

} // End of namespace Freescape
