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

#include "engines/nancy/action/puzzle/cubepuzzle.h"

namespace Nancy {
namespace Action {

void CubePuzzle::init() {
	Common::Rect screenBounds = NancySceneState.getViewport().getBounds();
	_drawSurface.create(screenBounds.width(), screenBounds.height(), g_nancy->_graphics->getInputPixelFormat());
	_drawSurface.clear(g_nancy->_graphics->getTransColor());
	setTransparent(true);
	setVisible(true);
	moveTo(screenBounds);

	g_nancy->_resource->loadImage(_imageName, _image);
	_image.setTransparentColor(_drawSurface.getTransparentColor());

	for (uint i = 0; i < 5; ++i) {
		_drawSurface.blitFrom(_image, _pieceSrcs[i], _pieceDests[i]);
	}

	_placedPieces.resize(5, false);
	_curRotation = _startRotation;
	_drawSurface.blitFrom(_image, _placedSrcs[_curRotation][0], _placedDest);
}

void CubePuzzle::registerGraphics() {
	_curPiece.registerGraphics();
	RenderActionRecord::registerGraphics();
}

void CubePuzzle::readData(Common::SeekableReadStream &stream) {
	readFilename(stream, _imageName);

	readRect(stream, _cwCursorDest);
	readRect(stream, _ccwCursorDest);

	readRect(stream, _placedDest);

	// four pieces on the side, 1 on top
	_pieceSrcs.resize(5);
	_pieceDests.resize(5);
	for (uint i = 0; i < 5; ++i) {
		readRect(stream, _pieceSrcs[i]);
		readRect(stream, _pieceDests[i]);
	}

	_placedSrcs.resize(4);
	for (uint i = 0; i < 4; ++i) {
		readRectArray(stream, _placedSrcs[i], 9);
	}

	_startRotation = stream.readUint16LE();

	_rotateSound.readNormal(stream);
	_pickUpSound.readNormal(stream);
	_placeDownSound.readNormal(stream);

	_solveSceneIDs.resize(4);
	for (uint i = 0; i < 3; ++i) {
		_solveSceneIDs[i] = stream.readUint16LE();
	}
	_solveScene.readData(stream);
	_solveSceneIDs[3] = _solveScene._sceneChange.sceneID;
	_solveSound.readNormal(stream);

	_exitScene.readData(stream);
	readRect(stream, _exitHotspot);
}

void CubePuzzle::execute() {
	switch (_state) {
	case kBegin:
		init();
		registerGraphics();

		g_nancy->_sound->loadSound(_rotateSound);
		g_nancy->_sound->loadSound(_pickUpSound);
		g_nancy->_sound->loadSound(_placeDownSound);

		_state = kRun;
		// fall through
	case kRun:
		for (uint i = 0; i < 5; ++i) {
			if (!_placedPieces[i]) {
				return;
			}
		}

		g_nancy->_sound->loadSound(_solveSound);
		g_nancy->_sound->playSound(_solveSound);
		NancySceneState.setEventFlag(_solveScene._flag);
		_completed = true;

		_state = kActionTrigger;
		break;
	case kActionTrigger:
		if (g_nancy->_sound->isSoundPlaying(_solveSound)) {
			return;
		}

		if (_completed) {
			_solveScene._sceneChange.sceneID = _solveSceneIDs[_curRotation];
			NancySceneState.changeScene(_solveScene._sceneChange);
		} else {
			_exitScene.execute();
		}

		g_nancy->_sound->stopSound(_solveSound);
		g_nancy->_sound->stopSound(_rotateSound);
		g_nancy->_sound->stopSound(_pickUpSound);
		g_nancy->_sound->stopSound(_placeDownSound);

		break;
	}
}

void CubePuzzle::handleInput(NancyInput &input) {
	if (_state != kRun) {
		return;
	}

	if (_pickedUpPiece == -1 && NancySceneState.getViewport().convertViewportToScreen(_exitHotspot).contains(input.mousePos)) {
		g_nancy->_cursor->setCursorType(g_nancy->_cursor->_puzzleExitCursor);

		if (input.input & NancyInput::kLeftMouseButtonUp) {
			_state = kActionTrigger;
			_completed = false;
		}
		return;
	}

	if (_pickedUpPiece == -1 && NancySceneState.getViewport().convertViewportToScreen(_cwCursorDest).contains(input.mousePos)) {
		g_nancy->_cursor->setCursorType(CursorManager::kRotateCW);

		if (input.input & NancyInput::kLeftMouseButtonUp && !g_nancy->_sound->isSoundPlaying(_rotateSound)) {
			g_nancy->_sound->playSound(_rotateSound);
			rotateBase(-1);
			return;
		}
	}

	if (_pickedUpPiece == -1 && NancySceneState.getViewport().convertViewportToScreen(_ccwCursorDest).contains(input.mousePos)) {
		g_nancy->_cursor->setCursorType(CursorManager::kRotateCCW);

		if (input.input & NancyInput::kLeftMouseButtonUp && !g_nancy->_sound->isSoundPlaying(_rotateSound)) {
			g_nancy->_sound->playSound(_rotateSound);
			rotateBase(1);
			return;
		}
	}

	for (uint i = 0; i < 5; ++i) {
		if (NancySceneState.getViewport().convertViewportToScreen(_pieceDests[i]).contains(input.mousePos)) {
			if (_pickedUpPiece == -1 && _placedPieces[i]) {
				return;
			}

			g_nancy->_cursor->setCursorType(CursorManager::kHotspot);

			if (input.input & NancyInput::kLeftMouseButtonUp) {
				if (_pickedUpPiece != -1) {
					_curPiece.putDown();
					_curPiece.setVisible(false);
					g_nancy->_sound->playSound(_placeDownSound);

					_drawSurface.fillRect(_pieceDests[_pickedUpPiece], _drawSurface.getTransparentColor());
					_drawSurface.blitFrom(_image, _pieceSrcs[_pickedUpPiece], _pieceDests[_pickedUpPiece]);
					_needsRedraw = true;
				}

				if (_pickedUpPiece != (int)i && !_placedPieces[i]) {
					// Clicked on another piece while holding, swap them
					_drawSurface.fillRect(_pieceDests[i], _drawSurface.getTransparentColor());

					if (_pickedUpPiece != -1) {
						_drawSurface.fillRect(_pieceDests[_pickedUpPiece], _drawSurface.getTransparentColor());
						_drawSurface.blitFrom(_image, _pieceSrcs[_pickedUpPiece], _pieceDests[_pickedUpPiece]);
					}

					_needsRedraw = true;

					_pickedUpPiece = i;
					g_nancy->_sound->playSound(_pickUpSound);

					_curPiece.pickUp();
					_curPiece.setVisible(true);
					_curPiece._drawSurface.create(_image, _pieceSrcs[i]);
					_curPiece.setVisible(true);
				} else {
					// Clicked the dest of the picked up piece, or an already placed one; simply put it down
					_pickedUpPiece = -1;
				}
			}
		}
	}

	if (_pickedUpPiece != -1) {
		// Piece picked up
		_curPiece.handleInput(input);

		bool canPlace = false;

		if (_pickedUpPiece == 0) {
			// Top piece can ble placed only in the correct rotation (which is 0)
			canPlace = _curRotation == 0;
		} else {
			canPlace = _curRotation + 1 == _pickedUpPiece;
		}

		if (canPlace) {
			if (NancySceneState.getViewport().convertViewportToScreen(_placedDest).contains(input.mousePos)) {
				g_nancy->_cursor->setCursorType(CursorManager::kHotspot);

				if (input.input & NancyInput::kLeftMouseButtonUp) {
					_placedPieces[_pickedUpPiece] = true;
					_curPiece.putDown();
					_curPiece.setVisible(false);
					_pickedUpPiece = -1;
					g_nancy->_sound->playSound(_placeDownSound);
					rotateBase(0);
				}
			}
		}
	}
}

void CubePuzzle::rotateBase(int dir) {
	_drawSurface.fillRect(_placedDest, _drawSurface.getTransparentColor());

	_curRotation += dir;
	if (_curRotation < 0) {
		_curRotation = 3;
	} else if (_curRotation > 3) {
		_curRotation = 0;
	}

	uint srcSelect = 0;

	if (_placedPieces[0]) {
		if (_placedPieces[_curRotation + 1]) {
			// Front piece is in, other checks are unnecessary
			srcSelect = 2;
		} else {
			int leftIndex = _curRotation + 1;
			if (leftIndex > 3) {
				leftIndex = 0;
			}

			int rightIndex = _curRotation - 1;
			if (rightIndex < 0) {
				rightIndex = 3;
			}

			int backIndex = (_curRotation + 2) % 4;

			if (_placedPieces[leftIndex + 1]) {
				// Left piece is in, check for right or back
				if (_placedPieces[rightIndex + 1]) {
					// Draw left & right piece
					srcSelect = 8;
				} else if (_placedPieces[backIndex]) {
					// Draw left & back piece
					srcSelect = 6;
				} else {
					// Draw left piece only
					srcSelect = 3;
				}
			} else if (_placedPieces[rightIndex + 1]) {
				// Right piece is in, check for back
				if (_placedPieces[backIndex + 1]) {
					// Draw right & back piece
					srcSelect = 7;
				} else {
					// Draw right piece only
					srcSelect = 4;
				}
			} else if (_placedPieces[backIndex + 1]) {
				// Draw back piece only
				srcSelect = 5;
			} else {
				// Draw top piece only
				srcSelect = 1;
			}
		}
	}

	_drawSurface.blitFrom(_image, _placedSrcs[_curRotation][srcSelect], _placedDest);
	_needsRedraw = true;
}

} // End of namespace Action
} // End of namespace Nancy
