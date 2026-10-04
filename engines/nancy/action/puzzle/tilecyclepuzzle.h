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

#ifndef NANCY_ACTION_TILECYCLEPUZZLE_H
#define NANCY_ACTION_TILECYCLEPUZZLE_H

#include "engines/nancy/commontypes.h"
#include "engines/nancy/action/puzzlerecord.h"

namespace Nancy {
namespace Action {

// Nancy17 grid of glass eyes. A click moves a tile, and the tiles linked to it, on
// to their next state. The puzzle never ends by itself: after every turn it sets
// the event flag of each of its goals to whether the tiles are in the goal's states.
// A tile can depend on an event flag, and is not there until that is set.
//
// The game has a single one, in scene 4450. Every eye is found somewhere in the
// game, and has a flag of its own. By row, from left to right:
//   A: 2071 2070 2072 2066 2077
//   B: 2075 2082 2057 2058 2056
//   C: 2059 2060 2080 2079 2081
//   D: 2076 2055 2067 2068 2062
//   E: 2063 2064 2065 2074 2073
class TileCyclePuzzle : public PuzzleRecord {
public:
	TileCyclePuzzle() : PuzzleRecord(7) {}
	virtual ~TileCyclePuzzle() {}

	void init() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

	bool isViewportRelative() const override { return true; }

protected:
	Common::String getRecordTypeName() const override { return "TileCyclePuzzle"; }

	// The states a kind of tile goes through; each is a list of frames
	struct Template {
		Common::String name;
		Common::Array<Common::Array<Common::Rect> > states;
	};

	struct Tile {
		Common::String name;
		int templateID = -1;
		Common::Array<Common::String> linkedTiles;
		Common::Rect dest;
		int16 presentFlag = -1;

		uint state = 0;
		bool isTurning = false;
	};

	struct TileState {
		Common::String tileName;
		int32 state = 0;
	};

	struct Goal {
		Common::Array<TileState> tileStates;
		bool needsAll = false;	// otherwise one tile in its state is enough
		FlagDescription flag;	// gets the opposite value while the goal is not met
	};

	bool isTilePresent(const Tile &tile) const;
	Tile *findTile(const Common::String &name);
	void finishTurn();
	void updateGoals();
	void redraw();

	// -- File data --
	uint16 _hoverCursor = 0;
	uint32 _turnTime = 0;	// in milliseconds

	Common::Array<Template> _templates;
	Common::Array<Tile> _tiles;

	RandomSoundBlock _clickSound;
	RandomSoundBlock _turnedSound;

	Common::Array<Goal> _goals;

	// -- Runtime --
	bool _isTurning = false;
	uint32 _turnEndTime = 0;
	bool _exitRequested = false;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_TILECYCLEPUZZLE_H
