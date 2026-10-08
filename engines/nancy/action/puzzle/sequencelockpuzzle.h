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

#ifndef NANCY_ACTION_SEQUENCELOCKPUZZLE_H
#define NANCY_ACTION_SEQUENCELOCKPUZZLE_H

#include "engines/nancy/commontypes.h"
#include "engines/nancy/action/puzzlerecord.h"

namespace Nancy {
namespace Action {

// Nancy18 sensor lock. A row of switches has to be thrown in an order that is
// picked at random. A switch moves once it is clicked; the wrong one makes all
// of them go back, and the player starts over, with the order staying as it was.
class SequenceLockPuzzle : public PuzzleRecord {
public:
	SequenceLockPuzzle() : PuzzleRecord(7) {}
	virtual ~SequenceLockPuzzle() {}

	void init() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

	bool isViewportRelative() const override { return true; }

protected:
	Common::String getRecordTypeName() const override { return "SequenceLockPuzzle"; }

	enum Direction { kBackward = -1, kStopped = 0, kForward = 1 };

	struct Switch {
		Common::Rect dest;
		Common::Rect hotspot;

		int frame = 0;
		int direction = kStopped;
	};

	void pressSwitch(uint switchID);
	void resetSwitches();
	bool animateSwitches(uint numFrames);
	void redraw();

	// -- File data --
	uint16 _hoverCursor = 0;
	Common::Array<Switch> _switches;
	int32 _resetFrame = 0;		// a wrong switch gets this far before they all go back
	int32 _framesPerSecond = 0;
	Common::Array<Common::Rect> _frameSrcs;

	RandomSoundBlock _pressSound;
	RandomSoundBlock _resetSound;
	RandomSoundBlock _solveSoundBlock;

	// -- Runtime --
	Common::Array<uint> _order;
	uint _numPressed = 0;
	int _wrongSwitch = -1;
	uint32 _lastFrameTime = 0;
	bool _isSolved = false;
	bool _exitRequested = false;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_SEQUENCELOCKPUZZLE_H
