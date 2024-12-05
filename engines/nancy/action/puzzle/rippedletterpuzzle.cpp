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
#include "engines/nancy/resource.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/input.h"
#include "engines/nancy/cursor.h"
#include "engines/nancy/puzzledata.h"

#include "engines/nancy/state/scene.h"
#include "engines/nancy/action/puzzle/rippedletterpuzzle.h"

#include "graphics/transform_struct.h"

namespace Nancy {
namespace Action {

void RippedLetterPuzzle::init() {
	Common::Rect screenBounds = NancySceneState.getViewport().getBounds();
	_drawSurface.create(screenBounds.width(), screenBounds.height(), g_nancy->_graphics->getInputPixelFormat());
	_drawSurface.clear(g_nancy->_graphics->getTransColor());
	setTransparent(true);
	setVisible(true);
	moveTo(screenBounds);

	g_nancy->_resource->loadImage(_imageName, _image);

	if (_useCustomPickUpTile) {
		_pickedUpPiece._drawSurface.create(_image, _customPickUpTileSrc);
	} else {
		_pickedUpPiece._drawSurface.create(_destRects[0].width(), _destRects[0].height(), g_nancy->_graphics->getInputPixelFormat());
	}

	_pickedUpPiece.setVisible(false);
}

void RippedLetterPuzzle::registerGraphics() {
	_pickedUpPiece.registerGraphics();
	RenderObject::registerGraphics();
}

void RippedLetterPuzzle::readData(Common::SeekableReadStream &stream) {
	_puzzleState = (RippedLetterPuzzleData *)NancySceneState.getPuzzleData(RippedLetterPuzzleData::getTag());
	assert(_puzzleState);

	readFilename(stream, _imageName);

	byte maxWidth = 6;
	byte maxHeight = g_nancy->getGameType() <= kGameTypeNancy6 ? 4 : 5;
	byte width = maxWidth;
	byte height = maxHeight;

	if (g_nancy->getGameType() >= kGameTypeNancy5) {
		width = stream.readByte();
		height = stream.readByte();
	}

	// All the checks for whether width is greater than maxWidth are
	// to account for nancy9 scene 2428, where the dimensions are 15x1

	for (uint i = 0; i < height; ++i) {
		readRectArray(stream, _srcRects, width, maxWidth);
	}
	stream.skip((maxWidth >= width ? (maxHeight - height) * maxWidth : maxWidth * maxHeight - width * height) * 16);

	for (uint i = 0; i < height; ++i) {
		readRectArray(stream, _destRects, width, maxWidth);
	}
	stream.skip((maxWidth >= width ? (maxHeight - height) * maxWidth : maxWidth * maxHeight - width * height) * 16);

	readRect(stream, _rotateHotspot);
	readRect(stream, _takeHotspot);
	readRect(stream, _dropHotspot);

	if (g_nancy->getGameType() >= kGameTypeNancy7) {
		_rotationType = (RotationType)stream.readUint16LE();
	}

	uint elemSize = g_nancy->getGameType() <= kGameTypeNancy8 ? 1 : 2;

	_initOrder.resize(width * height);
	for (uint i = 0; i < height; ++i) {
		for (uint j = 0; j < width; ++j) {
			_initOrder[i * width + j] = (elemSize == 1 ? stream.readByte() : stream.readSint16LE());
		}
		stream.skip(maxWidth > width ? (maxWidth - width) * elemSize : 0);
	}
	stream.skip((maxWidth > width ? (maxHeight - height) * maxWidth : maxWidth * maxHeight - width * height) * elemSize);

	_initRotations.resize(width * height);
	for (uint i = 0; i < height; ++i) {
		for (uint j = 0; j < width; ++j) {
			_initRotations[i * width + j] = (elemSize == 1 ? stream.readByte() : stream.readSint16LE());
		}
		stream.skip(maxWidth > width ? (maxWidth - width) * elemSize : 0);
	}
	stream.skip((maxWidth > width ? (maxHeight - height) * maxWidth : maxWidth * maxHeight - width * height) * elemSize);

	if (g_nancy->getGameType() >= kGameTypeNancy9) {
		uint16 numDoubledElements = stream.readUint16LE();
		_doubles.resize(numDoubledElements);
		uint i = 0;
		for (uint j = 0; j < 20; ++j) {
			int16 id = stream.readSint16LE();
			if (id == -1) {
				++i;
			} else {
				_doubles[i].push_back(id);
			}
		}
	}

	_solveOrder.resize(width * height);
	for (uint i = 0; i < height; ++i) {
		for (uint j = 0; j < width; ++j) {
			_solveOrder[i * width + j] = (elemSize == 1 ? stream.readByte() : stream.readSint16LE());
		}
		stream.skip(maxWidth > width ? (maxWidth - width) * elemSize : 0);
	}
	stream.skip((maxWidth > width ? (maxHeight - height) * maxWidth : maxWidth * maxHeight - width * height) * elemSize);

	_solveRotations.resize(width * height);
	for (uint i = 0; i < height; ++i) {
		for (uint j = 0; j < width; ++j) {
			_solveRotations[i * width + j] = (elemSize == 1 ? stream.readByte() : stream.readSint16LE());
		}
		stream.skip(maxWidth > width ? (maxWidth - width) * elemSize : 0);
	}
	stream.skip((maxWidth > width ? (maxHeight - height) * maxWidth : maxWidth * maxHeight - width * height) * elemSize);

	if (g_nancy->getGameType() >= kGameTypeNancy9) {
		_useAltSolution = stream.readByte();

		_solveOrderAlt.resize(width * height);
		for (uint i = 0; i < height; ++i) {
			for (uint j = 0; j < width; ++j) {
				_solveOrderAlt[i * width + j] = (elemSize == 1 ? stream.readByte() : stream.readSint16LE());
			}
			stream.skip(maxWidth > width ? (maxWidth - width) * elemSize : 0);
		}
		stream.skip((maxWidth > width ? (maxHeight - height) * maxWidth : maxWidth * maxHeight - width * height) * elemSize);

		_solveRotationsAlt.resize(width * height);
		for (uint i = 0; i < height; ++i) {
			for (uint j = 0; j < width; ++j) {
				_solveRotationsAlt[i * width + j] = (elemSize == 1 ? stream.readByte() : stream.readSint16LE());
			}
			stream.skip(maxWidth > width ? (maxWidth - width) * elemSize : 0);
		}
		stream.skip((maxWidth > width ? (maxHeight - height) * maxWidth : maxWidth * maxHeight - width * height) * elemSize);
	}

	if (g_nancy->getGameType() >= kGameTypeNancy7) {
		_useCustomPickUpTile = stream.readByte();
		readRect(stream, _customPickUpTileSrc);
	}

	_takeSound.readNormal(stream);
	_dropSound.readNormal(stream);
	_rotateSound.readNormal(stream);

	_solveExitScene.readData(stream);
	_solveSound.readNormal(stream);

	_exitScene.readData(stream);
	readRect(stream, _exitHotspot);

	if (g_nancy->getGameType() >= kGameTypeNancy9) {
		_customCursorID = stream.readSint16LE();
	}
}

void RippedLetterPuzzle::execute() {
	switch (_state) {
	case kBegin:
		_puzzleState = (RippedLetterPuzzleData *)NancySceneState.getPuzzleData(RippedLetterPuzzleData::getTag());
		assert(_puzzleState);

		init();
		registerGraphics();

		NancySceneState.setNoHeldItem();

		if (!_puzzleState->playerHasTriedPuzzle) {
			_puzzleState->order = _initOrder;
			_puzzleState->rotations = _initRotations;
			_puzzleState->playerHasTriedPuzzle = true;
		} else if (_puzzleState->_pickedUpPieceID != -1) {
			// Puzzle was left while still holding a piece (e.g. by clicking a scene item).
			// Make sure we put the held piece back in its place
			_puzzleState->order[_puzzleState->_pickedUpPieceLastPos] = _puzzleState->_pickedUpPieceID;
			_puzzleState->rotations[_puzzleState->_pickedUpPieceLastPos] = _puzzleState->_pickedUpPieceRot;
			_puzzleState->_pickedUpPieceID = -1;
			_puzzleState->_pickedUpPieceLastPos = -1;
			_puzzleState->_pickedUpPieceRot = 0;
		}

		for (uint i = 0; i < _puzzleState->order.size(); ++i) {
			drawPiece(i, _puzzleState->rotations[i], _puzzleState->order[i]);
		}

		g_nancy->_sound->loadSound(_takeSound);
		g_nancy->_sound->loadSound(_dropSound);
		g_nancy->_sound->loadSound(_rotateSound);

		_state = kRun;
		// fall through
	case kRun:
		switch (_solveState) {
		case kNotSolved :
			for (uint i = 0; i < _puzzleState->order.size(); ++i) {
				if (_puzzleState->rotations[i] != _solveRotations[i] || !checkOrder(false)) {
					if (_useAltSolution) {
						if (!checkOrder(true)) {
							return;
						}
					} else {
						return;
					}
				}
			}

			g_nancy->_sound->loadSound(_solveSound);
			g_nancy->_sound->playSound(_solveSound);
			_solveState = kWaitForSound;
			break;
		case kWaitForSound :
			if (!g_nancy->_sound->isSoundPlaying(_solveSound)) {
				g_nancy->_sound->stopSound(_solveSound);
				_state = kActionTrigger;
			}

			break;
		}

		break;
	case kActionTrigger :
		switch (_solveState) {
		case kNotSolved:
			_exitScene.execute();
			break;
		case kWaitForSound:
			if (_solveExitScene._sceneChange.sceneID == NancySceneState.getSceneInfo().sceneID) {
				// nancy9 scene 2484 is auto-solved for you, but has a valid scene change back to itself
				return;
			}
			_solveExitScene.execute();
			_puzzleState->playerHasTriedPuzzle = false;
			break;
		}

		g_nancy->_sound->stopSound(_takeSound);
		g_nancy->_sound->stopSound(_dropSound);
		g_nancy->_sound->stopSound(_rotateSound);
		finishExecution();
	}
}

void RippedLetterPuzzle::handleInput(NancyInput &input) {
	if (_state == kBegin) {
		return;
	}

	for (uint i = 0; i < _puzzleState->order.size(); ++i) {
		Common::Rect screenHotspot = NancySceneState.getViewport().convertViewportToScreen(_destRects[i]);
		if (screenHotspot.contains(input.mousePos)) {
			Common::Rect insideRect;
			if (_puzzleState->_pickedUpPieceID == -1) {
				// No piece picked up

				// Check if the mouse is inside the rotation hotspot
				insideRect = _rotateHotspot;
				insideRect.translate(screenHotspot.left, screenHotspot.top);

				if (_rotationType != kRotationNone && insideRect.contains(input.mousePos)) {
					g_nancy->_cursor->setCursorType(CursorManager::kRotateCW);

					if (input.input & NancyInput::kLeftMouseButtonUp) {
						// Player has clicked, rotate the piece
						int inc = (_rotationType == kRotation90 ? 1 : 2);
						if ((_puzzleState->rotations[i] += inc) > 3) {
							_puzzleState->rotations[i] -= 4;
						}

						drawPiece(i, _puzzleState->rotations[i], _puzzleState->order[i]);
						g_nancy->_sound->playSound(_rotateSound);
					}

					break;
				}

				// Check if the mouse is inside the pickup hotspot
				insideRect = _takeHotspot;
				insideRect.translate(screenHotspot.left, screenHotspot.top);

				if (insideRect.contains(input.mousePos)) {
					g_nancy->_cursor->setCursorType(CursorManager::kHotspot);

					if (input.input & NancyInput::kLeftMouseButtonUp) {
						// Player has clicked, take the piece

						// First, copy the graphic from the full drawSurface...
						if (!_useCustomPickUpTile) {
							_pickedUpPiece._drawSurface.clear(g_nancy->_graphics->getTransColor());
							_pickedUpPiece._drawSurface.blitFrom(_drawSurface, _destRects[i], Common::Point());
						}

						_pickedUpPiece.setVisible(true);
						_pickedUpPiece.setTransparent(true);
						_pickedUpPiece.pickUp();

						// ...then change the data...
						_puzzleState->_pickedUpPieceID = _puzzleState->order[i];
						_puzzleState->_pickedUpPieceRot = _puzzleState->rotations[i];
						_puzzleState->order[i] = -1;
						_puzzleState->_pickedUpPieceLastPos = i;

						// ...then clear the piece from the drawSurface
						drawPiece(i, 0);

						g_nancy->_sound->playSound(_takeSound);
					}

					break;
				}
			} else {
				// Currently carrying a piece

				// Check if the mouse is inside the drop hotspot
				insideRect = _dropHotspot;
				insideRect.translate(screenHotspot.left, screenHotspot.top);

				if (insideRect.contains(input.mousePos)) {
					g_nancy->_cursor->setCursorType(CursorManager::kHotspot);

					if (input.input & NancyInput::kLeftMouseButtonUp) {
						// Player has clicked, drop the piece and pick up a new one

						// Check if we should pick up a new piece
						if (_puzzleState->order[i] == -1) {
							// No, hide the picked up piece graphic
							_pickedUpPiece.setVisible(false);
							_puzzleState->_pickedUpPieceLastPos = -1;
						} else {
							// Yes, change the picked piece graphic
							if (!_useCustomPickUpTile) {
								_pickedUpPiece._drawSurface.clear(g_nancy->_graphics->getTransColor());
								_pickedUpPiece._drawSurface.blitFrom(_drawSurface, _destRects[i], Common::Point());
							}

							_pickedUpPiece.setVisible(true);
							_pickedUpPiece.setTransparent(true);
						}

						SWAP<int8>(_puzzleState->order[i], _puzzleState->_pickedUpPieceID);
						SWAP<byte>(_puzzleState->rotations[i], _puzzleState->_pickedUpPieceRot);

						// Draw the newly placed piece
						drawPiece(i, _puzzleState->rotations[i], _puzzleState->order[i]);

						g_nancy->_sound->playSound(_dropSound);
					}

					break;
				}
			}
		}
	}

	_pickedUpPiece.handleInput(input);

	if (_puzzleState->_pickedUpPieceID == -1) {
		// No piece picked up, check the exit hotspot
		if (NancySceneState.getViewport().convertViewportToScreen(_exitHotspot).contains(input.mousePos)) {
			g_nancy->_cursor->setCursorType(_customCursorID != -1 ? (CursorManager::CursorType)_customCursorID : g_nancy->_cursor->_puzzleExitCursor);

			if (input.input & NancyInput::kLeftMouseButtonUp) {
				// Player has clicked, exit
				_state = kActionTrigger;
			}
		}
	}
}

void RippedLetterPuzzle::drawPiece(const uint pos, const byte rotation, const int pieceID) {
	// Clear the selected position
	_drawSurface.fillRect(_destRects[pos], _drawSurface.getTransparentColor());
	_needsRedraw = true;

	// No piece, just clear
	if (pieceID == -1) {
		return;
	}

	// Create temporary ManagedSurfaces and call the custom rotation function
	Graphics::ManagedSurface srcSurf(_image, _srcRects[pieceID]);
	Graphics::ManagedSurface destSurf(_drawSurface, _destRects[pos]);
	GraphicsManager::rotateBlit(srcSurf, destSurf, rotation);
}

bool RippedLetterPuzzle::checkOrder(bool useAlt) {
	auto &current = _puzzleState->order;
	auto &correct = useAlt ? _solveOrderAlt : _solveOrder;

	if (!_doubles.size()) {
		return current == correct;
	}

	for (uint i = 0; i < current.size(); ++i) {
		bool foundCorrect = false;
		bool isDoubled = false;
		for (auto &d : _doubles) {
			for (byte e : d) {
				if (current[i] == e) {
					isDoubled = true;
					break;
				}
			}

			if (isDoubled) {
				for (byte e : d) {
					if (correct[i] == e) {
						foundCorrect = true;
						break;
					}
				}

				if (!foundCorrect) {
					return false;
				}

				break;
			}
		}

		if (!isDoubled) {
			if (current[i] != correct[i]) {
				return false;
			}
		}
	}

	return true;
}

} // End of namespace Action
} // End of namespace Nancy
