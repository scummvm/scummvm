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

#include "engines/nancy/action/puzzle/mazechasepuzzle.h"

namespace Nancy {
namespace Action {

void MazeChasePuzzle::init() {
	Common::Rect screenBounds = NancySceneState.getViewport().getBounds();
	_drawSurface.create(screenBounds.width(), screenBounds.height(), g_nancy->_graphics->getInputPixelFormat());
	_drawSurface.clear(g_nancy->_graphics->getTransColor());
	setTransparent(true);
	setVisible(true);
	moveTo(screenBounds);

	g_nancy->_resource->loadImage(_imageName, _image);
	_image.setTransparentColor(_drawSurface.getTransparentColor());

	for (uint i = 0; i < _startLocations.size(); ++i) {
		_pieces.push_back(Piece(_z + i + 1));
		_pieces[i]._drawSurface.create(_image, i == 0 ? _playerSrc : _enemySrc);
		Common::Rect pos = getScreenPosition(_startLocations[i]);
		_pieces[i].moveTo(pos);
		_pieces[i]._gridPos = _startLocations[i];
		_pieces[i]._lastPos = _pieces[i]._gridPos;
		_pieces[i].setVisible(true);
		_pieces[i].setTransparent(true);
	}

	if (NancySceneState.getEventFlag(_solveScene._flag)) {
		_drawSurface.blitFrom(_image, _lightSrc, _lightDest);
	}

	drawGrid();
	registerGraphics();
}

void MazeChasePuzzle::registerGraphics() {
	for (uint i = 0; i < _pieces.size(); ++i) {
		_pieces[i].registerGraphics();
	}
	RenderActionRecord::registerGraphics();
}

void MazeChasePuzzle::updateGraphics() {
	if (_currentAnimFrame != -1) {
		if (g_nancy->_sound->isSoundPlaying(_moveSound) || g_nancy->_sound->isSoundPlaying(_failSound)) {
			return;
		}

		// Framerate-dependent animation. Should be fine since we limit the engine to ~60fps
		++_currentAnimFrame;

		if (_reset) {
			reset();
			return;
		}

		for (uint i = 0; i < _pieces.size(); ++i) {
			Piece &cur = _pieces[i];
			if (cur._gridPos != cur._lastPos) {
				bool horizontal = cur._gridPos.x != cur._lastPos.x;

				Common::Rect destRect = getScreenPosition(cur._lastPos);
				Common::Rect endPos = getScreenPosition(cur._gridPos);

				// Make sure to adjust the frame id for enemies
				int frame = (i == 0) ? _currentAnimFrame : _currentAnimFrame - 1;

				Common::Point dest(destRect.left, destRect.top);
				if (horizontal) {
					dest.x = destRect.left + (endPos.left - dest.x) * frame / _framesPerMove;
				} else {
					dest.y = destRect.top + (endPos.top - dest.y) * frame / _framesPerMove;
				}

				cur.moveTo(dest);

				if (frame == _framesPerMove) {
					cur._lastPos = cur._gridPos;
				}
			}
		}

		if (_currentAnimFrame > 0) {
			if (!_solved) {
				// Make sure not to move pieces when the player is about to lose
				bool playerRanIntoEnemy = false;
				for (uint i = 1; i < _pieces.size(); ++i) {
					if (_pieces[0]._gridPos == _pieces[i]._gridPos) {
						playerRanIntoEnemy = true;
						break;
					}
				}

				if (!playerRanIntoEnemy) {
					// Each enemy moves one frame after the last one
					enemyMovement(_currentAnimFrame);
				}
			}

			if (_currentAnimFrame == 1) {
				// Clear the buttons
				Common::Rect fill = _upButtonDest;
				fill.extend(_downButtonDest);
				fill.extend(_leftButtonDest);
				fill.extend(_rightButtonDest);
				fill.extend(_resetButtonDest);
				_drawSurface.fillRect(fill, _drawSurface.getTransparentColor());
				_needsRedraw = true;
			} else if (_currentAnimFrame >= _framesPerMove + 1) {
				_currentAnimFrame = -1;
			}
		}
	}
}

void MazeChasePuzzle::readData(Common::SeekableReadStream &stream) {
	readFilename(stream, _imageName);

	uint width = stream.readUint16LE();
	uint height = stream.readUint16LE();
	uint numEnemies = stream.readUint16LE();

	_exitPos.x = stream.readUint16LE();
	_exitPos.y = stream.readUint16LE();

	_grid.resize(height, Common::Array<uint16>(width));
	for (uint y = 0; y < height; ++y) {
		for (uint x = 0; x < width; ++x) {
			_grid[y][x] = stream.readUint16LE();
		}
		stream.skip((8 - width) * 2);
	}
	stream.skip((8 - height) * 8 * 2);

	_startLocations.resize(numEnemies + 1);
	for (uint i = 0; i < _startLocations.size(); ++i) {
		_startLocations[i].x = stream.readUint16LE();
		_startLocations[i].y = stream.readUint16LE();
	}

	readRect(stream, _playerSrc);
	readRect(stream, _enemySrc);
	readRect(stream, _verticalWallSrc);
	readRect(stream, _horizontalWallSrc);
	readRect(stream, _lightSrc);

	readRect(stream, _upButtonSrc);
	readRect(stream, _rightButtonSrc);
	readRect(stream, _downButtonSrc);
	readRect(stream, _leftButtonSrc);
	readRect(stream, _resetButtonSrc);

	_gridPos.x = stream.readUint32LE();
	_gridPos.y = stream.readUint32LE();

	readRect(stream, _lightDest);

	readRect(stream, _upButtonDest);
	readRect(stream, _rightButtonDest);
	readRect(stream, _downButtonDest);
	readRect(stream, _leftButtonDest);
	readRect(stream, _resetButtonDest);

	_lineWidth = stream.readUint16LE();
	_framesPerMove = stream.readUint16LE();

	_failSound.readNormal(stream);
	_moveSound.readNormal(stream);

	_solveScene.readData(stream);
	_solveSoundDelay = stream.readUint16LE();
	_solveSound.readNormal(stream);

	_exitScene.readData(stream);
	readRect(stream, _exitHotspot);
}

void MazeChasePuzzle::execute() {
	switch (_state) {
	case kBegin :
		init();
		g_nancy->_sound->loadSound(_moveSound);
		g_nancy->_sound->loadSound(_failSound);
		_state = kRun;
		// fall through
	case kRun :
		if (_currentAnimFrame != -1) {
			return;
		}

		if (_pieces[0]._gridPos == _exitPos) {
			_pieces[0]._gridPos = _exitPos + Common::Point(_exitPos.x == 0 ? -1 : 1, 0);
			++_currentAnimFrame;
			g_nancy->_sound->loadSound(_solveSound);
			g_nancy->_sound->playSound(_solveSound);
			_solved = true;
			_state = kActionTrigger;
		} else {
			for (uint i = 1; i < _pieces.size(); ++i) {
				if (_pieces[i]._gridPos == _pieces[0]._gridPos) {
					g_nancy->_sound->playSound(_failSound);
					++_currentAnimFrame;
					_reset = true;
				}
			}
		}

		return;
	case kActionTrigger :
		if (_solved) {
			if (g_nancy->_sound->isSoundPlaying(_solveSound)) {
				return;
			}

			if (_solveSoundPlayTime == 0) {
				_solveSoundPlayTime = g_nancy->getTotalPlayTime() + _solveSoundDelay * 1000;
				return;
			} else if (_solveSoundPlayTime < g_nancy->getTotalPlayTime()) {
				_solveScene.execute();
			} else {
				return;
			}
		} else {
			_exitScene.execute();
		}

		g_nancy->_sound->stopSound(_solveSound);
		g_nancy->_sound->stopSound(_moveSound);
		g_nancy->_sound->stopSound(_failSound);

		finishExecution();
	}
}

void MazeChasePuzzle::handleInput(NancyInput &input) {
	if (_state != kRun || _solved) {
		return;
	}

	if (NancySceneState.getViewport().convertViewportToScreen(_exitHotspot).contains(input.mousePos)) {
		g_nancy->_cursor->setCursorType(g_nancy->_cursor->_puzzleExitCursor);

		if (input.input & NancyInput::kLeftMouseButtonUp) {
			_state = kActionTrigger;
		}
		return;
	}

	if (_currentAnimFrame != -1) {
		return;
	}

	Common::Rect buttonHotspot = _upButtonDest;
	buttonHotspot.grow(-10);

	if (NancySceneState.getViewport().convertViewportToScreen(buttonHotspot).contains(input.mousePos)) {
		if (canMove(0, kWallUp)) {
			g_nancy->_cursor->setCursorType(CursorManager::kHotspot);

			if (input.input & NancyInput::kLeftMouseButtonUp) {
				--_pieces[0]._gridPos.y;
				++_currentAnimFrame;
				g_nancy->_sound->playSound(_moveSound);
				_drawSurface.blitFrom(_image, _upButtonSrc, _upButtonDest);
				_needsRedraw = true;
			}
		}

		return;
	}

	buttonHotspot = _rightButtonDest;
	buttonHotspot.grow(-10);

	if (NancySceneState.getViewport().convertViewportToScreen(buttonHotspot).contains(input.mousePos)) {
		if (canMove(0, kWallRight)) {
			g_nancy->_cursor->setCursorType(CursorManager::kHotspot);

			if (input.input & NancyInput::kLeftMouseButtonUp) {
				++_pieces[0]._gridPos.x;
				++_currentAnimFrame;
				g_nancy->_sound->playSound(_moveSound);
				_drawSurface.blitFrom(_image, _rightButtonSrc, _rightButtonDest);
				_needsRedraw = true;
			}
		}

		return;
	}

	buttonHotspot = _downButtonDest;
	buttonHotspot.grow(-10);

	if (NancySceneState.getViewport().convertViewportToScreen(buttonHotspot).contains(input.mousePos)) {
		if (canMove(0, kWallDown)) {
			g_nancy->_cursor->setCursorType(CursorManager::kHotspot);

			if (input.input & NancyInput::kLeftMouseButtonUp) {
				++_pieces[0]._gridPos.y;
				++_currentAnimFrame;
				g_nancy->_sound->playSound(_moveSound);
				_drawSurface.blitFrom(_image, _downButtonSrc, _downButtonDest);
				_needsRedraw = true;
			}
		}

		return;
	}

	buttonHotspot = _leftButtonDest;
	buttonHotspot.grow(-10);

	if (NancySceneState.getViewport().convertViewportToScreen(buttonHotspot).contains(input.mousePos)) {
		if (canMove(0, kWallLeft)) {
			g_nancy->_cursor->setCursorType(CursorManager::kHotspot);

			if (input.input & NancyInput::kLeftMouseButtonUp) {
				--_pieces[0]._gridPos.x;
				++_currentAnimFrame;
				g_nancy->_sound->playSound(_moveSound);
				_drawSurface.blitFrom(_image, _leftButtonSrc, _leftButtonDest);
				_needsRedraw = true;
			}
		}

		return;
	}

	buttonHotspot = _resetButtonDest;
	buttonHotspot.grow(-10);

	if (NancySceneState.getViewport().convertViewportToScreen(buttonHotspot).contains(input.mousePos)) {
		g_nancy->_cursor->setCursorType(CursorManager::kHotspot);

		if (input.input & NancyInput::kLeftMouseButtonUp) {
			++_currentAnimFrame;
			g_nancy->_sound->playSound(_moveSound);
			_drawSurface.blitFrom(_image, _resetButtonSrc, _resetButtonDest);
			_needsRedraw = true;
			_reset = true;
		}
	}

	return;
}

Common::Rect MazeChasePuzzle::getScreenPosition(Common::Point gridPos) {
	Common::Rect dest = _playerSrc;

	dest.moveTo(0, 0);

	dest.right -= 1;
	dest.bottom -= 1;

	dest.moveTo(_gridPos);
	dest.translate(gridPos.x * _lineWidth, gridPos.y * _lineWidth);
	dest.translate(gridPos.x * dest.width(), gridPos.y *dest.height());

	if (gridPos.x < 0 || gridPos.x >= (int)_grid[0].size()) {
		// Make sure the end position is in the middle of the dancers
		dest.translate(12, 0);
	}

	dest.right += 1;
	dest.bottom += 1;

	return dest;
}

void MazeChasePuzzle::drawGrid() {
	for (uint y = 0; y < _grid.size(); ++y) {
		for (uint x = 0; x < _grid[y].size(); ++x) {
			uint16 cell = _grid[y][x];
			Common::Rect cellRect = getScreenPosition(Common::Point(x, y));
			Common::Point dest(cellRect.left, cellRect.top);

			if (cell == kWallUp || cell == kWallUpDown) {
				_drawSurface.blitFrom(_image, _horizontalWallSrc, dest - Common::Point(0, _lineWidth));
			}

			if (cell == kWallDown || cell == kWallUpDown) {
				_drawSurface.blitFrom(_image, _horizontalWallSrc, dest + Common::Point(0, cellRect.height() - 1));
			}

			if (cell == kWallLeft || cell == kWallLeftRight) {
				_drawSurface.blitFrom(_image, _verticalWallSrc, dest - Common::Point(_lineWidth, 0));
			}

			if (cell == kWallRight || cell == kWallLeftRight) {
				_drawSurface.blitFrom(_image, _verticalWallSrc, dest + Common::Point(cellRect.width() - 1, 0));
			}
		}
	}

	_needsRedraw = true;
}

void MazeChasePuzzle::enemyMovement(uint enemyID) {
	if (enemyID >= _pieces.size()) {
		return;
	}

	Piece &player = _pieces[0];
	Piece &enemy = _pieces[enemyID];
	Common::Point diff = player._gridPos - enemy._gridPos;

	// First, try to move vertically
	if (diff.y) {
		if (diff.y > 0) {
			// Player is lower than enemy, try to move down
			if (canMove(enemyID, kWallDown)) {
				++enemy._gridPos.y;
				return;
			}
		} else {
			// Player is higher than enemy, try to move up
			if (canMove(enemyID, kWallUp)) {
				--enemy._gridPos.y;
				return;
			}
		}
	}

	// Then, try to move horizontally. Note that when the player is on the same row,
	// the enemy will not move if adjacent to a wall; this is intentional
	if (diff.x) {
		if (diff.x > 0) {
			// Player is to the enemy's right
			if (canMove(enemyID, kWallRight)) {
				++enemy._gridPos.x;
				return;
			}
		} else {
			// Player is to the enemy's left
			if (canMove(enemyID, kWallLeft)) {
				--enemy._gridPos.x;
				return;
			}
		}
	}
}

bool MazeChasePuzzle::canMove(uint pieceID, WallType direction) {
	Piece &piece = _pieces[pieceID];
	switch (direction) {
	case kWallLeft :
		if (	piece._gridPos.x == 0 ||
				_grid[piece._gridPos.y][piece._gridPos.x - 1] == kWallRight ||
				_grid[piece._gridPos.y][piece._gridPos.x - 1] == kWallLeftRight ||
				_grid[piece._gridPos.y][piece._gridPos.x] == kWallLeft ||
				_grid[piece._gridPos.y][piece._gridPos.x] == kWallLeftRight) {
			return false;
		}

		if (pieceID != 0) {
			for (uint i = 1; i < _pieces.size(); ++i) {
				if (piece._gridPos + Common::Point(-1, 0) == _pieces[i]._gridPos) {
					return false;
				}
			}
		}

		return true;
	case kWallRight :
		if (	piece._gridPos.x == (int)_grid[0].size() - 1 ||
				_grid[piece._gridPos.y][piece._gridPos.x + 1] == kWallLeft ||
				_grid[piece._gridPos.y][piece._gridPos.x + 1] == kWallLeftRight ||
				_grid[piece._gridPos.y][piece._gridPos.x] == kWallRight ||
				_grid[piece._gridPos.y][piece._gridPos.x] == kWallLeftRight) {
			return false;
		}

		if (pieceID != 0) {
			for (uint i = 1; i < _pieces.size(); ++i) {
				if (piece._gridPos + Common::Point(1, 0) == _pieces[i]._gridPos) {
					return false;
				}
			}
		}

		return true;
	case kWallUp :
		if (	piece._gridPos.y == 0 ||
				_grid[piece._gridPos.y - 1][piece._gridPos.x] == kWallDown ||
				_grid[piece._gridPos.y - 1][piece._gridPos.x] == kWallUpDown ||
				_grid[piece._gridPos.y][piece._gridPos.x] == kWallUp ||
				_grid[piece._gridPos.y][piece._gridPos.x] == kWallUpDown) {
			return false;
		}

		if (pieceID != 0) {
			for (uint i = 1; i < _pieces.size(); ++i) {
				if (piece._gridPos + Common::Point(0, -1) == _pieces[i]._gridPos) {
					return false;
				}
			}
		}

		return true;
	case kWallDown :
		if (	piece._gridPos.y == (int)_grid.size() - 1 ||
				_grid[piece._gridPos.y + 1][piece._gridPos.x] == kWallUp ||
				_grid[piece._gridPos.y + 1][piece._gridPos.x] == kWallUpDown ||
				_grid[piece._gridPos.y][piece._gridPos.x] == kWallDown ||
				_grid[piece._gridPos.y][piece._gridPos.x] == kWallUpDown) {
			return false;
		}

		if (pieceID != 0) {
			for (uint i = 1; i < _pieces.size(); ++i) {
				if (piece._gridPos + Common::Point(0, 1) == _pieces[i]._gridPos) {
					return false;
				}
			}
		}

		return true;
	default :
		return true;
	}
}

void MazeChasePuzzle::reset() {
	for (uint i = 0; i < _pieces.size(); ++i) {
		_pieces[i]._gridPos = _pieces[i]._lastPos = _startLocations[i];
		_pieces[i].moveTo(getScreenPosition(_pieces[i]._gridPos));
	}

	Common::Rect fill = _upButtonDest;
	fill.extend(_downButtonDest);
	fill.extend(_leftButtonDest);
	fill.extend(_rightButtonDest);
	fill.extend(_resetButtonDest);
	_drawSurface.fillRect(fill, _drawSurface.getTransparentColor());

	_currentAnimFrame = -1;
	_reset = false;
	_needsRedraw = true;
}

} // End of namespace Action
} // End of namespace Nancy
