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

#ifndef NANCY_ACTION_ESCAPEGRIDPUZZLE_H
#define NANCY_ACTION_ESCAPEGRIDPUZZLE_H

#include "engines/nancy/commontypes.h"
#include "engines/nancy/action/puzzlerecord.h"

namespace Nancy {

class MoviePlayer;

namespace Action {

// Hex-grid race, new in Nancy15 (AR 184). The player and computer opponents take turns
// jumping to a neighboring solid hex, each racing to reach one of its goal cells first.
// Landing on a hex slides the whole line of hexes through it one step in the direction
// of the jump, carrying anyone standing on it, with the front hex wrapping around to the
// back; which hexes are solid and which are missing shifts along with it.
class EscapeGridPuzzle : public PuzzleRecord {
public:
	EscapeGridPuzzle() : PuzzleRecord(7) {}
	virtual ~EscapeGridPuzzle();

	void init() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

	bool isViewportRelative() const override { return true; }

	// Bits of a tile type id.
	enum TileFlags : int32 {
		kTileWalkable	= 0x001,
		kTileMovable	= 0x010,	// slides along when a line is pushed
		kTileMissing	= 0x100		// overrides kTileWalkable
	};

	// The six neighbors of a hex, in the order they are searched.
	enum Direction {
		kNorth		= 0,
		kNorthEast	= 1,
		kSouthEast	= 2,
		kSouth		= 3,
		kSouthWest	= 4,
		kNorthWest	= 5,
		kNumDirections
	};

	// Jump animations are indexed clockwise from north; a hex leaves east and west empty.
	static const uint kNumCompassDirections = 8;

	// A character's animations while it rides one kind of tile.
	enum CharacterTileAnim {
		kCharRise		= 0,
		kCharSinkRider	= 1,	// carried along a sliding line
		kCharSink		= 2,	// on the hex it just landed on
		kNumCharTileAnims
	};

	struct Animation {
		Common::Path name;
		Common::Array<Common::Rect> srcs;
	};

	struct TileType {
		int32 id = 0;
		Animation sink;
		RandomSoundBlock sinkSound;
		Animation rise;
		RandomSoundBlock riseSound;
		Common::Path imageName;
		Common::Rect src;
	};

	struct CharacterTileAnims {
		int32 tileTypeID = 0;
		Animation anims[kNumCharTileAnims];
	};

	struct Character {
		Common::Path imageName;
		Common::Rect src;
		int32 offsetX = 0;
		int32 offsetY = 0;
		Common::Array<CharacterTileAnims> tileAnims;
		Common::Path jumpNames[kNumCompassDirections];
		int32 jumpOffsetX[kNumCompassDirections] = {};
		int32 jumpOffsetY[kNumCompassDirections] = {};
	};

	struct Actor {
		int32 characterID = 0;
		Common::Point start;
		Common::Array<Common::Point> goals;
		bool isPlayer = false;
	};

protected:
	Common::String getRecordTypeName() const override { return "EscapeGridPuzzle"; }

	// Animations are timed in ticks of kTickLength ms.
	enum TileAnimKind {
		kTileStay,		// keeps showing the given tile
		kTileBlank,		// shows nothing
		kTileSink,
		kTileRise
	};

	struct TileAnim {
		TileAnimKind kind = kTileStay;
		int32 tileID = 0;
		int32 total = 0;
		int32 remaining = 0;
	};

	enum ActorAnimKind {
		kActorStay,
		kActorJump,
		kActorSink,
		kActorSinkRider,
		kActorRise
	};

	struct ActorAnim {
		ActorAnimKind kind = kActorStay;
		Common::Point from;
		Common::Point to;
		int32 tileID = 0;
		uint compassDir = 0;
		int32 total = 0;
		int32 remaining = 0;
	};

	struct TileImages {
		Graphics::ManagedSurface staticImage;
		Graphics::ManagedSurface sink;
		Graphics::ManagedSurface rise;
	};

	struct CharacterImages {
		Graphics::ManagedSurface root;
		Common::Array<Graphics::ManagedSurface> tileAnims;	// kNumCharTileAnims per entry of Character::tileAnims
	};

	static void readAnimation(Common::SeekableReadStream &stream, Animation &dst);

	static Common::Point step(const Common::Point &cell, Direction dir);
	static bool directionTo(const Common::Point &from, const Common::Point &to, Direction &dir);
	static int32 framesToTicks(uint numFrames);
	static uint frameAt(uint numFrames, int32 elapsed);

	Common::Rect cellRect(const Common::Point &cell) const;
	bool validCell(const Common::Point &cell) const;
	int32 &tileAt(const Common::Point &cell) { return _grid[cell.y * _numCols + cell.x]; }
	int32 tileAt(const Common::Point &cell) const { return _grid[cell.y * _numCols + cell.x]; }
	int tileTypeIndex(int32 id) const;
	const TileType *tileType(int32 id) const;
	const CharacterTileAnims *characterTileAnims(uint actor, int32 tileID, int &index) const;
	int actorAt(const Common::Point &cell) const;

