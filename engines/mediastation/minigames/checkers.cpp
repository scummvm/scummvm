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

#include "common/algorithm.h"
#include "common/system.h"

#include "mediastation/debugchannels.h"
#include "mediastation/mediascript/function.h"
#include "mediastation/mediastation.h"
#include "mediastation/minigames/checkers.h"

namespace MediaStation {

// For exact argument count.
#define SUBCOMMANDARGCHECK(n) \
	if (args.size() != (n)) { \
		warning("%s: subcommand %d: expected %d argument%s, got %d", __func__, subcommand, (n), ((n) == 1 ? "" : "s"), args.size()); \
	}

void FunctionManager::script_Checkers(Common::Array<ScriptValue> &args, ScriptValue &returnValue) {
	const int subcommand = static_cast<int>(args[0].asFloat());
	switch (subcommand) {
	case CheckersMinigame::kCheckersCommandPiecesThatCanMove: {
		// This indeed create a temporary board and then throws it away.
		SUBCOMMANDARGCHECK(3);
		Collection *collection = args[1].asCollection();
		CheckersMinigame::Checkers checkers(collection);
		if (!checkers.validateBoard()) {
			warning("%s: Board failed validation, trying to continue", __func__);
		}

		// Not sure why the script couldn't have just provided the right side value
		// to begin with, but we have to massage it here.
		CheckersMinigame::Side side = (args[2].asFloat() == 0.0) ? CheckersMinigame::kBlackSide : CheckersMinigame::kWhiteSide;

		Collection *result = new Collection();
		Common::Array<CheckersMinigame::Pair> piecesThatCanMove = checkers.piecesThatCanMove(side);
		for (const CheckersMinigame::Pair &pair : piecesThatCanMove) {
			int linearCellIndex = pair.toLinearCellIndex();
			ScriptValue value;
			value.setToFloat(linearCellIndex);
			result->push_back(value);
		}

		// The return value takes ownership of the collection.
		returnValue.setToCollection(result);
		break;
	}

	case CheckersMinigame::kCheckersGetValidMoves: {
		// This indeed creates a temporary checkers board and then throws it away.
		SUBCOMMANDARGCHECK(4);
		Collection *collection = args[1].asCollection();
		CheckersMinigame::Checkers checkers(collection);
		if (!checkers.validateBoard()) {
			warning("%s: Board failed validation, trying to continue", __func__);
		}

		int linearCellIndex = static_cast<int>(args[2].asFloat());
		if (linearCellIndex < 0 || linearCellIndex > CheckersMinigame::TOTAL_PLAYABLE_CELLS - 1) {
			error("%s: Linear cell index %d out of bounds", __func__, linearCellIndex);
		}
		bool getJumpsOnly = args[3].asBool();

		Collection *result = new Collection();
		CheckersMinigame::Pair position(linearCellIndex);
		Common::Array<CheckersMinigame::Move> moves;
		checkers.getValidMovesForPiece(position, moves, getJumpsOnly);
		for (const CheckersMinigame::Move &move : moves) {
			ScriptValue value;
			Collection *moveCollection = move.asCollection();
			value.setToCollection(moveCollection);
			result->push_back(value);
		}

		// The return value takes ownership of the collection.
		returnValue.setToCollection(result);
		break;
	}

	case CheckersMinigame::kCheckersNewGame: {
		SUBCOMMANDARGCHECK(4);
		bool ok = false;
		Collection *collection = args[1].asCollection();
		delete g_engine->getFunctionManager()->_checkers;
		g_engine->getFunctionManager()->_checkers = new CheckersMinigame::Checkers(collection);
		CheckersMinigame::Checkers *checkers = g_engine->getFunctionManager()->_checkers;
		if (!checkers->validateBoard()) {
			warning("%s: Board failed validation, trying to continue", __func__);
		}

		// Not sure why the script couldn't have just provided the right side value
		// to begin with, but we have to massage it here.
		CheckersMinigame::Side side = (args[2].asFloat() == 0.0) ? CheckersMinigame::kBlackSide : CheckersMinigame::kWhiteSide;

		// When playing against Hades, the chosen level seems to dictate Hades' move
		// search depth. Known versions use the following values:
		//  - Level 1: 1 move ahead
		//  - Level 2: 2 moves ahead
		//  - Level 3: 4 moves ahead
		int moveSearchDepth = MAX(1, static_cast<int>(args[3].asFloat()));
		if (checkers->calculateAndSortAllMovesForSide(side)) {
			checkers->prepareForBestMoveSearch(moveSearchDepth);
			ok = true;
		}

		returnValue.setToBool(ok);
		break;
	}

	case CheckersMinigame::kCheckersFindBestMove: {
		SUBCOMMANDARGCHECK(2);
		CheckersMinigame::Checkers *checkers = g_engine->getFunctionManager()->_checkers;
		if (checkers == nullptr) {
			error("%s: Board is not initialized", __func__);
		}

		bool foundBestMove = false;
		double maxSearchTimeInSeconds = args[1].asTime();
		int maxSearchTimeInMs = MAX(100, static_cast<int>(maxSearchTimeInSeconds * 1000.0));
		foundBestMove = checkers->searchForBestMove(maxSearchTimeInMs);
		returnValue.setToBool(foundBestMove);
		break;
	}

	case CheckersMinigame::kCheckersTakeMove: {
		CheckersMinigame::Checkers *checkers = g_engine->getFunctionManager()->_checkers;
		if (checkers == nullptr) {
			error("%s: Board is not initialized", __func__);
		}

		CheckersMinigame::Move &bestMove = checkers->getCalculatedBestMove();
		Collection *result = bestMove.asCollection();

		// The return value takes ownership of the collection.
		returnValue.setToCollection(result);
		break;
	}

	case CheckersMinigame::kCheckersCommandDebugPrint: {
		// This indeed creates a temporary checkers board and then throws it away.
		SUBCOMMANDARGCHECK(2);
		CheckersMinigame::Checkers checkers(args[1].asCollection());
		checkers.printToDebug();
		break;
	}

	case CheckersMinigame::kCheckersCommandValidateBoard: {
		// This looks like debug machinery left in the release version, but
		// scripts actually do call it, so it is reimplemented here.
		// This indeed creates a temporary checkers board and then throws it away.
		SUBCOMMANDARGCHECK(2);
		CheckersMinigame::Checkers checkers(args[1].asCollection());
		returnValue.setToBool(checkers.validateBoard());
		break;
	}

	default:
		warning("%s: Got unimplemented checkers subcommand %d", __func__, subcommand);
		break;
	}
}

namespace CheckersMinigame {

constexpr SideConstants BLACK_INFO = {
	Side::kWhiteSide,
	0,
	{
		{-1,  1}, { 1,  1},	// Forward (toward row 7).
		{-1, -1}, { 1, -1}	// Backward.
	}
};

constexpr SideConstants WHITE_INFO = {
	Side::kBlackSide,
	7,
	{
		{-1, -1}, { 1, -1},	// Forward (toward row 0).
		{-1,  1}, { 1,  1}	// Backward.
	}
};

// The two playable corner-adjacent squares on each side's home row. Keeping an
// uncrowned man on one of these denies the opponent an easy promotion.
static constexpr Pair BACK_RANK_DEFENDER_CELLS[] = {
	{1, 0}, {5, 0},  // Black's home row.
	{2, 7}, {6, 7}   // White's home row.
};

const int kLossScore = -29999;
const int kWinScore = 29999;

Checkers::Checkers(Collection *collection) {
	_board.resize(TOTAL_CELLS);

	if (collection == nullptr) {
		error("%s: Collection is null", __func__);
	} else if (collection->size() < TOTAL_PLAYABLE_CELLS) {
		error("%s: Source collection doesn't cover all playable cells", __func__);
	}

	Common::Array<int> cellValues(TOTAL_PLAYABLE_CELLS);
	for (uint i = 0; i < TOTAL_PLAYABLE_CELLS; i++) {
		cellValues[i] = static_cast<int>(collection->operator[](i).asFloat());
	}
	setBoard(cellValues);
}

void Checkers::setBoard(const Common::Array<int> &cellValues) {
	// Parse the cell values from the script into an actual board.
	uint padding = 1;
	uint linearCellIndex = 0;
	for (uint row = 0; row < ROW_COUNT; row++) {
		for (uint col = 0; col < PLAYABLE_CELLS_PER_ROW; col++) {
			Cell &cell = _board.cellAt(Pair(padding + col * 2, row));

			int cellValueToParse = cellValues[linearCellIndex];
			if (cellValueToParse == 0) {
				cell.isKing = false;
				cell.side = kUnoccupiedBySide;
				cell.pieceId = 0;
			} else {
				if (cellValueToParse < 101) {
					cell.isKing = false;
				} else {
					cell.isKing = true;
					cellValueToParse -= 100;
				}

				cell.side = cellValueToParse < 13 ? kBlackSide : kWhiteSide;
				cell.pieceId = cellValueToParse;
			}

			linearCellIndex++;
		}

		padding = (padding == 0);
	}
}

Cell &Board::cellAt(const Pair &position) {
	assert(position.isWithinBoard());
	return operator[](position.x * COLUMN_COUNT + position.y);
}

const Cell &Board::cellAt(const Pair &position) const {
	assert(position.isWithinBoard());
	return operator[](position.x * COLUMN_COUNT + position.y);
}

Cell &Board::cellAt(uint linearIndex) {
	return operator[](linearIndex);
}

void Checkers::updateMovesThatKing(const Board &board, Common::Array<Move> &moves) {
	if (moves.empty()) {
		return;
	}

	const Cell &firstMovePiece = board.cellAt(moves[0].from);
	const int promotionRow = (firstMovePiece.side == kBlackSide) ? (ROW_COUNT - 1) : 0;

	for (Move &move : moves) {
		const Cell &movingPiece = board.cellAt(move.from);
		if (!movingPiece.isKing && move.to.y == promotionRow) {
			move.willBecomeKing = true;
		}
	}
}

bool Checkers::validateBoard() const {
	bool valid = true;

	int seen[25] = {};
	seen[0] = -1;

	Pair pos;
	for (pos.y = 0; pos.y < ROW_COUNT; pos.y++) {
		const int startX = (pos.y & 1) ? 0 : 1;
		for (pos.x = startX; pos.x < COLUMN_COUNT; pos.x += 2) {
			const Cell &cell = _board.cellAt(pos);
			if (cell.pieceId == 0) {
				continue;
			}

			if (cell.pieceId < 1 || cell.pieceId > 24) {
				valid = false;
				continue;
			}

			if (seen[cell.pieceId] != 0) {
				valid = false;
			} else {
				seen[cell.pieceId] = pos.y * 8 + pos.x + 1;
			}

			if (cell.side == kBlackSide && pos.y == 7 && !cell.isKing) {
				warning("%s: Piece %d should be kinged", __func__, cell.pieceId);
				valid = false;
			}

			if (cell.side == kWhiteSide && pos.y == 0 && !cell.isKing) {
				warning("%s: Piece %d should be kinged", __func__, cell.pieceId);
				valid = false;
			}
		}
	}

	return valid;
}

void Checkers::printToDebug() const {
	bool filledCellStartsAtEvenX = false;

	for (int y = 0; y < ROW_COUNT; y++) {
		Common::String border;
		for (int x = 0; x < COLUMN_COUNT; x++) {
			border += "+------";
		}
		border += "+";
		debugC(5, kDebugMinigame, "%s", border.c_str());

		Common::String row;
		for (int x = 0; x < COLUMN_COUNT; x++) {
			const Cell &cell = _board.cellAt(Pair(x, y));
			Common::String cellText;

			if (cell.side == kUnoccupiedBySide) {
				cellText = (filledCellStartsAtEvenX == ((x % 2) == 0)) ? "####" : "    ";
			} else {
				const char kingChar = cell.isKing ? 'k' : ' ';
				const char sideChar = (cell.side == kBlackSide) ? 'B' : 'W';
				cellText = Common::String::format("%c%2d%c", sideChar, ((cell.pieceId - 1) % 12) + 1, kingChar);
			}

			row += Common::String::format("|  %s ", cellText.c_str());
		}
		row += "|";
		debugC(5, kDebugMinigame, "%s", row.c_str());

		filledCellStartsAtEvenX = !filledCellStartsAtEvenX;
	}

	Common::String border;
	for (int x = 0; x < COLUMN_COUNT; x++) {
		border += "+------";
	}
	border += "+\n";
	debugC(5, kDebugMinigame, "%s", border.c_str());
}

Common::Array<Pair> Checkers::piecesThatCanMove(Side side) {
	// Regenerate the legal moves list for this side.
	Common::Array<Pair> moveablePieces;
	Common::Array<Move> moves;
	bool canDoAtLeastOneJump = checkForJumps(_board, side, nullptr, moves);
	if (!canDoAtLeastOneJump) {
		checkForSlides(_board, side, nullptr, moves);
	}

	bool movable[COLUMN_COUNT][ROW_COUNT] = {};
	for (const Move &move : moves) {
		movable[move.from.x][move.from.y] = true;
	}

	for (int y = 0; y < ROW_COUNT; y++) {
		for (int x = 0; x < COLUMN_COUNT; x++) {
			if (movable[x][y]) {
				moveablePieces.push_back(Pair(x, y));
			}
		}
	}

	return moveablePieces;
}

void Checkers::getValidMovesForPiece(
	const Pair &position, Common::Array<Move> &moves, bool getJumpsOnly) {
	moves.clear();

	const Cell &cell = _board.cellAt(position);
	if (cell.isOccupied()) {
		const bool canDoAtLeastOneJump = checkForJumps(_board, cell.side, &position, moves);
		if (!getJumpsOnly) {
			if (!canDoAtLeastOneJump) {
				checkForSlides(_board, cell.side, &position, moves);
			}
		}

		updateMovesThatKing(_board, moves);
	}
}

bool Checkers::checkForJumps(const Board &board, Side mySide, const Pair *checkPosition, Common::Array<Move> &moves) {
	const SideConstants &constants = constantsForSide(mySide);

	// See if the current side can do any jumps (captures).
	bool anyValidJumps = false;
	Pair min(0, 0);
	Pair max(COLUMN_COUNT - 1, ROW_COUNT - 1);
	if (checkPosition != nullptr) {
		min.x = max.x = checkPosition->x;
		min.y = max.y = checkPosition->y;
	}

	for (int y = min.y; y <= max.y; y++) {
		for (int x = min.x; x <= max.x; x++) {
			// Only look at our pieces.
			Pair from(x, y);
			const Cell &piece = board.cellAt(from);
			if (piece.side != mySide) {
				continue;
			}

			// Kings can move diagonally backward and forward (4 total directions),
			// but men can only move diagonally forward (2 total directions).
			int totalValidDirections = piece.isKing ? 4 : 2;
			for (int i = 0; i < totalValidDirections; i++) {
				Pair capture = from + constants.validDirections[i];
				Pair landing = capture + constants.validDirections[i];

				// Make sure we will still land inside the board.
				if (landing.isWithinBoard()) {
					// We can jump iff the adjacent cell is occupied by an opponent...
					if (board.cellAt(capture).side != constants.opponent) {
						continue;
					}

					// ...and the cell that we'll land on is empty.
					if (board.cellAt(landing).isOccupied()) {
						continue;
					}

					addMove(moves, from, landing, &capture, false);
					anyValidJumps = true;
				}
			}
		}
	}

	return anyValidJumps;
}

bool Checkers::checkForSlides(const Board &board, Side side, const Pair *checkPosition, Common::Array<Move> &moves) {
	const SideConstants &constants = constantsForSide(side);

	// See if the current side can do any slides (moves without a capture).
	bool anyValidSlides = false;
	Pair min(0, 0);
	Pair max(COLUMN_COUNT - 1, ROW_COUNT - 1);
	if (checkPosition != nullptr) {
		min.x = max.x = checkPosition->x;
		min.y = max.y = checkPosition->y;
	}

	for (int y = min.y; y <= max.y; y++) {
		for (int x = min.x; x <= max.x; x++) {
			// Only look at our pieces.
			Pair from(x, y);
			const Cell &piece = board.cellAt(from);
			if (piece.side != side) {
				continue;
			}

			// Kings can move diagonally backward and forward (4 total directions),
			// but men can only move diagonally forward (2 total directions).
			int dirCount = piece.isKing ? 4 : 2;
			for (int i = 0; i < dirCount; i++) {
				Pair to = from + constants.validDirections[i];

				if (to.isWithinBoard()) {
					if (!board.cellAt(to).isOccupied()) {
						addMove(moves, from, to, nullptr, false);
						anyValidSlides = true;
					}
				}
			}
		}
	}

	return anyValidSlides;
}

void Checkers::addMove(Common::Array<Move> &moves, const Pair &from, const Pair &to, const Pair *captured, bool becameKing) {
	Move move;
	move.from = from;
	move.to = to;

	if (captured != nullptr) {
		move.willCapture = true;
		move.capture = *captured;
	} else {
		move.capture = {-1, -1};
	}
	moves.push_back(move);
}

void Checkers::makeMove(Board &board, const Move &move) {
	Cell &fromCell = board.cellAt(move.from);
	Cell &toCell = board.cellAt(move.to);
	// The piece moves, and its original position is now unoccupied.
	toCell = fromCell;
	fromCell = Cell();

	if (move.willBecomeKing) {
		toCell.isKing = true;
	}

	if (move.willCapture) {
		// The cell that we jumped over is now unoccupied,
		// as the piece inside it was captured.
		board.cellAt(move.capture) = Cell();
	}

	for (const Move &nextJump : move.nextJumps) {
		makeMove(board, nextJump);
	}
}

PieceCounts Checkers::getPieceCounts(const Board &board) {
	PieceCounts counts;
	Pair pos;
	for (pos.y = 0; pos.y < ROW_COUNT; pos.y++) {
		for (pos.x = 0; pos.x < COLUMN_COUNT; pos.x++) {
			const Cell &cell = board.cellAt(pos);
			if (cell.side == kBlackSide) {
				cell.isKing ? counts.totalBlackKings++ : counts.totalBlackMen++;

			} else if (cell.side == kWhiteSide) {
				cell.isKing ? counts.totalWhiteKings++ : counts.totalWhiteMen++;
			}
		}
	}
	return counts;
}

int Checkers::getTotalBoardScoreForSide(const Board &board, Side ourSide) {
	PieceCounts counts = getPieceCounts(board);
	uint blackMaterialScore = (counts.totalBlackKings * 200) + (counts.totalBlackMen * 100);
	uint whiteMaterialScore = (counts.totalWhiteKings * 200) + (counts.totalWhiteMen * 100);

	Side opposingSide = kUnoccupiedBySide;
	int homeRow = 0;
	int score = 0;
	switch (ourSide) {
	case kBlackSide:
		opposingSide = kWhiteSide;
		score = blackMaterialScore - whiteMaterialScore;
		break;

	case kWhiteSide:
		opposingSide = kBlackSide;
		homeRow = 7;
		score = whiteMaterialScore - blackMaterialScore;
		break;

	default:
		error("%s: Got bad side %d", __func__, static_cast<uint>(ourSide));
	}

	if (!canMove(board, ourSide)) {
		// We can't move, so we lost.
		return kLossScore;

	} else if (!canMove(board, opposingSide)) {
		// Our opponent can't move, so we won.
		return kWinScore;
	}

	// Award a small bonus for occupying one of the back-rank defender squares.
	// Keeping an uncrowned piece here discourages the opponent from obtaining
	// an easy promotion, so only men (not kings) get the bonus.
	for (const Pair &defenderCell : BACK_RANK_DEFENDER_CELLS) {
		const Cell &cell = board.cellAt(defenderCell);
		if (cell.isMan()) {
			score += (cell.side == ourSide) ? 7 : -7;
		}
	}

	// Reward uncrowned pieces for advancing from their home row.
	// This encourages men to make progress toward promotion.
	Pair pos;
	for (pos.y = 0; pos.y < ROW_COUNT; pos.y++) {
		for (pos.x = 0; pos.x < COLUMN_COUNT; pos.x++) {
			const Cell &cell = board.cellAt(pos);
			if (cell.side == ourSide && !cell.isKing) {
				score += ABS(homeRow - pos.y);
			}
		}
	}

	return score + getWeightedSquareScoreForSide(board, ourSide) - getWeightedSquareScoreForSide(board, opposingSide);
}

void Checkers::calculatePointsForMoves(const Board &board, Common::Array<Move> &moves) {
	if (moves.empty()) {
		return;
	}

	// The list of current moves should only contain moves from a single
	// side at one time. Thus, we can determine this side by looking at the
	// side of the first move.
	const Side side = board.cellAt(moves[0].from).side;

	for (Move &move : moves) {
		Board updatedBoard = board;
		makeMove(updatedBoard, move);
		move.score = getTotalBoardScoreForSide(updatedBoard, side);
	}
}

int Checkers::getWeightedSquareScoreForSide(const Board &board, Side side) {
	PieceCounts counts = getPieceCounts(board);
	const int ourKings = (side == kBlackSide) ? counts.totalBlackKings : counts.totalWhiteKings;
	const int opponentKings = (side == kBlackSide) ? counts.totalWhiteKings : counts.totalBlackKings;
	const bool behindInKings = ourKings < opponentKings;

	int score = 0;
	for (int linearCellIndex = 0; linearCellIndex < TOTAL_PLAYABLE_CELLS; linearCellIndex++) {
		// Only look at our own cells.
		const Cell &cell = board.cellAt(linearCellIndex);
		if (cell.side != side) {
			continue;
		}

		CellClass classification = CELL_CLASSIFICATION[linearCellIndex];
		switch (classification) {
		case kCellCorner:
			// Corners are relatively safe because they can only be
			// attacked from one direction.
			if (cell.isKing && behindInKings) {
				// If you’re behind in king count, keeping your kings
				// in corners makes them harder to trap.
				score += 10;
			} else {
				score += 2;
			}
			break;

		case kCellNearCorner:
			// These squares are adjacent to the corners and can become traps. A king
			// here has fewer escape routes than one actually in the corner, so this
			// is a much less ideal spot to be in than actually being in a corner.
			if (cell.isKing && behindInKings) {
				score -= 8;
			}
			break;

		case kCellCenter:
			// The four central playable squares can provide the greatest mobility and control.
			score += 4;
			break;

		case kCellOuterCenter:
			// Slightly discourage occupying these in favor of the true center.
			score -= 1;
			break;

		case kCellNormal:
			break;
		}
	}

	return score;
}

bool Checkers::canMove(const Board &board, Side side) {
	// Regenerate the legal moves list for this side.
	Common::Array<Move> moves;
	bool canMove = checkForJumps(board, side, nullptr, moves);
	if (!canMove) {
		canMove = checkForSlides(board, side, nullptr, moves);
	}

	return canMove;
}

int Checkers::alphaBetaEvaluate(
	const Board &board, Move &parentMove, Side sideToSolve, Side sideToMove, int alpha, int beta, int currentSearchDepth, uint32 searchDeadlineInMs) {
	// The original used an iterative search with an explicit state machine (and manually managed stack),
	// but that was judged needlessly complex to reimplement here. For modern hardware, just do it recursively.

	Common::Array<Move> moves;
	bool searchTimeExpired = static_cast<int32>(g_system->getMillis() - searchDeadlineInMs) >= 0;
	bool forceEndSearch = searchTimeExpired || currentSearchDepth > _maxSearchDepth;
	if (forceEndSearch) {
		// Just recommend the best move we found thus far.
		return getTotalBoardScoreForSide(board, sideToSolve);
	}

	// If we just jumped but can jump again from our new position, we must do so.
	// If we both captured and became a king on the same turn, we are already
	// at the last row on the board, so it is not possible to jump again from this position.
	bool canMaybeJumpAgain = parentMove.willCapture && !parentMove.willBecomeKing;
	bool requireContinuationJump = false;
	if (canMaybeJumpAgain) {
		const Side continuingSide = board.cellAt(parentMove.to).side;
		requireContinuationJump = checkForJumps(board, continuingSide, &parentMove.to, moves);
		if (requireContinuationJump) {
			// We must jump again, so ensure the turn remains ours.
			sideToMove = continuingSide;
		}
	}

	Side nextSideToMove = kUnoccupiedBySide;
	int nextSearchDepth = currentSearchDepth;
	if (requireContinuationJump) {
		nextSideToMove = sideToMove;
		updateMovesThatKing(board, moves);
		calculatePointsForMoves(board, moves);
		sortMovesByScore(moves);
		// We do not treat continuation jumps as separate moves in terms of
		// the search depth limit.
	} else {
		nextSideToMove = constantsForSide(sideToMove).opponent;
		calculateAndSortMovesForSide(board, sideToMove, moves);
		nextSearchDepth++;
	}

	bool searchIsFinished = moves.empty();
	if (searchIsFinished) {
		return getTotalBoardScoreForSide(board, sideToSolve);
	}

	int bestScore = 0;
	if (sideToMove == sideToSolve) {
		// It's our turn (the side we're solving for/maximizing player).
		// Evaluate every legal move and keep the highest score found thus far.
		// Thus, we start with the lowest possible score (a loss).
		bestScore = kLossScore;
		for (Move &move : moves) {
			Board updatedBoard = board;
			makeMove(updatedBoard, move);

			const int score = alphaBetaEvaluate(updatedBoard, move, sideToSolve, nextSideToMove, alpha, beta, nextSearchDepth, searchDeadlineInMs);
			move.score = score;
			if (score > bestScore) {
				// This is the best move we've seen so far, so record it.
				bestScore = score;
				if (requireContinuationJump) {
					parentMove.nextJumps.clear();
					parentMove.nextJumps.push_back(move);
				}
			}

			// Raise the lower bound on the score this position can achieve.
			alpha = MAX(alpha, bestScore);
			if (alpha >= beta) {
				// No remaining move can affect the final result.
				// Stop searching this branch.
				break;
			}
		}

	} else {
		// It's our opponent's turn (minimizing player). Evaluate every legal move and
		// assume they choose the move that give us the lowest score.
		bestScore = kWinScore;
		for (Move &move : moves) {
			Board updatedBoard = board;
			makeMove(updatedBoard, move);
			const int score = alphaBetaEvaluate(updatedBoard, move, sideToSolve, nextSideToMove, alpha, beta, nextSearchDepth, searchDeadlineInMs);
			move.score = score;

			if (score < bestScore) {
				// Record the worst score (from our perspective) seen so far.
				bestScore = score;
				if (requireContinuationJump) {
					parentMove.nextJumps.clear();
					parentMove.nextJumps.push_back(move);
				}
			}

			// Lower the upper bound on the score this position can achieve.
			beta = MIN(beta, bestScore);
			if (alpha >= beta) {
				// No remaining move can affect the final result, so stop searching
				// this branch early.
				break;
			}
		}
	}

	sortMovesByScore(moves);
	return bestScore;
}

void Checkers::prepareForBestMoveSearch(int moveSearchDepth) {
	_maxSearchDepth = MAX(1, moveSearchDepth);

	if (_moves.empty()) {
		_sideToMove = kUnoccupiedBySide;
		_bestMove = Move();
	} else {
		// The list of valid moves only contains moves from one side or the other
		// at any given time. So we can determine the current side just by looking
		// at the first move.
		_sideToMove = _board.cellAt(_moves[0].from).side;
		updateMovesThatKing(_board, _moves);
		calculatePointsForMoves(_board, _moves);
		sortMovesByScore(_moves);
		_bestMove = _moves[0];
	}
}

bool Checkers::searchForBestMove(int maxSearchTimeInMs) {
	// The original had this timing logic to make sure finding the best move
	// didn't take too much time. This should not be a concern on modern hardware,
	// but we will include it for accuracy to the original.
	maxSearchTimeInMs = MAX(1, maxSearchTimeInMs);
	const uint32 searchDeadlineInMs = g_system->getMillis() + maxSearchTimeInMs;

	if (_sideToMove == kUnoccupiedBySide) {
		if (_moves.empty()) {
			// TODO: Not sure why white is chosen here as black usually goes first?
			_sideToMove = kWhiteSide;
		} else {
			_sideToMove = _board.cellAt(_moves[0].from).side;
		}
	}

	bool canMove = calculateAndSortAllMovesForSide(_sideToMove) && !_moves.empty();
	if (!canMove) {
		return false;
	}

	// Search each legal move from the current position. As better moves are
	// found, raise alpha so subsequent searches can prune more aggressively.
	int alpha = kLossScore;
	const int beta = kWinScore;
	for (Move &move : _moves) {
		Board speculativeBoard = _board;
		makeMove(speculativeBoard, move);
		move.score = alphaBetaEvaluate(
			speculativeBoard, move, _sideToMove,
			constantsForSide(_sideToMove).opponent,
			alpha, beta, 0, searchDeadlineInMs);

		// Track the best score from the current position so later searches can prune sooner.
		alpha = MAX(alpha, move.score);
	}

	// Determine if there's more than one best move.
	sortMovesByScore(_moves);
	const int bestScore = _moves[0].score;
	uint tiedMoves = 0;
	for (const Move &move : _moves) {
		if (move.score == bestScore) {
			tiedMoves++;
		} else {
			break;
		}
	}
	// If there is more than one best move, choose one randomly.
	uint bestMoveIndex = 0;
	if (tiedMoves > 1) {
		bestMoveIndex = g_engine->_randomSource.getRandomNumber(tiedMoves - 1);
	}
	_bestMove = _moves[bestMoveIndex];

	return true;
}

Move &Checkers::getCalculatedBestMove() {
	return _bestMove;
}

Collection *Move::asCollection() const {
	// Serialize the move back out to a collection so scripts can use it.
	Collection *collection = new Collection();
	ScriptValue value;
	value.setToFloat(from.toLinearCellIndex());
	collection->push_back(value);

	const Move *currentMove = this;
	while (currentMove != nullptr) {
		value.setToFloat(currentMove->to.toLinearCellIndex());
		collection->push_back(value);

		int captured = -1;
		if (currentMove->willCapture == true) {
			captured = currentMove->capture.toLinearCellIndex();
		}

		if (currentMove->willBecomeKing) {
			captured += 100;
		}

		value.setToFloat(captured);
		collection->push_back(value);

		currentMove = currentMove->nextJumps.empty() ? nullptr : &currentMove->nextJumps[0];
	}

	// The caller takes ownership of the collection.
	return collection;
}

bool Checkers::calculateAndSortMovesForSide(const Board &board, Side side, Common::Array<Move> &moves) {
	// Regenerate the legal moves list for this side.
	moves.clear();
	bool canMove = true;
	bool canDoAtLeastOneJump = checkForJumps(board, side, nullptr, moves);
	if (!canDoAtLeastOneJump) {
		canMove = checkForSlides(board, side, nullptr, moves);
	}

	if (canMove) {
		updateMovesThatKing(board, moves);
		calculatePointsForMoves(board, moves);
		sortMovesByScore(moves);
	}

	return canMove;
}

bool Checkers::calculateAndSortAllMovesForSide(Side side) {
	return calculateAndSortMovesForSide(_board, side, _moves);
}

void Checkers::sortMovesByScore(Common::Array<Move> &moves) {
	// We can't easily use a Common::SortedArray here because the sort key (score)
	// is mutated after the elements are in the array. So we will explicitly sort
	// at the right times to ensure this invariant is respected.
	Common::sort(moves.begin(), moves.end(), MoveScoreGreater());
}

const SideConstants &Checkers::constantsForSide(Side side) const {
	switch (side) {
	case kBlackSide:
		return BLACK_INFO;

	case kWhiteSide:
		return WHITE_INFO;

	default:
		error("%s: Got bad side %d", __func__, static_cast<uint>(side));
	}
}

Pair::Pair(int linearCellIndex) {
	y = linearCellIndex / 4;
	x = (linearCellIndex % 4) * 2 + ((y + 1) & 1);
}

int Pair::toLinearCellIndex() const {
	return y * 4 + (x >> 1);
}

bool Pair::isWithinBoard() const {
	return (x >= 0 && x < COLUMN_COUNT) && (y >= 0 && y < ROW_COUNT);
}

Pair Pair::operator+(const Pair &other) const {
	return Pair(x + other.x, y + other.y);
}

} // End of namespace CheckersMinigame

} // End of namespace MediaStation
