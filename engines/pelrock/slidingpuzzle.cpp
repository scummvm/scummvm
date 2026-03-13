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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/system.h"
#include "graphics/paletteman.h"

#include "pelrock/slidingpuzzle.h"
#include "pelrock/pelrock.h"
#include "pelrock/events.h"
#include "pelrock/room.h"
#include "pelrock/sound.h"

namespace Pelrock {

byte SlidingPuzzle::_sizeIndex = 0;

SlidingPuzzle::SlidingPuzzle(PelrockEventManager *eventMan, SoundManager *sound)
	: _events(eventMan),
	  _sound(sound),
	  _tileSize(0),
	  _gridWidth(0),
	  _gridHeight(0),
	  _totalTiles(0),
	  _emptyPos(-1),
	  _tileMap(nullptr) {
}

SlidingPuzzle::~SlidingPuzzle() {
	delete[] _tileMap;
	_puzzleBuffer.free();
}

void SlidingPuzzle::run() {
	// calculate grid
	_tileSize   = kTileSizes[_sizeIndex];
	_gridWidth  = kPuzzleScreenWidth / _tileSize;
	_gridHeight = kPuzzleScreenHeight / _tileSize;
	_totalTiles = _gridWidth * _gridHeight;

	initGrid();

	// Copy the current scene into our working buffer.
	_puzzleBuffer.create(kPuzzleScreenWidth, kPuzzleScreenHeight, Graphics::PixelFormat::createFormatCLUT8());
	_puzzleBuffer.copyFrom(g_engine->_compositeBuffer);

	drawGridLines();
	present();

	_events->_lastKeyEvent = Common::KEYCODE_INVALID;

	//Shuffle
	shuffleLoop();

	// Guarantee the puzzle is solvable, fix it otherwise
	ensureSolvable();

	// Remove the last tile to create the empty slot (tile_count = total - 1).
	_emptyPos = _totalTiles - 1;
	fillEmptyTile(_emptyPos);

	g_engine->changeCursor(DEFAULT);
	present();

	puzzleLoop();

	// Cycle to the next tile size for the next activation.
	_sizeIndex = (_sizeIndex + 1) % kNumPuzzleSizes;
}

// After shuffling, ensure the resulting permutation is reachable from the solved state.
// For all our grid widths (8, 16, 32, 64 — all even) with the blank forced to the
// bottom-right corner (row-from-bottom = 0), the solvability condition is:
//   inversions in _tileMap[0 .. N-2] must be even.
// Each tile swap flips parity, so if we end up with an odd count, one extra swap fixes it.
void SlidingPuzzle::ensureSolvable() {
	int inversions = 0;
	for (int i = 0; i < _totalTiles - 1; i++) {
		for (int j = i + 1; j < _totalTiles - 1; j++) {
			if (_tileMap[i] > _tileMap[j])
				inversions++;
		}
	}
	if (inversions & 1)
		swapTiles(0, 1);
}

// Set up the identity permutation: tile i is in slot i.
void SlidingPuzzle::initGrid() {
	_tileMap = new uint16[_totalTiles];
	for (uint16 i = 0; i < _totalTiles; i++)
		_tileMap[i] = (uint16)i;
}

/**
 * Original draws grid lines to make tile separation evident.
 */
void SlidingPuzzle::drawGridLines() {
	for (int col = 0; col < _gridWidth; col++) {
		int x = col * _tileSize;
		_puzzleBuffer.drawLine(x, 0, x, kPuzzleScreenHeight - 1, kEmptyTileColor);
	}
	for (int row = 0; row < _gridHeight; row++) {
		int y = row * _tileSize;
		_puzzleBuffer.drawLine(0, y, kPuzzleScreenWidth - 1, y, kEmptyTileColor);
	}
}

void SlidingPuzzle::swapTiles(int a, int b) {
	SWAP(_tileMap[a], _tileMap[b]);

	// Pixel coordinates: skip the 1px grid line at top/left of each tile.
	int ax = (a % _gridWidth) * _tileSize + 1;
	int ay = (a / _gridWidth) * _tileSize + 1;
	int bx = (b % _gridWidth) * _tileSize + 1;
	int by = (b / _gridWidth) * _tileSize + 1;
	int inner = _tileSize - 1;

	for (int row = 0; row < inner; row++) {
		byte *pA = (byte *)_puzzleBuffer.getBasePtr(ax, ay + row);
		byte *pB = (byte *)_puzzleBuffer.getBasePtr(bx, by + row);
		for (int col = 0; col < inner; col++)
			SWAP(pA[col], pB[col]);
	}
}

void SlidingPuzzle::fillEmptyTile(int pos) {
	int ex = (pos % _gridWidth) * _tileSize + 1;
	int ey = (pos / _gridWidth) * _tileSize + 1;
	int inner = _tileSize - 1;
	_puzzleBuffer.fillRect(Common::Rect(ex, ey, ex + inner, ey + inner), kEmptyTileColor);
}

void SlidingPuzzle::present() {
	g_engine->_screen->blitFrom(_puzzleBuffer);
	g_engine->_screen->markAllDirty();
	g_engine->_screen->update();
}

void SlidingPuzzle::playTileSound() {
	_sound->playSound("11ZZZZZZ.SMP", -1);
}

void SlidingPuzzle::shuffleLoop() {
	const int shuffleRange = _totalTiles - 1; // never touch the blank slot (N-1)

	// scale tile swap with tile count
	const int kBaseTileCount = (kPuzzleScreenWidth / kTileSizes[0]) *
	                           (kPuzzleScreenHeight / kTileSizes[0]);
	const int swapsPerFrame = MAX(1, _totalTiles / kBaseTileCount);

	while (!g_engine->shouldQuit()) {
		_events->pollEvent();

		for (int s = 0; s < swapsPerFrame; s++) {
			int a, b;
			do {
				a = g_engine->getRandomNumber(shuffleRange - 1);
				b = g_engine->getRandomNumber(shuffleRange - 1);
			} while (a == b);

			swapTiles(a, b);
		}
		present();
		playTileSound();

		if (_events->_lastKeyEvent != Common::KEYCODE_INVALID) {
			_events->_lastKeyEvent = Common::KEYCODE_INVALID;
			break;
		}
		g_system->delayMillis(10);
	}
}

void SlidingPuzzle::puzzleLoop() {
	_events->_lastKeyEvent = Common::KEYCODE_INVALID;
	_events->_isKeydown = false;

	while (!g_engine->shouldQuit()) {
		_events->pollEvent();

		if (_events->_leftMouseClicked) {
			_events->_leftMouseClicked = false;
			if (handleClick(_events->_mouseClickX, _events->_mouseClickY))
				break;
		}

		if (_events->_lastKeyEvent == Common::KEYCODE_ESCAPE) {
			break;
		}

		// While any key is held, show the original (unscrambled) background.
		if (_events->_isKeydown && _events->_lastKeyEvent != Common::KEYCODE_ESCAPE) {
			showOriginalImage();
		}
		_events->_lastKeyEvent = Common::KEYCODE_INVALID;

		present();
		g_system->delayMillis(10);
	}
}

bool SlidingPuzzle::handleClick(int screenX, int screenY) {
	int col = screenX / _tileSize;
	int row = screenY / _tileSize;
	if (col >= _gridWidth || row >= _gridHeight)
		return false;

	int clicked = row * _gridWidth + col;
	if (clicked == _emptyPos)
		return false;

	// Check adjacency without row wrap-around.
	int emptyRow = _emptyPos / _gridWidth;
	int emptyCol = _emptyPos % _gridWidth;
	bool adjacent = (row == emptyRow && abs(col - emptyCol) == 1) ||
	                (col == emptyCol && abs(row - emptyRow) == 1);
	if (!adjacent)
		return false;

	swapTiles(clicked, _emptyPos);
	_emptyPos = clicked;
	fillEmptyTile(_emptyPos);
	playTileSound();
	present();

	if (isSolved()) {
		_sound->playSound("CHIQUITO.WAV", 3);
		while (!g_engine->shouldQuit() && _sound->isPlaying(3)) {
			_events->pollEvent();
			g_system->delayMillis(10);
		}
		_sound->stopSound(3);
		return true;
	}

	return false;
}

bool SlidingPuzzle::isSolved() const {
	for (int i = 0; i < _totalTiles - 1; i++) {
		if (_tileMap[i] != (uint16)i)
			return false;
	}
	return true;
}

// show the original background while a key is held.
void SlidingPuzzle::showOriginalImage() {
	// original game showed only the room background, we show the entire scene
	g_engine->_screen->blitFrom(g_engine->_compositeBuffer);
	g_engine->_screen->markAllDirty();
	g_engine->_screen->update();

	while (!g_engine->shouldQuit()) {
		_events->pollEvent();
		if (_events->_isKeydown == false)
			break;
		g_system->delayMillis(10);
	}
}

} // End of namespace Pelrock
