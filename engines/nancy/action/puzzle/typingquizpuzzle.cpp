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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/random.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/input.h"
#include "engines/nancy/util.h"
#include "engines/nancy/font.h"

#include "engines/nancy/state/scene.h"
#include "engines/nancy/action/puzzle/typingquizpuzzle.h"

namespace Nancy {
namespace Action {

// Persists the previous typing rate so the adaptive target can scale across plays.
static int s_lastScore = 0;

static char toAsciiUpper(char c) {
	return (c >= 'a' && c <= 'z') ? (char)(c - ('a' - 'A')) : c;
}

static bool caseFoldEquals(char a, char b) {
	return toAsciiUpper(a) == toAsciiUpper(b);
}

void TypingQuizPuzzle::readData(Common::SeekableReadStream &stream) {
	readFilename(stream, _imageName);          // 0x000

	_numImageRects = stream.readUint16LE();    // 0x021
	for (uint i = 0; i < kMaxImageRects; ++i)
		readRect(stream, _balloonSrcRects[i]); // 0x023

	readRect(stream, _poppedSrcRect);          // 0x163

	_numPositions = stream.readUint16LE();     // 0x173
	for (uint i = 0; i < kMaxBalloons; ++i)
		readRect(stream, _positionRects[i]);   // 0x175

	for (uint i = 0; i < kNumDigits; ++i)
		readRect(stream, _scoreDigitRects[i]); // 0x2b5
	for (uint i = 0; i < kNumDigits; ++i)
		readRect(stream, _timerDigitRects[i]); // 0x355

	stream.skip(16);                           // 0x3f5 timer bounding rect (unused)

	_scoreDest.x = stream.readSint32LE();      // 0x405
	_scoreDest.y = stream.readSint32LE();      // 0x409
	_timerDest.x = stream.readSint32LE();      // 0x40d
	_timerDest.y = stream.readSint32LE();      // 0x411
	stream.skip(8);                            // 0x415 score/timer clip widths (unused)

	readRect(stream, _passedMsgSrcRect);       // 0x41d
	_passedMsgDuration = stream.readUint32LE();// 0x42d

	_caseSensitive = stream.readByte() != 0;   // 0x431
	_allowDigits   = stream.readByte() != 0;   // 0x432
	_keyboardMode  = stream.readByte();        // 0x433
	stream.read(_allowedChars, kAllowedCharsLen); // 0x434

	_balloonLifeMin = stream.readUint16LE();   // 0x452
	_balloonLifeMax = stream.readUint16LE();   // 0x454
	_minBalloons    = stream.readUint16LE();   // 0x456
	_maxBalloons    = stream.readUint16LE();   // 0x458
	_timeLimit      = stream.readUint16LE();   // 0x45a
	stream.skip(2);                            // 0x45c (unused)
	_scoreThreshold     = stream.readUint16LE(); // 0x45e
	_targetScore        = stream.readUint16LE(); // 0x460
	_targetScorePercent = stream.readUint16LE(); // 0x462
	_minTargetScore     = stream.readUint16LE(); // 0x464

	_popSound.readNormal(stream);              // 0x466
	_wrongSound.readNormal(stream);            // 0x497
	_escapeSound.readNormal(stream);           // 0x4c8

	_winScene.readData(stream);                // 0x4f9 (20 bytes)
	_winScene.continueSceneSound = stream.readUint16LE(); // 0x50d
	_winFlag = stream.readSint16LE();          // 0x50f

	_winSound.readNormal(stream);              // 0x511

	_defaultScene.readData(stream);            // 0x542 (20 bytes)
	_defaultScene.continueSceneSound = stream.readUint16LE(); // 0x556
	_flagThreshold = stream.readSint16LE();    // 0x558
	_flagFail      = stream.readSint16LE();    // 0x55a

	stream.skip(18);                           // 0x55c (trailing, unused)

	if (_numImageRects > kMaxImageRects)
		_numImageRects = kMaxImageRects;
	if (_numPositions > (uint16)kMaxBalloons)
		_numPositions = kMaxBalloons;
}

void TypingQuizPuzzle::init() {
	Common::Rect vpBounds = NancySceneState.getViewport().getBounds();
	_drawSurface.create(vpBounds.width(), vpBounds.height(),
	                    g_nancy->_graphics->getInputPixelFormat());
	_drawSurface.clear(g_nancy->_graphics->getTransColor());
	setTransparent(true);
	setVisible(true);
	moveTo(vpBounds);

	g_nancy->_resource->loadImage(_imageName, _image);
	_image.setTransparentColor(g_nancy->_graphics->getTransColor());

	if (_popSound.name != "NO SOUND")
		g_nancy->_sound->loadSound(_popSound);
	if (_wrongSound.name != "NO SOUND")
		g_nancy->_sound->loadSound(_wrongSound);
	if (_escapeSound.name != "NO SOUND")
		g_nancy->_sound->loadSound(_escapeSound);

	// Adaptive target: scale the threshold to the previous run's typing rate
	if (_targetScorePercent < 1 || s_lastScore < 1) {
		_effectiveTarget = _targetScore;
	} else {
		_effectiveTarget = (uint16)(((int)s_lastScore * (int)_targetScorePercent) / 100);
		if (_effectiveTarget <= _minTargetScore)
			_effectiveTarget = _minTargetScore;
	}

	_startTime  = g_nancy->getTotalPlayTime();
	_gameState  = kPlaying;
	_pops       = 0;
	_score      = 0;
	_reachedTarget    = false;
	_reachedThreshold = false;

	spawnInitialBalloons();
	redraw();
}

bool TypingQuizPuzzle::isValidChar(byte c) const {
	if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'))
		return true;
	if (_allowDigits && c >= '0' && c <= '9')
		return true;
	if (_allowedChars[0] != 0) {
		for (uint i = 0; i < kAllowedCharsLen; ++i) {
			if (c == _allowedChars[i])
				return true;
		}
	}
	return false;
}

char TypingQuizPuzzle::pickRandomChar() {
	Common::RandomSource &rnd = *g_nancy->_randomSource;
	int maxChar = (_keyboardMode != 0) ? 0x100 : (_allowedChars[0] != 0 ? 0x7f : 0x7b);

	for (;;) {
		int c;
		if (_allowDigits)
			c = (int)rnd.getRandomNumber(maxChar - 0x30 - 1) + 0x30;
		else
			c = (int)rnd.getRandomNumber(maxChar - 0x41 - 1) + 0x41;

		if (isValidChar((byte)c))
			return (char)c;
	}
}

int TypingQuizPuzzle::pickFreePosition() {
	Common::RandomSource &rnd = *g_nancy->_randomSource;

	for (;;) {
		int idx = (int)rnd.getRandomNumber(_numPositions - 1);
		bool occupied = false;
		for (uint i = 0; i < _numPositions; ++i) {
			if (_balloons[i].state != kBalloonInactive && _balloons[i].posIndex == idx) {
				occupied = true;
				break;
			}
		}
		if (!occupied)
			return idx;
	}
}

void TypingQuizPuzzle::spawnBalloon(uint slot) {
	Common::RandomSource &rnd = *g_nancy->_randomSource;
	Balloon &b = _balloons[slot];

	b.posIndex = pickFreePosition();
	b.posRect  = _positionRects[b.posIndex];
	b.imgRect  = _balloonSrcRects[_numImageRects ? (int)rnd.getRandomNumber(_numImageRects - 1) : 0];
	b.ch       = pickRandomChar();
	b.spawnTime  = g_nancy->getTotalPlayTime();
	b.popEndTime = 0;
	b.lifetime = (uint16)((int)rnd.getRandomNumber(_balloonLifeMax - _balloonLifeMin) + _balloonLifeMin);
	b.state    = kBalloonActive;
}

void TypingQuizPuzzle::spawnInitialBalloons() {
	for (uint i = 0; i < kMaxBalloons; ++i)
		_balloons[i].state = kBalloonInactive;

	Common::RandomSource &rnd = *g_nancy->_randomSource;
	_activeCount = (int)rnd.getRandomNumber(_maxBalloons - _minBalloons) + _minBalloons;
	if (_activeCount > (int)_numPositions)
		_activeCount = _numPositions;

	for (int c = 0; c < _activeCount; ++c)
		spawnBalloon(c);
}

void TypingQuizPuzzle::respawnBalloons(int maxToAdd) {
	Common::RandomSource &rnd = *g_nancy->_randomSource;

	int target;
	if (_activeCount + maxToAdd < _minBalloons) {
		target = _minBalloons;
	} else {
		int add = (int)rnd.getRandomNumber(maxToAdd);
		while (_activeCount + add < _minBalloons || _activeCount + add > _maxBalloons)
			add = (int)rnd.getRandomNumber(maxToAdd);
		target = _activeCount + add;
	}

	int toSpawn = target - _activeCount;
	_activeCount = target;

	for (uint slot = 0; slot < _numPositions && toSpawn > 0; ++slot) {
		if (_balloons[slot].state == kBalloonInactive) {
			spawnBalloon(slot);
			--toSpawn;
		}
	}
}

void TypingQuizPuzzle::expireBalloons(uint32 now) {
	int expired = 0;

	for (uint i = 0; i < _numPositions; ++i) {
		Balloon &b = _balloons[i];

		// Active balloons float away once their lifetime elapses
		if (b.state == kBalloonActive && (now - b.spawnTime) / 1000 > b.lifetime) {
			b.state = kBalloonInactive;
			--_activeCount;
			++expired;
		}

		// Popped balloons clear once their burst animation finishes
		if (b.state == kBalloonPopped && now >= b.popEndTime) {
			b.state = kBalloonInactive;
			--_activeCount;
			respawnBalloons(2);
		}
	}

	if (expired > 0) {
		if (_escapeSound.name != "NO SOUND")
			g_nancy->_sound->playSound(_escapeSound);
		respawnBalloons(2);
	}
}

void TypingQuizPuzzle::processTyping(uint32 now) {
	for (uint k = 0; k < _pendingChars.size(); ++k) {
		char typed = _pendingChars[k];
		bool matched = false;

		for (uint i = 0; i < _numPositions; ++i) {
			Balloon &b = _balloons[i];
			if (b.state != kBalloonActive)
				continue;

			bool isMatch = _caseSensitive ? (b.ch == typed) : caseFoldEquals(b.ch, typed);
			if (isMatch) {
				b.state = kBalloonPopped;
				b.popEndTime = now + 1000;
				++_pops;
				if (_popSound.name != "NO SOUND")
					g_nancy->_sound->playSound(_popSound);
				matched = true;
				break;
			}
		}

		if (!matched && _wrongSound.name != "NO SOUND")
			g_nancy->_sound->playSound(_wrongSound);
	}

	_pendingChars.clear();
}

void TypingQuizPuzzle::updateScore(uint32 now) {
	uint32 elapsedSec = (now - _startTime) / 1000;

	// Score is a typing rate (characters per minute)
	if (elapsedSec > 0 && _pops > 0)
		_score = (int)((float)_pops * 60.0f / (float)elapsedSec);

	if (elapsedSec >= _timeLimit)
		_gameState = kEvaluate;
}

void TypingQuizPuzzle::drawNumber(int value, const Common::Rect *digitRects, int x, int y) {
	if (value < 0)
		value = 0;

	// Render digits most-significant first using each digit sprite's own width
	Common::String digits = Common::String::format("%d", value);
	int drawX = x;
	for (uint i = 0; i < digits.size(); ++i) {
		int d = digits[i] - '0';
		const Common::Rect &src = digitRects[d];
		_drawSurface.blitFrom(_image, src, Common::Point(drawX, y));
		drawX += src.width();
	}
}

void TypingQuizPuzzle::redraw() {
	_needsRedraw = true;
	_drawSurface.clear(g_nancy->_graphics->getTransColor());

	if (_gameState != kPlaying) {
		// Result message, centred in the viewport
		int w = _passedMsgSrcRect.width();
		int h = _passedMsgSrcRect.height();
		int x = (_drawSurface.w - w) / 2;
		int y = (_drawSurface.h - h) / 2;
		_drawSurface.blitFrom(_image, _passedMsgSrcRect, Common::Point(x, y));
		return;
	}

	const Font *font = g_nancy->_graphics->getFont(_fontID);

	// Balloons and their characters
	for (uint i = 0; i < _numPositions; ++i) {
		const Balloon &b = _balloons[i];

		if (b.state == kBalloonActive) {
			_drawSurface.blitFrom(_image, b.imgRect, Common::Point(b.posRect.left, b.posRect.top));

			if (font) {
				Common::String s(b.ch);
				int cw = font->getStringWidth(s);
				int ch = font->getFontHeight();
				int cx = b.posRect.left + (b.posRect.width() - cw) / 2;
				int cy = b.posRect.top + (b.posRect.height() - ch) / 2;
				font->drawString(&_drawSurface, s, cx, cy, b.posRect.width(), 0);
			}
		} else if (b.state == kBalloonPopped) {
			_drawSurface.blitFrom(_image, _poppedSrcRect, Common::Point(b.posRect.left, b.posRect.top));
		}
	}

	// Score and remaining-time displays
	drawNumber(_score, _scoreDigitRects, _scoreDest.x, _scoreDest.y);

	uint32 elapsedSec = (g_nancy->getTotalPlayTime() - _startTime) / 1000;
	int remaining = (int)_timeLimit - (int)elapsedSec;
	drawNumber(remaining, _timerDigitRects, _timerDest.x, _timerDest.y);
}

void TypingQuizPuzzle::triggerSceneChange() {
	if (_reachedTarget) {
		if (_winScene.sceneID != 9999)
			NancySceneState.changeScene(_winScene);
		if (_winFlag != -1)
			NancySceneState.setEventFlag(_winFlag, g_nancy->_true);
	} else {
		NancySceneState.changeScene(_defaultScene);
		if (_reachedThreshold && _flagThreshold != -1)
			NancySceneState.setEventFlag(_flagThreshold, g_nancy->_true);
		else if (_flagFail != -1)
			NancySceneState.setEventFlag(_flagFail, g_nancy->_true);
	}
}

void TypingQuizPuzzle::execute() {
	switch (_state) {
	case kBegin:
		init();
		registerGraphics();
		_state = kRun;
		// fall through
	case kRun: {
		uint32 now = g_nancy->getTotalPlayTime();

		switch (_gameState) {
		case kPlaying:
			processTyping(now);
			expireBalloons(now);
			updateScore(now);
			redraw();
			break;

		case kEvaluate:
			if (_score >= (int)_effectiveTarget) {
				_reachedTarget = true;
				if (_winSound.name != "NO SOUND") {
					g_nancy->_sound->loadSound(_winSound);
					g_nancy->_sound->playSound(_winSound);
				}
			} else {
				_reachedTarget    = false;
				_reachedThreshold = _score >= (int)_scoreThreshold;
			}

			_resultEndTime = now + _passedMsgDuration;
			_gameState = kResult;
			redraw();
			break;

		case kResult:
			redraw();
			if (now >= _resultEndTime) {
				s_lastScore = _score;
				_state = kActionTrigger;
			}
			break;
		}
		break;
	}
	case kActionTrigger:
		g_nancy->_sound->stopSound(_popSound);
		g_nancy->_sound->stopSound(_wrongSound);
		g_nancy->_sound->stopSound(_escapeSound);
		g_nancy->_sound->stopSound(_winSound);

		triggerSceneChange();
		finishExecution();
		break;
	}
}

void TypingQuizPuzzle::handleInput(NancyInput &input) {
	if (_state != kRun || _gameState != kPlaying)
		return;

	for (auto &key : input.otherKbdInput) {
		if (key.ascii != 0 && isValidChar((byte)key.ascii))
			_pendingChars += (char)key.ascii;
	}
}

} // End of namespace Action
} // End of namespace Nancy
