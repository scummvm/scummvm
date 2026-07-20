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

#ifndef NANCY_ACTION_BLOCKSPUZZLE_H
#define NANCY_ACTION_BLOCKSPUZZLE_H

#include "engines/nancy/commontypes.h"
#include "engines/nancy/action/actionrecord.h"

namespace Nancy {
namespace Action {

// Shaped-blocks puzzle, new in Nancy13 (AR 172). Carved blocks sit in the cells of an
// isometric board. A block is picked up by clicking it, carried on the cursor, and put
// down on another cell; dropping it onto an occupied cell swaps the two, leaving the
// displaced block in hand. A block can also be turned, stepping through its rotations one
// at a time with a short tweening animation. The board is solved once every cell holds
// its target block at its target rotation.
//
// Turning happens in one of two places. Boards that declare a turntable area have blocks
// carried onto it and turned there; boards without one (the turntable rect is empty) turn
// the carried block in hand instead, on a right click.
class BlocksPuzzle : public RenderActionRecord {
public:
	BlocksPuzzle() : RenderActionRecord(7) {}
	virtual ~BlocksPuzzle() {}

	void init() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

	bool isViewportRelative() const override { return true; }

protected:
	Common::String getRecordTypeName() const override { return "BlocksPuzzle"; }

	enum PuzzleState {
		kPlaying		= 0,	// waiting for the board to come out solved
		kStartSolved	= 1,	// solved: start the success sound
		kTurnFrame		= 2,	// show the next frame of the turn animation
		kTurnDelay		= 3,	// hold that frame for its share of the turn duration
		kWaitSolved		= 4		// let the success sound finish before leaving
	};

	enum SoundID { kTurnSound = 0, kHandleSound = 1, kSuccessSound = 2, kNumSounds = 3 };

	static const int16 kNoBlock = -1;
	// Stands in for a cell index when the turntable area is the thing being clicked.
	static const int16 kTurntableCell = 26;

	// Every sprite of a block lives in a single row of the atlas image, laid out as
	// numRotations groups of (numTweenFrames + 1) frames.
	struct Block {
		byte numRotations = 0;		// 0x00
		int16 numTweenFrames = 0;	// 0x01
		int16 gap = 0;				// 0x03 - padding between atlas frames
		int16 width = 0;			// 0x05
		int16 height = 0;			// 0x07
		int16 atlasX = 0;			// 0x09
		int16 atlasY = 0;			// 0x0b
	};

	struct Cell {
		int16 block = kNoBlock;			// 0x00
		byte rotation = 0;				// 0x02
		int16 targetBlock = kNoBlock;	// 0x03
		uint16 targetRotation = 0;		// 0x05
		Common::Rect dest;				// 0x07
		Common::Rect hotspot;			// 0x17 - the flat centre of the isometric tile
	};

	Common::Rect blockSrc(int16 block, byte rotation, int16 frame) const;
	// The cell whose hotspot contains the cursor, or kNoBlock. Picking up only considers
	// occupied cells; putting down considers every cell.
	int16 cellAtCursor(const Common::Point &mousePos, bool occupiedOnly) const;
	bool isSolved() const;
	void pickUp(int16 cell);
	void drop(int16 cell);
	void startTurn();
	void redraw();
	void setDataCursor(uint16 cursorType) const;
	SoundDescription playSoundBlock(const RandomSoundBlock &block);

	// -- File data (111-byte header) --
	Common::Path _imageName;			// 0x00
	uint16 _turnDuration = 0;			// 0x21 - total ms of one rotation's tween
	uint16 _carryCursorType = 0;		// 0x23 - raw Nancy13 cursor types
	uint16 _turnCursorType = 0;			// 0x25
	FlagDescription _startFlag;			// 0x27 - set as the puzzle opens
	Common::Rect _overlaySrc;			// 0x2a - board dressing, drawn over the blocks
	Common::Rect _overlayDest;			// 0x3a
	Common::Rect _turntableDest;		// 0x4a - empty when the board has no turntable
	Common::Rect _turntableHotspot;		// 0x5a - click to turn the block on the turntable
	SceneChangeDescription _solveScene;	// 0x6a
	FlagDescription _solveFlag;			// 0x6c

	Common::Array<Block> _blocks;
	Common::Array<Cell> _cells;

	// The clickable "give up / exit" hotspot (the base-class hotspot record).
	Common::Rect _exitHotspot;
	uint16 _exitCursorType = 0;
	SceneChangeDescription _exitScene;
	FlagDescription _exitFlag;			// set on give-up

	RandomSoundBlock _sounds[kNumSounds];	// turn / handle / success

	// -- Runtime state --
	PuzzleState _puzzleState = kPlaying;
	bool _hasTurntable = false;

	int16 _carriedBlock = kNoBlock;		// the block on the cursor, or kNoBlock
	byte _carriedRotation = 0;
	Common::Rect _carriedSrc;
	Common::Point _dragPos;				// cursor position (viewport space) while carrying

	int16 _turnBlock = kNoBlock;		// the block being turned, or kNoBlock
	byte _turnRotation = 0;
	Common::Rect _turnSrc;
	int16 _turnFrame = 0;
	uint32 _nextFrameTime = 0;

	bool _solved = false;
	bool _exitRequested = false;
	SoundDescription _solveSound;		// the cue we wait on before changing scene

	Graphics::ManagedSurface _image;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_BLOCKSPUZZLE_H
