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

#ifndef NANCY_ACTION_FOXGEESEPUZZLE_H
#define NANCY_ACTION_FOXGEESEPUZZLE_H

#include "engines/nancy/commontypes.h"
#include "engines/nancy/action/puzzlerecord.h"

namespace Nancy {
namespace Action {

// Nancy16 Fox and Geese board game (AR 186), played on the 33-hole cross-shaped board.
// The player moves the geese, and wins by trapping the computer-controlled fox, which
// moves along the board lines and captures geese by jumping over them.
class FoxGeesePuzzle : public PuzzleRecord {
public:
	FoxGeesePuzzle() : PuzzleRecord(7) {}
	virtual ~FoxGeesePuzzle() {}

	void init() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

	bool isViewportRelative() const override { return true; }

protected:
	Common::String getRecordTypeName() const override { return "FoxGeesePuzzle"; }

	static const int kBoardSize = 7;
	static const int kNumCells = 33;
	static const int kNumDirections = 8;

	enum Piece : byte {
		kEmpty,
		kGoose,
		kFox
	};

	struct Jump {
		Common::Point over;
		Common::Point to;
	};

	struct Ending {
		SceneChangeWithFlag scene;
		RandomSoundBlock sound;
	};

	struct WinEnding {
		Common::Array<Common::Point> foxPositions;
		Ending ending;
	};

	static bool isOnBoard(const Common::Point &pos);
	static void readGrid(Common::SeekableReadStream &stream, char grid[kBoardSize][kBoardSize]);
	static void readEnding(Common::SeekableReadStream &stream, Ending &ending);

	void addLine(const Common::Point &from, const Common::Point &to);
	Piece &pieceAt(const Common::Point &pos) { return _pieces[pos.y][pos.x]; }
	Piece pieceAt(const Common::Point &pos) const { return _pieces[pos.y][pos.x]; }
	bool cellAt(const Common::Point &mousePos, Common::Point &pos) const;
	const Common::Rect &cellDest(const Common::Point &pos) const { return _cellDests[pos.y][pos.x]; }

	void getMoves(const Common::Point &pos, Common::Array<Common::Point> &moves,
		Common::Array<Jump> *jumps = nullptr, Common::Array<Common::Point> *neighbors = nullptr) const;
	uint countGeese() const;
	bool findFoxMove(Common::Point &to, Common::Point &over);
	bool findMostMobileMove(const Common::Array<Common::Point> &moves, Common::Point &to) const;
	bool findMoveNearCenter(const Common::Array<Common::Point> &moves, Common::Point &to) const;

	void startFoxTurn();
	void findFox();
	void startWinEnding();
	void finishFoxMove();
	void placeGoose(const Common::Point &mousePos);
	void startEnding(const Ending &ending);
	void setCursor(const Common::Point &mousePos);

	void redraw();

	// -- File data --
	Common::Rect _cellDests[kBoardSize][kBoardSize];
	Piece _startPieces[kBoardSize][kBoardSize] = {};
	Common::Rect _foxSrc;
	Common::Rect _gooseSrc;
	Common::Point _heldGooseHotspot;		// moves the held goose away from the cursor's center

	uint16 _foxTurnCursor = 0;
	uint16 _heldGooseCursor = 0;
	uint16 _gooseCursor = 0;
	uint32 _foxMoveDuration = 0;		// ms
	bool _foxMovesFirst = false;
	bool _isSimpleFox = false;			// only takes jumps and moves at random

	RandomSoundBlock _gooseMoveSound;
	RandomSoundBlock _foxMoveSound;
	RandomSoundBlock _foxJumpSound;

	Common::Array<WinEnding> _winEndings;
	Ending _defaultWinEnding;
	Ending _loseEnding;

	// -- Runtime state --
	byte _links[kBoardSize][kBoardSize] = {};	// bitmask of the directions connected by a board line
	Piece _pieces[kBoardSize][kBoardSize] = {};

	bool _isFoxTurn = false;
	bool _isFoxMoving = false;
	bool _isFoxChainingJumps = false;
	Common::Point _foxFrom;
	Common::Point _foxTo;
	Common::Point _captured;
	uint32 _foxMoveStart = 0;

	bool _isGooseHeld = false;
	Common::Point _heldGooseFrom;
	Common::Point _mousePos;

	bool _isEnding = false;
	bool _exitRequested = false;
	SceneChangeWithFlag _endScene;
	SoundDescription _endSound;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_FOXGEESEPUZZLE_H
