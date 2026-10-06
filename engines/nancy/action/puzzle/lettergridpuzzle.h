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

#ifndef NANCY_ACTION_LETTERGRIDPUZZLE_H
#define NANCY_ACTION_LETTERGRIDPUZZLE_H

#include "engines/nancy/commontypes.h"
#include "engines/nancy/action/puzzlerecord.h"

namespace Nancy {
namespace Action {

// Grid of clickable cells spelling out a message, new in Nancy14 (AR 159, the
// original's "Grid Map"; unrelated to the Nancy10 GridMapPuzzle). Each column
// stands for one letter, each row for one position in the message strip below
// the grid. Only some cells of a column are clickable. Clicking one marks it and
// writes its column's letter at its row's strip position, clearing any other
// mark in that row; clicking a marked cell clears it. Solved once every row
// matches its target.
//
// The marks are global game state: they survive leaving the puzzle and are only
// reset by a new game. They are kept in GridMapPuzzleData.
class LetterGridPuzzle : public PuzzleRecord {
public:
	LetterGridPuzzle() : PuzzleRecord(7) {}
	virtual ~LetterGridPuzzle() {}

	void init() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

	bool isViewportRelative() const override { return true; }

protected:
	Common::String getRecordTypeName() const override { return "LetterGridPuzzle"; }

	static const uint kMaxColumns = 20;
	static const uint kMaxRows = 20;

	// Row targets
	static const int16 kAnyColumn = -1;		// row is not checked
	static const int16 kNoColumn = -2;		// row must stay unmarked

	struct Column {
		Common::Rect letterSrc;			// in the letter image
		Common::Array<int16> rows;		// rows with a clickable cell
	};

	Common::Rect cellRect(uint column, uint row) const;
	Common::Rect stripRect(uint row) const;
	bool cellAt(const Common::Point &point, uint &column, uint &row) const;
	void toggleCell(uint column, uint row);
	void checkSolved();
	void redraw();

	// -- File data --
	Common::Path _letterImageName;
	Common::Path _markImageName;

	uint16 _hoverCursorType = 0;	// raw Nancy14 cursor type, over a clickable cell

	Common::Rect _markSrc;			// in the mark image; also sets the cell size
	Common::Point _gridOrigin;
	int16 _spacingY = 0;
	int16 _spacingX = 0;

	Common::Array<Column> _columns;
	Common::Array<int16> _targets;	// per row: target column, kAnyColumn or kNoColumn

	Common::Point _stripOrigin;		// strip cells are as big as the first letter
	int16 _stripSpacing = 0;

	RandomSoundBlock _clickSound;

	RandomSoundBlock _solveSoundBlock;	// plays out before the scene change

	// -- Runtime state --
	Graphics::ManagedSurface _letterImage;
	Graphics::ManagedSurface _markImage;

	Common::Array<int16> _markedColumn;	// per row, -1 = unmarked

	bool _solved = false;
	bool _solveSoundStarted = false;
	bool _exitRequested = false;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_LETTERGRIDPUZZLE_H
