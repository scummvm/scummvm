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
	{41, 3}, {51, 3}, {65, 6}, {40, 16}, {58, 20}, {67, 10}, {37, 14}, {37, 15}, {67, 22}};

void SpiderEngine::runBeforeArcade(ArcadeShooting *arc) {
	_health = arc->health;
	_maxHealth = _health;
	resetStatistics();
	_checkpoint = _currentLevel;
	assert(!arc->player.empty());
	_playerFrames = decodeFrames(arc->player);
	_playerFrameSep = 0;

	for (Frames::iterator it =_playerFrames.begin(); it != _playerFrames.end(); ++it) {
		if ((*it)->getPixel(0, 0) == 255)
			break;
		if ((*it)->getPixel(0, 0) == 252)
			break;

		_playerFrameSep++;
	}

	if (_playerFrameSep == (int)_playerFrames.size()) {
		debugC(1, kHypnoDebugArcade, "No player separator frame found in %s! (size: %d)", arc->player.c_str(), _playerFrames.size());
	} else
		debugC(1, kHypnoDebugArcade, "Separator frame found at %d", _playerFrameSep);

	_playerFrameIdx = -1;
	_currentPlayerPosition = kPlayerLeft;
	_lastPlayerPosition = kPlayerLeft;
}

void SpiderEngine::runAfterArcade(ArcadeShooting *arc) {
	if (_health <= 0) {
		assert(_score >= _bonus);
		_score -= _bonus;
	}

	for (Frames::iterator it =_playerFrames.begin(); it != _playerFrames.end(); ++it) {
		(*it)->free();
		delete (*it);
	}
	_playerFrames.clear();

	if (isDemo() && _restoredContentEnabled) {
		if (_health == 0)
			showScore("Spider-man was defeated!");
		else
			showScore("Spider-Man saved the day!");
		_score = 0;
	}
}

void SpiderEngine::initSegment(ArcadeShooting *arc) {
	_segmentShootSequenceOffset = 0;
	_segmentShootSequenceMax = 0;

	uint32 randomSegmentShootSequence = _segmentShootSequenceOffset + _rnd->getRandomNumber(_segmentShootSequenceMax);
	SegmentShoots segmentShoots = arc->shootSequence[randomSegmentShootSequence];
	_shootSequence = segmentShoots.shootSequence;
	_segmentRepetitionMax = segmentShoots.segmentRepetition; // Usually zero
	_segmentRepetition = 0;
	_segmentOffset = 0;
	_segmentIdx = _segmentOffset;
}

void SpiderEngine::findNextSegment(ArcadeShooting *arc) {
	_segmentIdx = _segmentIdx + 1;
}


void SpiderEngine::pressedKey(const int keycode) {
	if (keycode == Common::KEYCODE_c) {
		if (_cheatsEnabled) {
			_skipLevel = true;
			return;
		}
	} else if (keycode == Common::KEYCODE_k) { // Added for testing
		_health = 0;
	} else if (keycode == Common::KEYCODE_LEFT) {
		_lastPlayerPosition = _currentPlayerPosition;
		_currentPlayerPosition = kPlayerLeft;
	} else if (keycode == Common::KEYCODE_DOWN) {
		_lastPlayerPosition = _currentPlayerPosition;
		_currentPlayerPosition = kPlayerBottom;
	} else if (keycode == Common::KEYCODE_RIGHT) {
		_lastPlayerPosition = _currentPlayerPosition;
		_currentPlayerPosition = kPlayerRight;
	} else if (keycode == Common::KEYCODE_UP) {
		_lastPlayerPosition = _currentPlayerPosition;
		_currentPlayerPosition = kPlayerTop;
	}
}

void SpiderEngine::missedTarget(Shoot *s, ArcadeShooting *arc) {
	if (_arcadeMode != "YC" && _arcadeMode != "YD")
		return;
	if ((uint32)(s->name[0]) == _currentPlayerPosition) {
		if (!_infiniteHealthCheat)
			_health = _health - s->attackWeight;
		hitPlayer();
	}
}


void SpiderEngine::hitPlayer() {
	if (_playerFrameSep < (int)_playerFrames.size()) {
		if (_playerFrameIdx < _playerFrameSep)
			_playerFrameIdx = _playerFrameSep;
	} else {
		uint32 c = kHypnoColorRed; // red
		_compositeSurface->fillRect(Common::Rect(0, 0, 640, 480), c);
		drawScreen();
	}
	if (!_hitSound.empty())
		playSound(_soundPath + _hitSound, 1, 11025);
}

void SpiderEngine::drawShoot(const Common::Point &target) {
	uint32 c = kSpiderColorWhite; // white
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
		} else if (_playerFrameIdx < 48 && _playerFrameIdx % 4 != 0 && _playerFrameIdx % 4 != 3) {
			_playerFrameIdx++;
			_lastPlayerPosition = _currentPlayerPosition;
		} else {
			if (_arcadeMode == "YD") {
				switch (_lastPlayerPosition) {
				case kPlayerTop:
					if ((_playerFrameIdx <= 11 && (_playerFrameIdx % 4 == 0 || _playerFrameIdx % 4 == 3)) || _playerFrameIdx >= 54)
						_playerFrameIdx = 49;
					else
						_playerFrameIdx++;
					break;

				case kPlayerBottom:
					if ((_playerFrameIdx <= 23  && (_playerFrameIdx % 4 == 0 || _playerFrameIdx % 4 == 3)) || _playerFrameIdx >= 65)
						_playerFrameIdx = 60;
					else
						_playerFrameIdx++;
					break;
				case kPlayerLeft:
					if ((_playerFrameIdx <= 35 && (_playerFrameIdx % 4 == 0 || _playerFrameIdx % 4 == 3)) || _playerFrameIdx >= 77)
						_playerFrameIdx = 72;
					else
						_playerFrameIdx++;
					break;

				case kPlayerRight:
					if ((_playerFrameIdx <= 47 && (_playerFrameIdx % 4 == 0 || _playerFrameIdx % 4 == 3)) || _playerFrameIdx >= 89)
						_playerFrameIdx = 84;
					else
						_playerFrameIdx++;
					break;
				}
			}
		}
	} else if (_arcadeMode == "YE" || _arcadeMode == "YF") {
		if (_arcadeMode == "YF") {
			int fraction = _background->decoder->getFrameCount() / (_maxHealth / 2);
			if (_background->decoder->getCurFrame() % fraction == 0)
				_health = MAX(1, _health - 1);

			if (checkArcadeObjectives())
				_skipLevel = true;
		}

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
	if (d >= 22)
		return;
	r = Common::Rect(256, 152 + d, 272, 174);
	if (d >= 11)
		c = kHypnoColorRed; // red
	else
		c = kHypnoColorGreen; // green

	_compositeSurface->fillRect(r, c);

	r = Common::Rect(256, 152, 272, 174);
	c = kSpiderColorBlue; // blue
	_compositeSurface->frameRect(r, c);

	drawString("block05.fgx", "ENERGY", 248, 180, 38, c);
}
byte *SpiderEngine::getTargetColor(Common::String name, int levelId) {
	return getPalette(kHypnoColorRed);
}

} // End of namespace Hypno
