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

#ifndef NANCY_ACTION_COLLISIONPUZZLE_H
#define NANCY_ACTION_COLLISIONPUZZLE_H

#include "engines/nancy/action/actionrecord.h"

namespace Nancy {
namespace Action {

// Class responsible for two similar puzzle types, both of which have
// rectangular tiles on a grid, which can move up/down/left/right until they
// hit a wall or another tile
// - CollisionPuzzle: Several 1x1 tiles, each of which has a "home" it needs to reach.
//		The grid contains walls. Tiles move in all directions.
// - TileMovePuzzle: Many differently-sized tiles, one of which must reach the exit.
//		Rectangular tiles can only move in the directions parallel to their longer sides.
//		Exit is outside of the tile grid.
class CollisionPuzzle : public RenderActionRecord {
public:
	enum PuzzleType { kCollision, kTileMove };
	CollisionPuzzle(PuzzleType type) : RenderActionRecord(7), _puzzleType(type) {}
	virtual ~CollisionPuzzle() {}

	void init() override;
	void registerGraphics() override;
	void updateGraphics() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

protected:
	// numbers 1-5 are home IDs, 0 is empty cell
	enum WallType { kWallLeft = 6, kWallUp = 7, kWallDown = 8, kWallRight = 9, kBlock = 10 };

	class Piece : public RenderObject {
	public:
		Piece() : RenderObject(9) {}
		virtual ~Piece() {}

		Common::Point _gridPos;
		uint _w = 1;
		uint _h = 1;

	protected:
		bool isViewportRelative() const override { return true; }
	};

	Common::String getRecordTypeName() const override { return _puzzleType == kCollision ? "CollisionPuzzle" : "TileMovePuzzle"; };
	bool isViewportRelative() const override { return true; }

	Common::Point movePiece(uint pieceID, WallType direction);
	Common::Rect getScreenPosition(Common::Point gridPos);
	void drawGrid();

	Common::Path _imageName;

	Common::Array<Common::Array<uint16>> _grid;
	Common::Array<Common::Point> _startLocations;

	Common::Array<Common::Rect> _pieceSrcs;
	Common::Array<Common::Rect> _homeSrcs;

	Common::Rect _verticalWallSrc;
	Common::Rect _horizontalWallSrc;
	Common::Rect _blockSrc;

	Common::Point _tileMoveExitPos = Common::Point(-1, -1);
	uint _tileMoveExitSize = 0;

	bool _usesExitButton = false;
	Common::Rect _exitButtonSrc;
	Common::Rect _exitButtonDest;

	Common::Point _gridPos;

	uint16 _lineWidth = 0;
	uint16 _framesPerMove = 0;

	uint32 _timerTime = 0; // in seconds
	Common::Array<Common::Rect> _timerSrcs;
	Common::Array<int16> _timerFlagIds;
	Common::Rect _timerDest;

	SoundDescription _moveSound;
	SoundDescription _homeSound;
	SoundDescription _wallHitSound;
	SoundDescription _exitButtonSound;

	SceneChangeWithFlag _solveScene;
	uint16 _solveSoundDelay = 0;
	SoundDescription _solveSound;

	SceneChangeWithFlag _exitScene;
	Common::Rect _exitHotspot;

	Graphics::ManagedSurface _image;
	Common::Array<Piece> _pieces;

	int _currentlyAnimating = -1;
	int _currentAnimFrame = -1;
	Common::Point _lastPosition = { -1, -1 };

	uint32 _solveSoundPlayTime = 0;
	bool _solved = false;

	uint32 _puzzleStartTime = 0;
	int _currentTimerGraphic = -1;

	PuzzleType _puzzleType;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_COLLISIONPUZZLE_H
