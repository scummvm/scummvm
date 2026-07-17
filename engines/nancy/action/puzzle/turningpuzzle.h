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

#ifndef NANCY_ACTION_TURNINGPUZZLE_H
#define NANCY_ACTION_TURNINGPUZZLE_H

#include "engines/nancy/action/actionrecord.h"

namespace Nancy {
namespace Action {

// Handles a specific type of puzzle where clicking an object rotates it,
// as well as several other objects linked to it. Examples are the sun/moon
// and staircase spindle puzzles in nancy3.
//
// Nancy13 moved this record from type 209 to 171 and reworked its chunk, but kept the
// mechanic identical. The differences: every object references a "type" that carries
// its own face count, turn-frame count and sprite geometry (instead of one global pair);
// the links are a byte array; each object stores its own destination rect; and the
// solution is given as up to three alternative face orders.
class TurningPuzzle : public RenderActionRecord {
public:
	enum SolveState { kNotSolved, kWaitForAnimation, kWaitBeforeSound, kWaitForSound };
	TurningPuzzle() : RenderActionRecord(7) {}
	virtual ~TurningPuzzle() {}

	void init() override;
	void updateGraphics() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

	bool isViewportRelative() const override { return true; }

protected:
	Common::String getRecordTypeName() const override { return "TurningPuzzle"; }

	// Nancy13: the per-object sprite/turn description its objects point at.
	struct PieceType {
		byte numFaces = 0;
		int16 framesPerTurn = 0;
		int16 gap = 0;			// horizontal padding between sprites in the strip
		int16 cellW = 0;
		int16 cellH = 0;
		int16 srcStartX = 0;
		int16 srcStartY = 0;	// this type's row in the strip
	};

	void readDataNancy13(Common::SeekableReadStream &stream);
	bool isSolved() const;
	uint numFacesOf(uint objectID) const;
	uint framesPerTurnOf(uint objectID) const;
	void drawAllObjects();
	SoundDescription playSoundBlock(const RandomSoundBlock &block);

	void drawObject(uint objectID, uint faceID, uint frameID);
	void turnLogic(uint objectID);

	Common::Path _imageName;

	uint16 _numFaces = 0;
	uint16 _numFramesPerTurn = 0;

	Common::Array<Common::Rect> _destRects;
	Common::Array<Common::Rect> _hotspots;
	Common::Array<uint16> _startPositions;

	bool _separateRows = false;

	Common::Point _startPos;
	Common::Point _srcIncrement;

	Common::Array<Common::Array<uint16>> _links;

	uint16 _solveDelayBetweenTurns = 0;
	bool _solveAnimate = false;
	uint16 _solveAnimationNumRepeats = 0;

	SoundDescription _turnSound;

	Common::Array<uint16> _correctOrder;

	SceneChangeWithFlag _solveScene;
	uint16 _solveSoundDelay = 0;
	SoundDescription _solveSound;

	SceneChangeWithFlag _exitScene;
	Common::Rect _exitHotspot;

	// -- Nancy13 only --
	Common::Array<PieceType> _pieceTypes;
	Common::Array<uint16> _pieceTypeIDs;					// per object
	Common::Array<Common::Array<uint16>> _correctOrders;	// up to three alternative solutions
	uint16 _turnDelay = 0;			// header 0x21 - length of a whole turn, in ms
	uint16 _hoverCursorType = 0;	// header 0x23 - raw Nancy13 cursor id (a turn cursor)
	uint16 _hitInset = 0;			// header 0x25 - hotspots are the dest rect shrunk by this
	int16 _turnFlagLabel = -1;		// header 0x27 - set once the player turns anything
	byte _turnFlagValue = 0;		// header 0x29
	uint16 _exitCursorType = 0;		// from the exit hotspot record
	RandomSoundBlock _turnSoundBlock;
	RandomSoundBlock _solveSoundBlock;
	bool _turnFlagSet = false;

	Graphics::ManagedSurface _image;
	Common::Array<uint16> _currentOrder;

	uint32 _solveSoundDelayTime = 0;
	uint32 _nextTurnTime = 0;
	int32 _objectCurrentlyTurning = -1;
	uint32 _turnFrameID = 0;

	uint32 _solveAnimLoop = 0;
	uint32 _solveAnimFace = 0;

	SolveState _solveState = kNotSolved;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_TURNINGPUZZLE_H
