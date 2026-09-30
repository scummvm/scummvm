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

#ifndef NANCY_ACTION_ROWMATCHPUZZLE_H
#define NANCY_ACTION_ROWMATCHPUZZLE_H

#include "engines/nancy/commontypes.h"
#include "engines/nancy/util.h"
#include "engines/nancy/action/puzzlerecord.h"

namespace Nancy {
namespace Action {

// Nancy16 secret passage puzzle (AR 192). Tiles are stacked in columns; pushing the
// spare tile into the top of a column shifts its tiles down, and the one at the bottom
// drops out, ready to be picked up and pushed into another column. The puzzle is solved
// when every column is in order and the original spare tile is back in its place.
class RowMatchPuzzle : public PuzzleRecord {
public:
	RowMatchPuzzle() : PuzzleRecord(7) {}
	virtual ~RowMatchPuzzle() {}

	void init() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

	bool isViewportRelative() const override { return true; }

protected:
	Common::String getRecordTypeName() const override { return "RowMatchPuzzle"; }

	static const int kNoTile = -1;

	struct Slot {
		int startTile = kNoTile;
		int solutionTile = kNoTile;
		int tile = kNoTile;
		Common::Rect dest;
	};

	struct Column {
		Common::Array<Slot> slots;
		Common::Rect insertHotspot;
		Common::Rect droppedTileDest;	// also where the dropped tile is picked up
		int droppedTile = kNoTile;
	};

	int getTile(int16 id) const { return id >= 0 && id < (int)_tileSrcs.size() ? id : kNoTile; }
	bool isSolved() const;
	void setCursor(const Common::Point &pos);
	void holdTile(int tile);
	void releaseTile();
	void drawTile(int tile, const Common::Point &pos);

	void redraw();

	// -- File data --
	Common::Array<Common::Rect> _tileSrcs;
	Common::Rect _spareDest;
	int _spareSolutionTile = kNoTile;
	Common::Array<Column> _columns;

	uint16 _insertCursor = 0;
	uint16 _takeCursor = 0;
	uint16 _heldTileCursor = 0;
	RandomSoundBlock _insertSound;
	RandomSoundBlock _takeSound;
	RandomSoundBlock _solveSoundBlock;
	Common::Array<ExitHotspot> _exits;

	// -- Runtime state --
	int _spareTile = kNoTile;
	int _heldTile = kNoTile;
	Common::Point _mousePos;

	bool _isSolved = false;
	bool _exitRequested = false;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_ROWMATCHPUZZLE_H
