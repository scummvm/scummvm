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

#ifndef NANCY_ACTION_PEGSPUZZLE_H
#define NANCY_ACTION_PEGSPUZZLE_H

#include "engines/nancy/commontypes.h"
#include "engines/nancy/action/actionrecord.h"

namespace Nancy {
namespace Action {

// Peg-solitaire puzzle, new in Nancy13 (AR 173, "leaping" jump game). The board is
// a grid of holes (in Nancy13, a 7x7 grid with the four 2x2 corners blocked off,
// i.e. the classic 33-hole English peg-solitaire cross). A peg jumps over an
// adjacent peg into an empty hole two cells away, removing the jumped peg. The
// puzzle is solved when no move remains and the number of pegs left is at most the
// target count.
//
// Pegs are dragged and dropped: clicking a movable peg picks it up (the hand/drag
// cursor carries the piece) and dropping it on a hole it can reach performs the jump.
// Cell state matches the original: 0 = peg, 1 = empty hole, 2 = blocked (no hole).
class PegsPuzzle : public RenderActionRecord {
public:
	PegsPuzzle() : RenderActionRecord(7) {}
	virtual ~PegsPuzzle() {}

	void init() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

	bool isViewportRelative() const override { return true; }

protected:
	Common::String getRecordTypeName() const override { return "PegsPuzzle"; }

	enum CellState : byte { kPeg = 0, kEmpty = 1, kBlocked = 2 };

	int cellIndex(int col, int row) const { return col + row * 8; }
	bool validCell(int col, int row) const;
	// Whether a peg at (col,row) can jump in the direction (dCol,dRow), and if so the
	// landing cell.
	bool canJump(int col, int row, int dCol, int dRow, int &destCol, int &destRow) const;
	// Whether the carried peg can jump onto (col,row).
	bool isCarriedTarget(int col, int row) const;
	// The board cell whose destination rect contains the (screen-space) cursor, or false.
	bool cellAtCursor(const Common::Point &mousePos, int &outCol, int &outRow) const;
	bool cellHasAnyMove(int col, int row) const;
	bool anyMovesLeft() const;
	int pegCount() const;
	void doJump(int fromCol, int fromRow, int destCol, int destRow);

	Common::Point cursorToViewport(const Common::Point &mousePos) const;
	// The puzzle's cursors are raw Nancy13 cursor type ids stored in the AR data.
	void setDataCursor(uint16 cursorType) const;

	void redraw();
	SoundDescription playSoundBlock(const RandomSoundBlock &block);

	// -- File data (96-byte header) --
	Common::Path _imageName;			// 0x00
	uint16 _hoverCursorType = 0;		// 0x21 - cursor while hovering a movable peg (open hand)
	uint16 _dragCursorType = 0;			// 0x23 - cursor while carrying a peg (pointing finger)
	byte _startEmptyFlag = 0;			// 0x25 - 0 => mark _startEmptyPos empty at init
	byte _startEmptyPos = 0;			// 0x26 - the initially-empty hole (usually the centre)
	byte _targetPegCount = 0;			// 0x27 - win when pegs remaining <= this
	SceneChangeDescription _winScene;	// 0x28 - shown when solved (9999 => none)
	SceneChangeDescription _loseScene;	// 0x2d - shown when no move remains and unsolved
	Common::Rect _pegSrc;				// 0x32 - normal peg sprite (viewport image)
	Common::Rect _selectedSrc;			// 0x42 - highlight sprite (selected peg / target holes)
	uint16 _originY = 0;				// 0x52 - board top-left (viewport space)
	uint16 _originX = 0;				// 0x54
	uint16 _numRows = 0;				// 0x56
	uint16 _numCols = 0;				// 0x58
	uint16 _pitchYBias = 0;				// 0x5a
	uint16 _pitchXBias = 0;				// 0x5c
	uint16 _numBlocked = 0;				// 0x5e - count of blocked-corner positions

	Common::Array<byte> _blockedPositions;

	// The clickable "give up / exit" hotspot (the base-class hotspot record).
	Common::Rect _exitHotspot;
	uint16 _exitCursorType = 0;			// the record's leading field is its cursor type
	SceneChangeDescription _exitScene;

	Common::Array<RandomSoundBlock> _sounds;	// 5 blocks: select / jump / pulse / win / lose

	// -- Runtime state --
	Common::Array<byte> _board;			// 64 cells (8-wide stride)
	Common::Array<Common::Rect> _destRects;
	int _carriedCol = -1;				// the peg currently picked up (dragged), or -1
	int _carriedRow = -1;
	Common::Point _dragPos;				// cursor position (viewport space) while dragging
	bool _ended = false;
	bool _solved = false;
	bool _exitRequested = false;
	uint32 _endTime = 0;
	SoundDescription _endSound;			// the win/lose cue we wait on before changing scene

	Graphics::ManagedSurface _image;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_PEGSPUZZLE_H
