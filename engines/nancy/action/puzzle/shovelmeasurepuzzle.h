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

#ifndef NANCY_ACTION_SHOVELMEASUREPUZZLE_H
#define NANCY_ACTION_SHOVELMEASUREPUZZLE_H

#include "engines/nancy/commontypes.h"
#include "engines/nancy/action/puzzlerecord.h"

namespace Nancy {
namespace Action {

// Nancy17 cemetery plot. A shovel is turned and moved over a grid, with the arrow
// keys and Enter or with the buttons on screen. A measuring tape shows how many
// steps there are, in the direction the shovel faces, to the end of one of the
// stretches of the path that has to be followed; with no such end ahead the tape
// shows a number that means nothing. Leaving the path too many times in a row
// loses, getting to its end wins.
class ShovelMeasurePuzzle : public PuzzleRecord {
public:
	ShovelMeasurePuzzle() : PuzzleRecord(7) {}
	virtual ~ShovelMeasurePuzzle() {}

	void init() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

	bool isViewportRelative() const override { return true; }

protected:
	Common::String getRecordTypeName() const override { return "ShovelMeasurePuzzle"; }

	enum Direction {
		kNorth,
		kSouth,
		kEast,
		kWest,
		kNumDirections,
		kNoDirection = -1
	};

	enum Command {
		kNoCommand,
		kFace,
		kStep
	};

	struct Cell {
		bool isValid = false;
		bool endsStretch[kNumDirections] = { false, false, false, false };
	};

	// A stretch of the path
	struct Stretch {
		Direction direction = kNorth;
		Common::Point from;
		Common::Point to;
	};

	enum Outcome {
		kNone,
		kWon,
		kLost,
		kExited
	};

	static Direction readDirection(Common::SeekableReadStream &stream);
	static Common::Point getNeighbor(const Common::Point &pos, Direction direction);
	// Clockwise, in radians
	static double getAngle(Direction direction);

	bool isValidCell(const Common::Point &pos) const;
	Cell &getCell(const Common::Point &pos) { return _cells[pos.y * _numColumns + pos.x]; }
	const Cell &getCell(const Common::Point &pos) const { return _cells[pos.y * _numColumns + pos.x]; }
	Common::Point getCellCenter(const Common::Point &pos) const;

	void face(Direction direction);
	void step();
	void finishMove();
	void updateMeasure();

	// Draws a picture turned clockwise around a point of its own, which goes to a point on screen
	void drawRotated(const Common::Rect &src, const Common::Point &srcPivot, const Common::Point &destPivot, double angle);
	void redraw();

	// -- File data --
	uint16 _buttonCursor = 0;
	int32 _maxWrongSteps = 0;
	uint32 _moveTime = 0;	// in milliseconds

	Common::Rect _directionButtons[kNumDirections];
	Common::Rect _dialSrc;
	Common::Rect _dialDest;
	Common::Point _dialPivot;	// relative to the dial
	Common::Rect _stepPressedSrc;
	Common::Rect _stepButton;

	Common::Point _startPos;
	Direction _startDirection = kNorth;

	Common::Rect _shovelSrc;
	Common::Rect _tapeDest;
	// The pictures of the tape being pulled out to each length
	Common::Array<Common::Array<Common::Rect> > _tapeSrcs;
	uint _maxMeasure = 0;

	Common::Rect _firstCell;
	Common::Point _cellSpacing;
	int _numColumns = 0;
	int _numRows = 0;
	Common::Array<Cell> _cells;
	Common::Array<Stretch> _path;

	RandomSoundBlock _turnSound;
	RandomSoundBlock _stepSound;
	RandomSoundBlock _winSound;
	SceneChangeWithFlag _loseScene;
	RandomSoundBlock _loseSound;

	// -- Runtime --
	Common::Point _pos;
	Common::Point _targetPos;
	Direction _direction = kNorth;
	Direction _targetDirection = kNorth;
	Command _move = kNoCommand;
	uint32 _moveStartTime = 0;

	uint _measure = 0;
	uint _shownMeasure = 0;
	// Where the tape last showed a meaningless number from; counting stops there
	Common::Point _lastGuessPos;

	uint _currentStretch = 0;
	int32 _numWrongSteps = 0;
	Outcome _outcome = kNone;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_SHOVELMEASUREPUZZLE_H
