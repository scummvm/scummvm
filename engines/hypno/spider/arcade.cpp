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

#include "hypno/grammar.h"
#include "hypno/hypno.h"

#include "common/events.h"
#include "graphics/cursorman.h"
#include "gui/message.h"

namespace Hypno {

static const int oIndexYB[9] = {0, 1, 2, 7, 8, 3, 6, 5, 4};
static const int oIndexYE[9] = {4, 3, 2, 1, 0};
static const int shootOriginIndex[9][2] = {
	{41, 3}, {51, 3}, {65, 6}, {68, 9}, {71, 22}, {57, 20}, {37, 14}, {37, 11}, {57, 20}};

void SpiderEngine::hitPlayer() {
	if (_playerFrameSep < (int)_playerFrames.size()) {
		if (_playerFrameIdx < _playerFrameSep)
			_playerFrameIdx = _playerFrameSep;
	} else {
		uint32 c = 250; // red
		_compositeSurface->fillRect(Common::Rect(0, 0, 640, 480), c);
		drawScreen();
	}
	// if (!_hitSound.empty())
	//	playSound(_soundPath + _hitSound, 1);
}

void SpiderEngine::drawShoot(const Common::Point &target) {
	uint32 c = 248; // white
	uint32 ox = 0;
	uint32 oy = 0;

	if (_arcadeMode == "YC" || _arcadeMode == "YD") {
		return; // Nothing to shoot
	} else if (_arcadeMode == "YE" || _arcadeMode == "YF") {
		ox = _screenW / 2;
		oy = _screenH - _playerFrames[0]->h / 2;
	} else if (_arcadeMode == "YB") {
		uint32 idx = MIN(2, target.x / (_screenW / 3)) + 3 * MIN(2, target.y / (_screenH / 3));
		ox = 60 + shootOriginIndex[idx][0];
		oy = 129 + shootOriginIndex[idx][1];
	} else
		error("Invalid arcade mode %s", _arcadeMode.c_str());

	_compositeSurface->drawLine(ox, oy, target.x + 2, target.y, c);
	_compositeSurface->drawLine(ox, oy, target.x, target.y, c);
	_compositeSurface->drawLine(ox, oy, target.x - 2, target.y, c);

	playSound(_soundPath + _shootSound, 1);
}

void SpiderEngine::drawPlayer() {
	uint32 ox = 0;
	uint32 oy = 0;

	if (_arcadeMode == "YC" || _arcadeMode == "YD") {
		disableCursor(); // Not sure this a good place
		ox = 0;
		oy = 0;

		if (_playerFrameIdx < 0)
			_playerFrameIdx = 0;
		else if (_lastPlayerPosition != _currentPlayerPosition && (_playerFrameIdx % 4 == 0 || _playerFrameIdx % 4 == 3)) {

			switch (_lastPlayerPosition) {
			case kPlayerLeft:
				switch (_currentPlayerPosition) {
				case kPlayerTop:
					_lastPlayerPosition = _currentPlayerPosition;
					_playerFrameIdx = 1;
					break;
				case kPlayerBottom:
					_lastPlayerPosition = _currentPlayerPosition;
					_playerFrameIdx = 13;
					break;
				case kPlayerRight:
					_lastPlayerPosition = _currentPlayerPosition;
					_playerFrameIdx = 45;
					break;
				}
				break;
			case kPlayerRight:
				switch (_currentPlayerPosition) {
				case kPlayerTop:
					_lastPlayerPosition = _currentPlayerPosition;
					_playerFrameIdx = 5;
					break;
				case kPlayerBottom:
					_lastPlayerPosition = _currentPlayerPosition;
					_playerFrameIdx = 17;
					break;
				case kPlayerLeft:
					_lastPlayerPosition = _currentPlayerPosition;
					_playerFrameIdx = 33;
					break;
				}
				break;
			case kPlayerBottom:
				switch (_currentPlayerPosition) {
				case kPlayerTop:
					_lastPlayerPosition = _currentPlayerPosition;
					_playerFrameIdx = 9;
					break;
				case kPlayerLeft:
					_lastPlayerPosition = _currentPlayerPosition;
					_playerFrameIdx = 29;
					break;
				case kPlayerRight:
					_lastPlayerPosition = _currentPlayerPosition;
					_playerFrameIdx = 41;
					break;
				}
				break;
			case kPlayerTop:
				switch (_currentPlayerPosition) {
				case kPlayerBottom:
					_playerFrameIdx = 21;
					break;
				case kPlayerLeft:
					_playerFrameIdx = 25;
					break;
				case kPlayerRight:
					_playerFrameIdx = 37;
					break;
				}
				break;
			}
			_lastPlayerPosition = _currentPlayerPosition;
		} else if (_playerFrameIdx % 4 != 0 && _playerFrameIdx % 4 != 3) {
			_playerFrameIdx++;
			_lastPlayerPosition = _currentPlayerPosition;
		}
	} else if (_arcadeMode == "YE" || _arcadeMode == "YF") {
		Common::Point mousePos = g_system->getEventManager()->getMousePos();
		uint32 idx = mousePos.x / (_screenW / 5);
		_playerFrameIdx = oIndexYE[idx];
		ox = _screenW / 2 - _playerFrames[0]->w / 2;
		oy = _screenH - _playerFrames[0]->h;
	} else if (_arcadeMode == "YB") {
		ox = 60;
		oy = 129;
		if (_playerFrameIdx < _playerFrameSep) {
			Common::Point mousePos = g_system->getEventManager()->getMousePos();
			uint32 idx = MIN(2, mousePos.x / (_screenW / 3)) + 3 * MIN(2, mousePos.y / (_screenH / 3));
			_playerFrameIdx = oIndexYB[idx];
		} else {
			_playerFrameIdx++;
			if (_playerFrameIdx >= (int)_playerFrames.size())
				_playerFrameIdx = 0;
		}
	} else
		error("Invalid arcade mode %s", _arcadeMode.c_str());

	drawImage(*_playerFrames[_playerFrameIdx], ox, oy, true);
}

void SpiderEngine::drawCursorArcade(const Common::Point &mousePos) {
	if (_arcadeMode != "YC" && _arcadeMode != "YD") {
		HypnoEngine::drawCursorArcade(mousePos);
	}
}

void SpiderEngine::drawHealth() {
	Common::Rect r;
	uint32 c;
	int d = (22 * (_maxHealth - _health) / _maxHealth);

	r = Common::Rect(256, 152 + d, 272, 174);
	if (d >= 11)
		c = 250; // red
	else
		c = 251; // green

	_compositeSurface->fillRect(r, c);

	r = Common::Rect(256, 152, 272, 174);
	c = 252; // blue
	_compositeSurface->frameRect(r, c);

	drawString("ENERGY", 248, 180, 38, c);
}

bool SpiderEngine::checkArcadeLevelCompleted(MVideo &background) {
	if (_skipLevel)
		return true;

	if (_arcadeMode == "YF") {
		if (!background.decoder || background.decoder->endOfVideo())
			_health = 0;

		if (_shoots.size() == 0)
			return false;

		for (Shoots::iterator it = _shoots.begin(); it != _shoots.end(); ++it)
			if (!it->destroyed)
				return false;

		return true;
	}
	return !background.decoder || background.decoder->endOfVideo();
}

} // End of namespace Hypno
