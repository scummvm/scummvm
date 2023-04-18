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
#include "engines/nancy/resource.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/input.h"
#include "engines/nancy/util.h"

#include "engines/nancy/action/rotatinglockpuzzle.h"

#include "engines/nancy/state/scene.h"

namespace Nancy {
namespace Action {

void RotatingLockPuzzle::init() {
	_drawSurface.create(_screenPosition.width(), _screenPosition.height(), g_nancy->_graphicsManager->getInputPixelFormat());
	_drawSurface.clear(g_nancy->_graphicsManager->getTransColor());

	setTransparent(true);

	g_nancy->_resource->loadImage(_imageName, _image);
}

void RotatingLockPuzzle::readData(Common::SeekableReadStream &stream) {
	readFilename(stream, _imageName);

	uint numDials = stream.readUint16LE();

	_srcRects.reserve(10);
	for (uint i = 0; i < 10; ++i) {
		_srcRects.push_back(Common::Rect());
		readRect(stream, _srcRects.back());
	}

	_destRects.reserve(numDials);
	for (uint i = 0; i < numDials; ++i) {
		_destRects.push_back(Common::Rect());
		readRect(stream, _destRects.back());

		if (i == 0) {
			_screenPosition = _destRects.back();
		} else {
			_screenPosition.extend(_destRects.back());
		}
	}

	stream.skip((8 - numDials) * 16);

	_upHotspots.reserve(numDials);
	for (uint i = 0; i < numDials; ++i) {
		_upHotspots.push_back(Common::Rect());
		readRect(stream, _upHotspots.back());
	}

	_downHotspots.reserve(numDials);
	stream.skip((8 - numDials) * 16);

	for (uint i = 0; i < numDials; ++i) {
		_downHotspots.push_back(Common::Rect());
		readRect(stream, _downHotspots.back());
	}

	stream.skip((8 - numDials) * 16);

	_correctSequence.reserve(numDials);
	for (uint i = 0; i < numDials; ++i) {
		_correctSequence.push_back(stream.readByte());
	}

	stream.skip(8 - numDials);

	_clickSound.read(stream, SoundDescription::kNormal);
	_solveExitScene.readData(stream);
	_solveSoundDelay = stream.readUint16LE();
	_solveSound.read(stream, SoundDescription::kNormal);

	_exitScene.readData(stream);
	readRect(stream, _exitHotspot);
}

void RotatingLockPuzzle::execute() {
	switch (_state) {
	case kBegin:
		init();
		registerGraphics();

		for (uint i = 0; i < _correctSequence.size(); ++i) {
			_currentSequence.push_back(g_nancy->_randomSource->getRandomNumber(9));
			drawDial(i);
		}

		g_nancy->_sound->loadSound(_clickSound);
		g_nancy->_sound->loadSound(_solveSound);
		_state = kRun;
		// fall through
	case kRun:
		switch (_solveState) {
		case kNotSolved:
			for (uint i = 0; i < _correctSequence.size(); ++i) {
				if (_currentSequence[i] != (int16)_correctSequence[i]) {
					return;
				}
			}

			NancySceneState.setEventFlag(_solveExitScene._flag);
			_solveSoundPlayTime = g_nancy->getTotalPlayTime() + _solveSoundDelay * 1000;
			_solveState = kPlaySound;
			// fall through
		case kPlaySound:
			if (g_nancy->getTotalPlayTime() <= _solveSoundPlayTime) {
				break;
			}

			g_nancy->_sound->playSound(_solveSound);
			_solveState = kWaitForSound;
			break;
		case kWaitForSound:
			if (!g_nancy->_sound->isSoundPlaying(_solveSound)) {
				_state = kActionTrigger;
			}

			break;
		}
		break;
	case kActionTrigger:
		g_nancy->_sound->stopSound(_clickSound);
		g_nancy->_sound->stopSound(_solveSound);

		if (_solveState == kNotSolved) {
			_exitScene.execute();
		} else {
			NancySceneState.changeScene(_solveExitScene._sceneChange);
		}

		finishExecution();
	}
}

void RotatingLockPuzzle::handleInput(NancyInput &input) {
	if (_solveState != kNotSolved) {
		return;
	}

	if (NancySceneState.getViewport().convertViewportToScreen(_exitHotspot).contains(input.mousePos)) {
		g_nancy->_cursorManager->setCursorType(CursorManager::kExit);

		if (input.input & NancyInput::kLeftMouseButtonUp) {
			_state = kActionTrigger;
		}

		return;
	}

	for (uint i = 0; i < _upHotspots.size(); ++i) {
		if (NancySceneState.getViewport().convertViewportToScreen(_upHotspots[i]).contains(input.mousePos)) {
			g_nancy->_cursorManager->setCursorType(CursorManager::kHotspot);

			if (input.input & NancyInput::kLeftMouseButtonUp) {
				g_nancy->_sound->playSound(_clickSound);

				_currentSequence[i] = ++_currentSequence[i] > 9 ? 0 : _currentSequence[i];
				drawDial(i);
			}

			return;
		}
	}

	for (uint i = 0; i < _downHotspots.size(); ++i) {
		if (NancySceneState.getViewport().convertViewportToScreen(_downHotspots[i]).contains(input.mousePos)) {
			g_nancy->_cursorManager->setCursorType(CursorManager::kHotspot);

			if (input.input & NancyInput::kLeftMouseButtonUp) {
				g_nancy->_sound->playSound(_clickSound);

				int8 n = _currentSequence[i];
				n = --n < 0 ? 9 : n;
				_currentSequence[i] = n;
				drawDial(i);
			}

			return;
		}
	}
}

void RotatingLockPuzzle::onPause(bool pause) {
	if (!pause) {
		registerGraphics();
	}
}

void RotatingLockPuzzle::drawDial(uint id) {
	Common::Point destPoint(_destRects[id].left - _screenPosition.left, _destRects[id].top - _screenPosition.top);
	_drawSurface.blitFrom(_image, _srcRects[_currentSequence[id]], destPoint);

	_needsRedraw = true;
}

} // End of namespace Action
} // End of namespace Nancy
