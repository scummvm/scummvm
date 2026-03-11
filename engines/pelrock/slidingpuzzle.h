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

#ifndef PELROCK_SLIDING_PUZZLE_H
#define PELROCK_SLIDING_PUZZLE_H

#include "common/scummsys.h"
#include "graphics/managed_surface.h"

#include "pelrock/events.h"
#include "pelrock/sound.h"

namespace Pelrock {

static const int kPuzzleScreenWidth = 640;
static const int kPuzzleScreenHeight = 400;
static const int kNumPuzzleSizes = 4;
static const int kTileSizes[kNumPuzzleSizes] = {80, 40, 20, 10};
static const byte kEmptyTileColor = 0x0F;

class SlidingPuzzle {
public:
	SlidingPuzzle(PelrockEventManager *eventMan, SoundManager *sound);
	~SlidingPuzzle();

	void run();

	// Persisted across activations so the size cycles each time.
	static byte _sizeIndex;

private:
	PelrockEventManager *_events;
	SoundManager *_sound;

	void playTileSound();

	int _tileSize;
	int _gridWidth;
	int _gridHeight;
	int _totalTiles;       // gridWidth * gridHeight`
	int _emptyPos;         // index of the empty (removed) tile
	uint16 *_tileMap;      // logical position -> original tile index

	Graphics::ManagedSurface _puzzleBuffer;

	void initGrid();
	void drawGridLines();
	void swapTiles(int a, int b);
	void fillEmptyTile(int pos);
	void shuffleLoop();
	void ensureSolvable();
	void puzzleLoop();
	bool handleClick(int screenX, int screenY);
	bool isSolved() const;
	void showOriginalImage();
	void present();
};

} // End of namespace Pelrock

#endif // PELROCK_SLIDING_PUZZLE_H
