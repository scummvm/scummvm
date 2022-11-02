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
#include "engines/nancy/input.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/util.h"

#include "engines/nancy/action/orderingpuzzle.h"

#include "engines/nancy/state/scene.h"

namespace Nancy {
namespace Action {

void OrderingPuzzle::init() {
	// Screen position is initialized in readData and fits exactly the bounds of all elements on screen.
	// This is a hacky way to make this particular action record work with this implementation's graphics manager
	_drawSurface.create(_screenPosition.width(), _screenPosition.height(), g_nancy->_graphicsManager->getInputPixelFormat());
	clearAllElements();

	setTransparent(true);

	g_nancy->_resource->loadImage(_imageName, _image);

	setVisible(false);

	RenderObject::init();
}

void OrderingPuzzle::readData(Common::SeekableReadStream &stream) {
	readFilename(stream, _imageName);
	uint16 numElements = stream.readUint16LE();

	_srcRects.reserve(numElements);
	for (uint i = 0; i < numElements; ++i) {
		_srcRects.push_back(Common::Rect());
		readRect(stream, _srcRects.back());
	}

	stream.skip(16 * (15 - numElements));

	_destRects.reserve(numElements);
	_drawnElements.reserve(numElements);
	for (uint i = 0; i < numElements; ++i) {
		_destRects.push_back(Common::Rect());
		readRect(stream, _destRects.back());

		if (i == 0) {
			_screenPosition = _destRects[i];
		} else {
			_screenPosition.extend(_destRects[i]);
		}

		_drawnElements.push_back(false);
	}

	stream.skip(16 * (15 - numElements));

	_sequenceLength = stream.readUint16LE();

	_correctSequence.reserve(15);
	for (uint i = 0; i < 15; ++i) {
		_correctSequence.push_back(stream.readByte());
	}

	_clickSound.read(stream, SoundDescription::kNormal);
	_solveExitScene.readData(stream);
	stream.skip(2); // shouldStopRendering, useless
	_flagOnSolve.label = stream.readSint16LE();
	_flagOnSolve.flag = (NancyFlag)stream.readByte();
	_solveSoundDelay = stream.readUint16LE();
	_solveSound.read(stream, SoundDescription::kNormal);
	_exitScene.readData(stream);
	stream.skip(2); // shouldStopRendering, useless
	_flagOnExit.label = stream.readSint16LE();
	_flagOnExit.flag = (NancyFlag)stream.readByte();
	readRect(stream, _exitHotspot);
}

void OrderingPuzzle::execute() {
	switch (_state) {
	case kBegin:
		init();
		registerGraphics();
		g_nancy->_sound->loadSound(_clickSound);
		g_nancy->_sound->loadSound(_solveSound);
		_state = kRun;
		// fall through
	case kRun:
		switch (_solveState) {
		case kNotSolved:
			if (_clickedSequence.size() != _sequenceLength) {
				return;
			}

			for (uint i = 0; i < _sequenceLength; ++i) {
				if (_clickedSequence[i] != (int16)_correctSequence[i]) {
					return;
				}
			}

			NancySceneState.setEventFlag(_flagOnSolve);
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
			NancySceneState.changeScene(_exitScene);
			NancySceneState.setEventFlag(_flagOnExit);
		} else {
			NancySceneState.changeScene(_solveExitScene);
		}

		finishExecution();
		break;
	}
}

void OrderingPuzzle::handleInput(NancyInput &input) {
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

	for (int i = 0; i < (int)_destRects.size(); ++i) {
		if (NancySceneState.getViewport().convertViewportToScreen(_destRects[i]).contains(input.mousePos)) {
			g_nancy->_cursorManager->setCursorType(CursorManager::kHotspot);

			if (input.input & NancyInput::kLeftMouseButtonUp) {
				g_nancy->_sound->playSound(_clickSound);

				for (uint j = 0; j < _clickedSequence.size(); ++j) {
					if (_clickedSequence[j] == i && _drawnElements[i] == true) {
						undrawElement(i);
						if (_clickedSequence.back() == i) {
							_clickedSequence.pop_back();
						}

						return;
					}
				}

				_clickedSequence.push_back(i);

				if (_clickedSequence.size() > (uint)_sequenceLength + 1) {
					clearAllElements();
				} else {
					drawElement(i);
				}
			}
			return;
		}
	}
}

void OrderingPuzzle::onPause(bool pause) {
	if (!pause) {
		registerGraphics();
	}
}

void OrderingPuzzle::drawElement(uint id) {
	_drawnElements[id] = true;
	Common::Point destPoint(_destRects[id].left - _screenPosition.left, _destRects[id].top - _screenPosition.top);
	_drawSurface.blitFrom(_image, _srcRects[id], destPoint);
	setVisible(true);
}

void OrderingPuzzle::undrawElement(uint id) {
	_drawnElements[id] = false;
	Common::Rect bounds = _destRects[id];
	bounds.translate(-_screenPosition.left, -_screenPosition.top);

	_drawSurface.fillRect(bounds, g_nancy->_graphicsManager->getTransColor());
	_needsRedraw = true;
}

void OrderingPuzzle::clearAllElements() {
	_drawSurface.clear(g_nancy->_graphicsManager->getTransColor());
	setVisible(false);
	_clickedSequence.clear();
	return;
}

} // End of namespace Action
} // End of namespace Nancy
