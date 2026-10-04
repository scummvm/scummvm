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

#include "common/algorithm.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/input.h"
#include "engines/nancy/util.h"

#include "engines/nancy/state/scene.h"
#include "engines/nancy/action/puzzle/booksortpuzzle.h"

namespace Nancy {
namespace Action {

void BookSortPuzzle::readData(Common::SeekableReadStream &stream) {
	// The sizes of the books are counted in cells, so the rects are read as they are
	readExclusiveRect(stream, _firstCell);
	_numColumns = stream.readSint32LE();
	_numRows = stream.readSint32LE();
	stream.skip(1);	// Marks the cells that keep a carried book from fitting; off in the game data

	readFilename(stream, _imageName);

	_placeCursor = stream.readUint16LE();
	_blockedCursor = stream.readUint16LE();
	_pickUpCursor = stream.readUint16LE();
	_carryCursor = stream.readUint16LE();

	const int cellWidth = _firstCell.width();
	const int cellHeight = _firstCell.height();

	_books.resize(stream.readUint16LE());
	for (Book &book : _books) {
		book.attachPoint.x = stream.readSint16LE();
		book.attachPoint.y = stream.readSint16LE();
		readExclusiveRect(stream, book.src);
		readExclusiveRect(stream, book.home);

		book.attachPoint.x -= book.src.left;
		book.attachPoint.y -= book.src.top;

		// The parts of the picture that take up room
		uint16 numCovered = stream.readUint16LE();
		for (uint i = 0; i < numCovered; ++i) {
			Common::Rect covered;
			readExclusiveRect(stream, covered);
			covered.translate(-book.src.left, -book.src.top);

			for (int y = covered.top / cellHeight; y < covered.bottom / cellHeight; ++y) {
				for (int x = covered.left / cellWidth; x < covered.right / cellWidth; ++x) {
					Common::Point cell(x, y);
					if (Common::find(book.cells.begin(), book.cells.end(), cell) == book.cells.end()) {
						book.cells.push_back(cell);
					}
				}
			}
		}
	}

	_pickUpSound.readData(stream);
	_putBackSound.readData(stream);
	_placeSound.readData(stream);
	_takeOutSound.readData(stream);
	_blockedSound.readData(stream);

	_solveScene._sceneChange.sceneID = stream.readUint16LE();
	_solveScene._sceneChange.frameID = stream.readUint16LE();
	_solveScene._sceneChange.continueSceneSound = kContinueSceneSound;
	_solveScene._flag.label = stream.readSint16LE();
	_solveScene._flag.flag = stream.readByte();
	_solveSoundBlock.readData(stream);

	readExitHotspot(stream);
}

void BookSortPuzzle::init() {
	initViewportSurface();
	loadImage();

	_grid.resize(_numColumns * _numRows);
	for (int16 &cell : _grid) {
		cell = kNoBook;
	}

	for (uint i = 0; i < _books.size(); ++i) {
		_tableBooks.push_back(i);
	}

	redraw();
}

Common::Point BookSortPuzzle::getBookCell(const Book &book, uint cellID, const Common::Point &gridPos) const {
	return Common::Point(book.cells[cellID].x + gridPos.x - book.attachPoint.x / _firstCell.width(),
						book.cells[cellID].y + gridPos.y - book.attachPoint.y / _firstCell.height());
}

bool BookSortPuzzle::canPlaceBook(const Book &book, const Common::Point &gridPos) {
	for (uint i = 0; i < book.cells.size(); ++i) {
		Common::Point cell = getBookCell(book, i, gridPos);
		if (!isInGrid(cell) || gridAt(cell) != kNoBook) {
			return false;
		}
	}

	return true;
}

void BookSortPuzzle::setBookCells(uint bookID, int16 value) {
	const Book &book = _books[bookID];
	for (uint i = 0; i < book.cells.size(); ++i) {
		gridAt(getBookCell(book, i, book.gridPos)) = value;
	}
}

int16 BookSortPuzzle::findBookOnTable(const Common::Point &pos) const {
	for (uint bookID : _tableBooks) {
		const Book &book = _books[bookID];
		if (!book.home.contains(pos)) {
			continue;
		}

		Common::Point cell((pos.x - book.home.left) / _firstCell.width(), (pos.y - book.home.top) / _firstCell.height());
		if (Common::find(book.cells.begin(), book.cells.end(), cell) != book.cells.end()) {
			return bookID;
		}
	}

	return kNoBook;
}

void BookSortPuzzle::redraw() {
	_drawSurface.clear(_drawSurface.getTransparentColor());

	for (uint bookID : _tableBooks) {
		_drawSurface.blitFrom(_image, _books[bookID].src, _books[bookID].home.origin());
	}

	const int cellWidth = _firstCell.width();
	const int cellHeight = _firstCell.height();

	for (const Book &book : _books) {
		if (book.isPlaced) {
			Common::Point topLeft = getBookCell(book, 0, book.gridPos) - book.cells[0];
			_drawSurface.blitFrom(_image, book.src,
				Common::Point(_firstCell.left + topLeft.x * cellWidth, _firstCell.top + topLeft.y * cellHeight));
		}
	}

	if (_heldBook != kNoBook) {
		const Book &book = _books[_heldBook];

		if (_isMouseInGrid) {
			// Over the grid the book shows where it would go
			Common::Point topLeft = getBookCell(book, 0, _mouseCell) - book.cells[0];
			_drawSurface.blitFrom(_image, book.src,
				Common::Point(_firstCell.left + topLeft.x * cellWidth, _firstCell.top + topLeft.y * cellHeight));
		} else {
			_drawSurface.blitFrom(_image, book.src, _mousePos - book.attachPoint);
		}
	}

	_needsRedraw = true;
}

void BookSortPuzzle::execute() {
	switch (_state) {
	case kBegin:
		init();
		registerGraphics();
		_state = kRun;
		// fall through
	case kRun:
		if (_exitRequested || _isSolved) {
			_state = kActionTrigger;
		}

		break;
	case kActionTrigger:
		if (_isSolved) {
			playSoundBlock(_solveSoundBlock);
			_solveScene.execute();
		} else {
			_exitScene.execute();
		}

		finishExecution();
		break;
	}
}

void BookSortPuzzle::handleInput(NancyInput &input) {
	if (_state != kRun || _exitRequested || _isSolved) {
		return;
	}

	Common::Rect viewportBounds = NancySceneState.getViewport().getBounds();
	if (!NancySceneState.getViewport().convertViewportToScreen(viewportBounds).contains(input.mousePos)) {
		return;
	}

	const bool clicked = input.input & NancyInput::kLeftMouseButtonUp;

	Common::Rect mouseRect = NancySceneState.getViewport().convertScreenToViewport(
		Common::Rect(input.mousePos.x, input.mousePos.y, input.mousePos.x + 1, input.mousePos.y + 1));
	_mousePos = mouseRect.origin();

	_mouseCell = Common::Point((_mousePos.x - _firstCell.left) / _firstCell.width(), (_mousePos.y - _firstCell.top) / _firstCell.height());
	_isMouseInGrid = _mousePos.x >= _firstCell.left && _mousePos.y >= _firstCell.top && isInGrid(_mouseCell);

	if (_heldBook != kNoBook) {
		Book &book = _books[_heldBook];

		if (!_isMouseInGrid) {
			setDataCursor(_carryCursor, false);

			if (clicked) {
				// Back to where it was lying
				_tableBooks.push_back(_heldBook);
				_heldBook = kNoBook;
				playSoundBlock(_putBackSound);
				input.eatMouseInput();
			}
		} else if (canPlaceBook(book, _mouseCell)) {
			setDataCursor(_placeCursor);

			if (clicked) {
				book.isPlaced = true;
				book.gridPos = _mouseCell;
				setBookCells(_heldBook, _heldBook);
				_heldBook = kNoBook;
				playSoundBlock(_placeSound);
				input.eatMouseInput();

				_isSolved = _tableBooks.empty();
			}
		} else {
			setDataCursor(_blockedCursor, false);

			if (clicked) {
				playSoundBlock(_blockedSound);
				input.eatMouseInput();
			}
		}

		redraw();
		return;
	}

	if (_isMouseInGrid && gridAt(_mouseCell) != kNoBook) {
		setDataCursor(_pickUpCursor);

		if (clicked) {
			_heldBook = gridAt(_mouseCell);
			setBookCells(_heldBook, kNoBook);
			_books[_heldBook].isPlaced = false;
			playSoundBlock(_takeOutSound);
			input.eatMouseInput();
			redraw();
		}

		return;
	}

	int16 bookID = findBookOnTable(_mousePos);
	if (bookID != kNoBook) {
		setDataCursor(_pickUpCursor);

		if (clicked) {
			for (uint i = 0; i < _tableBooks.size(); ++i) {
				if (_tableBooks[i] == (uint)bookID) {
					_tableBooks.remove_at(i);
					break;
				}
			}

			_heldBook = bookID;
			playSoundBlock(_pickUpSound);
			input.eatMouseInput();
			redraw();
		}

		return;
	}

	if (hoverExitHotspot(input) && clicked) {
		_exitRequested = true;
		input.eatMouseInput();
	}
}

} // End of namespace Action
} // End of namespace Nancy
