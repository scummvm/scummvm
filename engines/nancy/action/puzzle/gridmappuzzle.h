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

#ifndef NANCY_ACTION_GRIDMAPPUZZLE_H
#define NANCY_ACTION_GRIDMAPPUZZLE_H

#include "engines/nancy/action/actionrecord.h"
#include "engines/nancy/commontypes.h"

namespace Nancy {
namespace Action {

// Three-grid placement puzzle: items live in a "map" grid and an "items" grid.
// The player picks an item from one grid and drops it onto an empty cell of
// either. Wins when the items-grid layout matches any of the solutions stored
// in the action record.
// Called from scene 6240 in Nancy10.
class GridMapPuzzle : public RenderActionRecord {
public:
	GridMapPuzzle() : RenderActionRecord(7) {}
	virtual ~GridMapPuzzle() {}

	void init() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

	bool isViewportRelative() const override { return true; }

protected:
	Common::String getRecordTypeName() const override { return "GridMapPuzzle"; }

	static const int kMaxItems       = 20;
	static const int kMaxSolutions   = 8;
	static const int kMaxMapRows     = 20;
	static const int kMaxMapCols     = 20;
	static const int kMaxItemsRows   = 20;
	static const int kMaxItemsCols   = 20;
	static const int kMaxResultRects = 20;

	struct ItemSlot {
		bool  inMap     = false;
		bool  inItems   = false;
		int16 mapRow    = -1;
		int16 mapCol    = -1;
		int16 itemsRow  = -1;
		int16 itemsCol  = -1;
	};

	// File data

	Common::Path _boardImageName;
	Common::Path _cursorImageName;

	bool   _retainState = false;
	uint16 _mapRows = 0, _mapCols = 0;
	uint16 _itemsRows = 0, _itemsCols = 0;
	uint16 _resultsRows = 0, _resultsCols = 0;
	uint16 _numItems = 0;

	uint16 _mapOriginX = 0,  _mapOriginY = 0;
	uint16 _mapSpacingX = 0, _mapSpacingY = 0;
	int16  _mapCellW = 0,    _mapCellH = 0;

	uint16 _itemsOriginX = 0,  _itemsOriginY = 0;
	uint16 _itemsSpacingX = 0, _itemsSpacingY = 0;
	int16  _itemsCellW = 0,    _itemsCellH = 0;

	uint16 _resultsOriginX = 0,  _resultsOriginY = 0;
	uint16 _resultsSpacingX = 0, _resultsSpacingY = 0;
	int16  _resultsCellW = 0,    _resultsCellH = 0;

	Common::Rect _mapItemSrcRects[kMaxItems];
	Common::Rect _itemsItemSrcRects[kMaxItems];
	Common::Rect _resultSrcRects[kMaxResultRects];

	// Result-letter lookups are indexed by the placed item's MAP coordinates,
	// not by its item index. _letterByMapCol[mapCol] is rendered as the left
	// half of the item's letter pair; _letterByMapRow[mapRow] as the right.
	int16 _letterByMapRow[kMaxItems] = {};
	int16 _letterByMapCol[kMaxItems] = {};
	int16 _resultSlot[kMaxItems] = {};

	int16 _autoPlaceFlag[kMaxItems] = {};

	uint16 _numSolutions = 0;
	int16 _solutionRows[kMaxSolutions][kMaxItems] = {};
	int16 _solutionCols[kMaxSolutions][kMaxItems] = {};

	SoundDescription _pickupSound;
	SoundDescription _placeSound;
	SoundDescription _winSound;

	SceneChangeDescription _winScene;
	FlagDescription        _winFlag;
	SceneChangeDescription _cancelScene;
	FlagDescription        _cancelFlag;

	Common::Rect _exitHotspot;

	// Runtime state

	enum SubState {
		kPlaying = 0,
		kPlayWinSound,
		kWaitWinSound,
		kExitToWin,
		kExitToCancel
	};

	SubState _subState = kPlaying;

	ItemSlot _items[kMaxItems];
	int      _heldItem = -1;
	Common::Point _heldDrawPos;
	// Set on a swap (drop onto an occupied cell) so the freshly-placed
	// glyph isn't covered by the picked-up one drawn at the cursor.
	// Cleared on the next mouse move.
	bool     _skipHeldDraw = false;
	bool     _isSolved = false;

	Graphics::ManagedSurface _boardImage;
	Graphics::ManagedSurface _cursorImage; // item sprite atlas (right-side panel)

	void initState();
	void persistState();
	void redraw();
	Common::Rect mapCellRect(int row, int col) const;
	Common::Rect itemsCellRect(int row, int col) const;
	Common::Rect resultsCellRect(int row, int col) const;
	bool hitTestMap(const Common::Point &p, int &outRow, int &outCol) const;
	bool hitTestItems(const Common::Point &p, int &outRow, int &outCol) const;
	int findItemInMap(int row, int col) const;
	int findItemInItems(int row, int col) const;
	bool isValidMapSlot(int row, int col) const;
	void checkSolved();
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_GRIDMAPPUZZLE_H
