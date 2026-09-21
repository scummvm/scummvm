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

#ifndef NANCY_ACTION_NECKLACEPUZZLE_H
#define NANCY_ACTION_NECKLACEPUZZLE_H

#include "engines/nancy/commontypes.h"
#include "engines/nancy/action/actionrecord.h"
#include "engines/nancy/misc/mousefollow.h"

namespace Nancy {
namespace Action {

// Nancy15 rewrote AR 243 into a shell-stringing puzzle: a box of shells on one
// side, and a set of strands hanging from needles on the other. A shell is taken
// from the box onto the cursor and dropped on a needle, where it slides down the
// strand and stacks on the ones already there. Every strand has to end up holding
// the exact sequence the design calls for.
//
// How many of each shell are left is not kept in the record; it lives in the
// shared table values, so the box counts survive leaving and re-entering. Shells
// left on a strand are handed back when the puzzle is abandoned unsolved.
class NecklacePuzzle : public RenderActionRecord {
public:
	NecklacePuzzle() : RenderActionRecord(7) {}
	virtual ~NecklacePuzzle();

	void init() override;
	void registerGraphics() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

	bool isViewportRelative() const override { return true; }

protected:
	Common::String getRecordTypeName() const override { return "NecklacePuzzle"; }

	static const int16 kNoBead = -1;
	static const int16 kNoTableIndex = 255;

	// A kind of shell. All shells of a kind share their three sprite sheets: one
	// for the cursor, one for the strand, one for the box.
	struct BeadType {
		int32 key = 0;
		bool overlaps = false;		// this kind sits into the one below it
		int cursorImage = -1;		// indices into _images
		int strandImage = -1;
		int boxImage = -1;
	};

	struct Bead {
		byte type = 0;				// BeadType::key
		Common::Rect cursorSrc;
		Common::Rect strandSrc;
		Common::Rect boxSrc;
		Common::Rect boxDest;		// also the hotspot for taking one out of the box
		int16 tableIndex = kNoTableIndex;	// table value holding how many are left
	};

	struct PlacedBead {
		int16 bead = kNoBead;
		Common::Rect rect;
	};

	struct Strand {
		bool grabByTopBead = false;	// take the top shell back by clicking it, not the needle
		bool allowSwap = false;		// a full strand lets its top shell be replaced
		Common::Array<byte> allowedTypes;
		Common::Array<uint16> solution;	// required table indices, top of the strand first
		Common::Rect needleHotspot;
		Common::Rect strandRect;
		float dropSeconds = 0.0f;	// time a shell takes to slide the strand's full length

		Common::Array<PlacedBead> placed;
	};

	// What the cursor is over. Only one of the two is ever set.
	struct Hover {
		int strand = -1;
		int bead = -1;
		bool swapTop = false;		// dropping here replaces the strand's top shell
	};

	int findBeadType(byte key) const;
	bool strandAccepts(const Strand &strand, byte type) const;

	// Y the next shell dropped on `strand` comes to rest against, and the same one
	// level further down (used to decide whether a full strand can still swap).
	int stackTop(const Strand &strand) const;
	int stackTopBelow(const Strand &strand) const;

	Hover hitTest(const Common::Point &mousePos) const;

	int16 beadCount(const Bead &bead) const;
	void adjustBeadCount(const Bead &bead, int16 delta);

	void holdBead(int16 bead, NancyInput *input);
	void releaseBead();
	void dropBeadOn(int strand);
	void checkSolved();
	void refundBeads();

	void playSoundBlock(const RandomSoundBlock &block);
	void redraw();
	void drawBeadCount(const Bead &bead);

	// -- File data --
	bool _refundOnExit = false;
	int32 _overlapPlaced = 0;		// taken off an overlapping shell's width on a strand
	int32 _overlapFalling = 0;		// how far an overlapping shell sinks into the one below
	FlagDescription _dropFlag;		// set every time a shell is strung
	uint16 _heldCursorType = 0;
	uint16 _hoverCursorType = 0;
	int32 _heldOffsetX = 0;			// where the held shell sits relative to the cursor
	int32 _heldOffsetY = 0;
	uint16 _countFontID = 0;
	uint16 _unknownA0 = 0;
	uint16 _countTextExtraY = 0;
	int32 _countTextX = 0;			// offset from the box rect's top left
	int32 _countTextY = 0;

	Common::Array<Common::Path> _imageNames;
	Common::Array<BeadType> _beadTypes;
	Common::Array<Bead> _beads;
	Common::Array<Strand> _strands;

	RandomSoundBlock _pickUpSound;
	RandomSoundBlock _putDownSound;
	RandomSoundBlock _unknownSound;	// no known trigger; "NO SOUND" in the shipped record

	SceneChangeDescription _solvedScene;
	RandomSoundBlock _solvedSound;

	// Played when the puzzle is left unfinished. Its scene is 9999 in the shipped
	// record, so the exit hotspot's own scene is what the player ends up in.
	SceneChangeDescription _unsolvedScene;
	RandomSoundBlock _unsolvedSound;

	Common::Rect _exitHotspot;
	uint16 _exitCursorType = 0;
	SceneChangeDescription _exitScene;
	FlagDescription _exitFlag;

	// -- Runtime state --
	Common::Array<Graphics::ManagedSurface> _images;

	int16 _heldBead = kNoBead;
	Misc::MouseFollowObject _heldBeadObject;

	int _fallingStrand = -1;
	int _fallingStartY = 0;
	int _fallingEndY = 0;
	uint32 _fallStartTime = 0;
	uint32 _fallDuration = 0;

	bool _solved = false;
	bool _exitRequested = false;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_NECKLACEPUZZLE_H
