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

#ifndef NANCY_ACTION_BOOKSORTPUZZLE_H
#define NANCY_ACTION_BOOKSORTPUZZLE_H

#include "engines/nancy/commontypes.h"
#include "engines/nancy/action/puzzlerecord.h"

namespace Nancy {
namespace Action {

// Nancy17 book shelf. The books lying around have to be packed into a grid of
// small cells, where each of them covers a block of cells. A book is carried by
// the cell under the cursor, and snaps to the grid while it is over it. The puzzle
// is solved once no book is left outside of the grid.
class BookSortPuzzle : public PuzzleRecord {
public:
	BookSortPuzzle() : PuzzleRecord(7) {}
	virtual ~BookSortPuzzle() {}

	void init() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

	bool isViewportRelative() const override { return true; }

protected:
	Common::String getRecordTypeName() const override { return "BookSortPuzzle"; }

	static const int16 kNoBook = -1;

	struct Book {
		Common::Point attachPoint;	// what the cursor holds the book by, relative to its picture
		Common::Rect src;
		Common::Rect home;			// where the book lies while it is not in the grid
		Common::Array<Common::Point> cells;	// the cells the book covers, relative to its top left one

		bool isPlaced = false;
		Common::Point gridPos;		// the cell the attach point is in
	};

	bool isInGrid(const Common::Point &cell) const { return cell.x >= 0 && cell.y >= 0 && cell.x < _numColumns && cell.y < _numRows; }
	int16 &gridAt(const Common::Point &cell) { return _grid[cell.y * _numColumns + cell.x]; }

	// The grid cell a book's own cell ends up in, with the book held over a grid cell
	Common::Point getBookCell(const Book &book, uint cellID, const Common::Point &gridPos) const;
	bool canPlaceBook(const Book &book, const Common::Point &gridPos);
	void setBookCells(uint bookID, int16 value);
	// The book lying outside of the grid at a position, kNoBook if there is none
	int16 findBookOnTable(const Common::Point &pos) const;
	void redraw();

	// -- File data --
	Common::Rect _firstCell;	// the top left cell of the grid
	int32 _numColumns = 0;
	int32 _numRows = 0;

	uint16 _placeCursor = 0;	// carrying a book that fits where it is
	uint16 _blockedCursor = 0;	// carrying a book that does not
	uint16 _pickUpCursor = 0;
	uint16 _carryCursor = 0;	// carrying a book outside of the grid

	Common::Array<Book> _books;

	RandomSoundBlock _pickUpSound;
	RandomSoundBlock _putBackSound;
	RandomSoundBlock _placeSound;
	RandomSoundBlock _takeOutSound;
	RandomSoundBlock _blockedSound;
	RandomSoundBlock _solveSoundBlock;

	// -- Runtime --
	Common::Array<int16> _grid;			// the book in every cell
	Common::Array<uint> _tableBooks;	// the books outside of the grid; a click picks the first one it hits
	int16 _heldBook = kNoBook;
	Common::Point _mousePos;
	bool _isMouseInGrid = false;
	Common::Point _mouseCell;
	bool _isSolved = false;
	bool _exitRequested = false;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_BOOKSORTPUZZLE_H
