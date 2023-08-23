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

#include "common/serializer.h"

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
	g_nancy->_resource->loadImage(_imageName, _image);
	_drawSurface.create(_screenPosition.width(), _screenPosition.height(), g_nancy->_graphicsManager->getInputPixelFormat());

	if (_image.hasPalette()) {
		uint8 palette[256 * 3];
		_image.grabPalette(palette, 0, 256);
		_drawSurface.setPalette(palette, 0, 256);
	}

	setTransparent(true);
	setVisible(false);
	clearAllElements();

	RenderObject::init();
}

void OrderingPuzzle::readData(Common::SeekableReadStream &stream) {
	bool isPiano = _puzzleType == kPiano;
	readFilename(stream, _imageName);
	Common::Serializer ser(&stream, nullptr);
	ser.setVersion(g_nancy->getGameType());

	uint16 numElements;
	if (ser.getVersion() == kGameTypeVampire) {
		// Hardcoded in The Vampire Diaries
		numElements = 5;
	} else {
		ser.syncAsUint16LE(numElements);
	}

	readRectArray(stream, _srcRects, numElements);

	ser.skip(16 * (15 - numElements), kGameTypeNancy1);

	readRectArray(stream, _destRects, numElements);

	ser.skip(16 * (15 - numElements), kGameTypeNancy1);

	_hotspots.resize(numElements);
	if (isPiano) {
		readRectArray(stream, _hotspots, numElements);
		ser.skip(16 * (15 - numElements));
	} else {
		_hotspots = _destRects;
	}

	_drawnElements.resize(numElements, false);
	for (uint i = 0; i < numElements; ++i) {
		if (i == 0) {
			_screenPosition = _destRects[i];
		} else {
			_screenPosition.extend(_destRects[i]);
		}
	}

	if (ser.getVersion() == kGameTypeVampire) {
		_sequenceLength = 5;
	} else {
		ser.syncAsUint16LE(_sequenceLength);
	}

	_correctSequence.resize(_sequenceLength);
	for (uint i = 0; i < _sequenceLength; ++i) {
		if (isPiano) {
			ser.syncAsUint16LE(_correctSequence[i]);
		} else {
			ser.syncAsByte(_correctSequence[i]);
		}
	}

	ser.skip((15 - _sequenceLength) * (isPiano ? 2 : 1), kGameTypeNancy1);

	if (ser.getVersion() != kGameTypeVampire) {
		_clickSound.readNormal(stream);
	}

	_solveExitScene.readData(stream, ser.getVersion() == kGameTypeVampire);
	ser.syncAsUint16LE(_solveSoundDelay);
	_solveSound.readNormal(stream);
	_exitScene.readData(stream, ser.getVersion() == kGameTypeVampire);
	readRect(stream, _exitHotspot);
}

void OrderingPuzzle::execute() {
	switch (_state) {
	case kBegin:
		init();
		registerGraphics();
		if (g_nancy->getGameType () != kGameTypeVampire) {
			g_nancy->_sound->loadSound(_clickSound);
		}
		g_nancy->_sound->loadSound(_solveSound);
		_state = kRun;
		// fall through
	case kRun:
		switch (_solveState) {
		case kNotSolved:
			if (_puzzleType == kOrdering) {
				if (_clickedSequence.size() <  _sequenceLength) {
					return;
				}

				for (uint i = 0; i < _sequenceLength; ++i) {
					if (_clickedSequence[i] != (int16)_correctSequence[i]) {
						if (_clickedSequence.size() > (g_nancy->getGameType() == kGameTypeVampire ? 4 : (uint)_sequenceLength + 1)) {
							clearAllElements();
						}

						return;
					}
				}
			} else {
				if (g_nancy->_sound->isSoundPlaying(_clickSound)) {
					return;
				}

				for (uint i = 0; i < _drawnElements.size(); ++i) {
					if (_drawnElements[i]) {
						undrawElement(i);
					}
				}
				
				if (_clickedSequence.size() <  _sequenceLength) {
					return;
				}

				// Arbitrary number
				if (_clickedSequence.size() > 30) {
					_clickedSequence.erase(&_clickedSequence[0], &_clickedSequence[_clickedSequence.size() - 6]);
				}

				for (uint i = 0; i < _sequenceLength; ++i) {
					if (_clickedSequence[_clickedSequence.size() - _sequenceLength + i] != (int16)_correctSequence[i]) {
						return;
					}
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
		if (g_nancy->getGameType() == kGameTypeVampire) {
			g_nancy->_sound->stopSound("BUOK");
		} else {
			g_nancy->_sound->stopSound(_clickSound);
		}
		g_nancy->_sound->stopSound(_solveSound);

		if (_solveState == kNotSolved) {
			_exitScene.execute();
		} else {
			NancySceneState.changeScene(_solveExitScene._sceneChange);
		}

		finishExecution();
		break;
	}
}

void OrderingPuzzle::handleInput(NancyInput &input) {
	if (_solveState != kNotSolved) {
		return;
	}

	bool canClick = true;
	if (_puzzleType == kPiano && g_nancy->_sound->isSoundPlaying(_clickSound)) {
		canClick = false;
	}

	if (NancySceneState.getViewport().convertViewportToScreen(_exitHotspot).contains(input.mousePos)) {
		g_nancy->_cursorManager->setCursorType(CursorManager::kExit);

		if (canClick && input.input & NancyInput::kLeftMouseButtonUp) {
			_state = kActionTrigger;
		}
		return;
	}

	for (int i = 0; i < (int)_hotspots.size(); ++i) {
		if (NancySceneState.getViewport().convertViewportToScreen(_hotspots[i]).contains(input.mousePos)) {
			g_nancy->_cursorManager->setCursorType(CursorManager::kHotspot);

			if (canClick && input.input & NancyInput::kLeftMouseButtonUp) {
				if (g_nancy->getGameType() == kGameTypeVampire) {
					g_nancy->_sound->playSound("BUOK");
				} else {
					if (_puzzleType == kPiano) {
						if (Common::isDigit(_clickSound.name.lastChar())) {
							_clickSound.name.deleteLastChar();
						}

						_clickSound.name.insertChar('0' + i, _clickSound.name.size());
						g_nancy->_sound->loadSound(_clickSound);
					}

					g_nancy->_sound->playSound(_clickSound);
				}

				if (_puzzleType == kOrdering) {
					for (uint j = 0; j < _clickedSequence.size(); ++j) {
						if (_clickedSequence[j] == i && _drawnElements[i] == true) {
							undrawElement(i);
							if (_clickedSequence.back() == i) {
								_clickedSequence.pop_back();
							}

							return;
						}
					}
				}

				_clickedSequence.push_back(i);
				drawElement(i);
			}

			return;
		}
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
