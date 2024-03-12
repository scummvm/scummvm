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

#ifndef NANCY_ACTION_MAZECHASEPUZZLE_H
#define NANCY_ACTION_MAZECHASEPUZZLE_H

#include "engines/nancy/action/actionrecord.h"

namespace Nancy {
namespace Action {

// Implements a puzzle introduced in nancy5 where the player controls
// one piece being chased by several other pieces on a grid. Movement
// is performed via buttons, and both player and enemy navigate one
// tile at a time. Has some similarities to CollisionPuzzle, but was
// different enough to warrant its own class.
class MazeChasePuzzle : public RenderActionRecord {
public:
	MazeChasePuzzle() : RenderActionRecord(7) {}
	virtual ~MazeChasePuzzle() {}

	void init() override;
	void registerGraphics() override;
	void updateGraphics() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

protected:
	enum WallType { kWallLeft = 1, kWallUp = 2, kWallRight = 3, kWallDown = 4, kWallLeftRight = 6, kWallUpDown = 6 };

	class Piece : public RenderObject {
	public:
		Piece(uint z) : RenderObject(z) {}
		virtual ~Piece() {}

		Common::Point _gridPos;
		Common::Point _lastPos;

	protected:
		bool isViewportRelative() const override { return true; }
	};

	Common::String getRecordTypeName() const override { return "MazeChasePuzzle"; };
	bool isViewportRelative() const override { return true; }

	Common::Rect getScreenPosition(Common::Point gridPos);
	void drawGrid();
	void enemyMovement(uint enemyID);
	bool canMove(uint pieceID, WallType direction);
	void reset();

	Common::Path _imageName;

	Common::Point _exitPos = Common::Point(-1, -1);

	Common::Array<Common::Array<uint16>> _grid;
	Common::Array<Common::Point> _startLocations;

	Common::Rect _playerSrc;
	Common::Rect _enemySrc;
	Common::Rect _verticalWallSrc;
	Common::Rect _horizontalWallSrc;
	Common::Rect _lightSrc;

	Common::Rect _upButtonSrc;
	Common::Rect _rightButtonSrc;
	Common::Rect _downButtonSrc;
	Common::Rect _leftButtonSrc;
	Common::Rect _resetButtonSrc;

	Common::Point _gridPos;

	Common::Rect _lightDest;

	Common::Rect _upButtonDest;
	Common::Rect _rightButtonDest;
	Common::Rect _downButtonDest;
	Common::Rect _leftButtonDest;
	Common::Rect _resetButtonDest;

	uint16 _lineWidth = 0;
	uint16 _framesPerMove = 0;

	SoundDescription _failSound;
	SoundDescription _moveSound;

	SceneChangeWithFlag _solveScene;
	uint16 _solveSoundDelay = 0;
	SoundDescription _solveSound;

	SceneChangeWithFlag _exitScene;
	Common::Rect _exitHotspot;

	Graphics::ManagedSurface _image;
	Common::Array<Piece> _pieces;

	int _currentAnimFrame = -1;

	uint32 _solveSoundPlayTime = 0;
	bool _solved = false;
	bool _reset = false;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_MAZECHASEPUZZLE_H
