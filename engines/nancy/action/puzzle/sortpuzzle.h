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

#ifndef NANCY_ACTION_SORTPUZZLE_H
#define NANCY_ACTION_SORTPUZZLE_H

#include "engines/nancy/action/actionrecord.h"
#include "engines/nancy/commontypes.h"

namespace Nancy {
namespace Action {

// Tile-sorting puzzle: cells of a shuffled grid are picked up and swapped one
// at a time until every cell is back in the position it held in the original
// (pre-shuffle) layout. Tiles are sorted per type, color and size, which is
// visible in the back of each gem when picked up.
// Called from scene 2036 in Nancy10.
class SortPuzzle : public RenderActionRecord {
public:
	SortPuzzle() : RenderActionRecord(7) {}
	virtual ~SortPuzzle() {}

	void init() override;

	void readData(Common::SeekableReadStream &stream) override;
	void readDataNancy12(Common::SeekableReadStream &stream);
	void execute() override;
	void handleInput(NancyInput &input) override;

	bool isViewportRelative() const override { return true; }

	static const int kMaxRows = 9;
	static const int kMaxCols = 9;

	struct Cell {
		int16 srcRow = 0;
		int16 srcCol = 0;
		int16 value = 0;
		bool isEmpty = false;
	};

protected:
	Common::String getRecordTypeName() const override { return "SortPuzzle"; }

	static const int kMaxSourceRows = 8;
	static const int kMaxSourceCols = 5;
	static const int kNumCursors    = 10;
	static const int kNumValueRects = 114; // Nancy 12: per-kind/size source sprites (19 kinds x 6 sizes)
	static const int kSizesPerKind  = 6;   // Nancy 12: stride between kinds in _valueSrcRects

	// File data

	Common::Path _boardImageName;
	Common::Path _cursorImageName;

	bool   _retainState  = false;
	uint16 _rows         = 0;
	uint16 _cols         = 0;
	uint16 _groupDivisor = 1;
	uint16 _valueRange   = 1;

	Common::Rect _cellSrcRects[kMaxSourceRows][kMaxSourceCols];
	Common::Rect _cursorSrcRects[kNumCursors];
	Common::Array<Common::Rect> _valueSrcRects; // Nancy 12: indexed by gem value

	uint16 _originX  = 0;
	uint16 _originY  = 0;
	uint16 _spacingX = 0;
	uint16 _spacingY = 0;
	int16  _cellWidth  = 0;
	int16  _cellHeight = 0;

	SoundDescription _pickupSound;
	SoundDescription _dropSound;

	SceneChangeDescription _winScene;
	FlagDescription        _winFlag;
	SoundDescription       _winSound;
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

	Cell _current[kMaxRows][kMaxCols];
	Cell _solved[kMaxRows][kMaxCols];

	Cell _held;
	bool _hasHeld   = false;
	bool _isSolved  = false;

	Common::Point _heldDrawPos;

	Graphics::ManagedSurface _boardImage;
	Graphics::ManagedSurface _cursorImage;

	void initState();
	void persistState();
	void redraw();
	void checkSolved();
	Common::Rect cellRect(int row, int col) const;
	Common::Rect cellSprite(const Cell &cell) const;
	bool hitTestCell(const Common::Point &p, int &outRow, int &outCol) const;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_SORTPUZZLE_H
