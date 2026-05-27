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
#include "engines/nancy/puzzledata.h"
#include "engines/nancy/action/puzzle/gridmappuzzle.h"

namespace Nancy {
namespace Action {

void GridMapPuzzle::readData(Common::SeekableReadStream &stream) {
	uint pos = stream.pos();
	Common::DumpFile d;
	d.open("nancy10_gridmappuzzle.dat");
	d.writeStream(&stream, stream.size());
	d.close();
	stream.seek(pos);

	readFilename(stream, _boardImageName);
	readFilename(stream, _cursorImageName);

	_retainState  = (stream.readByte() != 0);
	_mapRows      = stream.readUint16LE();
	_mapCols      = stream.readUint16LE();
	_itemsRows    = stream.readUint16LE();
	_itemsCols    = stream.readUint16LE();
	_resultsRows  = stream.readUint16LE();
	_resultsCols  = stream.readUint16LE();
	_numItems     = stream.readUint16LE();

	_mapOriginX     = stream.readUint16LE();
	_mapOriginY     = stream.readUint16LE();
	_mapSpacingY    = stream.readUint16LE();
	_mapSpacingX    = stream.readUint16LE();
	_itemsOriginX   = stream.readUint16LE();
	_itemsOriginY   = stream.readUint16LE();
	_itemsSpacingY  = stream.readUint16LE();
	_itemsSpacingX  = stream.readUint16LE();
	_resultsOriginX  = stream.readUint16LE();
	_resultsOriginY  = stream.readUint16LE();
	_resultsSpacingY = stream.readUint16LE();
	_resultsSpacingX = stream.readUint16LE();

	for (int i = 0; i < kMaxItems; ++i)
		readRect(stream, _mapItemSrcRects[i]);
	for (int i = 0; i < kMaxItems; ++i)
		readRect(stream, _itemsItemSrcRects[i]);

	// Results image atlas: 20 letter-half src rects, then 1 byte of padding,
	// then three 40-byte per-item index tables.
	for (int i = 0; i < kMaxResultRects; ++i)
		readRect(stream, _resultSrcRects[i]);
	stream.skip(1);

	for (int i = 0; i < kMaxItems; ++i)
		_leftHalfIdx[i] = stream.readSint16LE();
	for (int i = 0; i < kMaxItems; ++i)
		_rightHalfIdx[i] = stream.readSint16LE();
	for (int i = 0; i < kMaxItems; ++i)
		_resultSlot[i] = stream.readSint16LE();

	for (int i = 0; i < kMaxItems; ++i)
		_autoPlaceFlag[i] = stream.readSint16LE();

	_numSolutions = stream.readUint16LE();
	if (_numSolutions > kMaxSolutions)
		_numSolutions = kMaxSolutions;

	for (int s = 0; s < _numSolutions; ++s) {
		for (int i = 0; i < kMaxItems; ++i)
			_solutionRows[s][i] = stream.readSint16LE();
		for (int i = 0; i < kMaxItems; ++i)
			_solutionCols[s][i] = stream.readSint16LE();
	}

	_pickupSound.readNormal(stream);
	_placeSound.readNormal(stream);

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
	stream.skip(18); // trailing cursor type + unused fields

	if (_numItems > kMaxItems) _numItems = kMaxItems;
	if (_mapRows > kMaxMapRows) _mapRows = kMaxMapRows;
	if (_mapCols > kMaxMapCols) _mapCols = kMaxMapCols;
	if (_itemsRows > kMaxItemsRows) _itemsRows = kMaxItemsRows;
	if (_itemsCols > kMaxItemsCols) _itemsCols = kMaxItemsCols;

	_mapCellW     = _mapItemSrcRects[0].width();
	_mapCellH     = _mapItemSrcRects[0].height();
	_itemsCellW   = _itemsItemSrcRects[0].width();
	_itemsCellH   = _itemsItemSrcRects[0].height();
	_resultsCellW = _resultSrcRects[0].width();
	_resultsCellH = _resultSrcRects[0].height();
}

void GridMapPuzzle::initState() {
	GridMapPuzzleData *gmd = (GridMapPuzzleData *)NancySceneState.getPuzzleData(GridMapPuzzleData::getTag());
	if (_retainState && gmd && gmd->itemState.size() >= (uint)_numItems * 6) {
		const Common::Array<int16> &state = gmd->itemState;
		for (int i = 0; i < (int)_numItems; ++i) {
			_items[i].inMap    = (state[i * 6 + 0] != 0);
			_items[i].inItems  = (state[i * 6 + 1] != 0);
			_items[i].mapRow   = state[i * 6 + 2];
			_items[i].mapCol   = state[i * 6 + 3];
			_items[i].itemsRow = state[i * 6 + 4];
			_items[i].itemsCol = state[i * 6 + 5];
		}
		return;
	}

	for (int i = 0; i < kMaxItems; ++i)
		_items[i] = ItemSlot();

	// Items with their autoplace flag set start placed in the items grid
	// (acting as the source/inventory). Other items stay hidden until released
	// by gameplay events outside this puzzle.
	const int itemsCols = MAX<int>(1, (int)_itemsCols);
	for (int i = 0; i < (int)_numItems; ++i) {
		if (_autoPlaceFlag[i] != -1 && NancySceneState.getEventFlag(_autoPlaceFlag[i], g_nancy->_true)) {
			_items[i].inItems = true;
			_items[i].itemsRow = (int16)(i / itemsCols);
			_items[i].itemsCol = (int16)(i % itemsCols);
		}
	}

	persistState();
}

void GridMapPuzzle::persistState() {
	GridMapPuzzleData *gmd = (GridMapPuzzleData *)NancySceneState.getPuzzleData(GridMapPuzzleData::getTag());
	if (!gmd)
		return;
	gmd->itemState.clear();
	for (int i = 0; i < (int)_numItems; ++i) {
		gmd->itemState.push_back(_items[i].inMap   ? 1 : 0);
		gmd->itemState.push_back(_items[i].inItems ? 1 : 0);
		gmd->itemState.push_back(_items[i].mapRow);
		gmd->itemState.push_back(_items[i].mapCol);
		gmd->itemState.push_back(_items[i].itemsRow);
		gmd->itemState.push_back(_items[i].itemsCol);
	}
}

void GridMapPuzzle::init() {
	Common::Rect vpBounds = NancySceneState.getViewport().getBounds();
	_drawSurface.create(vpBounds.width(), vpBounds.height(),
		g_nancy->_graphics->getInputPixelFormat());
	_drawSurface.clear(g_nancy->_graphics->getTransColor());
	setTransparent(true);
	setVisible(true);
	moveTo(vpBounds);

	g_nancy->_resource->loadImage(_boardImageName, _boardImage);
	_boardImage.setTransparentColor(_drawSurface.getTransparentColor());

	initState();

	_heldItem = -1;
	_isSolved = false;
	_subState = kPlaying;

	redraw();
}

void GridMapPuzzle::execute() {
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
		g_nancy->_sound->stopSound(_placeSound);
		g_nancy->_sound->stopSound(_winSound);
		if (_subState == kExitToWin) {
			GridMapPuzzleData *gmd = (GridMapPuzzleData *)NancySceneState.getPuzzleData(GridMapPuzzleData::getTag());
			if (gmd)
				gmd->itemState.clear();
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

Common::Rect GridMapPuzzle::mapCellRect(int row, int col) const {
	int x = (int)_mapOriginX + col * ((int)_mapSpacingX + _mapCellW);
	int y = (int)_mapOriginY + row * ((int)_mapSpacingY + _mapCellH);
	return Common::Rect(x, y, x + _mapCellW, y + _mapCellH);
}

Common::Rect GridMapPuzzle::itemsCellRect(int row, int col) const {
	int x = (int)_itemsOriginX + col * ((int)_itemsSpacingX + _itemsCellW);
	int y = (int)_itemsOriginY + row * ((int)_itemsSpacingY + _itemsCellH);
	return Common::Rect(x, y, x + _itemsCellW, y + _itemsCellH);
}

bool GridMapPuzzle::hitTestMap(const Common::Point &p, int &outRow, int &outCol) const {
	for (int r = 0; r < (int)_mapRows; ++r) {
		for (int c = 0; c < (int)_mapCols; ++c) {
			if (mapCellRect(r, c).contains(p)) {
				outRow = r;
				outCol = c;
				return true;
			}
		}
	}
	return false;
}

bool GridMapPuzzle::hitTestItems(const Common::Point &p, int &outRow, int &outCol) const {
	for (int r = 0; r < (int)_itemsRows; ++r) {
		for (int c = 0; c < (int)_itemsCols; ++c) {
			if (itemsCellRect(r, c).contains(p)) {
				outRow = r;
				outCol = c;
				return true;
			}
		}
	}
	return false;
}

int GridMapPuzzle::findItemInMap(int row, int col) const {
	for (int i = 0; i < (int)_numItems; ++i) {
		if (_items[i].inMap && _items[i].mapRow == row && _items[i].mapCol == col)
			return i;
	}
	return -1;
}

int GridMapPuzzle::findItemInItems(int row, int col) const {
	for (int i = 0; i < (int)_numItems; ++i) {
		if (_items[i].inItems && _items[i].itemsRow == row && _items[i].itemsCol == col)
			return i;
	}
	return -1;
}

Common::Rect GridMapPuzzle::resultsCellRect(int row, int col) const {
	int x = (int)_resultsOriginX + col * ((int)_resultsSpacingX + _resultsCellW);
	int y = (int)_resultsOriginY + row * ((int)_resultsSpacingY + _resultsCellH);
	return Common::Rect(x, y, x + _resultsCellW, y + _resultsCellH);
}

bool GridMapPuzzle::isCorrectMapPlacement(int item, int row, int col) const {
	for (int s = 0; s < (int)_numSolutions; ++s) {
		if (_solutionRows[s][item] == (int16)row && _solutionCols[s][item] == (int16)col)
			return true;
	}
	return false;
}

void GridMapPuzzle::handleInput(NancyInput &input) {
	if (_state != kRun || _subState != kPlaying)
		return;

	Common::Rect vpScreen = NancySceneState.getViewport().getScreenPosition();
	Common::Point mouseVP = input.mousePos - Common::Point(vpScreen.left, vpScreen.top);

	if (_heldItem != -1 && _heldDrawPos != mouseVP) {
		_heldDrawPos = mouseVP;
		redraw();
	}

	int row = 0, col = 0;
	bool hitMap   = hitTestMap(mouseVP, row, col);
	bool hitItems = false;
	int  iRow = 0, iCol = 0;
	if (!hitMap)
		hitItems = hitTestItems(mouseVP, iRow, iCol);

	if (!hitMap && !hitItems) {
		if (!_exitHotspot.isEmpty() && _exitHotspot.contains(mouseVP)) {
			g_nancy->_cursor->setCursorType(g_nancy->_cursor->_puzzleExitCursor);
			if (input.input & NancyInput::kLeftMouseButtonUp)
				_subState = kExitToCancel;
		} else {
			g_nancy->_cursor->setCursorType(_heldItem != -1 ? CursorManager::kDragHand
			                                                : CursorManager::kNormal);
		}
		return;
	}

	g_nancy->_cursor->setCursorType(_heldItem != -1 ? CursorManager::kDragHand
	                                                : CursorManager::kNormal);
	if (!(input.input & NancyInput::kLeftMouseButtonUp))
		return;

	int existingItem = hitMap ? findItemInMap(row, col) : findItemInItems(iRow, iCol);

	if (_heldItem == -1) {
		if (existingItem == -1)
			return;
		_heldItem = existingItem;
		if (hitMap)
			_items[_heldItem].inMap = false;
		else
			_items[_heldItem].inItems = false;
		if (_pickupSound.name != "NO SOUND") {
			g_nancy->_sound->loadSound(_pickupSound);
			g_nancy->_sound->playSound(_pickupSound);
		}
	} else {
		// Drops onto the map grid only succeed when the cell is the correct
		// placement for the held item per any solution; otherwise the cell
		// rejects the drop and the item stays held.
		if (hitMap && !isCorrectMapPlacement(_heldItem, row, col))
			return;

		if (existingItem != -1) {
			// Items-grid cells allow swap: the existing item becomes the new
			// held one. Map cells are gated by the correctness check above, so
			// they can't be occupied by anything except the held item itself.
			if (hitMap)
				return;
			_items[existingItem].inItems = false;
			_items[_heldItem].inItems = true;
			_items[_heldItem].itemsRow = (int16)iRow;
			_items[_heldItem].itemsCol = (int16)iCol;
			_heldItem = existingItem;
			if (_pickupSound.name != "NO SOUND") {
				g_nancy->_sound->loadSound(_pickupSound);
				g_nancy->_sound->playSound(_pickupSound);
			}
		} else {
			if (hitMap) {
				_items[_heldItem].inMap = true;
				_items[_heldItem].mapRow = (int16)row;
				_items[_heldItem].mapCol = (int16)col;
			} else {
				_items[_heldItem].inItems = true;
				_items[_heldItem].itemsRow = (int16)iRow;
				_items[_heldItem].itemsCol = (int16)iCol;
			}
			_heldItem = -1;
			if (_placeSound.name != "NO SOUND") {
				g_nancy->_sound->loadSound(_placeSound);
				g_nancy->_sound->playSound(_placeSound);
			}
			checkSolved();
		}
	}

	persistState();

	redraw();
}

void GridMapPuzzle::checkSolved() {
	for (int s = 0; s < (int)_numSolutions; ++s) {
		bool match = true;
		for (int i = 0; i < (int)_numItems && match; ++i) {
			if (!_items[i].inMap) {
				match = false;
				break;
			}
			if (_items[i].mapRow != _solutionRows[s][i] ||
			    _items[i].mapCol != _solutionCols[s][i]) {
				match = false;
				break;
			}
		}
		if (match) {
			_isSolved = true;
			_subState = kPlayWinSound;
			return;
		}
	}
}

void GridMapPuzzle::redraw() {
	_drawSurface.clear(_drawSurface.getTransparentColor());

	for (int i = 0; i < (int)_numItems; ++i) {
		const ItemSlot &slot = _items[i];
		if (slot.inMap && slot.mapRow >= 0 && slot.mapCol >= 0) {
			const Common::Rect &src = _mapItemSrcRects[i];
			if (!src.isEmpty()) {
				Common::Rect dst = mapCellRect(slot.mapRow, slot.mapCol);
				_drawSurface.blitFrom(_boardImage, src, Common::Point(dst.left, dst.top));
			}
		}
		if (slot.inItems && slot.itemsRow >= 0 && slot.itemsCol >= 0) {
			const Common::Rect &src = _itemsItemSrcRects[i];
			if (!src.isEmpty()) {
				Common::Rect dst = itemsCellRect(slot.itemsRow, slot.itemsCol);
				_drawSurface.blitFrom(_boardImage, src, Common::Point(dst.left, dst.top));
			}
		}
	}

	// Each item placed in the map contributes two letter halves to the results
	// bar (its assigned slot indexes a pair of adjacent cells). When all items
	// are correctly placed the letters spell out the solution sentence.
	const int resultsCols = MAX<int>(1, (int)_resultsCols);
	for (int i = 0; i < (int)_numItems; ++i) {
		if (!_items[i].inMap)
			continue;
		const int slot = _resultSlot[i];
		if (slot < 0)
			continue;
		const int base = slot * 2;
		const int row  = base / resultsCols;
		const int col  = base % resultsCols;
		const int li = _leftHalfIdx[i];
		const int ri = _rightHalfIdx[i];
		if (li >= 0 && li < kMaxResultRects && !_resultSrcRects[li].isEmpty()) {
			Common::Rect dst = resultsCellRect(row, col);
			_drawSurface.blitFrom(_boardImage, _resultSrcRects[li], Common::Point(dst.left, dst.top));
		}
		if (ri >= 0 && ri < kMaxResultRects && !_resultSrcRects[ri].isEmpty()) {
			Common::Rect dst = resultsCellRect(row, col + 1);
			_drawSurface.blitFrom(_boardImage, _resultSrcRects[ri], Common::Point(dst.left, dst.top));
		}
	}

	if (_heldItem >= 0 && _heldItem < (int)_numItems) {
		const Common::Rect &src = _itemsItemSrcRects[_heldItem].isEmpty()
		                          ? _mapItemSrcRects[_heldItem]
		                          : _itemsItemSrcRects[_heldItem];
		if (!src.isEmpty()) {
			int x = _heldDrawPos.x - src.width()  / 2;
			int y = _heldDrawPos.y - src.height() / 2;
			_drawSurface.blitFrom(_boardImage, src, Common::Point(x, y));
		}
	}

	_needsRedraw = true;
}

} // End of namespace Action
} // End of namespace Nancy
