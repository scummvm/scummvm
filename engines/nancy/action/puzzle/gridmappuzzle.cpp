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
		_letterByMapRow[i] = stream.readSint16LE();
	for (int i = 0; i < kMaxItems; ++i)
		_letterByMapCol[i] = stream.readSint16LE();
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
	} else {
		for (int i = 0; i < kMaxItems; ++i)
			_items[i] = ItemSlot();
	}

	// Always re-check autoplace flags: items released after the last save
	// (e.g. discovered by examining a wall between visits) should appear in
	// the items grid even when restoring a previous state.
	const int itemsCols = MAX<int>(1, (int)_itemsCols);
	for (int i = 0; i < (int)_numItems; ++i) {
		if (_items[i].inMap || _items[i].inItems)
			continue;
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

	g_nancy->_resource->loadImage(_cursorImageName, _cursorImage);
	_cursorImage.setTransparentColor(_drawSurface.getTransparentColor());

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
	// Stride uses the raw src-rect dimensions (right - left, before readRect's
	// inclusive→exclusive +1), to match the original's cell layout.
	int x = (int)_mapOriginX + col * ((int)_mapSpacingX + _mapCellW - 1);
	int y = (int)_mapOriginY + row * ((int)_mapSpacingY + _mapCellH - 1);
	return Common::Rect(x, y, x + _mapCellW, y + _mapCellH);
}

Common::Rect GridMapPuzzle::itemsCellRect(int row, int col) const {
	int x = (int)_itemsOriginX + col * ((int)_itemsSpacingX + _itemsCellW - 1);
	int y = (int)_itemsOriginY + row * ((int)_itemsSpacingY + _itemsCellH - 1);
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

bool GridMapPuzzle::isValidMapSlot(int row, int col) const {
	// A map cell is a real placement slot iff at least one solution places
	// some item there. Empty map cells outside any solution reject drops.
	for (int s = 0; s < (int)_numSolutions; ++s) {
		for (int i = 0; i < (int)_numItems; ++i) {
			if (_solutionRows[s][i] == (int16)row && _solutionCols[s][i] == (int16)col)
				return true;
		}
	}
	return false;
}

Common::Rect GridMapPuzzle::resultsCellRect(int row, int col) const {
	int x = (int)_resultsOriginX + col * ((int)_resultsSpacingX + _resultsCellW - 1);
	int y = (int)_resultsOriginY + row * ((int)_resultsSpacingY + _resultsCellH - 1);
	return Common::Rect(x, y, x + _resultsCellW, y + _resultsCellH);
}

void GridMapPuzzle::handleInput(NancyInput &input) {
	if (_state != kRun || _subState != kPlaying)
		return;

	Common::Rect vpScreen = NancySceneState.getViewport().getScreenPosition();
	Common::Point mouseVP = input.mousePos - Common::Point(vpScreen.left, vpScreen.top);

	if (_heldItem != -1 && _heldDrawPos != mouseVP) {
		_heldDrawPos = mouseVP;
		_skipHeldDraw = false;
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

	// Reset the held-draw suppression flag on every click; the swap branch
	// below sets it back to true when appropriate.
	_skipHeldDraw = false;

	// Sync the held draw position to the click point. Mouse-move tracking
	// only runs while something is held, so without this a pickup right
	// after a plain drop would render the new held glyph at the previous
	// drop's coordinates for one frame.
	_heldDrawPos = mouseVP;

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
		// Drop. Map cells only accept the held item if they are real
		// placement slots (i.e. used by some solution); items cells always
		// accept. An occupied cell sends its current occupant back to the
		// cursor so the player can swap.
		if (hitMap && !isValidMapSlot(row, col))
			return;

		if (existingItem != -1) {
			if (hitMap) {
				_items[existingItem].inMap = false;
				_items[_heldItem].inMap = true;
				_items[_heldItem].mapRow = (int16)row;
				_items[_heldItem].mapCol = (int16)col;
			} else {
				_items[existingItem].inItems = false;
				_items[_heldItem].inItems = true;
				_items[_heldItem].itemsRow = (int16)iRow;
				_items[_heldItem].itemsCol = (int16)iCol;
			}
			_heldItem = existingItem;
			_skipHeldDraw = true;
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
		// Small map-glyph atlas is on the board image; large items-grid
		// sprites and the drag cursor are on the cursor image.
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
				_drawSurface.blitFrom(_cursorImage, src, Common::Point(dst.left, dst.top));
			}
		}
	}

	// Each item placed in the map contributes two letter halves to the results
	// bar at the item's fixed slot. The letters themselves are looked up from
	// the placement coordinates: column picks the left half, row the right.
	// When the right items end up at the right cells the strip spells out
	// the solution sentence.
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
		const int mapRow = (int)_items[i].mapRow;
		const int mapCol = (int)_items[i].mapCol;
		const int li = (mapCol >= 0 && mapCol < kMaxItems) ? _letterByMapCol[mapCol] : -1;
		const int ri = (mapRow >= 0 && mapRow < kMaxItems) ? _letterByMapRow[mapRow] : -1;
		if (li >= 0 && li < kMaxResultRects && !_resultSrcRects[li].isEmpty()) {
			Common::Rect dst = resultsCellRect(row, col);
			_drawSurface.blitFrom(_boardImage, _resultSrcRects[li], Common::Point(dst.left, dst.top));
		}
		if (ri >= 0 && ri < kMaxResultRects && !_resultSrcRects[ri].isEmpty()) {
			Common::Rect dst = resultsCellRect(row, col + 1);
			_drawSurface.blitFrom(_boardImage, _resultSrcRects[ri], Common::Point(dst.left, dst.top));
		}
	}

	if (_heldItem >= 0 && _heldItem < (int)_numItems && !_skipHeldDraw) {
		const Common::Rect &src = _itemsItemSrcRects[_heldItem].isEmpty()
		                          ? _mapItemSrcRects[_heldItem]
		                          : _itemsItemSrcRects[_heldItem];
		if (!src.isEmpty()) {
			int x = _heldDrawPos.x - src.width()  / 2;
			int y = _heldDrawPos.y - src.height() / 2;
			_drawSurface.blitFrom(_cursorImage, src, Common::Point(x, y));
		}
	}

	_needsRedraw = true;
}

} // End of namespace Action
} // End of namespace Nancy
