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

#ifndef NANCY_ACTION_SKEEBALLPUZZLE_H
#define NANCY_ACTION_SKEEBALLPUZZLE_H

#include "engines/nancy/commontypes.h"
#include "engines/nancy/movieplayer.h"
#include "engines/nancy/action/puzzlerecord.h"
#include "engines/nancy/action/physicalobject.h"

namespace Nancy {
namespace Action {

// Nancy17 skee ball. A click on the dispenser brings out a ball; the ball is pulled
// back and let go to roll it. Past the ramp it jumps, and where it comes down the
// nearest target spins on to its next picture. Each goal is met once every target
// shows its picture.
class SkeeBallPuzzle : public PuzzleRecord {
public:
	SkeeBallPuzzle() : PuzzleRecord(7) {}
	virtual ~SkeeBallPuzzle();

	void init() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

	bool isViewportRelative() const override { return true; }

protected:
	Common::String getRecordTypeName() const override { return "SkeeBallPuzzle"; }

	// A picture a target can show, by the frame that shows it
	struct TargetState {
		Common::String name;
		int32 frame = 0;
	};

	struct Target {
		Common::Rect dest;
		uint stateID = 0;
		int32 frame = 0;

		bool isSpinning = false;
		uint32 spinStartTime = 0;
		int32 spinStartFrame = 0;
		int32 spinEndFrame = 0;
	};

	struct Ball {
		PhysicalObject body;
		uint32 lastUpdateTime = 0;
		double height = 0;			// above the lane, in pixels
		double verticalSpeed = 0;
		double frame = 0;
	};

	struct Goal {
		uint stateID = 0;
		SceneChangeWithFlag scene;
		RandomSoundBlock sound;
		bool isMet = false;
	};

	static bool compareStateNames(const TargetState &a, const TargetState &b);
	uint findState(const Common::String &name) const;
	void createBall();
	// Returns whether the ball is gone
	bool updateBall(Ball &ball, uint32 numTicks);
	void spinTarget(Target &target);
	void updateTargets();
	void drawBall(const Ball &ball);
	void drawArrow();
	void redraw();

	// -- File data --
	int32 _numSpins = 0;			// whole turns a target makes before it stops
	uint32 _spinTime = 0;			// in milliseconds
	Common::Array<Common::Rect> _targetSrcs;
	Common::Array<TargetState> _states;
	Common::Array<Target> _targets;

	uint16 _dispenserCursor = 0;
	Common::Rect _dispenserHotspot;
	Common::Rect _dispenserDest;
	Common::Path _dispenserMovieName;

	uint16 _aimCursor = 0;
	Common::Rect _arrowSrc;
	Common::Rect _aimHotspot;
	double _minAngle = 0;
	double _maxAngle = 0;
	double _minPower = 0;
	double _maxPower = 0;

	Common::Rect _laneBounds;
	Common::Rect _ramp;
	double _friction = 0;
	Common::Rect _ballStart;
	double _gravity = 0;
	double _heightScale = 0;		// a ball this high is drawn twice its size
	Common::Array<Common::Rect> _ballSrcs;

	RandomSoundBlock _pickUpSound;
	RandomSoundBlock _rollSound;
	RandomSoundBlock _spinSound;
	Common::Array<Goal> _goals;

	// -- Runtime --
	MoviePlayer _dispenserMovie;
	Ball *_heldBall = nullptr;
	bool _isAiming = false;
	double _angle = 0;
	double _power = 0;
	Common::Array<Ball *> _balls;
	uint32 _lastUpdateTime = 0;
	int _metGoal = -1;
	bool _exitRequested = false;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_SKEEBALLPUZZLE_H
