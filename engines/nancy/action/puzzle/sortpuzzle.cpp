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

#include "common/random.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/input.h"
#include "engines/nancy/util.h"

#include "engines/nancy/state/scene.h"
#include "engines/nancy/puzzledata.h"
#include "engines/nancy/action/puzzle/sortpuzzle.h"

namespace Nancy {
namespace Action {

// Tiles within a color group share their board sprite (srcRow, srcCol); the
// hidden `value` field disambiguates them, which is visible in the back of
// each gem, when picking it up.
static void packGrid(const SortPuzzle::Cell grid[][SortPuzzle::kMaxCols], uint16 rows, uint16 cols, Common::Array<int16> &out) {
	out.clear();
	out.push_back((int16)rows);
	out.push_back((int16)cols);
	for (uint16 r = 0; r < rows; ++r) {
		for (uint16 c = 0; c < cols; ++c) {
			const SortPuzzle::Cell &cell = grid[r][c];
			out.push_back(cell.srcRow);
			out.push_back(cell.srcCol);
			out.push_back(cell.value);
			out.push_back(cell.isEmpty ? 1 : 0);
		}
	}
}

static bool unpackGrid(const Common::Array<int16> &in, SortPuzzle::Cell grid[][SortPuzzle::kMaxCols], uint16 expectedRows, uint16 expectedCols) {
	if (in.size() < 2 || in[0] != (int16)expectedRows || in[1] != (int16)expectedCols)
		return false;
	uint32 need = 2 + (uint32)expectedRows * expectedCols * 4;
	if (in.size() < need)
		return false;
	uint32 idx = 2;
	for (uint16 r = 0; r < expectedRows; ++r) {
		for (uint16 c = 0; c < expectedCols; ++c) {
			SortPuzzle::Cell &cell = grid[r][c];
			cell.srcRow  = in[idx++];
			cell.srcCol  = in[idx++];
			cell.value   = in[idx++];
			cell.isEmpty = (in[idx++] != 0);
		}
	}
	return true;
}

void SortPuzzle::readData(Common::SeekableReadStream &stream) {
	readFilename(stream, _boardImageName);
	readFilename(stream, _cursorImageName);

	_retainState  = (stream.readByte() != 0);
	_rows         = stream.readUint16LE();
	_cols         = stream.readUint16LE();
	stream.skip(2);
	_groupDivisor = stream.readUint16LE();
	_valueRange   = stream.readUint16LE();

	for (int r = 0; r < kMaxSourceRows; ++r)
		for (int c = 0; c < kMaxSourceCols; ++c)
			readRect(stream, _cellSrcRects[r][c]);

	for (int i = 0; i < kNumCursors; ++i)
		readRect(stream, _cursorSrcRects[i]);

	_originX  = stream.readUint16LE();
	_originY  = stream.readUint16LE();
	_spacingY = stream.readUint16LE();
	_spacingX = stream.readUint16LE();

	_pickupSound.readNormal(stream);
	_dropSound.readNormal(stream);

	_winScene.readData(stream);
	stream.skip(2);
	_winFlag.label = stream.readSint16LE();
	_winFlag.flag  = stream.readByte();
	_winSound.readNormal(stream);

	_cancelScene.readData(stream);
	stream.skip(2);
	_cancelFlag.label = stream.readSint16LE();
	_cancelFlag.flag  = stream.readByte();

	readRect(stream, _exitHotspot);
	stream.skip(2); // exit cursor type id

	if (_rows > kMaxRows) _rows = kMaxRows;
	if (_cols > kMaxCols) _cols = kMaxCols;
	if (_groupDivisor == 0) _groupDivisor = 1;
	if (_valueRange == 0) _valueRange = 1;

	_cellWidth  = _cellSrcRects[0][0].width();
	_cellHeight = _cellSrcRects[0][0].height();
}

void SortPuzzle::initState() {
	SortPuzzleData *spd = (SortPuzzleData *)NancySceneState.getPuzzleData(SortPuzzleData::getTag());
	if (_retainState && spd && !spd->currentState.empty() && !spd->solvedState.empty()) {
		if (unpackGrid(spd->currentState, _current, _rows, _cols) &&
		    unpackGrid(spd->solvedState, _solved, _rows, _cols))
			return;
	}

	const int groupSize = MAX<int>(1, (int)_cols / (int)_groupDivisor);

	for (int r = 0; r < (int)_rows; ++r) {
		for (int c = 0; c < (int)_cols; ++c) {
			Cell &cell = _solved[r][c];
			cell.srcRow  = (int16)r;
			cell.srcCol  = (int16)(c / groupSize);
			cell.value   = (int16)(g_nancy->_randomSource->getRandomNumber(_valueRange - 1));
			cell.isEmpty = false;

			int groupStart = (c / groupSize) * groupSize;
			int k = c;
			while (k > groupStart && _solved[r][k].value < _solved[r][k - 1].value) {
				SWAP(_solved[r][k], _solved[r][k - 1]);
				--k;
			}
		}
	}

	const int total = (int)_rows * (int)_cols;
	bool taken[kMaxRows * kMaxCols] = {};
	for (int r = 0; r < (int)_rows; ++r) {
		for (int c = 0; c < (int)_cols; ++c) {
			int idx;
			do {
				idx = (int)g_nancy->_randomSource->getRandomNumber(total - 1);
			} while (taken[idx]);
			taken[idx] = true;
			int sr = idx / (int)_cols;
			int sc = idx % (int)_cols;
			_current[r][c] = _solved[sr][sc];
		}
	}

	persistState();
}

void SortPuzzle::persistState() {
	SortPuzzleData *spd = (SortPuzzleData *)NancySceneState.getPuzzleData(SortPuzzleData::getTag());
	if (!spd)
		return;
	packGrid(_current, _rows, _cols, spd->currentState);
	packGrid(_solved,  _rows, _cols, spd->solvedState);
}

void SortPuzzle::init() {
	Common::Rect vpBounds = NancySceneState.getViewport().getBounds();
	_drawSurface.create(vpBounds.width(), vpBounds.height(),
		g_nancy->_graphics->getInputPixelFormat());
	_drawSurface.clear(g_nancy->_graphics->getTransColor());
	setTransparent(true);
	setVisible(true);
	moveTo(vpBounds);

	g_nancy->_resource->loadImage(_boardImageName, _boardImage);
	_boardImage.setTransparentColor(_drawSurface.getTransparentColor());
	g_nancy->_resource->loadImage(_cursorImageName, _cursorImage);
	_cursorImage.setTransparentColor(_drawSurface.getTransparentColor());

	initState();

	_hasHeld  = false;
	_isSolved = false;
	_subState = kPlaying;

	redraw();
}

void SortPuzzle::execute() {
	switch (_state) {
	case kBegin:
		init();
		registerGraphics();
		_state = kRun;
		// fall through

	case kRun:
		switch (_subState) {
		case kPlaying:
			break;
		case kPlayWinSound:
			if (_winSound.name != "NO SOUND") {
				g_nancy->_sound->loadSound(_winSound);
				g_nancy->_sound->playSound(_winSound);
				_subState = kWaitWinSound;
			} else {
				_subState = kExitToWin;
			}
			break;
		case kWaitWinSound:
			if (!g_nancy->_sound->isSoundPlaying(_winSound)) {
				g_nancy->_sound->stopSound(_winSound);
				_subState = kExitToWin;
			}
			break;
		case kExitToWin:
		case kExitToCancel:
			_state = kActionTrigger;
			break;
		}
		break;

	case kActionTrigger:
		g_nancy->_sound->stopSound(_pickupSound);
		g_nancy->_sound->stopSound(_dropSound);
		g_nancy->_sound->stopSound(_winSound);
		if (_subState == kExitToWin) {
			SortPuzzleData *spd = (SortPuzzleData *)NancySceneState.getPuzzleData(SortPuzzleData::getTag());
			if (spd) {
				spd->currentState.clear();
				spd->solvedState.clear();
			}
			if (_winFlag.label != -1)
				NancySceneState.setEventFlag(_winFlag);
			if (_winScene.sceneID != kNoScene)
				NancySceneState.changeScene(_winScene);
		} else {
			if (_cancelFlag.label != -1)
				NancySceneState.setEventFlag(_cancelFlag);
			if (_cancelScene.sceneID != kNoScene)
				NancySceneState.changeScene(_cancelScene);
		}
		finishExecution();
		break;
	}
}

Common::Rect SortPuzzle::cellRect(int row, int col) const {
	// Original uses inclusive-coordinate width (right_raw - left_raw) for the
	// per-col / per-row step, which is cellWidth-1 in our exclusive convention.
	int x = (int)_originX + col * ((int)_spacingX + _cellWidth  - 1);
	int y = (int)_originY + row * ((int)_spacingY + _cellHeight - 1);
	return Common::Rect(x, y, x + _cellWidth, y + _cellHeight);
}

bool SortPuzzle::hitTestCell(const Common::Point &p, int &outRow, int &outCol) const {
	for (int r = 0; r < (int)_rows; ++r) {
		for (int c = 0; c < (int)_cols; ++c) {
			if (cellRect(r, c).contains(p)) {
				outRow = r;
				outCol = c;
				return true;
			}
		}
	}
	return false;
}

void SortPuzzle::handleInput(NancyInput &input) {
	if (_state != kRun || _subState != kPlaying)
		return;

	Common::Rect vpScreen = NancySceneState.getViewport().getScreenPosition();
	Common::Point mouseVP = input.mousePos - Common::Point(vpScreen.left, vpScreen.top);

	// Cheat: show the grid with the value of each gem in the console
	if (!input.otherKbdInput.empty() && input.otherKbdInput[0].keycode == Common::KEYCODE_SPACE) {
		debug("-----");
		for (int r = 0; r < (int)_rows; ++r) {
			Common::String line;
			for (int c = 0; c < (int)_cols; ++c) {
				const Cell &cur = _current[r][c];
				line += Common::String::format("%d ", cur.value);
			}
			debug("%s", line.c_str());
		}
		debug("-----");
	}

	if (_hasHeld && _heldDrawPos != mouseVP) {
		_heldDrawPos = mouseVP;
		redraw();
	}

	int row, col;
	bool hitCell = hitTestCell(mouseVP, row, col);

	if (!hitCell) {
		if (!_exitHotspot.isEmpty() && _exitHotspot.contains(mouseVP)) {
			g_nancy->_cursor->setCursorType(g_nancy->_cursor->_puzzleExitCursor);
			if (input.input & NancyInput::kLeftMouseButtonUp)
				_subState = kExitToCancel;
		}
		return;
	}

	g_nancy->_cursor->setCursorType(_hasHeld ? CursorManager::kDropHand
	                                         : CursorManager::kHotspot);
	if (!(input.input & NancyInput::kLeftMouseButtonUp))
		return;

	if (!_hasHeld) {
		if (_current[row][col].isEmpty)
			return;
		_held = _current[row][col];
		_hasHeld = true;
		_current[row][col].isEmpty = true;
		if (_pickupSound.name != "NO SOUND") {
			g_nancy->_sound->loadSound(_pickupSound);
			g_nancy->_sound->playSound(_pickupSound);
		}
	} else {
		Cell target = _current[row][col];
		_current[row][col] = _held;
		_current[row][col].isEmpty = false;
		if (_dropSound.name != "NO SOUND") {
			g_nancy->_sound->loadSound(_dropSound);
			g_nancy->_sound->playSound(_dropSound);
		}
		if (target.isEmpty) {
			_hasHeld = false;
			checkSolved();
		} else {
			_held = target;
		}
	}

	persistState();

	redraw();
}

void SortPuzzle::checkSolved() {
	for (int r = 0; r < (int)_rows; ++r) {
		for (int c = 0; c < (int)_cols; ++c) {
			const Cell &cur = _current[r][c];
			const Cell &sol = _solved[r][c];
			if (cur.isEmpty || cur.srcRow != sol.srcRow ||
			    cur.srcCol != sol.srcCol || cur.value != sol.value)
				return;
		}
	}
	_isSolved = true;
	_subState = kPlayWinSound;
}

void SortPuzzle::redraw() {
	_drawSurface.clear(_drawSurface.getTransparentColor());

	for (int r = 0; r < (int)_rows; ++r) {
		for (int c = 0; c < (int)_cols; ++c) {
			const Cell &cell = _current[r][c];
			if (cell.isEmpty)
				continue;
			if (cell.srcRow < 0 || cell.srcRow >= kMaxSourceRows ||
			    cell.srcCol < 0 || cell.srcCol >= kMaxSourceCols)
				continue;
			const Common::Rect &src = _cellSrcRects[cell.srcRow][cell.srcCol];
			if (src.isEmpty())
				continue;
			Common::Rect dst = cellRect(r, c);
			_drawSurface.blitFrom(_boardImage, src, Common::Point(dst.left, dst.top));
		}
	}

	if (_hasHeld) {
		bool drawn = false;
		if (_held.value >= 0 && _held.value < kNumCursors) {
			const Common::Rect &src = _cursorSrcRects[_held.value];
			if (!src.isEmpty()) {
				int x = _heldDrawPos.x - src.width()  / 2;
				int y = _heldDrawPos.y - src.height() / 2;
				_drawSurface.blitFrom(_cursorImage, src, Common::Point(x, y));
				drawn = true;
			}
		}
		if (!drawn && _held.srcRow >= 0 && _held.srcRow < kMaxSourceRows &&
		    _held.srcCol >= 0 && _held.srcCol < kMaxSourceCols) {
			const Common::Rect &src = _cellSrcRects[_held.srcRow][_held.srcCol];
			if (!src.isEmpty()) {
				int x = _heldDrawPos.x - _cellWidth  / 2;
				int y = _heldDrawPos.y - _cellHeight / 2;
				_drawSurface.blitFrom(_boardImage, src, Common::Point(x, y));
			}
		}
	}

	_needsRedraw = true;
}

} // End of namespace Action
} // End of namespace Nancy