	bool canStepOnto(const Common::Array<int32> &grid, const Common::Array<Common::Point> &cells,
		const Common::Point &cell) const;
	void collectLine(const Common::Array<int32> &grid, const Common::Point &from, const Common::Point &to,
		Common::Array<Common::Point> &line) const;
	void rotateLine(Common::Array<int32> &grid, Common::Array<Common::Point> &cells,
		const Common::Array<Common::Point> &line) const;
	void slideBoard(const Common::Point &from, const Common::Point &to);

	int32 tileSinkTime(int32 tileID) const;
	int32 tileRiseTime(int32 tileID) const;
	int32 characterSinkTime(uint actor, int32 tileID) const;
	int32 characterRiseTime(uint actor, int32 tileID) const;

	void pushTileAnim(const Common::Point &cell, TileAnimKind kind, int32 tileID, int32 time);
	void pushActorAnim(uint actor, ActorAnimKind kind, const Common::Point &cell, int32 tileID, int32 time);
	int32 cellQueueTime(const Common::Point &cell) const;
	int32 actorQueueTime(uint actor) const;
	bool actorsAnimating() const;
	void updateActorAnims(int32 ticks);
	bool updateTileAnims(int32 ticks);

	bool hasAnyMove(uint actor) const;
	Common::Point chooseOpponentMove(uint actor);
	void searchPath(uint actor, int depth, Common::Array<int32> &grid, Common::Array<Common::Point> &cells,
		Common::Array<Common::Point> &path, int32 &bestDist, Common::Array<Common::Point> &bestPath) const;

	void startMove(const Common::Point &target);
	void updateTurn(int32 ticks);
	// The outcome the race has reached, or -1 while it is still on.
	int checkOutcome() const;
	void endPuzzle(int outcome);

	bool tileRefColor(const Common::Point &cell, const Common::Point &pos, byte &r, byte &g, byte &b) const;
	bool isCellPixel(const Common::Point &cell, const Common::Point &pos) const;
	bool findHoveredCell(const Common::Point &mousePos, Common::Point &outCell) const;

	void drawAnimFrame(const Graphics::ManagedSurface &image, const Animation &anim, int32 elapsed,
		const Common::Point &pos);
	void drawJump(uint actor, const ActorAnim &anim);
	void drawActor(uint actor);
	void drawCell(const Common::Point &cell);
	void redraw();

	// -- File data --
	uint16 _busyCursorType = 0;		// while it isn't the player's move
	uint16 _hoverCursorType = 0;	// over a hex the player can jump onto
	Common::Path _tileRefName;		// hit mask for a single hex, see findHoveredCell()
	int32 _defaultSinkTime = 0;		// for animations without frames
	int32 _defaultRiseTime = 0;

	Common::Array<TileType> _tileTypes;

	// The top-left hex, the hex one row down and the hex one column across; together they
	// give the row pitch and the staggered column pitch of the honeycomb.
	Common::Rect _originCell;
	Common::Rect _rowStepCell;
	Common::Rect _colStepCell;

	byte _unknownGridFlag = 0;
	int32 _numCols = 0;
	int32 _numRows = 0;
	int32 _hexDelay = 0;			// between one hex of a sliding line and the next
	Common::Array<int32> _grid;		// row-major tile type ids; 0 means no tile

	Common::Array<Character> _characters;
	Common::Array<Actor> _actors;

	RandomSoundBlock _solveSoundBlock;
	SceneChangeDescription _failScene;	// an opponent got there first
	FlagDescription _failFlag;
	RandomSoundBlock _failSound;

	// -- Runtime state --
	Common::Array<Common::Point> _actorCells;
	// An opponent does not head back to the hex it jumped to the turn before last.
	Common::Array<Common::Point> _lastTargets;
	Common::Array<Common::Point> _previousTargets;
	uint _turn = 0;
	bool _moveIssued = false;		// the actor whose turn it is has jumped
	bool _slidingPhase = false;		// the jump has landed and the hexes are moving
	Common::Point _clickedCell;
	Common::Array<ActorAnim> _landedJumps;
	Common::Array<Common::Array<TileAnim>> _cellAnims;
	Common::Array<Common::Array<ActorAnim>> _actorAnims;
	uint32 _lastTickTime = 0;
	int _outcome = -1;		// 0 solved, 1 failed
	bool _exitRequested = false;
	SoundDescription _endSound;

	Graphics::ManagedSurface _tileRef;
	Common::Array<TileImages> _tileImages;
	Common::Array<CharacterImages> _characterImages;
	Common::Array<MoviePlayer *> _jumpMovies;	// kNumCompassDirections per character
	Graphics::ManagedSurface _jumpFrame;
	const MoviePlayer *_jumpFrameMovie = nullptr;
	int _jumpFrameIndex = -1;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_ESCAPEGRIDPUZZLE_H
