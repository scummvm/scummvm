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

#include "engines/nancy/action/collisionpuzzle.h"

namespace Nancy {
namespace Action {

void CollisionPuzzle::init() {
	Common::Rect screenBounds = NancySceneState.getViewport().getBounds();
	_drawSurface.create(screenBounds.width(), screenBounds.height(), g_nancy->_graphicsManager->getInputPixelFormat());
	_drawSurface.clear(g_nancy->_graphicsManager->getTransColor());
	setTransparent(true);
	setVisible(true);
	moveTo(screenBounds);

	g_nancy->_resource->loadImage(_imageName, _image);
	_image.setTransparentColor(_drawSurface.getTransparentColor());

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

	drawGrid();
	registerGraphics();
}

void CollisionPuzzle::registerGraphics() {
	for (uint i = 0; i < _pieces.size(); ++i) {
		_pieces[i].registerGraphics();
	}

	RenderActionRecord::registerGraphics();
}

void CollisionPuzzle::updateGraphics() {
	if (_state == kRun && _currentlyAnimating != -1) {
		// Framerate-dependent animation. Should be fine since we limit the engine to ~60fps
		++_currentAnimFrame;
		bool horizontal = _lastPosition.x != _pieces[_currentlyAnimating]._gridPos.x;
		int diff = horizontal ?
				_lastPosition.x - _pieces[_currentlyAnimating]._gridPos.x :
				_lastPosition.y - _pieces[_currentlyAnimating]._gridPos.y;

		int maxFrames = _framesPerMove * abs(diff);
		if (_currentAnimFrame > maxFrames) {
			if (_grid[_pieces[_currentlyAnimating]._gridPos.y][_pieces[_currentlyAnimating]._gridPos.x] == _currentlyAnimating + 1) {
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

void CollisionPuzzle::readData(Common::SeekableReadStream &stream) {
	readFilename(stream, _imageName);

	uint16 width = stream.readUint16LE();
	uint16 height = stream.readUint16LE();
	uint16 numPieces = stream.readUint16LE();

	_grid.resize(height, Common::Array<uint16>(width));
	for (uint y = 0; y < height; ++y) {
		for (uint x = 0; x < width; ++x) {
			_grid[y][x] = stream.readUint16LE();
		}
		stream.skip((8 - width) * 2);
	}
	stream.skip((8 - height) * 8 * 2);

	_startLocations.resize(numPieces);
	for (uint i = 0; i < numPieces; ++i) {
		_startLocations[i].x = stream.readUint16LE(); 
		_startLocations[i].y = stream.readUint16LE();
	}
	stream.skip((5 - numPieces) * 4);

	readRectArray(stream, _pieceSrcs, numPieces);
	stream.skip((5 - numPieces) * 16);

	readRectArray(stream, _homeSrcs, numPieces);
	stream.skip((5 - numPieces) * 16);
	
	readRect(stream, _verticalWallSrc);
	readRect(stream, _horizontalWallSrc);
	readRect(stream, _blockSrc);

	_gridPos.x = stream.readUint32LE();
	_gridPos.y = stream.readUint32LE();

	_lineWidth = stream.readUint16LE();
	_framesPerMove = stream.readUint16LE();

	stream.skip(3);

	_moveSound.readNormal(stream);
	_homeSound.readNormal(stream);
	_wallHitSound.readNormal(stream);

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
		_state = kRun;
		// fall through
	case kRun :
		if (_currentlyAnimating != -1) {
			return;
		}

		for (uint i = 0; i < _pieces.size(); ++i) {
			if (_grid[_pieces[i]._gridPos.y][_pieces[i]._gridPos.x] != i + 1) {
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

	int lastPos = inc > 0 ? (horizontal ? (int)_grid[0].size() : (int)_grid.size()) : -1;
	for (int i = (horizontal ? newPos.x : newPos.y) + inc; i != lastPos; i += inc) {
		// First, check if other pieces would block
		Common::Point comparePos = newPos;
		if (horizontal) {
			comparePos.x = i;
		} else {
			comparePos.y = i;
		}

		for (uint j = 0; j < _pieces.size(); ++j) {
			if (pieceID == j) {
				continue;
			}

			if (_pieces[j]._gridPos == comparePos) {
				done = true;
				break;
			}
		}

		if (done) {
			break;
		}

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

		if (horizontal) {
			newPos.x = i;
		} else {
			newPos.y = i;
		}
	}

	return newPos;
}

Common::Rect CollisionPuzzle::getScreenPosition(Common::Point gridPos) {
	Common::Rect dest = _pieces[0]._drawSurface.getBounds();
	dest.right -= 1;
	dest.bottom -= 1;
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

				if (_lineWidth != 6) {  // Improvement
					++dest.y;
				}

				_drawSurface.blitFrom(_image, _horizontalWallSrc, dest);

				break;
			default :
				if (cell == 0) {
					continue;
				}

				if (_lineWidth == 6) {  // Improvement
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

	if (NancySceneState.getViewport().convertViewportToScreen(_exitHotspot).contains(input.mousePos)) {
		// For some reason, this puzzle uses the backwards arrow for exit
		g_nancy->_cursorManager->setCursorType(CursorManager::kMoveBackward);

		if (input.input & NancyInput::kLeftMouseButtonUp) {
			_state = kActionTrigger;
		}
		return;
	}

	Common::Rect left, right, up, down;
	left.setWidth(10);
	left.setHeight(_pieceSrcs[0].height() - 20);
	left.moveTo(0, 10);
	right = left;
	right.translate(_pieceSrcs[0].width() - 10, 0);

	up.setHeight(10);
	up.setWidth(_pieceSrcs[0].width() - 20);
	up.moveTo(10, 0);
	down = up;
	down.translate(0, _pieceSrcs[0].width() - 10);

	for (uint i = 0; i < _pieces.size(); ++i) {
		Common::Rect gridPos = getScreenPosition(_pieces[i]._gridPos);

		left.translate(gridPos.left, gridPos.top);
		right.translate(gridPos.left, gridPos.top);
		up.translate(gridPos.left, gridPos.top);
		down.translate(gridPos.left, gridPos.top);

		Common::Point checkPos = movePiece(i, kWallLeft);
		if (checkPos != _pieces[i]._gridPos) {
			if (NancySceneState.getViewport().convertViewportToScreen(left).contains(input.mousePos)) {
				g_nancy->_cursorManager->setCursorType(CursorManager::kTurnLeft);

				if (input.input & NancyInput::kLeftMouseButtonUp) {
					_lastPosition = _pieces[i]._gridPos;
					_pieces[i]._gridPos = checkPos;
					_currentlyAnimating = i;
					g_nancy->_sound->playSound(_moveSound);
				}

				return;
			}
		}

		checkPos = movePiece(i, kWallRight);
		if (checkPos != _pieces[i]._gridPos) {
			if (NancySceneState.getViewport().convertViewportToScreen(right).contains(input.mousePos)) {
				g_nancy->_cursorManager->setCursorType(CursorManager::kTurnRight);

				if (input.input & NancyInput::kLeftMouseButtonUp) {
					_lastPosition = _pieces[i]._gridPos;
					_pieces[i]._gridPos = checkPos;
					_currentlyAnimating = i;
					g_nancy->_sound->playSound(_moveSound);
				}

				return;
			}
		}

		checkPos = movePiece(i, kWallUp);
		if (checkPos != _pieces[i]._gridPos) {
			if (NancySceneState.getViewport().convertViewportToScreen(up).contains(input.mousePos)) {
				g_nancy->_cursorManager->setCursorType(CursorManager::kMoveUp);

				if (input.input & NancyInput::kLeftMouseButtonUp) {
					_lastPosition = _pieces[i]._gridPos;
					_pieces[i]._gridPos = checkPos;
					_currentlyAnimating = i;
					g_nancy->_sound->playSound(_moveSound);
				}

				return;
			}
		}

		checkPos = movePiece(i, kWallDown);
		if (checkPos != _pieces[i]._gridPos) {
			if (NancySceneState.getViewport().convertViewportToScreen(down).contains(input.mousePos)) {
				g_nancy->_cursorManager->setCursorType(CursorManager::kMoveDown);

				if (input.input & NancyInput::kLeftMouseButtonUp) {
					_lastPosition = _pieces[i]._gridPos;
					_pieces[i]._gridPos = checkPos;
					_currentlyAnimating = i;
					g_nancy->_sound->playSound(_moveSound);
				}

				return;
			}
		}

		left.translate(-gridPos.left, -gridPos.top);
		right.translate(-gridPos.left, -gridPos.top);
		up.translate(-gridPos.left, -gridPos.top);
		down.translate(-gridPos.left, -gridPos.top);
	}
}

} // End of namespace Action
} // End of namespace Nancy
