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

#ifndef MEDIASTATION_MINIGAMES_CHECKERS_H
#define MEDIASTATION_MINIGAMES_CHECKERS_H

#include "common/array.h"

#include "mediastation/actor.h"
#include "mediastation/mediascript/collection.h"

namespace MediaStation {

namespace CheckersMinigame {

const int COLUMN_COUNT = 8;
const int ROW_COUNT = 8;
const int PLAYABLE_CELLS_PER_ROW = 4;
const int TOTAL_CELLS = COLUMN_COUNT * ROW_COUNT;
const int TOTAL_PLAYABLE_CELLS = ROW_COUNT * PLAYABLE_CELLS_PER_ROW;

enum Side {
	kUnoccupiedBySide = 0,
	// "Black" is actually red in the game.
	// Hades always plays black.
	kBlackSide = 1,
	kWhiteSide = 2
};

enum CellClass {
	kCellNormal,
	kCellCorner,
	kCellNearCorner,
	kCellOuterCenter,
	kCellCenter
};

// Classification used by the positional evaluation score heuristic.
// Indexed by the standard linear playable-cell numbering.
static constexpr CellClass CELL_CLASSIFICATION[TOTAL_PLAYABLE_CELLS] = {
	kCellCorner,        kCellNearCorner,   kCellNearCorner,   kCellNormal,
	kCellCorner,        kCellNormal,       kCellNormal,       kCellNormal,
	kCellNormal,        kCellNormal,       kCellNormal,       kCellOuterCenter,
	kCellOuterCenter,   kCellCenter,       kCellCenter,       kCellNormal,
	kCellNormal,        kCellCenter,       kCellCenter,       kCellOuterCenter,
	kCellOuterCenter,   kCellNormal,       kCellNormal,       kCellNormal,
	kCellNormal,        kCellNormal,       kCellNormal,       kCellCorner,
	kCellNormal,        kCellNearCorner,   kCellNearCorner,   kCellCorner
};

struct Cell {
	// Scripts use pieceId to track which piece image to show where.
	int pieceId = 0;
	Side side = kUnoccupiedBySide;
	bool isKing = false;

	bool isOccupied() const { return side != kUnoccupiedBySide; }
	bool isMan() const { return isOccupied() && !isKing; }
};

// The coordinates of a particular cell on the checkers board.
struct Pair {
	int x = 0;
	int y = 0;

	constexpr Pair() = default;
	constexpr Pair(int x_, int y_) : x(x_), y(y_) {}

	Pair(int linearCellIndex); // notationToPair
	int toLinearCellIndex() const; // pairToNotation

	bool isWithinBoard() const;
	Pair operator+(const Pair &other) const;
};

struct SideConstants {
	Side opponent;
	int homeRow;
	CheckersMinigame::Pair validDirections[4];
};

struct PieceCounts {
	int totalBlackMen = 0;
	int totalBlackKings = 0;
	int totalWhiteMen = 0;
	int totalWhiteKings = 0;
};

struct Move {
	Pair from;
	Pair to;
	Pair capture;
	bool willCapture = false;
	bool willBecomeKing = false;
	int score = 0;
	Common::Array<Move> nextJumps;

	Collection *asCollection() const; // moveToCollection
};

enum CheckersCommand {
	kCheckersCommandPiecesThatCanMove = 1,
	kCheckersGetValidMoves = 2,
	kCheckersNewGame = 3,
	kCheckersFindBestMove = 4,
	kCheckersTakeMove = 5,
	kCheckersCommandDebugPrint = -2,
	kCheckersCommandValidateBoard = -1
};

// The full 8x8 checkers grid. Although only the dark squares are ever playable,
// every square is stored so that a Pair's (x, y) coordinates map directly onto
// the backing storage.
class Board : public Common::Array<Cell> {
public:
	Cell &cellAt(const Pair &position);
	const Cell &cellAt(const Pair &position) const;
	Cell &cellAt(uint linearIndex);
};

// A simple checkers engine that uses minimax with alpha/beta pruning.
class Checkers {
public:
	Checkers(Collection *collection); // checkers_initBoard

	void setBoard(const Common::Array<int> &squares);
	Common::Array<CheckersMinigame::Pair> piecesThatCanMove(Side side);
	void getValidMovesForPiece(
		const Pair &position, Common::Array<Move> &moves, bool capturesOnly); // movesOfPiece
	bool calculateAndSortAllMovesForSide(Side side);
	void prepareForBestMoveSearch(int depth);
	bool searchForBestMove(int maxSearchTimeInMs);
	Move &getCalculatedBestMove(); // bestMoveCalculated
	bool validateBoard() const;
	void printToDebug() const;

private:
	Board _board;
	Common::Array<Move> _moves;
	int _maxSearchDepth = 1;
	Side _sideToMove = kUnoccupiedBySide;
	Move _bestMove;

	const SideConstants &constantsForSide(Side side) const;
	bool checkForJumps(const Board &board, Side side, const Pair *checkPosition, Common::Array<Move> &moves); // checkForJumps
	bool checkForSlides(const Board &board, Side side, const Pair *checkPosition, Common::Array<Move> &moves);
	void updateMovesThatKing(const Board &board, Common::Array<Move> &moves);
	bool canMove(const Board &board, Side side);
	void addMove(Common::Array<Move> &moves, const Pair &start, const Pair &end, const Pair *captured, bool becameKing);
	void makeMove(Board &board, const Move &move);

	bool calculateAndSortMovesForSide(const Board &board, Side side, Common::Array<Move> &moves);
	void calculatePointsForMoves(const Board &board, Common::Array<Move> &moves); // calculateMovePoints
	int alphaBetaEvaluate(const Board &board, Move &parentMove, Side maximizingSide, Side sideToMove, int alpha, int beta, int ply, uint32 deadline);

	// This is the score that the minimax is optimizing.
	int getTotalBoardScoreForSide(const Board &board, Side side);
	int getWeightedSquareScoreForSide(const Board &board, Side side);
	PieceCounts getPieceCounts(const Board &board);
	void sortMovesByScore(Common::Array<Move> &moves);
};

// Functor to allow sorting moves by their score.
struct MoveScoreGreater {
	bool operator()(const CheckersMinigame::Move &left, const CheckersMinigame::Move &right) const {
		return left.score > right.score;
	}
};

} // End of namespace CheckersMinigame

} // End of namespace MediaStation

#endif
