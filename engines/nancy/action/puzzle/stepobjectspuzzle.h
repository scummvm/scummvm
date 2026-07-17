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

#ifndef NANCY_ACTION_STEPOBJECTSPUZZLE_H
#define NANCY_ACTION_STEPOBJECTSPUZZLE_H

#include "engines/nancy/commontypes.h"
#include "engines/nancy/action/actionrecord.h"

namespace Nancy {
namespace Action {

// Nancy13 AR 169
// Dance-step puzzle. Each object (a shoe) is dragged across a grid of
// cells (the dance floor). A footprint is left on each cell that a dance
// shoe visits, so each cell can only be stepped on once, and the overall
// dance has to be done in one go. The puzzle is solved once the sequence
// of dance steps matches the scripted one.
class StepObjectsPuzzle : public RenderActionRecord {
public:
	StepObjectsPuzzle() : RenderActionRecord(7) {}
	virtual ~StepObjectsPuzzle() {}

	void init() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

	bool isViewportRelative() const override { return true; }

protected:
	Common::String getRecordTypeName() const override { return "StepObjectsPuzzle"; }

	enum SoundID {
		kSoundPickUp = 0,
		kSoundStep = 1,
		kSoundCorrectStep = 2,
		kSoundWrongStep = 3,
		kSoundReset = 4,
		kSoundSolved = 5,
		kNumSounds = 6
	};

	enum PuzzleState {
		kIdle,
		kStepping,	// a footstep sound is playing; the board ignores input
		kSolved
	};

	// A single placement: which object was put down, and on which cell.
	struct Step {
		byte objectID = 0;
		int16 row = 0;
		int16 col = 0;
	};

	struct StepObject {
		bool undoWrongStep = false;
		Common::Rect srcRect;
		Common::Rect footprintSrcRect;	// when empty, the object's own sprite is used, faded
		Common::Point gridOrigin;
		uint16 startRow = 0;
		uint16 startCol = 0;

		int16 row = 0;
		int16 col = 0;
	};

	Common::Rect getCellRect(const StepObject &object, int row, int col) const;
	bool isHovered(const Common::Rect &viewportRect, const Common::Point &mousePos) const;
	bool cellAtCursor(const StepObject &object, const Common::Point &mousePos, int &outRow, int &outCol) const;
	bool canStepOn(uint objectID, int row, int col) const;
	bool isSolutionMatched() const;

	void pickUp(uint objectID);
	void drop(int row, int col);
	void resetBoard();
	void beginStepSound(SoundID sound, bool isDrop);
	void setDataCursor(uint16 cursorType) const;

	void redraw();
	void drawSprite(const Common::Rect &srcRect, const Common::Point &destPos, byte alpha);
	SoundDescription playSoundBlock(const RandomSoundBlock &block);

	// File data
	Common::Path _imageName;
	// The puzzle carries its own cursor as a raw Nancy13 cursor type id; the same
	// one is used for hovering and for carrying an object.
	uint16 _cursorType = 0;
	uint16 _numRows = 0;
	uint16 _numCols = 0;
	uint16 _pitchY = 0;
	uint16 _pitchX = 0;
	SceneChangeDescription _solveScene;
	FlagDescription _solveFlag;
	uint16 _numSteps = 0;

	Common::Array<Step> _solution;
	Common::Array<StepObject> _objects;

	// The clickable "give up / leave" hotspot.
	Common::Rect _exitHotspot;
	uint16 _exitCursorType = 0;
	SceneChangeDescription _exitScene;

	Common::Array<RandomSoundBlock> _sounds;

	// Runtime state
	Common::Array<Step> _trail;
	Common::Array<Step> _playerSteps;
	PuzzleState _puzzleState = kIdle;
	int _carriedID = -1;
	Common::Rect _carriedRect;
	uint32 _stepSoundEnd = 0;
	bool _lastStepWasDrop = false;
	bool _lastStepCorrect = false;
	bool _solved = false;
	bool _exitRequested = false;
	SoundDescription _solveSound;

	Graphics::ManagedSurface _image;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_STEPOBJECTSPUZZLE_H
