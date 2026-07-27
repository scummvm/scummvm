/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of their respective copyright holders.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef RIPPER_PUZZLES_WEB_GRID_SHIFT_H
#define RIPPER_PUZZLES_WEB_GRID_SHIFT_H

#include "audio/mixer.h"
#include "common/array.h"
#include "common/random.h"
#include "common/rect.h"

#include "ripper/display.h"

namespace Graphics {
struct Surface;
}

namespace Ripper {

class RipperEngine;

class WebGridShiftPuzzle {
public:
	enum Result {
		kExited,
		kSolved,
		kLoadFailed
	};

	explicit WebGridShiftPuzzle(RipperEngine *engine);

	Result run(uint completionFlag);

private:
	bool loadResources();
	bool captureGrid();
	void drawTileClipped(Graphics::Surface &screen, uint tile, int x, int y,
		const Common::Rect &clip) const;
	void renderOrder(const uint *order) const;
	void renderShift(uint targetCell, uint progress) const;
	bool animateShift(uint targetCell, bool autoPlay);
	bool shiftToCell(uint targetCell, bool autoPlay);
	int targetForDirection(uint direction) const;
	bool runAutoPlay(uint moveLimit, uint completionFlag, Result &result);
	bool showPreview();
	bool waitTicks(uint ticks);
	bool updateCursor(const Common::Point &point);
	int findCell(const Common::Point &point) const;
	bool isSolved() const;
	bool complete(uint completionFlag);
	bool handleKeyword(uint16 command, uint completionFlag, Result &result);
	void playMoveCue();
	void stopAudio();
	Common::String orderString() const;

	RipperEngine *_engine;
	Common::RandomSource _random;
	IndexedDisplaySnapshot _backgroundDisplay;
	Common::Array<byte> _tiles[25];
	uint _tileOrder[25];
	Audio::SoundHandle _audioHandles[3];
	uint _currentRow;
	uint _currentCol;
	uint _keywordProgress;
	int _hoveredCell;
};

} // End of namespace Ripper

#endif // RIPPER_PUZZLES_WEB_GRID_SHIFT_H
