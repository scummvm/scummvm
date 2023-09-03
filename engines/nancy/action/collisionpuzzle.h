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

class CollisionPuzzle : public RenderActionRecord {
public:
	CollisionPuzzle() : RenderActionRecord(7) {}
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
	
	protected:
		bool isViewportRelative() const override { return true; }
	};

	Common::String getRecordTypeName() const override { return "CollisionPuzzle"; };
	bool isViewportRelative() const override { return true; }

	Common::Point movePiece(uint pieceID, WallType direction);
	Common::Rect getScreenPosition(Common::Point gridPos);
	void drawGrid();

	Common::String _imageName;

	Common::Array<Common::Array<uint16>> _grid;
	Common::Array<Common::Point> _startLocations;

	Common::Array<Common::Rect> _pieceSrcs;
	Common::Array<Common::Rect> _homeSrcs;

	Common::Rect _verticalWallSrc;
	Common::Rect _horizontalWallSrc;
	Common::Rect _blockSrc;

	Common::Point _gridPos;

	uint16 _lineWidth;
	uint16 _framesPerMove;

	SoundDescription _moveSound;
	SoundDescription _homeSound;
	SoundDescription _wallHitSound;

	SceneChangeWithFlag _solveScene;
	uint16 _solveSoundDelay;
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
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_COLLISIONPUZZLE_H
