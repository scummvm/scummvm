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

#include "engines/nancy/util.h"
#include "engines/nancy/nancy.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/input.h"
#include "engines/nancy/puzzledata.h"
#include "engines/nancy/state/scene.h"

#include "engines/nancy/action/puzzle/twodialpuzzle.h"

namespace Nancy {
namespace Action {

void TwoDialPuzzle::init() {
	Common::Rect screenBounds = NancySceneState.getViewport().getBounds();
	_drawSurface.create(screenBounds.width(), screenBounds.height(), g_nancy->_graphics->getInputPixelFormat());
	_drawSurface.clear(g_nancy->_graphics->getTransColor());
	setTransparent(true);
	setVisible(true);
	moveTo(screenBounds);

	g_nancy->_resource->loadImage(_imageName, _image);
	registerGraphics();
}

void TwoDialPuzzle::readData(Common::SeekableReadStream &stream) {
	readFilename(stream, _imageName);

	uint16 num1 = stream.readUint16LE();
	uint16 num2 = stream.readUint16LE();

	_isClockwise[0] = stream.readByte();
	_isClockwise[1] = stream.readByte();

	_startPositions[0] = stream.readUint16LE();
	_startPositions[1] = stream.readUint16LE();

	readRect(stream, _hotspots[0]);
	readRect(stream, _hotspots[1]);
	readRect(stream, _dests[0]);
	readRect(stream, _dests[1]);
	readRectArray(stream, _srcs[0], num1, 20);
	readRectArray(stream, _srcs[1], num2, 20);

	_correctPositions[0] = stream.readUint16LE();
	_correctPositions[1] = stream.readUint16LE();

	_rotateSounds[0].readNormal(stream);
	_rotateSounds[1].readNormal(stream);

	_solveScene.readData(stream);
	_solveSoundDelay = stream.readUint16LE();
	_solveSound.readNormal(stream);

	_exitScene.readData(stream);
	readRect(stream, _exitHotspot);
}

void TwoDialPuzzle::execute() {
	switch(_state) {
	case kBegin:
		init();
		g_nancy->_sound->loadSound(_rotateSounds[0]);
		g_nancy->_sound->loadSound(_rotateSounds[1]);
		_currentPositions[0] = _startPositions[0];
		_currentPositions[1] = _startPositions[1];

		_drawSurface.blitFrom(_image, _srcs[0][_currentPositions[0]], _dests[0]);
		_drawSurface.blitFrom(_image, _srcs[1][_currentPositions[1]], _dests[1]);
		_needsRedraw = true;

		NancySceneState.setNoHeldItem();

		_state = kRun;
		// fall through
	case kRun:
		if (g_nancy->_sound->isSoundPlaying(_rotateSounds[0]) || g_nancy->_sound->isSoundPlaying(_rotateSounds[1])) {
			return;
		}

		if ((uint)_currentPositions[0] == _correctPositions[0] && (uint)_currentPositions[1] == _correctPositions[1]) {
			_state = kActionTrigger;
			_isSolved = true;
			_solveSoundDelayTime = g_nancy->getTotalPlayTime() + (_solveSoundDelay * 1000);
		}

		break;
	case kActionTrigger:
		if (_isSolved) {
			if (_solveSoundDelayTime != 0) {
				if (g_nancy->getTotalPlayTime() < _solveSoundDelayTime) {
					return;
				}

				_solveSoundDelayTime = 0;
				g_nancy->_sound->loadSound(_solveSound);
				g_nancy->_sound->playSound(_solveSound);
				NancySceneState.setEventFlag(_solveScene._flag);
				return;
			} else {
				if (g_nancy->_sound->isSoundPlaying(_solveSound)) {
					return;
				}

				g_nancy->_sound->stopSound(_solveSound);
				NancySceneState.changeScene(_solveScene._sceneChange);
			}
		} else {
			_exitScene.execute();
		}

		g_nancy->_sound->stopSound(_rotateSounds[0]);
		g_nancy->_sound->stopSound(_rotateSounds[1]);
		finishExecution();
	}
}

void TwoDialPuzzle::handleInput(NancyInput &input) {
	bool canClick = (_state == kRun) && !g_nancy->_sound->isSoundPlaying(_rotateSounds[0]) && !g_nancy->_sound->isSoundPlaying(_rotateSounds[1]);

	if (NancySceneState.getViewport().convertViewportToScreen(_exitHotspot).contains(input.mousePos)) {
		g_nancy->_cursor->setCursorType(g_nancy->_cursor->_puzzleExitCursor);

		if (canClick && input.input & NancyInput::kLeftMouseButtonUp) {
			_state = kActionTrigger;
		}

		return;
	}

	for (uint i = 0; i <= 1; ++i) {
		if (NancySceneState.getViewport().convertViewportToScreen(_hotspots[i]).contains(input.mousePos)) {
			g_nancy->_cursor->setCursorType(_isClockwise[i] ? CursorManager::kRotateCW : CursorManager::kRotateCCW);

			if (canClick && input.input & NancyInput::kLeftMouseButtonUp) {
				_currentPositions[i] += _isClockwise[i] ? -1 : 1;

				if (_currentPositions[i] < 0) {
					_currentPositions[i] = _srcs[i].size() - 1;
				} else if ((uint)_currentPositions[i] >= _srcs[i].size()) {
					_currentPositions[i] = 0;
				}

				g_nancy->_sound->playSound(_rotateSounds[i]);
				_drawSurface.fillRect(_dests[0].findIntersectingRect(_dests[1]), _drawSurface.getTransparentColor());

				// Blit both dials just in case
				_drawSurface.blitFrom(_image, _srcs[0][_currentPositions[0]], _dests[0]);
				_drawSurface.blitFrom(_image, _srcs[1][_currentPositions[1]], _dests[1]);
				_needsRedraw = true;
			}

			return;
		}
	}
}

} // End of namespace Action
} // End of namespace Nancy
