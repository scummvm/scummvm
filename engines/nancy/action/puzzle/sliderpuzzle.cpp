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
#include "engines/nancy/puzzledata.h"

#include "engines/nancy/action/puzzle/sliderpuzzle.h"

#include "engines/nancy/state/scene.h"

namespace Nancy {
namespace Action {

void SliderPuzzle::init() {
	_drawSurface.create(_screenPosition.width(), _screenPosition.height(), g_nancy->_graphics->getInputPixelFormat());
	_drawSurface.clear(g_nancy->_graphics->getTransColor());

	setTransparent(true);

	g_nancy->_resource->loadImage(_imageName, _image);
}

void SliderPuzzle::readData(Common::SeekableReadStream &stream) {
	_puzzleState = (SliderPuzzleData *)NancySceneState.getPuzzleData(SliderPuzzleData::getTag());
	assert(_puzzleState);

	readFilename(stream, _imageName);

	_width = stream.readUint16LE();
	_height = stream.readUint16LE();

	_srcRects.resize(_height);
	for (uint y = 0; y < _height; ++y) {
		readRectArray(stream, _srcRects[y], _width, 6);
	}
	stream.skip((6 - _height) * 6 * 16);

	_destRects.resize(_height);
	for (uint y = 0; y < _height; ++y) {
		readRectArray(stream, _destRects[y], _width, 6);
	}
	stream.skip((6 - _height) * 6 * 16);

	_screenPosition = _destRects[0][0];
	for (uint y = 0; y < _height; ++y) {
		for (uint x = 0; x < _width; ++x) {
			_screenPosition.extend(_destRects[y][x]);
		}
	}

	if (g_nancy->getGameType() >= kGameTypeNancy9) {
		_retainState = stream.readByte();

		_startTileOrder.resize(_height);
		for (uint y = 0; y < _height; ++y) {
			_startTileOrder[y].resize(_width);
			for (uint x = 0; x < _width; ++x) {
				_startTileOrder[y][x] = stream.readSint16LE();
			}
			stream.skip((6 - _width) * 2);
		}
		stream.skip((6 - _height) * 6 * 2);
	} else {
		auto *spuzData = GetEngineData(SPUZ);
		assert(spuzData);
		_startTileOrder = spuzData->tileOrder;
	}

	_correctTileOrder.resize(_height);
	for (uint y = 0; y < _height; ++y) {
		_correctTileOrder[y].resize(_width);
		for (uint x = 0; x < _width; ++x) {
			_correctTileOrder[y][x] = stream.readSint16LE();
		}
		stream.skip((6 - _width) * 2);
	}
	stream.skip((6 - _height) * 6 * 2);

	_clickSound.readNormal(stream);
	_solveExitScene.readData(stream);
	_solveSound.readNormal(stream);
	_exitScene.readData(stream);
	readRect(stream, _exitHotspot);
}

void SliderPuzzle::execute() {
	switch (_state) {
	case kBegin:
		init();
		registerGraphics();
		if (!_puzzleState->playerHasTriedPuzzle || !_retainState) {
			_puzzleState->playerTileOrder = _startTileOrder;
			_puzzleState->playerHasTriedPuzzle = true;
		}

		for (uint y = 0; y < _height; ++y) {
			for (uint x = 0; x < _width; ++x) {
				drawTile(_puzzleState->playerTileOrder[y][x], x, y);
			}
		}

		NancySceneState.setNoHeldItem();

		g_nancy->_sound->loadSound(_clickSound);
		_state = kRun;
		// fall through
	case kRun:
		switch (_solveState) {
		case kNotSolved:
			for (uint y = 0; y < _height; ++y) {
				for (uint x = 0; x < _width; ++x) {
					if (_puzzleState->playerTileOrder[y][x] != _correctTileOrder[y][x]) {
						return;
					}
				}
			}

			g_nancy->_sound->loadSound(_solveSound);
			g_nancy->_sound->playSound(_solveSound);
			_solveState = kWaitForSound;
			break;
		case kWaitForSound:
			if (!g_nancy->_sound->isSoundPlaying(_solveSound)) {
				g_nancy->_sound->stopSound(_solveSound);
				_state = kActionTrigger;
			}

			break;
		}

		break;
	case kActionTrigger:
		switch (_solveState) {
		case kNotSolved:
			_exitScene.execute();
			break;
		case kWaitForSound:
			_solveExitScene.execute();
			_puzzleState->playerHasTriedPuzzle = false;
			break;
		}

		g_nancy->_sound->stopSound(_clickSound);
		finishExecution();
	}
}

void SliderPuzzle::handleInput(NancyInput &input) {
	if (_state != kRun || _solveState != kNotSolved) {
		return;
	}

	if (NancySceneState.getViewport().convertViewportToScreen(_exitHotspot).contains(input.mousePos)) {
		g_nancy->_cursor->setCursorType(g_nancy->_cursor->_puzzleExitCursor);

		if (input.input & NancyInput::kLeftMouseButtonUp) {
			_state = kActionTrigger;
		}

		return;
	}

	int currentTileX = -1;
	int currentTileY = -1;
	uint direction = 0;
	for (uint y = 0; y < _height; ++y) {
		bool shouldBreak = false;
		for (uint x = 0; x < _width; ++x) {
			if (x > 0 && _puzzleState->playerTileOrder[y][x - 1] < 0) {
				if (NancySceneState.getViewport().convertViewportToScreen(_destRects[y][x]).contains(input.mousePos)) {
					currentTileX = x;
					currentTileY = y;
					direction = kLeft;
					shouldBreak = true;
					break;
				}
			} else if ((int)x < _width - 1 && _puzzleState->playerTileOrder[y][x + 1] < 0) {
				if (NancySceneState.getViewport().convertViewportToScreen(_destRects[y][x]).contains(input.mousePos)) {
					currentTileX = x;
					currentTileY = y;
					direction = kRight;
					shouldBreak = true;
					break;
				}
			} else if (y > 0 && _puzzleState->playerTileOrder[y - 1][x] < 0) {
				if (NancySceneState.getViewport().convertViewportToScreen(_destRects[y][x]).contains(input.mousePos)) {
					currentTileX = x;
					currentTileY = y;
					direction = kUp;
					shouldBreak = true;
					break;
				}
			} else if ((int)y < _height - 1 && _puzzleState->playerTileOrder[y + 1][x] < 0) {
				if (NancySceneState.getViewport().convertViewportToScreen(_destRects[y][x]).contains(input.mousePos)) {
					currentTileX = x;
					currentTileY = y;
					direction = kDown;
					shouldBreak = true;
					break;
				}
			}
		}

		if (shouldBreak) {
			break;
		}
	}

	if (currentTileX != -1) {
		g_nancy->_cursor->setCursorType(CursorManager::kHotspot);

		if (!g_nancy->_sound->isSoundPlaying(_clickSound) && input.input & NancyInput::kLeftMouseButtonUp) {
			g_nancy->_sound->playSound(_clickSound);
			switch (direction) {
			case kUp: {
				uint curTileID = _puzzleState->playerTileOrder[currentTileY][currentTileX];
				drawTile(curTileID, currentTileX, currentTileY - 1);
				undrawTile(currentTileX, currentTileY);
				_puzzleState->playerTileOrder[currentTileY - 1][currentTileX] = curTileID;
				_puzzleState->playerTileOrder[currentTileY][currentTileX] = -10;
				break;
			}
			case kDown: {
				uint curTileID = _puzzleState->playerTileOrder[currentTileY][currentTileX];
				drawTile(curTileID, currentTileX, currentTileY + 1);
				undrawTile(currentTileX, currentTileY);
				_puzzleState->playerTileOrder[currentTileY + 1][currentTileX] = curTileID;
				_puzzleState->playerTileOrder[currentTileY][currentTileX] = -10;
				break;
			}
			case kLeft: {
				uint curTileID = _puzzleState->playerTileOrder[currentTileY][currentTileX];
				drawTile(curTileID, currentTileX - 1, currentTileY);
				undrawTile(currentTileX, currentTileY);
				_puzzleState->playerTileOrder[currentTileY][currentTileX - 1] = curTileID;
				_puzzleState->playerTileOrder[currentTileY][currentTileX] = -10;
				break;
			}
			case kRight: {
				uint curTileID = _puzzleState->playerTileOrder[currentTileY][currentTileX];
				drawTile(curTileID, currentTileX + 1, currentTileY);
				undrawTile(currentTileX, currentTileY);
				_puzzleState->playerTileOrder[currentTileY][currentTileX + 1] = curTileID;
				_puzzleState->playerTileOrder[currentTileY][currentTileX] = -10;
				break;
			}
			}
		}
	}
}

void SliderPuzzle::drawTile(int tileID, uint posX, uint posY) {
	if (tileID < 0) {
		undrawTile(posX, posY);
		return;
	}

	Common::Point destPoint(_destRects[posY][posX].left - _screenPosition.left, _destRects[posY][posX].top - _screenPosition.top);
	_drawSurface.blitFrom(_image, _srcRects[tileID / _height][tileID % _width], destPoint);

	_needsRedraw = true;
}

void SliderPuzzle::undrawTile(uint posX, uint posY) {
	Common::Rect bounds = _destRects[posY][posX];
	bounds.translate(-_screenPosition.left, -_screenPosition.top);
	_drawSurface.fillRect(bounds, g_nancy->_graphics->getTransColor());

	_needsRedraw = true;
}

} // End of namespace Action
} // End of namespace Nancy
