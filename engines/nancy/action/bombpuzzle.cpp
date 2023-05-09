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

#include "engines/nancy/nancy.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/input.h"
#include "engines/nancy/util.h"

#include "engines/nancy/state/scene.h"

#include "engines/nancy/action/bombpuzzle.h"

namespace Nancy {
namespace Action {

void BombPuzzle::init() {
	_screenPosition = _displayBounds;
	for (Common::Rect &r : _wireDests) {
		_screenPosition.extend(r);
	}

	_drawSurface.create(_screenPosition.width(), _screenPosition.height(), g_nancy->_graphicsManager->getInputPixelFormat());
	_drawSurface.clear(g_nancy->_graphicsManager->getTransColor());

	setTransparent(true);

	g_nancy->_resource->loadImage(_imageName, _image);
	RenderActionRecord::init();
}

void BombPuzzle::readData(Common::SeekableReadStream &stream) {
	readFilename(stream, _imageName);

	readRectArray(stream, _wireSrcs, 4);
	readRectArray(stream, _wireDests, 4);
	readRectArray(stream, _digitSrcs, 10);
	readRectArray(stream, _digitDests, 4);
	readRect(stream, _colonSrc);
	readRect(stream, _colonDest);
	readRect(stream, _displayBounds);

	_solveOrder.resize(4);
	for (uint i = 0; i < 4; ++i) {
		_solveOrder[i] = stream.readByte();
	}

	_snipSound.readNormal(stream);
	_noToolSound.readNormal(stream);
	_toolID = stream.readUint16LE();

	_solveSceneChange.readData(stream);
	stream.skip(2);
	_solveSound.readNormal(stream);

	_failSceneChange.readData(stream);
	stream.skip(2);
	_failSound.readNormal(stream);

	switch (NancySceneState.getDifficulty()) {
		case 0:
			_timerTotalTime = 30 * 1000;
			break;
		case 1:
			_timerTotalTime = 25 * 1000;
			break;
		case 2:
			_timerTotalTime = 20 * 1000;
			break;
	}

	_nextBlinkTime = _timerTotalTime;
	_timerBlinkTime = 10 * 1000; // 10 seconds for all difficulties
}

void BombPuzzle::updateGraphics() {
	if (_state != kRun) {
		return;
	}

	Time timeRemaining = NancySceneState.getTimerTime();

	if (timeRemaining == 0) {
		return;
	}

	if (timeRemaining > _timerTotalTime) {
		timeRemaining = 0;
	} else {
		timeRemaining = _timerTotalTime - timeRemaining;
	}

	bool toggleBlink = false;

	if (timeRemaining < _nextBlinkTime) {
		_nextBlinkTime = timeRemaining - 300; // hardcoded to 300 ms
		toggleBlink = timeRemaining < _timerBlinkTime;
	}

	if (_lastDrawnTime == timeRemaining.getSeconds() && !toggleBlink) {
		// State is the same as last call, do not redraw
		return;
	}

	Common::Rect t = _displayBounds;
	t.translate(-_screenPosition.left, -_screenPosition.top);

	_lastDrawnTime = timeRemaining.getSeconds();

	// Clear the display
	_drawSurface.fillRect(t, _drawSurface.getTransparentColor());

	if (toggleBlink) {
		if (!_isBlinking) {
			// Only clear the display
			_isBlinking = true;
			_needsRedraw = true;
			return;
		} else {
			// Redraw the display
			_isBlinking = false;
		}
	} else {
		if (_isBlinking) {
			// Only clear the display
			_needsRedraw = true;
			return;
		}
	}

	// Add 1 second to timer so it starts at 30/25/20 seconds
	timeRemaining += 1000;

	// Draw 10s of minutes
	t = _digitDests[0];
	t.translate(-_screenPosition.left, -_screenPosition.top);
	_drawSurface.blitFrom(_image, _digitSrcs[timeRemaining.getMinutes() / 10], t);

	// Draw 1s of minutes
	t = _digitDests[1];
	t.translate(-_screenPosition.left, -_screenPosition.top);
	_drawSurface.blitFrom(_image, _digitSrcs[timeRemaining.getMinutes() % 10], t);

	// Draw 10s of seconds
	t = _digitDests[2];
	t.translate(-_screenPosition.left, -_screenPosition.top);
	_drawSurface.blitFrom(_image, _digitSrcs[timeRemaining.getSeconds() / 10], t);

	// Draw 1s of seconds
	t = _digitDests[3];
	t.translate(-_screenPosition.left, -_screenPosition.top);
	_drawSurface.blitFrom(_image, _digitSrcs[timeRemaining.getSeconds() % 10], t);

	// Draw colon
	t = _colonDest;
	t.translate(-_screenPosition.left, -_screenPosition.top);
	_drawSurface.blitFrom(_image, _colonSrc, t);

	_needsRedraw = true;
}

void BombPuzzle::execute() {
	switch (_state) {
	case kBegin:
		init();
		registerGraphics();

		g_nancy->_sound->loadSound(_snipSound);
		g_nancy->_sound->loadSound(_noToolSound);

		_state = kRun;
		break;
	case kRun: {
		bool fail = false;

		for (uint i = 0; i < _playerOrder.size(); ++i) {
			if (_playerOrder[i] != _solveOrder[i]) {
				fail = true;
				break;
			}
		}

		if (fail) {
			_failed = true;
			_state = kActionTrigger;
			g_nancy->_sound->loadSound(_failSound);
			g_nancy->_sound->playSound(_failSound);

			return;
		}

		if (_playerOrder.size() == _solveOrder.size()) {
			_failed = false;
			_state = kActionTrigger;
			g_nancy->_sound->loadSound(_solveSound);
			g_nancy->_sound->playSound(_solveSound);
		}

		break;
	}
	case kActionTrigger:
		if (_failed) {
			if (g_nancy->_sound->isSoundPlaying(_failSound)) {
				return;
			}

			g_nancy->_sound->stopSound(_failSound);
			_failSceneChange.execute();
		} else {
			if (g_nancy->_sound->isSoundPlaying(_solveSound)) {
				return;
			}

			g_nancy->_sound->stopSound(_solveSound);
			_solveSceneChange.execute();
		}

		g_nancy->_sound->stopSound(_snipSound);
		g_nancy->_sound->stopSound(_noToolSound);

		finishExecution();
	}
}

void BombPuzzle::handleInput(NancyInput &input) {
	for (uint i = 0 ; i < _wireDests.size(); ++i) {
		if (NancySceneState.getViewport().convertViewportToScreen(_wireDests[i]).contains(input.mousePos)) {
			for (byte j : _playerOrder) {
				if (i == j) {
					// Wire already snipped, do nothing
					return;
				}
			}

			g_nancy->_cursorManager->setCursorType(CursorManager::kHotspot);

			if (input.input & NancyInput::kLeftMouseButtonUp) {
				if (NancySceneState.getHeldItem() == _toolID) {
					_playerOrder.push_back(i);
					g_nancy->_sound->playSound(_snipSound);
					Common::Rect dest = _wireDests[i];
					dest.translate(-_screenPosition.left, -_screenPosition.top);
					_drawSurface.blitFrom(_image, _wireSrcs[i], dest);
					_needsRedraw = true;
				} else {
					g_nancy->_sound->playSound(_noToolSound);
				}
			}

			break;
		}
	}
}

} // End of namespace Action
} // End of namespace Nancy
