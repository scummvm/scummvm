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

#include "common/random.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/input.h"
#include "engines/nancy/util.h"

#include "engines/nancy/state/scene.h"
#include "engines/nancy/action/puzzle/foxgeesepuzzle.h"

namespace Nancy {
namespace Action {

static const Common::Point kDirections[] = {
	Common::Point(-1, -1), Common::Point(0, -1), Common::Point(1, -1), Common::Point(1, 0),
	Common::Point(1, 1), Common::Point(0, 1), Common::Point(-1, 1), Common::Point(-1, 0)
};

static const Common::Point kNoPosition(-1, -1);
static const Common::Point kBoardCenter(3, 3);

// The geese win once they're down to this many
static const uint kMinGeese = 4;

// The board is a cross made of three rows and three columns of seven holes each
bool FoxGeesePuzzle::isOnBoard(const Common::Point &pos) {
	if (pos.x < 0 || pos.y < 0 || pos.x >= kBoardSize || pos.y >= kBoardSize) {
		return false;
	}

	return (pos.x >= 2 && pos.x <= 4) || (pos.y >= 2 && pos.y <= 4);
}

void FoxGeesePuzzle::readGrid(Common::SeekableReadStream &stream, char grid[kBoardSize][kBoardSize]) {
	for (int y = 0; y < kBoardSize; ++y) {
		stream.read(grid[y], kBoardSize);
	}
}

void FoxGeesePuzzle::readEnding(Common::SeekableReadStream &stream, Ending &ending) {
	ending.scene._sceneChange.sceneID = stream.readUint16LE();
	ending.scene._sceneChange.frameID = stream.readUint16LE();
	ending.scene._sceneChange.continueSceneSound = kContinueSceneSound;
	ending.scene._flag.label = stream.readSint16LE();
	ending.scene._flag.flag = stream.readByte();
	ending.sound.readData(stream);
}

void FoxGeesePuzzle::readData(Common::SeekableReadStream &stream) {
	if (stream.readUint16LE() != kNumCells) {
		error("FoxGeesePuzzle: the board needs %d cells", kNumCells);
	}

	for (int y = 0; y < kBoardSize; ++y) {
		for (int x = 0; x < kBoardSize; ++x) {
			if (isOnBoard(Common::Point(x, y))) {
				readRect(stream, _cellDests[y][x]);
			}
		}
	}

	readFilename(stream, _imageName);
	readRect(stream, _foxSrc);
	readRect(stream, _gooseSrc);
	_heldGooseHotspot.x = stream.readSint32LE();
	_heldGooseHotspot.y = stream.readSint32LE();

	// The starting position, with an 'F' for the fox and a 'G' for each goose
	char grid[kBoardSize][kBoardSize];
	readGrid(stream, grid);
	uint numFoxes = 0;
	for (int y = 0; y < kBoardSize; ++y) {
		for (int x = 0; x < kBoardSize; ++x) {
			if (!isOnBoard(Common::Point(x, y))) {
				continue;
			}

			if (grid[y][x] == 'F') {
				_startPieces[y][x] = kFox;
				++numFoxes;
			} else if (grid[y][x] == 'G') {
				_startPieces[y][x] = kGoose;
			}
		}
	}

	if (numFoxes != 1) {
		error("FoxGeesePuzzle: the board needs exactly one fox");
	}

	_foxTurnCursor = stream.readUint16LE();
	_heldGooseCursor = stream.readUint16LE();
	_gooseCursor = stream.readUint16LE();
	_foxMoveDuration = (uint32)(stream.readFloatLE() * 1000);
	_foxMovesFirst = stream.readByte() == 1;
	_isSimpleFox = stream.readByte() == 0;

	_gooseMoveSound.readData(stream);
	_foxMoveSound.readData(stream);
	_foxJumpSound.readData(stream);

	// Each win ending marks the fox positions it applies to with a '*'
	_winEndings.resize(stream.readSint16LE());
	for (WinEnding &win : _winEndings) {
		readGrid(stream, grid);
		for (int y = 0; y < kBoardSize; ++y) {
			for (int x = 0; x < kBoardSize; ++x) {
				if (grid[y][x] == '*' && isOnBoard(Common::Point(x, y))) {
					win.foxPositions.push_back(Common::Point(x, y));
				}
			}
		}

		readEnding(stream, win.ending);
	}

	readEnding(stream, _defaultWinEnding);
	readEnding(stream, _loseEnding);

	readExitHotspot(stream);
}

void FoxGeesePuzzle::init() {
	initViewportSurface();
	loadImage();

	memcpy(_pieces, _startPieces, sizeof(_pieces));

	memset(_links, 0, sizeof(_links));
	for (int i = 0; i < kBoardSize; ++i) {
		// Columns and rows
		bool isArm = i < 2 || i > 4;
		addLine(Common::Point(i, isArm ? 2 : 0), Common::Point(i, isArm ? 4 : 6));
		addLine(Common::Point(isArm ? 2 : 0, i), Common::Point(isArm ? 4 : 6, i));
	}

	// Diagonals
	addLine(Common::Point(0, 4), Common::Point(4, 0));
	addLine(Common::Point(2, 4), Common::Point(4, 2));
	addLine(Common::Point(2, 6), Common::Point(6, 2));
	addLine(Common::Point(0, 2), Common::Point(4, 6));
	addLine(Common::Point(2, 2), Common::Point(4, 4));
	addLine(Common::Point(2, 0), Common::Point(6, 4));

	_isFoxTurn = _foxMovesFirst;

	NancySceneState.setNoHeldItem();
	redraw();
}

// Connects every pair of neighboring holes along a straight or diagonal line
void FoxGeesePuzzle::addLine(const Common::Point &from, const Common::Point &to) {
	Common::Point step(CLIP<int16>(to.x - from.x, -1, 1), CLIP<int16>(to.y - from.y, -1, 1));
	int dir = 0;
	while (kDirections[dir] != step) {
		++dir;
	}

	int reverseDir = (dir + kNumDirections / 2) % kNumDirections;
	for (Common::Point pos = from; pos != to; pos += step) {
		Common::Point next = pos + step;
		_links[pos.y][pos.x] |= 1 << dir;
		_links[next.y][next.x] |= 1 << reverseDir;
	}
}

bool FoxGeesePuzzle::cellAt(const Common::Point &mousePos, Common::Point &pos) const {
	for (int y = 0; y < kBoardSize; ++y) {
		for (int x = 0; x < kBoardSize; ++x) {
			if (isOnBoard(Common::Point(x, y)) && _cellDests[y][x].contains(mousePos)) {
				pos = Common::Point(x, y);
				return true;
			}
		}
	}

	return false;
}

// Moves go to a connected empty hole. Jumps go over a connected piece, onto the
// empty hole after it on the same line.
void FoxGeesePuzzle::getMoves(const Common::Point &pos, Common::Array<Common::Point> &moves,
		Common::Array<Jump> *jumps, Common::Array<Common::Point> *neighbors) const {
	for (int dir = 0; dir < kNumDirections; ++dir) {
		if (!(_links[pos.y][pos.x] & (1 << dir))) {
			continue;
		}

		Common::Point next = pos + kDirections[dir];
		if (neighbors) {
			neighbors->push_back(next);
		}

		if (pieceAt(next) == kEmpty) {
			moves.push_back(next);
		} else if (jumps && (_links[next.y][next.x] & (1 << dir))) {
			Common::Point landing = next + kDirections[dir];
			if (pieceAt(landing) == kEmpty) {
				Jump jump;
				jump.over = next;
				jump.to = landing;
				jumps->push_back(jump);
			}
		}
	}
}

uint FoxGeesePuzzle::countGeese() const {
	uint count = 0;
	for (int y = 0; y < kBoardSize; ++y) {
		for (int x = 0; x < kBoardSize; ++x) {
			if (_pieces[y][x] == kGoose) {
				++count;
			}
		}
	}

	return count;
}

// Returns false when the fox is trapped. Otherwise, fills in the move's destination, and
// the goose it jumps over, if any; a destination off the board means the fox ends its turn.
bool FoxGeesePuzzle::findFoxMove(Common::Point &to, Common::Point &over) {
	Common::Array<Common::Point> moves;
	Common::Array<Jump> jumps;
	getMoves(_foxFrom, moves, &jumps);

	if (moves.empty() && jumps.empty()) {
		return false;
	}

	to = kNoPosition;
	over = kNoPosition;

	Common::RandomSource *rnd = g_nancy->_randomSource;
	if (_isFoxChainingJumps) {
		if (!jumps.empty()) {
			const Jump &jump = jumps[rnd->getRandomNumber(jumps.size() - 1)];
			to = jump.to;
			over = jump.over;
		}
	} else if (_isSimpleFox || rnd->getRandomNumber(9) >= 2) {
		if (!jumps.empty()) {
			const Jump &jump = jumps[rnd->getRandomNumber(jumps.size() - 1)];
			to = jump.to;
			over = jump.over;
		} else if (_isSimpleFox || (!findMostMobileMove(moves, to) && !findMoveNearCenter(moves, to))) {
			to = moves[rnd->getRandomNumber(moves.size() - 1)];
		}
	} else {
		// Occasionally, any move goes
		uint choice = rnd->getRandomNumber(jumps.size() + moves.size() - 1);
		if (choice < jumps.size()) {
			to = jumps[choice].to;
			over = jumps[choice].over;
		} else {
			to = moves[choice - jumps.size()];
		}
	}

	return true;
}

// Picks one of the moves that leave the fox with the most moves afterwards,
// unless they all do equally well
bool FoxGeesePuzzle::findMostMobileMove(const Common::Array<Common::Point> &moves, Common::Point &to) const {
	Common::Array<Common::Point> best;
	uint bestCount = 0;
	for (const Common::Point &move : moves) {
		Common::Array<Common::Point> nextMoves;
		getMoves(move, nextMoves);
		if (nextMoves.size() > bestCount) {
			best.clear();
		}

		if (nextMoves.size() >= bestCount) {
			best.push_back(move);
			bestCount = nextMoves.size();
		}
	}

	if (best.size() == moves.size()) {
		return false;
	}

	to = best[g_nancy->_randomSource->getRandomNumber(best.size() - 1)];
	return true;
}

// Picks one of the moves next to the center of the board
bool FoxGeesePuzzle::findMoveNearCenter(const Common::Array<Common::Point> &moves, Common::Point &to) const {
	Common::Array<Common::Point> candidates;
	for (const Common::Point &move : moves) {
		Common::Array<Common::Point> nextMoves;
		Common::Array<Common::Point> neighbors;
		getMoves(move, nextMoves, nullptr, &neighbors);
		for (const Common::Point &neighbor : neighbors) {
			if (neighbor == kBoardCenter) {
				candidates.push_back(move);
				break;
			}
		}
	}

	if (candidates.empty()) {
		return false;
	}

	to = candidates[g_nancy->_randomSource->getRandomNumber(candidates.size() - 1)];
	return true;
}

void FoxGeesePuzzle::findFox() {
	for (int y = 0; y < kBoardSize; ++y) {
		for (int x = 0; x < kBoardSize; ++x) {
			if (_pieces[y][x] == kFox) {
				_foxFrom = Common::Point(x, y);
			}
		}
	}
}

// The fox is trapped; the ending may depend on where
void FoxGeesePuzzle::startWinEnding() {
	for (const WinEnding &win : _winEndings) {
		for (const Common::Point &pos : win.foxPositions) {
			if (pos == _foxFrom) {
				startEnding(win.ending);
				return;
			}
		}
	}

	startEnding(_defaultWinEnding);
}

void FoxGeesePuzzle::startFoxTurn() {
	findFox();

	Common::Point to, over;
	if (!findFoxMove(to, over)) {
		startWinEnding();
		return;
	}

	if (!isOnBoard(to)) {
		_isFoxChainingJumps = false;
		_isFoxTurn = false;
		return;
	}

	_foxTo = to;
	_captured = over;
	_isFoxMoving = true;
	_foxMoveStart = g_nancy->getTotalPlayTime();

	// After a jump, the fox gets to keep jumping
	if (isOnBoard(over)) {
		playSoundBlock(_foxJumpSound);
		_isFoxChainingJumps = true;
	} else {
		playSoundBlock(_foxMoveSound);
		_isFoxTurn = false;
	}
}

void FoxGeesePuzzle::finishFoxMove() {
	_isFoxMoving = false;
	pieceAt(_foxFrom) = kEmpty;
	pieceAt(_foxTo) = kFox;
	if (isOnBoard(_captured)) {
		pieceAt(_captured) = kEmpty;
		_captured = kNoPosition;
	}

	if (countGeese() <= kMinGeese) {
		startEnding(_loseEnding);
	}
}

void FoxGeesePuzzle::placeGoose(const Common::Point &mousePos) {
	_isGooseHeld = false;

	Common::Point pos;
	if (cellAt(mousePos, pos)) {
		Common::Array<Common::Point> moves;
		getMoves(_heldGooseFrom, moves);
		for (const Common::Point &move : moves) {
			if (move == pos) {
				pieceAt(pos) = kGoose;
				playSoundBlock(_gooseMoveSound);
				_isFoxTurn = true;
				return;
			}
		}
	}

	pieceAt(_heldGooseFrom) = kGoose;
}

void FoxGeesePuzzle::startEnding(const Ending &ending) {
	_isEnding = true;
	_endScene = ending.scene;
	_endSound = playSoundBlock(ending.sound);
}

void FoxGeesePuzzle::setCursor(const Common::Point &mousePos) {
	if (_isFoxTurn || _isFoxMoving) {
		setDataCursor(_foxTurnCursor, false);
		return;
	}

	Common::Point pos;
	bool isOverCell = cellAt(mousePos, pos);
	if (_isGooseHeld) {
		bool canMove = false;
		if (isOverCell) {
			Common::Array<Common::Point> moves;
			getMoves(_heldGooseFrom, moves);
			for (const Common::Point &move : moves) {
				if (move == pos) {
					canMove = true;
					break;
				}
			}
		}

		setDataCursor(_heldGooseCursor, canMove);
	} else if (isOverCell && pieceAt(pos) == kGoose) {
		setDataCursor(_gooseCursor);
	}
}

void FoxGeesePuzzle::redraw() {
	_drawSurface.clear(_drawSurface.getTransparentColor());

	for (int y = 0; y < kBoardSize; ++y) {
		for (int x = 0; x < kBoardSize; ++x) {
			Common::Point pos(x, y);
			if (_isFoxMoving && (pos == _foxFrom || pos == _captured)) {
				continue;
			}

			if (_pieces[y][x] != kEmpty) {
				_drawSurface.blitFrom(_image, _pieces[y][x] == kFox ? _foxSrc : _gooseSrc, _cellDests[y][x].origin());
			}
		}
	}

	// The moving fox slides between holes, while the goose it jumps over fades out
	if (_isFoxMoving) {
		float progress = _foxMoveDuration ? (float)(g_nancy->getTotalPlayTime() - _foxMoveStart) / _foxMoveDuration : 1.0f;
		progress = CLIP(progress, 0.0f, 1.0f);

		if (isOnBoard(_captured)) {
			_drawSurface.transBlitFrom(_image, _gooseSrc, cellDest(_captured).origin(), _image.getTransparentColor(),
				false, (uint32)((1.0f - progress) * 255));
		}

		const Common::Rect &from = cellDest(_foxFrom);
		const Common::Rect &to = cellDest(_foxTo);
		Common::Point foxPos(from.left + (int16)((to.left - from.left) * progress),
			from.top + (int16)((to.top - from.top) * progress));
		_drawSurface.blitFrom(_image, _foxSrc, foxPos);
	}

	if (_isGooseHeld) {
		_drawSurface.blitFrom(_image, _gooseSrc, getHeldPiecePos(_mousePos, _gooseSrc, _heldGooseHotspot));
	}

	_needsRedraw = true;
}

void FoxGeesePuzzle::execute() {
	switch (_state) {
	case kBegin:
		init();
		registerGraphics();
		_state = kRun;
		// fall through
	case kRun:
		if (_exitRequested) {
			_state = kActionTrigger;
			break;
		}

		if (_isEnding) {
			if (_endSound.name.empty() || !g_nancy->_sound->isSoundPlaying(_endSound)) {
				_state = kActionTrigger;
			}
		} else if (_isFoxMoving) {
			if (g_nancy->getTotalPlayTime() - _foxMoveStart >= _foxMoveDuration) {
				finishFoxMove();
			}
		} else if (_isFoxTurn) {
			startFoxTurn();
		}

		redraw();
		break;
	case kActionTrigger:
		if (_exitRequested) {
			_exitScene.execute();
		} else {
			_endScene.execute();
		}

		finishExecution();
		break;
	}
}

void FoxGeesePuzzle::handleInput(NancyInput &input) {
	if (_state != kRun || _isEnding || _exitRequested) {
		return;
	}

	// Cheat: Ctrl+Shift+W traps the fox where it stands, and wins the game
	for (const Common::KeyState &key : input.otherKbdInput) {
		if ((key.flags & Common::KBD_CTRL) && (key.flags & Common::KBD_SHIFT) && key.keycode == Common::KEYCODE_w) {
			debug("Fox and geese cheat: game won");
			findFox();
			startWinEnding();
			return;
		}
	}

	Common::Rect viewportBounds = NancySceneState.getViewport().getBounds();
	if (!NancySceneState.getViewport().convertViewportToScreen(viewportBounds).contains(input.mousePos)) {
		return;
	}

	Common::Rect mouseRect = NancySceneState.getViewport().convertScreenToViewport(
		Common::Rect(input.mousePos.x, input.mousePos.y, input.mousePos.x + 1, input.mousePos.y + 1));
	_mousePos = mouseRect.origin();

	setCursor(_mousePos);

	if (!_isGooseHeld && hoverExitHotspot(input)) {
		if (input.input & NancyInput::kLeftMouseButtonUp) {
			_exitRequested = true;
			input.eatMouseInput();
		}

		return;
	}

	if (_isFoxTurn || _isFoxMoving || !(input.input & NancyInput::kLeftMouseButtonUp)) {
		return;
	}

	if (_isGooseHeld) {
		placeGoose(_mousePos);
	} else {
		Common::Point pos;
		if (cellAt(_mousePos, pos) && pieceAt(pos) == kGoose) {
			pieceAt(pos) = kEmpty;
			_heldGooseFrom = pos;
			_isGooseHeld = true;
		}
	}

	input.eatMouseInput();
}

} // End of namespace Action
} // End of namespace Nancy
