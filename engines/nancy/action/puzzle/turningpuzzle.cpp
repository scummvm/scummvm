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

#include "engines/nancy/action/puzzle/turningpuzzle.h"

namespace Nancy {
namespace Action {

void TurningPuzzle::init() {
	Common::Rect screenBounds = NancySceneState.getViewport().getBounds();
	_drawSurface.create(screenBounds.width(), screenBounds.height(), g_nancy->_graphics->getInputPixelFormat());
	_drawSurface.clear(g_nancy->_graphics->getTransColor());
	setTransparent(true);
	setVisible(true);
	moveTo(screenBounds);

	g_nancy->_resource->loadImage(_imageName, _image);
	registerGraphics();
}

void TurningPuzzle::updateGraphics() {
	if (_state == kBegin) {
		return;
	}

	if (_solveState == kWaitForAnimation) {
		if (g_nancy->getTotalPlayTime() > _nextTurnTime) {
			_nextTurnTime = g_nancy->getTotalPlayTime() + (_solveDelayBetweenTurns * 1000 / _currentOrder.size());

			if (	(_turnFrameID == 0 && _solveAnimFace == 0) ||
					(_turnFrameID == 1 && _solveAnimFace > 0 && (int)_solveAnimFace < _numFaces - 1)) {
				g_nancy->_sound->playSound(_turnSound);
			}

			if (_turnFrameID >= _numFramesPerTurn) {
				++_solveAnimFace;
				_turnFrameID = 0;
				_nextTurnTime += 1000 * _solveDelayBetweenTurns;
			}

			for (uint i = 0; i < _currentOrder.size(); ++i) {
				uint faceID = _currentOrder[i] + _solveAnimFace;
				if (faceID >= _numFaces) {
					faceID -= _numFaces;
				}

				drawObject(i, faceID, _turnFrameID);
			}

			if ((int)_solveAnimFace >= _numFaces - 1) {
				_solveAnimFace = 0;
				++_solveAnimLoop;

				if (_solveAnimLoop >= _solveAnimationNumRepeats) {
					_solveState = kWaitBeforeSound;
					_objectCurrentlyTurning = -1;
				}
			}

			++_turnFrameID;
		}

		return;
	}

	if (_objectCurrentlyTurning != -1) {
		if (g_nancy->getTotalPlayTime() > _nextTurnTime) {
			_nextTurnTime = g_nancy->getTotalPlayTime() + (_solveDelayBetweenTurns * 1000 / _currentOrder.size());
			++_turnFrameID;

			uint faceID = _currentOrder[_objectCurrentlyTurning];
			uint frameID = _turnFrameID;

			if (frameID == _numFramesPerTurn && (int)faceID == _numFaces - 1) {
				faceID = frameID = 0;
			}

			// Draw clicked spindle
			drawObject(_objectCurrentlyTurning, faceID, frameID);

			// Draw linked spindles
			for (uint i = 0; i < _links[_objectCurrentlyTurning].size(); ++i) {
				faceID = _currentOrder[_links[_objectCurrentlyTurning][i] - 1];
				frameID = _turnFrameID;

				if (frameID == _numFramesPerTurn && (int)faceID == _numFaces - 1) {
					faceID = frameID = 0;
				}

				drawObject(_links[_objectCurrentlyTurning][i] - 1, faceID, frameID);
			}

			if (_turnFrameID >= _numFramesPerTurn) {
				turnLogic(_objectCurrentlyTurning);
				_objectCurrentlyTurning = -1;
				_turnFrameID = 0;
				_nextTurnTime = 0;
			}
		}
	}
}

void TurningPuzzle::readData(Common::SeekableReadStream &stream) {
	readFilename(stream, _imageName);
	uint numSpindles = stream.readUint16LE();
	_numFaces = stream.readUint16LE();
	_numFramesPerTurn = stream.readUint16LE();

	_startPositions.resize(numSpindles);
	for (uint i = 0; i < numSpindles; ++i) {
		_startPositions[i] = stream.readUint16LE();
	}
	stream.skip((16 - numSpindles) * 2);

	readRectArray(stream, _destRects, numSpindles, 16);
	readRectArray(stream, _hotspots, numSpindles, 16);

	_separateRows = stream.readByte();

	_startPos.x = stream.readSint32LE();
	_startPos.y = stream.readSint32LE();
	_srcIncrement.x = stream.readSint16LE();
	_srcIncrement.y = stream.readSint16LE();

	_links.resize(numSpindles);
	for (uint i = 0; i < numSpindles; ++i) {
		for (uint j = 0; j < 4; ++j) {
			uint16 val = stream.readUint16LE();
			if (val == 0) {
				break;
			}

			_links[i].push_back(val);
		}

		if (_links[i].size() < 4) {
			stream.skip((4 - _links[i].size() - 1) * 2);
		}
	}

	stream.skip((16 - numSpindles) * 4 * 2);

	_solveDelayBetweenTurns = stream.readUint16LE();
	_solveAnimate = stream.readByte();
	_solveAnimationNumRepeats = stream.readUint16LE();

	_turnSound.readNormal(stream);

	_correctOrder.resize(numSpindles);
	for (uint i = 0; i < numSpindles; ++i) {
		_correctOrder[i] = stream.readUint16LE();
	}
	stream.skip((16 - numSpindles) * 2);

	_solveScene.readData(stream);
	_solveSoundDelay = stream.readUint16LE();
	_solveSound.readNormal(stream);

	_exitScene.readData(stream);
	readRect(stream, _exitHotspot);
}

void TurningPuzzle::execute() {
	switch (_state) {
	case kBegin :
		init();
		g_nancy->_sound->loadSound(_turnSound);
		_currentOrder = _startPositions;
		for (uint i = 0; i < _currentOrder.size(); ++i) {
			drawObject(i, _currentOrder[i], 0);
		}

		NancySceneState.setNoHeldItem();

		_state = kRun;
		// fall through
	case kRun :
		if (_objectCurrentlyTurning != -1) {
			return;
		}

		if (_currentOrder == _correctOrder) {
			_state = kActionTrigger;
			if (_solveAnimate) {
				_solveState = kWaitForAnimation;
			} else {
				_solveState = kWaitForSound;
				NancySceneState.setEventFlag(_solveScene._flag);
			}
			_objectCurrentlyTurning = -1;
			_turnFrameID = 0;
			_nextTurnTime = g_nancy->getTotalPlayTime() + (_solveDelayBetweenTurns * 1000 / _currentOrder.size());
		}

		break;
	case kActionTrigger :
		switch (_solveState) {
		case kWaitForAnimation :
			if (_nextTurnTime == 0) {
				_solveState = kWaitForSound;
			}
			return;
		case kWaitBeforeSound :
			if (_solveSoundDelayTime == 0) {
				_solveSoundDelayTime = g_nancy->getTotalPlayTime() + (_solveSoundDelay * 1000);
			} else if (g_nancy->getTotalPlayTime() > _solveSoundDelayTime) {
				g_nancy->_sound->loadSound(_solveSound);
				g_nancy->_sound->playSound(_solveSound);
				NancySceneState.setEventFlag(_solveScene._flag);
				_solveState = kWaitForSound;
			}

			return;
		case kWaitForSound :
			if (g_nancy->_sound->isSoundPlaying(_solveSound) || g_nancy->_sound->isSoundPlaying(_turnSound)) {
				return;
			}

			NancySceneState.changeScene(_solveScene._sceneChange);
			break;
		case kNotSolved :
			_exitScene.execute();
		}

		g_nancy->_sound->stopSound(_turnSound);
		g_nancy->_sound->stopSound(_solveSound);
		finishExecution();
	}
}

void TurningPuzzle::handleInput(NancyInput &input) {
	if (NancySceneState.getViewport().convertViewportToScreen(_exitHotspot).contains(input.mousePos)) {
		g_nancy->_cursor->setCursorType(g_nancy->_cursor->_puzzleExitCursor);

		if (input.input & NancyInput::kLeftMouseButtonUp) {
			_state = kActionTrigger;
		}

		return;
	}

	for (uint i = 0; i < _hotspots.size(); ++i) {
		if (NancySceneState.getViewport().convertViewportToScreen(_hotspots[i]).contains(input.mousePos)) {
			g_nancy->_cursor->setCursorType(CursorManager::kHotspot);

			if (_objectCurrentlyTurning != -1) {
				break;
			}

			if (input.input & NancyInput::kLeftMouseButtonUp) {
				g_nancy->_sound->playSound(_turnSound);
				_objectCurrentlyTurning = i;
			}

			// fixes nancy4 scene 4308
			input.eatMouseInput();

			return;
		}
	}
}

void TurningPuzzle::drawObject(uint objectID, uint faceID, uint frameID) {
	Common::Rect srcRect = _destRects[objectID];
	srcRect.moveTo(_startPos);
	Common::Point inc(_srcIncrement.x == 1 ? srcRect.width() : _srcIncrement.x, _srcIncrement.y == -2 ? srcRect.height() : _srcIncrement.y);
	srcRect.translate(	inc.x * frameID + inc.x * _numFramesPerTurn * faceID,
						_separateRows ? inc.y * objectID : 0);

	_drawSurface.blitFrom(_image, srcRect, _destRects[objectID]);
	_needsRedraw = true;
}

void TurningPuzzle::turnLogic(uint objectID) {
	++_currentOrder[objectID];
	if (_currentOrder[objectID] >= _numFaces) {
		_currentOrder[objectID] = 0;
	}

	for (uint j = 0; j < _links[objectID].size(); ++j) {
		++_currentOrder[_links[objectID][j] - 1];
		if (_currentOrder[_links[objectID][j] - 1] >= _numFaces) {
			_currentOrder[_links[objectID][j] - 1] = 0;
		}
	}
}

} // End of namespace Action
} // End of namespace Nancy
