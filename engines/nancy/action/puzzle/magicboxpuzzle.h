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

#ifndef NANCY_ACTION_MAGICBOXPUZZLE_H
#define NANCY_ACTION_MAGICBOXPUZZLE_H

#include "engines/nancy/commontypes.h"
#include "engines/nancy/action/actionrecord.h"
#include "engines/nancy/misc/mousefollow.h"

namespace Nancy {
namespace Action {

// Number-square puzzle, new in Nancy15 (AR 183). A grid of numbered cells has a few of
// its cells left blank; the matching number of loose tiles waits in a tray beside the
// board. A tile is picked up by clicking it, carried on the cursor and dropped into a
// blank cell (or back into the tray). The board is solved once every row and every
// column adds up to the target printed beside it.
//
// The record is loaded through the shared GridFlow factory, whose leading type word must
// be zero for this puzzle. The grid is also divided into equal subgrids, which is what
// spaces the cell rects out into blocks on screen.
class MagicBoxPuzzle : public RenderActionRecord {
public:
	MagicBoxPuzzle() : RenderActionRecord(7) {}
	virtual ~MagicBoxPuzzle() {}

	void init() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

	bool isViewportRelative() const override { return true; }

protected:
	Common::String getRecordTypeName() const override { return "MagicBoxPuzzle"; }

	// The board slots are the blank cells, in row-major order; one dest rect per slot.
	int slotAtCursor(const Common::Point &mousePos) const;
	int trayPieceAtCursor(const Common::Point &mousePos) const;

	int32 rowSum(int row) const;
	int32 colSum(int col) const;
	bool isSolved() const;

	// Lifts the given piece onto the cursor, or sets the carried one down for an index of -1.
	void carryPiece(int piece, NancyInput &input);
	Common::Rect tileSrc(int32 value) const;

	void setDataCursor(uint16 cursorType, bool hotspotVariant = true) const;
	SoundDescription playSoundBlock(const RandomSoundBlock &block);
	void redraw();

	static const uint kNumSounds = 10;

	// -- File data --
	uint16 _gridFlowType = 0;			// 0x00, validated as 0 by the factory
	int32 _numCols = 0;					// 0x02
	int32 _numRows = 0;					// 0x06
	int32 _subgridCols = 0;				// 0x0a
	int32 _subgridRows = 0;				// 0x0e
	byte _allCellsBlank = 0;			// 0x12, forces every cell to be a droppable slot
	byte _unknown13 = 0;				// 0x13
	byte _allowTakeBack = 0;			// 0x14, lets a placed tile be picked up again

	Common::Array<int32> _cellValues;			// row-major, 0 marks a blank cell
	Common::Array<Common::Rect> _slotDests;		// one per blank cell, in row-major order
	Common::Array<int32> _pieceValues;			// the loose tiles
	Common::Path _tileImageName;
	Common::Array<Common::Rect> _tileSrcs;		// indexed by tile value - 1
	Common::Array<Common::Rect> _trayDests;		// resting place of each loose tile
	Common::Path _indicatorImageName;
	Common::Array<Common::Rect> _indicatorSrcs;
	Common::Array<Common::Rect> _indicatorDests;	// _numRows row markers, then _numCols column ones
	Common::Array<int32> _rowTargets;
	Common::Array<int32> _colTargets;
	uint16 _hoverCursorType = 0;
	uint16 _dragCursorType = 0;

	RandomSoundBlock _sounds[kNumSounds];

	SceneChangeDescription _solveScene;
	FlagDescription _solveFlag;
	RandomSoundBlock _solveSound;
	// The second outcome of the record. Neither of the shipped Nancy15 boards can be
	// failed, so nothing reaches it.
	SceneChangeDescription _failScene;
	FlagDescription _failFlag;
	RandomSoundBlock _failSound;

	Common::Rect _exitHotspot;
	uint16 _exitCursorType = 0;
	SceneChangeDescription _exitScene;
	FlagDescription _exitFlag;

	// -- Runtime state --
	Common::Array<int> _slotCells;		// board cell index of each slot
	Common::Array<int> _slotContents;	// piece in each slot, or -1
	Common::Array<int> _piecePlacement;	// slot each piece sits in, or -1 while in its tray spot
	int _carriedPiece = -1;
	Misc::MouseFollowObject _carriedObject;
	bool _solved = false;
	bool _exitRequested = false;
	SoundDescription _endSound;

	Graphics::ManagedSurface _tileImage;
	Graphics::ManagedSurface _indicatorImage;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_MAGICBOXPUZZLE_H
