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

#ifndef NANCY_ACTION_WOLFMAZEPUZZLE_H
#define NANCY_ACTION_WOLFMAZEPUZZLE_H

#include "engines/nancy/commontypes.h"
#include "engines/nancy/action/puzzlerecord.h"

namespace Nancy {
namespace Action {

// Nancy16 wolf maze (AR 199). Isis runs through a map of the sewers, following the
// commands given to her through WolfCommands. Moves turn her and send her running until
// she reaches a junction or a wall, following the bends of the tunnels on the way. She
// jumps over pits and paws switches, which open barricades. Once the commands run out,
// or a closed barricade blocks her, she comes back the way she went; reaching the exit
// solves the puzzle. Only the tracks she leaves on the map get drawn.
class WolfMazePuzzle : public PuzzleRecord {
public:
	WolfMazePuzzle() : PuzzleRecord(7) {}
	virtual ~WolfMazePuzzle() {}

	void init() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

	bool isViewportRelative() const override { return true; }

protected:
	Common::String getRecordTypeName() const override { return "WolfMazePuzzle"; }

	enum TileType {
		kEmpty,
		kPath,
		kTrack,			// a path that shows the wolf's tracks
		kJunction,		// a path with more than two neighbors, where the wolf stops
		kPit,
		kSwitch,
		kBarricadeAcross,	// in a path running east to west
		kBarricadeUpDown,	// in a path running north to south
		kExit
	};

	// Clockwise from north
	enum Direction {
		kNorth, kNorthEast, kEast, kSouthEast, kSouth, kSouthWest, kWest, kNorthWest,
		kNumDirections
	};

	enum Command {
		kCommandNone,
		kRight,
		kLeft,
		kForward,
		kBackward,
		kAround,
		kBark,
		kJump,
		kPaw,
		kReturn			// given once the queued commands run out
	};

	enum WolfStatus {
		kDone,			// standing, waiting for a command
		kRunning,
		kExited,
		kReturned,
		kBlocked
	};

	struct Cell {
		TileType type = kEmpty;
		Common::Rect dest;
		uint visits = 0;
		FlagDescription flag;	// a switch sets it; a barricade is open while it is set
	};

	struct Ending {
		SceneChangeWithFlag scene;
		RandomSoundBlock sound;
	};

	static Common::Point step(const Common::Point &cell, int direction);
	static Command commandFromName(const Common::String &name);
	static void readEnding(Common::SeekableReadStream &stream, Ending &ending);

	bool isValid(const Common::Point &cell) const;
	Cell &cellAt(const Common::Point &cell) { return _cells[cell.y * _width + cell.x]; }
	const Cell &cellAt(const Common::Point &cell) const { return _cells[cell.y * _width + cell.x]; }
	TileType typeAt(const Common::Point &cell) const { return isValid(cell) ? cellAt(cell).type : kEmpty; }
	int countNeighbors(const Common::Point &cell, bool orthogonalOnly) const;
	bool hasNeighbor(const Common::Point &cell, int direction) const;

	void enterCell(const Common::Point &cell);
	void followBend();
	WolfStatus updateWolf(uint32 elapsed);
	bool runCommand(Command command);
	void startEnding(const Ending &ending);

	void redraw();

	// -- File data --
	Common::String _playlistName;
	uint16 _cursor = 0;
	uint _width = 0;
	uint _height = 0;
	Common::Array<Cell> _cells;
	Common::Rect _trackSrc;			// in _image
	Common::Point _wolfStart;
	int _wolfStartDirection = kNorth;
	uint32 _moveTime = 0;			// ms per cell
	uint32 _returnTime = 0;
	Common::HashMap<Common::String, Command, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> _movieCommands;
	RandomSoundBlock _commandSound;
	RandomSoundBlock _blockedSound;
	RandomSoundBlock _invalidSound;
	RandomSoundBlock _runSound;
	Ending _failEnding;
	Ending _solveEnding;

	// -- Runtime state --
	Common::Array<Command> _commands;
	Common::Point _wolf;
	int _direction = kNorth;
	bool _isMoving = false;
	bool _isJumping = false;
	bool _isReturning = false;
	uint32 _timer = 0;
	Common::Array<Common::Point> _track;	// the cells the wolf went through, to come back by
	SoundDescription _runSoundDesc;

	uint32 _lastTime = 0;
	bool _isEnding = false;
	bool _exitRequested = false;
	SceneChangeWithFlag _endScene;
	SoundDescription _endSound;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_WOLFMAZEPUZZLE_H
