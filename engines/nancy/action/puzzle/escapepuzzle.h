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

#ifndef NANCY_ACTION_ESCAPEPUZZLE_H
#define NANCY_ACTION_ESCAPEPUZZLE_H

#include "engines/nancy/commontypes.h"
#include "engines/nancy/action/puzzlerecord.h"

namespace Nancy {
namespace Action {

// Nancy17 maze with lasers. The player is walked one cell at a time with the arrow
// keys or the buttons on screen. Walking into a ball or a box pushes it: a ball rolls
// until something stops it, a box moves one cell. Each laser shines in the direction
// it faces until a wall or an object is in the way; the player can't cross a beam,
// but balls and boxes can block it. Getting an object to its goal cell wins.
class EscapePuzzle : public PuzzleRecord {
public:
	EscapePuzzle() : PuzzleRecord(7) {}
	virtual ~EscapePuzzle() {}

	void init() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

	bool isViewportRelative() const override { return true; }

protected:
	Common::String getRecordTypeName() const override { return "EscapePuzzle"; }

	enum Direction {
		kNorth,
		kSouth,
		kEast,
		kWest,
		kNumDirections,
		kNoDirection = -1
	};

	// What a cell holds, as far as something moving into it is concerned
	enum CellContents {
		kEmpty,
		kObject,
		kLaserBeam,
		kWall
	};

	enum ObjectType {
		kPlayer,
		kBall,
		kBox,
		kLaser,
		kNumObjectTypes
	};

	enum Motion {
		kStill,
		kMoving,
		kPushing
	};

	// The pictures of an object facing each direction
	struct DirectionalAnimation {
		Common::Array<Common::Rect> frames[kNumDirections];

		void readData(Common::SeekableReadStream &stream);
	};

	// What all objects of a type share
	struct ObjectKind {
		uint32 moveTime = 0;	// in milliseconds
		DirectionalAnimation animation;
		DirectionalAnimation secondAnimation;	// the player pushing, a laser's beam
		RandomSoundBlock moveSound;
		RandomSoundBlock pushSound;
		RandomSoundBlock bumpSound;
	};

	struct BeamCell {
		Common::Point pos;
		Direction direction = kNorth;
	};

	struct Object {
		Common::String name;
		ObjectType type = kPlayer;
		Common::Point pos;
		Direction facing = kNorth;
		Direction movingTo = kNoDirection;	// where the object is about to go
		Motion motion = kStill;
		uint32 time = 0;	// since the object last moved
		Common::Array<BeamCell> beam;
	};

	struct Goal {
		Common::String objectName;
		Common::Point pos;
		SceneChangeWithFlag scene;
		RandomSoundBlock sound;
	};

	struct Button {
		Direction direction = kNorth;
		Common::Rect pressedSrc;
		Common::Rect dest;
		bool isPressed = false;
		uint32 pressTime = 0;
	};

	static Direction readDirection(Common::SeekableReadStream &stream);
	static Common::Point getNeighbor(const Common::Point &pos, Direction direction);
	static Direction getOpposite(Direction direction);
	static bool compareObjectNames(const Object &a, const Object &b);

	Common::Rect getCellRect(const Common::Point &pos) const;
	CellContents getContents(const Common::Point &pos) const;
	Object *findObject(const Common::String &name);
	void push(const Common::Point &pos, Direction direction);
	// Returns whether the object moved
	bool updateObject(Object &object, uint32 time);
	void updateBeam(Object &laser);
	void pressButton(Button &button);
	void drawObject(const Object &object);
	void redraw();

	// -- File data --
	uint32 _buttonPressTime = 0;	// in milliseconds
	Button _buttons[kNumDirections];
	uint16 _buttonCursor = 0;
	Common::Rect _firstCell;
	Common::Point _cellSpacing;
	int _numColumns = 0;
	int _numRows = 0;
	Common::Array<bool> _walls;
	ObjectKind _kinds[kNumObjectTypes];
	Common::Array<Object> _objects;	// in the order of their names
	Common::Array<Goal> _goals;

	// -- Runtime --
	Common::String _playerName;
	uint32 _lastUpdateTime = 0;
	Direction _heldKey = kNoDirection;
	int _reachedGoal = -1;
	bool _exitRequested = false;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_ESCAPEPUZZLE_H
