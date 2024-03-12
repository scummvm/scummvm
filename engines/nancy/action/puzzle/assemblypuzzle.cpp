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

#include "engines/nancy/action/puzzle/assemblypuzzle.h"

namespace Nancy {
namespace Action {

void AssemblyPuzzle::init() {
	g_nancy->_resource->loadImage(_imageName, _image);
	_image.setTransparentColor(_drawSurface.getTransparentColor());

	for (uint i = 0; i < _pieces.size(); ++i) {
		Piece &piece = _pieces[i];
		piece.curRotation = piece.placed ? piece.correctRotation : 0;
		piece._drawSurface.create(_image, piece.srcRects[piece.curRotation]);
		piece.setVisible(true);
		piece.setTransparent(true);
		piece.moveTo(piece.placed ? piece.destRects[piece.curRotation] : piece.startRect);
		piece.setZ(_z + i + _pieces.size());
	}

	rotateBase(true);
	rotateBase(false);
}

void AssemblyPuzzle::registerGraphics() {
	for (uint i = 0; i < _pieces.size(); ++i) {
		_pieces[i].registerGraphics();
	}
}

void AssemblyPuzzle::readData(Common::SeekableReadStream &stream) {
	_puzzleState = (AssemblyPuzzleData *)NancySceneState.getPuzzleData(AssemblyPuzzleData::getTag());
	assert(_puzzleState);

	readFilename(stream, _imageName);

	uint16 numPieces = stream.readUint16LE();
	_height = stream.readUint16LE();

	readRect(stream, _cwCursorDest);
	readRect(stream, _ccwCursorDest);

	_pieces.resize(numPieces);
	for (uint i = 0; i < numPieces; ++i) {
		Piece &piece = _pieces[i];
		readRectArray(stream, piece.srcRects, 4);
		readRectArray(stream, piece.destRects, 4);

		readRect(stream, piece.startRect);

		piece.correctRotation = stream.readUint16LE();
		piece.layer = stream.readUint16LE();
		piece.placed = stream.readUint16LE();

		if (_puzzleState->solvedPuzzle) {
			piece.placed = true;
		}
	}
	stream.skip((12 - numPieces) * 150);

	_rotateSound.readNormal(stream);
	_pickUpSound.readNormal(stream);
	_placeDownSound.readNormal(stream);

	_allowWrongPieceHotspot = stream.readUint16LE();

	_wrongPieceSounds.resize(4);
	_wrongPieceTexts.resize(4);

	for (uint i = 0; i < 4; ++i) {
		_wrongPieceSounds[i].readNormal(stream);
	}

	char buf[200];
	for (uint i = 0; i < 4; ++i) {
		stream.read(buf, 200);
		assembleTextLine(buf, _wrongPieceTexts[i], 200);
	}

	_solveScene.readData(stream);
	_solveSound.readNormal(stream);
	stream.read(buf, 200);
	assembleTextLine(buf, _solveText, 200);

	_exitScene.readData(stream);
	readRect(stream, _exitHotspot);
}

void AssemblyPuzzle::execute() {
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
		if (_layersAssembled != _height) {
			return;
		}

		g_nancy->_sound->loadSound(_solveSound);
		g_nancy->_sound->playSound(_solveSound);
		NancySceneState.getTextbox().clear();
		NancySceneState.getTextbox().addTextLine(_solveText);
		NancySceneState.setEventFlag(_solveScene._flag);
		_completed = true;

		_state = kActionTrigger;
		break;
	case kActionTrigger:
		if (g_nancy->_sound->isSoundPlaying(_solveSound)) {
			return;
		}

		if (_completed) {
			_puzzleState->solvedPuzzle = true;
			NancySceneState.changeScene(_solveScene._sceneChange);
		} else {
			_exitScene.execute();
		}

		break;
	}
}

void AssemblyPuzzle::handleInput(NancyInput &input) {
	if (_state == kActionTrigger && _completed && g_nancy->_sound->isSoundPlaying(_solveSound)) {
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
			rotateBase(false);
			return;
		}
	}

	if (_pickedUpPiece == -1 && NancySceneState.getViewport().convertViewportToScreen(_ccwCursorDest).contains(input.mousePos)) {
		g_nancy->_cursor->setCursorType(CursorManager::kRotateCCW);

		if (input.input & NancyInput::kLeftMouseButtonUp && !g_nancy->_sound->isSoundPlaying(_rotateSound)) {
			g_nancy->_sound->playSound(_rotateSound);
			rotateBase(true);
			return;
		}
	}

	if (_completed) {
		return;
	}

