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
#include "engines/nancy/action/rippedletterpuzzle.h"

#include "graphics/transform_struct.h"

namespace Nancy {
namespace Action {

void RippedLetterPuzzle::init() {
	Common::Rect screenBounds = NancySceneState.getViewport().getBounds();
	_drawSurface.create(screenBounds.width(), screenBounds.height(), g_nancy->_graphicsManager->getInputPixelFormat());
	_drawSurface.clear(g_nancy->_graphicsManager->getTransColor());
	setTransparent(true);
	setVisible(true);
	moveTo(screenBounds);

	_pickedUpPiece._drawSurface.create(_destRects[0].width(), _destRects[0].height(), g_nancy->_graphicsManager->getInputPixelFormat());
	_pickedUpPiece.setVisible(false);

	g_nancy->_resource->loadImage(_imageName, _image);
}

void RippedLetterPuzzle::registerGraphics() {
	_pickedUpPiece.registerGraphics();
	RenderObject::registerGraphics();
}

void RippedLetterPuzzle::readData(Common::SeekableReadStream &stream) {
	_puzzleState = (RippedLetterPuzzleData *)NancySceneState.getPuzzleData(RippedLetterPuzzleData::getTag());
	assert(_puzzleState);

	readFilename(stream, _imageName);

	readRectArray(stream, _srcRects, 24);
	readRectArray(stream, _destRects, 24);

	readRect(stream, _rotateHotspot);
	readRect(stream, _takeHotspot);
	readRect(stream, _dropHotspot);

	_initOrder.resize(24);
	for (uint i = 0; i < 24; ++i) {
		_initOrder[i] = stream.readByte();
	}

	_initRotations.resize(24);
	for (uint i = 0; i < 24; ++i) {
		_initRotations[i] = stream.readByte();
	}

	_solveOrder.resize(24);
	for (uint i = 0; i < 24; ++i) {
		_solveOrder[i] = stream.readByte();
	}

	_solveRotations.resize(24);
	for (uint i = 0; i < 24; ++i) {
		_solveRotations[i] = stream.readByte();
	}

	_takeSound.readData(stream, SoundDescription::kNormal);
	_dropSound.readData(stream, SoundDescription::kNormal);
	_rotateSound.readData(stream, SoundDescription::kNormal);

	_solveExitScene.readData(stream);
	_solveSound.readData(stream, SoundDescription::kNormal);

	_exitScene.readData(stream);
	readRect(stream, _exitHotspot);
}

void RippedLetterPuzzle::execute() {
	switch (_state) {
	case kBegin:
		init();
		registerGraphics();
		if (!_puzzleState->playerHasTriedPuzzle) {
			_puzzleState->order = _initOrder;
			_puzzleState->rotations = _initRotations;
			_puzzleState->playerHasTriedPuzzle = true;
		}

		for (uint i = 0; i < 24; ++i) {
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
			for (uint i = 0; i < 24; ++i) {
				if (_puzzleState->order[i] != _solveOrder[i] || _puzzleState->rotations[i] != _solveRotations[i]) {
					return;
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
	if (_state != kRun && _solveState != kNotSolved) {
		return;
	}

	for (uint i = 0; i < 24; ++i) {
		Common::Rect screenHotspot = NancySceneState.getViewport().convertViewportToScreen(_destRects[i]);
		if (screenHotspot.contains(input.mousePos)) {
			Common::Rect insideRect;
			if (_pickedUpPieceID == -1) {
				// No piece picked up

				// Check if the mouse is inside the rotation hotspot
				insideRect = _rotateHotspot;
				insideRect.translate(screenHotspot.left, screenHotspot.top);

				if (insideRect.contains(input.mousePos)) {
					g_nancy->_cursorManager->setCursorType(CursorManager::kRotate);

					if (input.input & NancyInput::kLeftMouseButtonUp) {
						// Player has clicked, rotate the piece
						if (++_puzzleState->rotations[i] > 3) {
							_puzzleState->rotations[i] = 0;
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
					g_nancy->_cursorManager->setCursorType(CursorManager::kHotspot);

					if (input.input & NancyInput::kLeftMouseButtonUp) {
						// Player has clicked, take the piece

						// First, copy the graphic from the full drawSurface...
						_pickedUpPiece._drawSurface.clear(g_nancy->_graphicsManager->getTransColor());
						_pickedUpPiece._drawSurface.blitFrom(_drawSurface, _destRects[i], Common::Point());
						_pickedUpPiece.setVisible(true);
						_pickedUpPiece.setTransparent(true);

						// ...then change the data...
						_pickedUpPieceID = _puzzleState->order[i];
						_pickedUpPieceRot = _puzzleState->rotations[i];
						_puzzleState->order[i] = -1;

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
					g_nancy->_cursorManager->setCursorType(CursorManager::kHotspot);

					if (input.input & NancyInput::kLeftMouseButtonUp) {
						// Player has clicked, drop the piece and pick up a new one

						// Check if we should pick up a new piece
						if (_puzzleState->order[i] == -1) {
							// No, hide the picked up piece graphic
							_pickedUpPiece.setVisible(false);
						} else {
							// Yes, change the picked piece graphic
							_pickedUpPiece._drawSurface.clear(g_nancy->_graphicsManager->getTransColor());
							_pickedUpPiece._drawSurface.blitFrom(_drawSurface, _destRects[i], Common::Point());
							_pickedUpPiece.setVisible(true);
							_pickedUpPiece.setTransparent(true);
						}

						SWAP<int8>(_puzzleState->order[i], _pickedUpPieceID);
						SWAP<byte>(_puzzleState->rotations[i], _pickedUpPieceRot);

						// Draw the newly placed piece
						drawPiece(i, _puzzleState->rotations[i], _puzzleState->order[i]);

						g_nancy->_sound->playSound(_dropSound);
					}

					break;
				}
			}
		}
	}

	// Now move the carried piece
	if (_pickedUpPieceID != -1) {
		// First, move the piece so its center is below the mouse hotspot
		Common::Rect newLocation = _pickedUpPiece._drawSurface.getBounds();
		newLocation.moveTo(input.mousePos.x, input.mousePos.y);
		newLocation.translate(-newLocation.width() / 2, -newLocation.height() / 2);

		// Then, make sure it doesn't escape outside the viewport bounds
		Common::Rect screen = NancySceneState.getViewport().getScreenPosition();
		
		if (newLocation.left < screen.left) {
			newLocation.translate(screen.left - newLocation.left, 0);
		}

		if (newLocation.top < screen.top) {
			newLocation.translate(0, screen.top - newLocation.top);
		}

		if (newLocation.right > screen.right) {
			newLocation.translate(screen.right - newLocation.right, 0);
		}

		if (newLocation.bottom > screen.bottom) {
			newLocation.translate(0, screen.bottom - newLocation.bottom);
		}

		_pickedUpPiece.moveTo(newLocation);
	} else {
		// No piece picked up, check the exit hotspot
		if (NancySceneState.getViewport().convertViewportToScreen(_exitHotspot).contains(input.mousePos)) {
			g_nancy->_cursorManager->setCursorType(CursorManager::kExit);

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

} // End of namespace Action
} // End of namespace Nancy
