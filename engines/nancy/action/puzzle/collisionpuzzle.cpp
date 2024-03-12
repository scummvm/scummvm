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

#include "engines/nancy/action/puzzle/collisionpuzzle.h"

namespace Nancy {
namespace Action {

void CollisionPuzzle::init() {
	Common::Rect screenBounds = NancySceneState.getViewport().getBounds();
	_drawSurface.create(screenBounds.width(), screenBounds.height(), g_nancy->_graphics->getInputPixelFormat());
	_drawSurface.clear(g_nancy->_graphics->getTransColor());
	setTransparent(true);
	setVisible(true);
	moveTo(screenBounds);

	g_nancy->_resource->loadImage(_imageName, _image);
	_image.setTransparentColor(_drawSurface.getTransparentColor());

	if (_puzzleType == kCollision) {
		_pieces.resize(_pieceSrcs.size(), Piece());
		for (uint i = 0; i < _pieceSrcs.size(); ++i) {
			_pieces[i]._drawSurface.create(_image, _pieceSrcs[i]);
			Common::Rect pos = getScreenPosition(_startLocations[i]);
			if (_lineWidth == 6) {
				pos.translate(-1, 0); // Improvement
			}
			_pieces[i].moveTo(pos);
			_pieces[i]._gridPos = _startLocations[i];
			_pieces[i].setVisible(true);
			_pieces[i].setTransparent(true);
		}
	} else {
		for (uint y = 0; y < _grid.size(); ++y) {
			for (uint x = 0; x < _grid[0].size(); ++x) {
				if (_grid[y][x] == 0) {
					continue;
				}

				Piece newPiece;
				uint id = _grid[y][x];

				switch (id) {
				case 1 :
					newPiece._w = 2;
					break;
				case 2 :
					newPiece._h = 2;
					break;
				case 3 :
					newPiece._w = 3;
					break;
				case 4 :
					newPiece._h = 3;
					break;
				case 5 :
					newPiece._w = 2;
					newPiece._h = 2;
					break;
				case 6 :
					newPiece._w = 2;
					break;
				default :
					continue;
				}

				newPiece._drawSurface.create(_image, _pieceSrcs[id - 1]);
				Common::Rect pos = getScreenPosition(Common::Point(x, y));
				if (_lineWidth == 6) {
					pos.translate(-1, 0); // Improvement
				}
				pos.setWidth(newPiece._drawSurface.w);
				pos.setHeight(newPiece._drawSurface.h);
				newPiece.moveTo(pos);
				newPiece._gridPos = Common::Point(x, y);
				newPiece.setVisible(true);
				newPiece.setTransparent(true);

				if (id == 6) {
					// The solve piece is pushed to the front
					_pieces.insert_at(0, newPiece);
				} else {
					_pieces.push_back(newPiece);
				}
			}
		}
	}

	if (_puzzleType == kCollision) {
		drawGrid();
	}

	registerGraphics();
}

void CollisionPuzzle::registerGraphics() {
	for (uint i = 0; i < _pieces.size(); ++i) {
		_pieces[i].registerGraphics();
	}

	RenderActionRecord::registerGraphics();
}

void CollisionPuzzle::updateGraphics() {
	if (_state == kRun) {
		if (_timerSrcs.size()) {
			uint32 currentTime = g_nancy->getTotalPlayTime() - _puzzleStartTime;
			int graphicForTime = currentTime / ((_timerTime * 1000) / _timerSrcs.size());
			if (graphicForTime != _currentTimerGraphic) {
				_drawSurface.fillRect(_timerDest, _drawSurface.getTransparentColor());
				_drawSurface.blitFrom(_image, _timerSrcs[graphicForTime], _timerDest);
				_needsRedraw = true;
				_currentTimerGraphic = graphicForTime;
				NancySceneState.setEventFlag(_timerFlagIds[graphicForTime], g_nancy->_true);
			}
		}

		if (_currentlyAnimating != -1) {
			// Framerate-dependent animation. Should be fine since we limit the engine to ~60fps
			++_currentAnimFrame;
			bool horizontal = _lastPosition.x != _pieces[_currentlyAnimating]._gridPos.x;
			int diff = horizontal ?
					_lastPosition.x - _pieces[_currentlyAnimating]._gridPos.x :
					_lastPosition.y - _pieces[_currentlyAnimating]._gridPos.y;

			int maxFrames = _framesPerMove * abs(diff);
			if (_currentAnimFrame > maxFrames) {
				if (_puzzleType == kCollision && _grid[_pieces[_currentlyAnimating]._gridPos.y][_pieces[_currentlyAnimating]._gridPos.x] == _currentlyAnimating + 1) {
					g_nancy->_sound->playSound(_homeSound);
				} else {
					g_nancy->_sound->playSound(_wallHitSound);
				}

				_currentlyAnimating = -1;
				_currentAnimFrame = -1;
				return;
			}

			Common::Rect destRect = getScreenPosition(_lastPosition);
			Common::Rect endPos = getScreenPosition(_pieces[_currentlyAnimating]._gridPos);

			if (_lineWidth == 6) {
				destRect.translate(-1, 0); // Improvement
				endPos.translate(-1, 0); // Improvement
			}

			Common::Point dest(destRect.left, destRect.top);
			if (horizontal) {
				dest.x = destRect.left + (endPos.left - dest.x) * _currentAnimFrame / maxFrames;
			} else {
				dest.y = destRect.top + (endPos.top - dest.y) * _currentAnimFrame / maxFrames;
			}

			_pieces[_currentlyAnimating].moveTo(dest);
		}
	}
}

void CollisionPuzzle::readData(Common::SeekableReadStream &stream) {
	readFilename(stream, _imageName);
	uint16 numPieces = 0;

	uint16 width = stream.readUint16LE();
	uint16 height = stream.readUint16LE();

	if (_puzzleType == kCollision) {
		numPieces = stream.readUint16LE();
	} else {
		_tileMoveExitPos.y = stream.readUint16LE();
		_tileMoveExitPos.x = stream.readUint16LE();
		_tileMoveExitSize = stream.readUint16LE();
		numPieces = 6;
	}

	_grid.resize(height, Common::Array<uint16>(width));
	for (uint y = 0; y < height; ++y) {
		for (uint x = 0; x < width; ++x) {
			_grid[y][x] = stream.readUint16LE();
		}
		stream.skip((8 - width) * 2);
	}
	stream.skip((8 - height) * 8 * 2);

	if (_puzzleType == kCollision) {
		_startLocations.resize(numPieces);
		for (uint i = 0; i < numPieces; ++i) {
			_startLocations[i].x = stream.readUint16LE();
			_startLocations[i].y = stream.readUint16LE();
		}
		stream.skip((5 - numPieces) * 4);

		readRectArray(stream, _pieceSrcs, numPieces, 5);
		readRectArray(stream, _homeSrcs, numPieces, 5);

		readRect(stream, _verticalWallSrc);
		readRect(stream, _horizontalWallSrc);
		readRect(stream, _blockSrc);
	} else {
		readRectArray(stream, _pieceSrcs, 6);

		if (g_nancy->getGameType() >= kGameTypeNancy8) {
			_usesExitButton = stream.readByte();
			readRect(stream, _exitButtonSrc);
			readRect(stream, _exitButtonDest);
		}
	}

	_gridPos.x = stream.readUint32LE();
	_gridPos.y = stream.readUint32LE();

	_lineWidth = stream.readUint16LE();
	_framesPerMove = stream.readUint16LE();

	if (g_nancy->getGameType() <= kGameTypeNancy7) {
		stream.skip(3);
	} else if (_puzzleType == kTileMove) {
		uint16 numTimerGraphics = stream.readUint16LE();
		_timerTime = stream.readUint32LE();
		readRectArray(stream, _timerSrcs, numTimerGraphics, 10);
		_timerFlagIds.resize(numTimerGraphics);
		for (uint i = 0; i < numTimerGraphics; ++i) {
			_timerFlagIds[i] = stream.readSint16LE();
		}
		stream.skip((10 - numTimerGraphics) * 2);
		readRect(stream, _timerDest);
	}

	_moveSound.readNormal(stream);
	if (_puzzleType == kCollision) {
		_homeSound.readNormal(stream);
	}
	_wallHitSound.readNormal(stream);
	if (_puzzleType == kTileMove && g_nancy->getGameType() >= kGameTypeNancy8) {
		_exitButtonSound.readNormal(stream);
	}

	_solveScene.readData(stream);
	_solveSoundDelay = stream.readUint16LE();
	_solveSound.readNormal(stream);

	_exitScene.readData(stream);
	readRect(stream, _exitHotspot);
}

void CollisionPuzzle::execute() {
	switch (_state) {
	case kBegin :
		init();
		g_nancy->_sound->loadSound(_moveSound);
		g_nancy->_sound->loadSound(_wallHitSound);
		g_nancy->_sound->loadSound(_homeSound);
		NancySceneState.setNoHeldItem();
		_state = kRun;
		// fall through
	case kRun :
		if (_currentlyAnimating != -1) {
			return;
		}

		// Check timer
		if (_timerSrcs.size()) {
			if ((g_nancy->getTotalPlayTime() - _puzzleStartTime) > _timerTime * 1000) {
				_state = kActionTrigger;
				return;
			}
		}

		if (_puzzleType == kCollision) {
			// Check if every tile is in its "home"
			for (uint i = 0; i < _pieces.size(); ++i) {
				if (_grid[_pieces[i]._gridPos.y][_pieces[i]._gridPos.x] != i + 1) {
					return;
				}
			}
		} else {
			// Check if either:
			// - the solve tile is over the exit or;
			// - the solve tile is outside the bounds of the grid (and is thus inside the exit)
			Common::Point pos = _pieces[0]._gridPos;
			Common::Rect posRect(pos.x, pos.y, pos.x + _pieces[0]._w, pos.y + _pieces[0]._h);
			Common::Rect gridRect(_grid.size(), _grid[0].size());
			if (!posRect.contains(_tileMoveExitPos) && gridRect.contains(pos)) {
				return;
			}
		}

		_solveSoundPlayTime = g_nancy->getTotalPlayTime() + _solveSoundDelay * 1000;
		_state = kActionTrigger;
		_solved = true;
		return;
	case kActionTrigger :
		if (_solved) {
			if (_solveSoundPlayTime != 0) {
				if (g_nancy->getTotalPlayTime() < _solveSoundPlayTime) {
					return;
				}

				g_nancy->_sound->loadSound(_solveSound);
				g_nancy->_sound->playSound(_solveSound);
				NancySceneState.setEventFlag(_solveScene._flag);
				_solveSoundPlayTime = 0;
				return;
			} else {
				if (g_nancy->_sound->isSoundPlaying(_solveSound)) {
					return;
				}

				NancySceneState.changeScene(_solveScene._sceneChange);
			}
		} else {
			if (g_nancy->_sound->isSoundPlaying(_exitButtonSound)) {
				return;
			}
			_exitScene.execute();
		}

		g_nancy->_sound->stopSound(_solveSound);
		g_nancy->_sound->stopSound(_moveSound);
		g_nancy->_sound->stopSound(_wallHitSound);
		g_nancy->_sound->stopSound(_homeSound);

		finishExecution();
	}
}

Common::Point CollisionPuzzle::movePiece(uint pieceID, WallType direction) {
	Common::Point newPos = _pieces[pieceID]._gridPos;
	bool done = false;

	uint preStopWallType = 0;
	uint postStopWallType = 0;
	int inc = 0;
	bool horizontal = false;

	switch (direction) {
	case kWallLeft :
		preStopWallType = kWallRight;
		postStopWallType = kWallLeft;
		inc = -1;
		horizontal = true;

		break;
	case kWallRight :
		preStopWallType = kWallLeft;
		postStopWallType = kWallRight;
		inc = 1;
		horizontal = true;

		break;
	case kWallUp :
		preStopWallType = kWallDown;
		postStopWallType = kWallUp;
		inc = -1;
		horizontal = false;

		break;
	case kWallDown :
		preStopWallType = kWallUp;
		postStopWallType = kWallDown;
		inc = 1;
		horizontal = false;

		break;
	default:
		return { -1, -1 };
	}

	// Set the last possible position to check before the piece would be out of bounds
	int lastPos = inc > 0 ? (horizontal ? (int)_grid[0].size() : (int)_grid.size()) : -1;
	if (lastPos != -1) {
		// For TileMove, ensure wider pieces won't clip out
		lastPos -= inc * ((horizontal ? _pieces[pieceID]._w : _pieces[pieceID]._h) - 1);
	}

	for (int i = (horizontal ? newPos.x : newPos.y) + inc; (inc > 0 ? i < lastPos : i > lastPos); i += inc) {
		// First, check if other pieces would block
		Common::Point comparePos = newPos;
		if (horizontal) {
			comparePos.x = i;
		} else {
			comparePos.y = i;
		}

		Common::Rect compareRect(comparePos.x, comparePos.y, comparePos.x + _pieces[pieceID]._w, comparePos.y + _pieces[pieceID]._h);

		for (uint j = 0; j < _pieces.size(); ++j) {
			if (pieceID == j) {
				continue;
			}

			Common::Rect pieceBounds(	_pieces[j]._gridPos.x,
										_pieces[j]._gridPos.y,
										_pieces[j]._gridPos.x + _pieces[j]._w,
										_pieces[j]._gridPos.y + _pieces[j]._h);
			if (pieceBounds.intersects(compareRect)) {
				done = true;
				break;
			}
		}

		if (done) {
			break;
		}

		if (_puzzleType == kCollision) {
			// Next, check the grid for blocking walls
			uint16 evalVal = horizontal ? _grid[newPos.y][i] : _grid[i][newPos.x];
			if (evalVal == postStopWallType) {
				if (horizontal) {
					newPos.x = i;
				} else {
					newPos.y = i;
				}

				break;
			} else if (evalVal == preStopWallType || evalVal == kBlock) {
				break;
			}
		}

		if (horizontal) {
			newPos.x = i;
		} else {
			newPos.y = i;
		}
	}

	// Move result outside of grid when the exit is at an edge, and the moved piece is on top of the exit
	if (_puzzleType == kTileMove && pieceID == 0) {
		Common::Rect compareRect(newPos.x, newPos.y, newPos.x + _pieces[pieceID]._w, newPos.y + _pieces[pieceID]._h);
		if (compareRect.contains(_tileMoveExitPos)) {
			if (horizontal && (_tileMoveExitPos.x == 0 || _tileMoveExitPos.x == (int)_grid[0].size() - 1)) {
				newPos.x += inc * _tileMoveExitSize;
			} else if (!horizontal && (_tileMoveExitPos.y == 0 || _tileMoveExitPos.y == (int)_grid.size() - 1)) {
				newPos.y += inc * _tileMoveExitSize;
			}
		}
	}

	return newPos;
}

Common::Rect CollisionPuzzle::getScreenPosition(Common::Point gridPos) {
	Common::Rect dest = _pieceSrcs[0];

	dest.moveTo(0, 0);

	dest.right -= 1;
	dest.bottom -= 1;

	if (_puzzleType == kTileMove) {
		dest.setWidth(dest.width() / 2);
	}

	dest.moveTo(_gridPos);
	dest.translate(gridPos.x * dest.width(), gridPos.y *dest.height());
	dest.translate(gridPos.x * _lineWidth, gridPos.y * _lineWidth);

	dest.right += 1;
	dest.bottom += 1;

	return dest;
}

void CollisionPuzzle::drawGrid() {
	// Improvement: original rendering does not line up with the grid on either difficulty, but ours does
	// The differences are marked below
	for (uint y = 0; y < _grid.size(); ++y) {
		for (uint x = 0; x < _grid[y].size(); ++x) {
			uint16 cell = _grid[y][x];
			Common::Rect cellRect = getScreenPosition(Common::Point(x, y));
			Common::Point dest(cellRect.left, cellRect.top);

			switch (cell) {
			case kBlock :

				if (_lineWidth != 6) { // Improvement
					dest.x += 1;
					dest.y += 1;
				}

				_drawSurface.blitFrom(_image, _blockSrc, dest);
				break;
			case kWallLeft :
				dest.x -= _lineWidth - _lineWidth / 6;
				dest.y = cellRect.top + (cellRect.height() - _verticalWallSrc.height()) / 2;
				_drawSurface.blitFrom(_image, _verticalWallSrc, dest);

				break;
			case kWallRight :
				dest.x = cellRect.right - 1 + _lineWidth / 6;
				dest.y = cellRect.top + (cellRect.height() - _verticalWallSrc.height()) / 2;
				_drawSurface.blitFrom(_image, _verticalWallSrc, dest);

				break;
			case kWallUp :
				dest.x += (cellRect.width() - _horizontalWallSrc.width()) / 2;
				dest.y -= _lineWidth - _lineWidth / 6;
				_drawSurface.blitFrom(_image, _horizontalWallSrc, dest);

				break;
			case kWallDown :
				dest.x += (cellRect.width() - _horizontalWallSrc.width()) / 2;
				dest.y = cellRect.bottom - 1 + _lineWidth / 6;

				if (_lineWidth != 6) { // Improvement
					++dest.y;
				}

				_drawSurface.blitFrom(_image, _horizontalWallSrc, dest);

				break;
			default :
				if (cell == 0) {
					continue;
				}

				if (_lineWidth == 6) { // Improvement
					dest.x -= 1;
				} else {
					dest.x += 1;
					dest.y += 1;
				}

				_drawSurface.blitFrom(_image, _homeSrcs[cell - 1], dest);
			}
		}
	}

	_needsRedraw = true;
}

void CollisionPuzzle::handleInput(NancyInput &input) {
	if (_state != kRun) {
		return;
	}

	if (_usesExitButton) {
		if (NancySceneState.getViewport().convertViewportToScreen(_exitButtonDest).contains(input.mousePos)) {
			g_nancy->_cursor->setCursorType(CursorManager::kHotspot);

			if (input.input & NancyInput::kLeftMouseButtonUp) {
				_drawSurface.blitFrom(_image, _exitButtonSrc, _exitButtonDest);
				_needsRedraw = true;
				g_nancy->_sound->loadSound(_exitButtonSound);
				g_nancy->_sound->playSound(_exitButtonSound);
				_state = kActionTrigger;
			}
			return;
		}
	} else {
		if (NancySceneState.getViewport().convertViewportToScreen(_exitHotspot).contains(input.mousePos)) {
			g_nancy->_cursor->setCursorType(g_nancy->_cursor->_puzzleExitCursor);

			if (input.input & NancyInput::kLeftMouseButtonUp) {
				_state = kActionTrigger;
			}
			return;
		}
	}

	if (_currentlyAnimating != -1) {
		return;
	}

	for (uint i = 0; i < _pieces.size(); ++i) {
		Common::Point checkPos;
		Common::Rect left, right, up, down;
		Common::Rect screenPos = _pieces[i].getScreenPosition();

		if (_pieces[i]._w == _pieces[i]._h) {
			// Width == height, all movement is permitted, hotspots are 10 pixels wide
			left.setWidth(10);
			left.setHeight(screenPos.height() - 20);
			left.moveTo(screenPos.left, screenPos.top + 10);
			right = left;
			right.translate(screenPos.width() - 10, 0);

			up.setHeight(10);
			up.setWidth(screenPos.width() - 20);
			up.moveTo(screenPos.left + 10, screenPos.top);
			down = up;
			down.translate(0, screenPos.height() - 10);
		} else if (_pieces[i]._w > _pieces[i]._h) {
			// Width > height, only left/right movement is permitted, hotspots are the size of 1 cell
			left.setWidth(screenPos.width() / _pieces[i]._w);
			left.setHeight(screenPos.height() / _pieces[i]._h);
			left.moveTo(screenPos.left, screenPos.top);
			right = left;
			right.translate(right.width() * (_pieces[i]._w - 1), 0);
		} else {
			// Width < height, only up/down movement is permitted, hotspots are the size of 1 cell
			up.setWidth(screenPos.width() / _pieces[i]._w);
			up.setHeight(screenPos.height() / _pieces[i]._h);
			up.moveTo(screenPos.left, screenPos.top);
			down = up;
			down.translate(0, down.height() * (_pieces[i]._h - 1));
		}

		if (!left.isEmpty()) {
			if (left.contains(input.mousePos)) {
				checkPos = movePiece(i, kWallLeft);
				if (checkPos != _pieces[i]._gridPos) {
					g_nancy->_cursor->setCursorType(CursorManager::kMoveLeft);

					if (input.input & NancyInput::kLeftMouseButtonUp) {
						_lastPosition = _pieces[i]._gridPos;
						_pieces[i]._gridPos = checkPos;
						_currentlyAnimating = i;
						g_nancy->_sound->playSound(_moveSound);
					}

					return;
				}
			}
		}

		if (!right.isEmpty()) {
			if (right.contains(input.mousePos)) {
				checkPos = movePiece(i, kWallRight);
				if (checkPos != _pieces[i]._gridPos) {
					g_nancy->_cursor->setCursorType(CursorManager::kMoveRight);

					if (input.input & NancyInput::kLeftMouseButtonUp) {
						_lastPosition = _pieces[i]._gridPos;
						_pieces[i]._gridPos = checkPos;
						_currentlyAnimating = i;
						g_nancy->_sound->playSound(_moveSound);
					}

					return;
				}
			}
		}

		if (!up.isEmpty()) {
			if (up.contains(input.mousePos)) {
				checkPos = movePiece(i, kWallUp);
				if (checkPos != _pieces[i]._gridPos) {
					g_nancy->_cursor->setCursorType(CursorManager::kMoveUp);

					if (input.input & NancyInput::kLeftMouseButtonUp) {
						_lastPosition = _pieces[i]._gridPos;
						_pieces[i]._gridPos = checkPos;
						_currentlyAnimating = i;
						g_nancy->_sound->playSound(_moveSound);
					}

					return;
				}
			}
		}

		if (!down.isEmpty()) {
			if (down.contains(input.mousePos)) {
				checkPos = movePiece(i, kWallDown);
				if (checkPos != _pieces[i]._gridPos) {
					g_nancy->_cursor->setCursorType(CursorManager::kMoveDown);

					if (input.input & NancyInput::kLeftMouseButtonUp) {
						_lastPosition = _pieces[i]._gridPos;
						_pieces[i]._gridPos = checkPos;
						_currentlyAnimating = i;
						g_nancy->_sound->playSound(_moveSound);
					}

					return;
				}
			}
		}
	}
}

} // End of namespace Action
} // End of namespace Nancy