	for (uint i = 0; i < _pieces.size(); ++i) {
		if (NancySceneState.getViewport().convertViewportToScreen(_pieces[i].startRect).contains(input.mousePos)) {
			if (_pickedUpPiece == -1 && _pieces[i].placed) {
				return;
			}

			g_nancy->_cursor->setCursorType(CursorManager::kHotspot);

			if (input.input & NancyInput::kLeftMouseButtonUp) {
				if (_pickedUpPiece != -1) {
					_pieces[_pickedUpPiece].putDown();
					_pieces[_pickedUpPiece].moveTo(_pieces[_pickedUpPiece].startRect);
					g_nancy->_sound->playSound(_placeDownSound);
				}

				if (_pickedUpPiece != (int)i && !_pieces[i].placed) {
					// Clicked on another piece while holding, swap them
					_pickedUpPiece = i;
					_pieces[i].pickUp();
					g_nancy->_sound->playSound(_pickUpSound);

					for (uint j = 1; j < _pieces.size(); ++j) {
						Piece &piece = _pieces[j];
						if (!piece.placed && piece.getZOrder() > _pieces[i].getZOrder()) {
							piece.setZ(piece.getZOrder() - 1);
							piece.registerGraphics();
						}
					}

					_pieces[i].setZ(_z + _pieces.size() * 2);
					_pieces[i].registerGraphics();
				} else {
					// Clicked the dest of the picked up piece, or an already placed one; simply put it down
					_pickedUpPiece = -1;
				}
			}
		}
	}

	if (_pickedUpPiece != -1) {
		// Piece picked up
		Piece &pickedUpPiece = _pieces[_pickedUpPiece];
		pickedUpPiece.handleInput(input);

		bool isWrong = _curRotation != pickedUpPiece.correctRotation;
		bool otherIsPlaced = false;
		for (uint i = 0; i < _pieces.size(); ++i) {
			if (_pieces[i].placed && _pieces[i].curRotation == 0 && _pieces[i].layer - 1 == (int)_layersAssembled) {
				otherIsPlaced = true;
				break;
			}
		}

		if (!otherIsPlaced && (!isWrong || (_allowWrongPieceHotspot && _layersAssembled + 1 == pickedUpPiece.layer))) {
			if (NancySceneState.getViewport().convertViewportToScreen(pickedUpPiece.destRects[0]).contains(input.mousePos)) {
				g_nancy->_cursor->setCursorType(CursorManager::kHotspot);

				if (input.input & NancyInput::kLeftMouseButtonUp) {
					if (!isWrong) {
						pickedUpPiece.putDown();
						pickedUpPiece.moveTo(pickedUpPiece.destRects[0]);
						g_nancy->_sound->playSound(_placeDownSound);
						pickedUpPiece.placed = true;
						_pickedUpPiece = -1;

						// Check for finished layer
						uint placedOnLayer = 0;
						for (uint i = 0; i < _pieces.size(); ++i) {
							if (_pieces[i].layer == _layersAssembled + 1 && _pieces[i].placed) {
								++placedOnLayer;
							}
						}

						if (placedOnLayer == 4) {
							++_layersAssembled;
						}
					} else if (_allowWrongPieceHotspot) {
						// Wrong place, play a sound
						g_nancy->_sound->loadSound(_wrongPieceSounds[_curRotation]);
						g_nancy->_sound->playSound(_wrongPieceSounds[_curRotation]);
						if (!_wrongPieceTexts[_curRotation].empty()) {
							NancySceneState.getTextbox().addTextLine(_wrongPieceTexts[_curRotation], 4000); // check
						}

						// and put down the piece
						pickedUpPiece.putDown();
						pickedUpPiece.moveTo(pickedUpPiece.startRect);
						_pickedUpPiece = -1;
					}
				}
			}
		}
	}
}

void AssemblyPuzzle::rotateBase(bool ccw) {
	// _curRotation moves in the opposite direction to pieces' rotations
	_curRotation += ccw ? 1 : -1;
	if (_curRotation < 0) {
		_curRotation = 3;
	} else if (_curRotation > 3) {
		_curRotation = 0;
	}

	for (uint i = 0; i < _pieces.size(); ++i) {
		Piece &piece = _pieces[i];
		if (piece.placed) {
			piece.curRotation += ccw ? -1 : 1;
			if (piece.curRotation < 0) {
				piece.curRotation = 3;
			} else if (piece.curRotation > 3) {
				piece.curRotation = 0;
			}

			uint base = 0;
			if (piece.curRotation == 0) {
				base = 2;
			} else if (piece.curRotation == 1 || piece.curRotation == 3) {
				base = 1;
			}

			piece.setZ(_z + base + 4 * (piece.layer - 1));
			piece.registerGraphics();

			piece.moveTo(piece.destRects[piece.curRotation]);
			piece._drawSurface.create(_image, piece.srcRects[piece.curRotation]);
			piece.setTransparent(true);
		}
	}
}

} // End of namespace Action
} // End of namespace Nancy
