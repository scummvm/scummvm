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

#include "engines/nancy/graphics.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/nancy.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/input.h"
#include "engines/nancy/util.h"

#include "engines/nancy/action/puzzle/leverpuzzle.h"

#include "engines/nancy/state/scene.h"

namespace Nancy {
namespace Action {

void LeverPuzzle::init() {
	_drawSurface.create(_screenPosition.width(), _screenPosition.height(), g_nancy->_graphics->getInputPixelFormat());
	_drawSurface.clear(g_nancy->_graphics->getTransColor());

	setTransparent(true);

	g_nancy->_resource->loadImage(_imageName, _image);
}

void LeverPuzzle::readData(Common::SeekableReadStream &stream) {
	readFilename(stream, _imageName);

	_srcRects.reserve(3);
	for (uint leverID = 0; leverID < 3; ++leverID) {
		_srcRects.push_back(Common::Array<Common::Rect>());
		_srcRects.back().reserve(3);
		for (uint i = 0; i < 4; ++i) {
			_srcRects.back().push_back(Common::Rect());
			readRect(stream, _srcRects.back().back());
		}
	}

	_destRects.reserve(3);
	for (uint leverID = 0; leverID < 3; ++leverID) {
		_destRects.push_back(Common::Rect());
		readRect(stream, _destRects.back());

		if (leverID == 0) {
			_screenPosition = _destRects.back();
		} else {
			_screenPosition.extend(_destRects.back());
		}
	}

	_playerSequence.reserve(3);
	_leverDirection.reserve(3);
	for (uint leverID = 0; leverID < 3; ++leverID) {
		_playerSequence.push_back(stream.readByte());
		_leverDirection.push_back(true);
	}

	_correctSequence.reserve(3);
	for (uint leverID = 0; leverID < 3; ++leverID) {
		_correctSequence.push_back(stream.readByte());
	}

	_moveSound.readNormal(stream);
	_noMoveSound.readNormal(stream);
	_solveExitScene.readData(stream);
	_solveSoundDelay = stream.readUint16LE();
	_solveSound.readNormal(stream);
	_exitScene.readData(stream);
	readRect(stream, _exitHotspot);
}

void LeverPuzzle::execute() {
	switch (_state) {
	case kBegin:
		init();
		registerGraphics();
		g_nancy->_sound->loadSound(_moveSound);
		g_nancy->_sound->loadSound(_noMoveSound);

		NancySceneState.setNoHeldItem();

		for (uint i = 0; i < 3; ++i) {
			drawLever(i);
		}

		_state = kRun;
		// fall through
	case kRun:
		switch (_solveState) {
		case kNotSolved:
			for (uint i = 0; i < 3; ++i) {
				if (_playerSequence[i] != _correctSequence[i]) {
					return;
				}
			}

			NancySceneState.setEventFlag(_solveExitScene._flag);
			_solveSoundPlayTime = g_nancy->getTotalPlayTime() + _solveSoundDelay * 1000;
			_solveState = kPlaySound;
			break;
		case kPlaySound:
			if (g_nancy->getTotalPlayTime() <= _solveSoundPlayTime) {
				break;
			}

			g_nancy->_sound->loadSound(_solveSound);
			g_nancy->_sound->playSound(_solveSound);
			_solveState = kWaitForSound;
			break;
		case kWaitForSound:
			if (!g_nancy->_sound->isSoundPlaying(_solveSound)) {
				g_nancy->_sound->stopSound(_solveSound);
				_state = kActionTrigger;
			}

			break;
		}

		break;
	case kActionTrigger:
		g_nancy->_sound->stopSound(_moveSound);
		g_nancy->_sound->stopSound(_noMoveSound);

		if (_solveState == kNotSolved) {
			_exitScene.execute();
		} else {
			NancySceneState.changeScene(_solveExitScene._sceneChange);
		}

		finishExecution();
	}
}

void LeverPuzzle::handleInput(NancyInput &input) {
	if (_solveState != kNotSolved) {
		return;
	}

	if (NancySceneState.getViewport().convertViewportToScreen(_exitHotspot).contains(input.mousePos)) {
		g_nancy->_cursor->setCursorType(g_nancy->_cursor->_puzzleExitCursor);

		if (input.input & NancyInput::kLeftMouseButtonUp) {
			_state = kActionTrigger;
		}
		return;
	}

	for (uint i = 0; i < 3; ++i) {
		if (NancySceneState.getViewport().convertViewportToScreen(_destRects[i]).contains(input.mousePos)) {
			g_nancy->_cursor->setCursorType(CursorManager::kHotspot);

			if (input.input & NancyInput::kLeftMouseButtonUp) {
				bool isMoving = false;
				// Hardcoded by the original engine
				switch (i) {
				case 0:
					isMoving = true;
					break;
				case 1:
					if (_playerSequence[0] == 1) {
						isMoving = true;
					}

					break;
				case 2:
					if (_playerSequence[0] == 2) {
						isMoving = true;
					}

					break;
				}

				if (isMoving) {
					g_nancy->_sound->playSound(_moveSound);

					if (_leverDirection[i]) {
						// Moving down
						if (_playerSequence[i] == 3) {
							--_playerSequence[i];
							_leverDirection[i] = false;
						} else {
							++_playerSequence[i];
						}
					} else {
						// Moving up
						if (_playerSequence[i] == 0) {
							++_playerSequence[i];
							_leverDirection[i] = true;
						} else {
							--_playerSequence[i];
						}
					}

					drawLever(i);
				} else {
					g_nancy->_sound->playSound(_noMoveSound);
					return;
				}
			}
		}
	}
}

void LeverPuzzle::drawLever(uint id) {
	Common::Point destPoint(_destRects[id].left - _screenPosition.left, _destRects[id].top - _screenPosition.top);
	_drawSurface.blitFrom(_image, _srcRects[id][_playerSequence[id]], destPoint);

	_needsRedraw = true;
}

} // End of namespace Action
} // End of namespace Nancy
