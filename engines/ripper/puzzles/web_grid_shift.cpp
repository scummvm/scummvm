/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of their respective copyright holders.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ripper/puzzles/web_grid_shift.h"

#include "common/debug.h"
#include "common/ptr.h"
#include "common/stream.h"
#include "common/system.h"
#include "graphics/surface.h"

#include "ripper/cursor.h"
#include "ripper/detection.h"
#include "ripper/input.h"
#include "ripper/media.h"
#include "ripper/milestones.h"
#include "ripper/modal_dialog.h"
#include "ripper/ripper.h"
#include "ripper/settings.h"

namespace Ripper {

namespace {

static const uint kGridSize = 5;
static const uint kCellCount = kGridSize * kGridSize;
static const int kCellWidth = 85;
static const int kCellHeight = 70;
static const uint kDefaultCursor = 14;
static const uint kSelectionCursor = 16;
static const uint kHelpSelectionTable = 0x1ad;
static const uint16 kEscapeCommand = 0x1b;
static const uint16 kHelpCommand = 0x3b00;
static const uint16 kPreviewCommand = 0x2300;
static const uint16 kUpCommand = 0x4800;
static const uint16 kDownCommand = 0x5000;
static const uint16 kLeftCommand = 0x4b00;
static const uint16 kRightCommand = 0x4d00;
static const uint kAutoMovesPerLevel = 0x96;
static const uint kAnimationStep = 5;
static const uint kAnimationFrameMillis = 1;
static const uint kDosTickMillis = 55;
static const uint kPreviewHoldTicks = 0x36;
static const uint kFadeSteps = 9;
static const char *const kCompletionKeyword = "zztop";

// RunWebGridShiftPuzzleScene at 0x2cdce reads these as physical coordinates.
// Its displayBaseY calculation subtracts the 50-pixel scene origin when it is
// active, so adding kScenePresentationTop here would invert that translation.
static const int kRowY[kGridSize] = {
	26, 96, 166, 236, 306
};
static const int kColumnX[kGridSize] = {
	107, 192, 277, 362, 447
};

static const char *const kAudioNames[3] = {
	"web0.wav", "web1.wav", "web2.wav"
};

static uint cellIndex(uint row, uint column) {
	return row * kGridSize + column;
}

} // End of anonymous namespace

WebGridShiftPuzzle::WebGridShiftPuzzle(RipperEngine *engine) :
		Puzzle(engine), _random("ripper-web-grid-shift"),
		_currentRow(0), _currentCol(4), _keywordProgress(0),
		_hoveredCell(-1) {
	for (uint cell = 0; cell < kCellCount; ++cell)
		_tileOrder[cell] = cell;
}

bool WebGridShiftPuzzle::loadResources() {
	for (uint cue = 0; cue < ARRAYSIZE(kAudioNames); ++cue) {
		Common::ScopedPtr<Common::SeekableReadStream> stream(
			_engine->getResources()->createReadStreamForPath(kAudioNames[cue]));
		if (!stream) {
			warning("Ripper: web grid shift audio '%s' is missing",
				kAudioNames[cue]);
			return false;
		}
	}

	debugC(1, kDebugPuzzles,
		"Ripper: loaded web grid shift resources audio=[%s,%s,%s] "
		"tiles=captured-display",
		kAudioNames[0], kAudioNames[1], kAudioNames[2]);
	return true;
}

bool WebGridShiftPuzzle::captureGrid() {
	if (!_backgroundDisplay.capture() ||
			_backgroundDisplay.bounds().width() != kRipperScreenWidth ||
			_backgroundDisplay.bounds().height() != kRipperScreenHeight)
		return false;

	const Common::Array<byte> &pixels = _backgroundDisplay.pixels();
	for (uint row = 0; row < kGridSize; ++row) {
		for (uint column = 0; column < kGridSize; ++column) {
			Common::Array<byte> &tile = _tiles[cellIndex(row, column)];
			tile.resize(kCellWidth * kCellHeight);
			for (int y = 0; y < kCellHeight; ++y) {
				memcpy(tile.data() + y * kCellWidth,
					pixels.data() + (kRowY[row] + y) * kRipperScreenWidth +
						kColumnX[column],
					kCellWidth);
			}
		}
	}

	debugC(1, kDebugPuzzles,
		"Ripper: captured web grid shift tiles count=%u cell=%dx%d "
		"rowsY=26,96,166,236,306 columnsX=107,192,277,362,447 "
		"coordinates=physical",
		kCellCount, kCellWidth, kCellHeight);
	return true;
}

void WebGridShiftPuzzle::drawTileClipped(Graphics::Surface &screen, uint tile,
		int x, int y, const Common::Rect &clip) const {
	if (tile >= kCellCount || _tiles[tile].size() != kCellWidth * kCellHeight)
		return;

	const int left = MAX<int>(x, clip.left);
	const int top = MAX<int>(y, clip.top);
	const int right = MIN<int>(x + kCellWidth, clip.right);
	const int bottom = MIN<int>(y + kCellHeight, clip.bottom);
	if (left >= right || top >= bottom)
		return;

	for (int destinationY = top; destinationY < bottom; ++destinationY) {
		const byte *source = _tiles[tile].data() +
			(destinationY - y) * kCellWidth + left - x;
		memcpy(screen.getBasePtr(left, destinationY), source, right - left);
	}
}

void WebGridShiftPuzzle::renderOrder(const uint *order) const {
	if (!_backgroundDisplay.restorePixels())
		return;

	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1) {
		if (screen)
			g_system->unlockScreen();
		return;
	}

	const Common::Rect screenBounds(0, 0, screen->w, screen->h);
	for (uint row = 0; row < kGridSize; ++row) {
		for (uint column = 0; column < kGridSize; ++column) {
			const uint position = cellIndex(row, column);
			drawTileClipped(*screen, order[position], kColumnX[column],
				kRowY[row], screenBounds);
		}
	}
	g_system->unlockScreen();
	_backgroundDisplay.restorePalette();
	presentScreen();
}

void WebGridShiftPuzzle::renderShift(uint targetCell, uint progress) const {
	const uint targetRow = targetCell / kGridSize;
	const uint targetColumn = targetCell % kGridSize;
	const bool horizontal = targetRow == _currentRow;
	const uint currentAxis = horizontal ? _currentCol : _currentRow;
	const uint targetAxis = horizontal ? targetColumn : targetRow;
	const uint first = MIN<uint>(currentAxis, targetAxis);
	const uint last = MAX<uint>(currentAxis, targetAxis);
	const int delta = currentAxis < targetAxis ? -(int)progress : (int)progress;

	if (!_backgroundDisplay.restorePixels())
		return;
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1) {
		if (screen)
			g_system->unlockScreen();
		return;
	}

	const Common::Rect screenBounds(0, 0, screen->w, screen->h);
	for (uint row = 0; row < kGridSize; ++row) {
		for (uint column = 0; column < kGridSize; ++column) {
			const bool inShift = horizontal ?
				(row == _currentRow && column >= first && column <= last) :
				(column == _currentCol && row >= first && row <= last);
			if (inShift)
				continue;
			const uint position = cellIndex(row, column);
			drawTileClipped(*screen, _tileOrder[position],
				kColumnX[column], kRowY[row], screenBounds);
		}
	}

	Common::Rect clip;
	if (horizontal) {
		clip = Common::Rect(kColumnX[first], kRowY[_currentRow],
			kColumnX[last] + kCellWidth, kRowY[_currentRow] + kCellHeight);
	} else {
		clip = Common::Rect(kColumnX[_currentCol], kRowY[first],
			kColumnX[_currentCol] + kCellWidth, kRowY[last] + kCellHeight);
	}
	for (int y = clip.top; y < clip.bottom; ++y)
		memset(screen->getBasePtr(clip.left, y), 0, clip.width());

	if (horizontal) {
		for (uint column = first; column <= last; ++column) {
			const uint position = cellIndex(_currentRow, column);
			drawTileClipped(*screen, _tileOrder[position],
				kColumnX[column] + delta, kRowY[_currentRow], clip);
		}
		const int span = (last - first + 1) * kCellWidth;
		const int wrapOffset = _currentCol < targetColumn ? span : -span;
		const uint currentCell = cellIndex(_currentRow, _currentCol);
		drawTileClipped(*screen, _tileOrder[currentCell],
			kColumnX[_currentCol] + delta + wrapOffset,
			kRowY[_currentRow], clip);
	} else {
		for (uint row = first; row <= last; ++row) {
			const uint position = cellIndex(row, _currentCol);
			drawTileClipped(*screen, _tileOrder[position],
				kColumnX[_currentCol], kRowY[row] + delta, clip);
		}
		const int span = (last - first + 1) * kCellHeight;
		const int wrapOffset = _currentRow < targetRow ? span : -span;
		const uint currentCell = cellIndex(_currentRow, _currentCol);
		drawTileClipped(*screen, _tileOrder[currentCell],
			kColumnX[_currentCol],
			kRowY[_currentRow] + delta + wrapOffset, clip);
	}

	g_system->unlockScreen();
	_backgroundDisplay.restorePalette();
	presentScreen();
}

bool WebGridShiftPuzzle::animateShift(uint targetCell, bool autoPlay) {
	const uint targetRow = targetCell / kGridSize;
	const uint extent = targetRow == _currentRow ? kCellWidth : kCellHeight;
	_engine->getCursor()->setVisible(false);
	for (uint progress = kAnimationStep; progress <= extent;
			progress += kAnimationStep) {
		if (_engine->getInput()->pollEvents()) {
			_engine->quitGame();
			return false;
		}
		renderShift(targetCell, MIN<uint>(progress, extent));
		g_system->delayMillis(kAnimationFrameMillis);
	}
	if (!autoPlay)
		_engine->getCursor()->setVisible(true);
	return !_engine->shouldQuit();
}

bool WebGridShiftPuzzle::shiftToCell(uint targetCell, bool autoPlay) {
	if (targetCell >= kCellCount)
		return false;
	const uint targetRow = targetCell / kGridSize;
	const uint targetColumn = targetCell % kGridSize;
	if ((targetRow != _currentRow && targetColumn != _currentCol) ||
			(targetRow == _currentRow && targetColumn == _currentCol))
		return false;

	playMoveCue();
	debugC(autoPlay ? 3 : 2, kDebugPuzzles,
		"Ripper: web grid shift move phase=%s from=%u,%u to=%u,%u "
		"axis=%s coordinates=row,column",
		autoPlay ? "autoplay" : "manual", _currentRow, _currentCol,
		targetRow, targetColumn,
		targetRow == _currentRow ? "row" : "column");
	if (!animateShift(targetCell, autoPlay))
		return false;

	if (targetRow == _currentRow) {
		if (_currentCol < targetColumn) {
			const uint wrapped = _tileOrder[cellIndex(_currentRow, _currentCol)];
			for (uint column = _currentCol; column < targetColumn; ++column)
				_tileOrder[cellIndex(_currentRow, column)] =
					_tileOrder[cellIndex(_currentRow, column + 1)];
			_tileOrder[cellIndex(_currentRow, targetColumn)] = wrapped;
		} else {
			const uint wrapped = _tileOrder[cellIndex(_currentRow, _currentCol)];
			for (uint column = _currentCol; column > targetColumn; --column)
				_tileOrder[cellIndex(_currentRow, column)] =
					_tileOrder[cellIndex(_currentRow, column - 1)];
			_tileOrder[cellIndex(_currentRow, targetColumn)] = wrapped;
		}
		_currentCol = targetColumn;
	} else {
		if (_currentRow < targetRow) {
			const uint wrapped = _tileOrder[cellIndex(_currentRow, _currentCol)];
			for (uint row = _currentRow; row < targetRow; ++row)
				_tileOrder[cellIndex(row, _currentCol)] =
					_tileOrder[cellIndex(row + 1, _currentCol)];
			_tileOrder[cellIndex(targetRow, _currentCol)] = wrapped;
		} else {
			const uint wrapped = _tileOrder[cellIndex(_currentRow, _currentCol)];
			for (uint row = _currentRow; row > targetRow; --row)
				_tileOrder[cellIndex(row, _currentCol)] =
					_tileOrder[cellIndex(row - 1, _currentCol)];
			_tileOrder[cellIndex(targetRow, _currentCol)] = wrapped;
		}
		_currentRow = targetRow;
	}
	renderOrder(_tileOrder);
	debugC(3, kDebugPuzzles,
		"Ripper: web grid shift order current=%u,%u tiles=[%s]",
		_currentRow, _currentCol, orderString().c_str());
	return true;
}

int WebGridShiftPuzzle::targetForDirection(uint direction) const {
	// Retail maps the DOS arrow commands to the opposite physical direction:
	// Up grows the row, Down shrinks it, Left grows the column, and Right
	// shrinks it. The autoplay direction ids 1..4 use the same mapping.
	switch (direction) {
	case 1:
		return _currentRow < kGridSize - 1 ?
			cellIndex(_currentRow + 1, _currentCol) : -1;
	case 2:
		return _currentRow > 0 ?
			cellIndex(_currentRow - 1, _currentCol) : -1;
	case 3:
		return _currentCol < kGridSize - 1 ?
			cellIndex(_currentRow, _currentCol + 1) : -1;
	case 4:
		return _currentCol > 0 ?
			cellIndex(_currentRow, _currentCol - 1) : -1;
	default:
		return -1;
	}
}

bool WebGridShiftPuzzle::runAutoPlay(uint moveLimit, uint completionFlag,
		Result &result) {
	if (!_engine->getMedia()->playSoundEffect(kAudioNames[0],
			_audioHandles[0], 100, true))
		warning("Ripper: could not start web grid shift autoplay audio");

	uint lastDirection = 0;
	uint attemptedMoves = 0;
	while (attemptedMoves < moveLimit && !_engine->shouldQuit()) {
		uint direction = 0;
		do {
			direction = _random.getRandomNumber(3) + 1;
		} while ((direction == 1 && lastDirection == 2) ||
				(direction == 2 && lastDirection == 1) ||
				(direction == 3 && lastDirection == 4) ||
				(direction == 4 && lastDirection == 3));
		lastDirection = direction;
		++attemptedMoves;
		const bool finalAttempt = attemptedMoves >= moveLimit;
		if (finalAttempt)
			_engine->getMedia()->stopSoundEffect(_audioHandles[0]);

		const int targetCell = targetForDirection(direction);
		if (targetCell >= 0) {
			if (!shiftToCell(targetCell, !finalAttempt))
				return false;
			if (finalAttempt && isSolved()) {
				result = complete(completionFlag) ? kSolved : kLoadFailed;
				return false;
			}
		}
	}

	_engine->getCursor()->setVisible(true);
	debugC(2, kDebugPuzzles,
		"Ripper: web grid shift autoplay completed attempts=%u "
		"current=%u,%u solved=%d tiles=[%s]",
		attemptedMoves, _currentRow, _currentCol, isSolved(),
		orderString().c_str());
	return !_engine->shouldQuit();
}

bool WebGridShiftPuzzle::waitTicks(uint ticks) {
	const uint32 end = g_system->getMillis() + ticks * kDosTickMillis;
	while (!_engine->shouldQuit() &&
			(int32)(end - g_system->getMillis()) > 0) {
		if (_engine->getInput()->pollEvents()) {
			_engine->quitGame();
			return false;
		}
		g_system->delayMillis(10);
	}
	return !_engine->shouldQuit();
}

bool WebGridShiftPuzzle::showPreview() {
	uint previewOrder[kCellCount];
	for (uint cell = 0; cell < kCellCount; ++cell)
		previewOrder[cell] = _tileOrder[cell];

	_engine->getCursor()->setVisible(false);
	debugC(1, kDebugPuzzles,
		"Ripper: web grid shift preview starting command=0x%04x "
		"holdTicks=%u",
		kPreviewCommand, kPreviewHoldTicks);
	for (uint cell = 0; cell < kCellCount; ++cell) {
		previewOrder[cell] = cell;
		playMoveCue();
		renderOrder(previewOrder);
		if (!waitTicks(1))
			return false;
	}

	_engine->getCursor()->setVisible(true);
	_engine->getInput()->drainKeys();
	_engine->getInput()->discardMouseTransitions();
	const uint32 end = g_system->getMillis() +
		kPreviewHoldTicks * kDosTickMillis;
	while (!_engine->shouldQuit() &&
			(int32)(end - g_system->getMillis()) > 0) {
		if (_engine->getInput()->pollEvents()) {
			_engine->quitGame();
			return false;
		}
		if (_engine->getInput()->hasPendingKey())
			break;
		const MouseState mouse = _engine->getInput()->publishMouseState();
		if ((mouse.pressed & kMouseButtonLeft) != 0)
			break;
		g_system->delayMillis(10);
	}

	_engine->getCursor()->setVisible(false);
	for (uint cell = 0; cell < kCellCount; ++cell) {
		previewOrder[cell] = _tileOrder[cell];
		playMoveCue();
		renderOrder(previewOrder);
		if (!waitTicks(1))
			return false;
	}
	_engine->getInput()->drainKeys();
	_engine->getInput()->discardMouseTransitions();
	_engine->getCursor()->setVisible(true);
	debugC(1, kDebugPuzzles,
		"Ripper: web grid shift preview completed restoredTiles=[%s]",
		orderString().c_str());
	return true;
}

int WebGridShiftPuzzle::findCell(const Common::Point &point) const {
	for (uint row = 0; row < kGridSize; ++row) {
		for (uint column = 0; column < kGridSize; ++column) {
			if (Common::Rect(kColumnX[column], kRowY[row],
					kColumnX[column] + kCellWidth,
					kRowY[row] + kCellHeight).contains(point))
				return cellIndex(row, column);
		}
	}
	return -1;
}

bool WebGridShiftPuzzle::updateCursor(const Common::Point &point) {
	const int cell = findCell(point);
	const bool changed = cell != _hoveredCell;
	if (changed) {
		debugC(2, kDebugInput,
			"Ripper: web grid shift hover cell=%d previous=%d "
			"point=%d,%d current=%u",
			cell, _hoveredCell, point.x, point.y,
			cellIndex(_currentRow, _currentCol));
		_hoveredCell = cell;
	}
	const uint cursor = cell >= 0 &&
		cell != (int)cellIndex(_currentRow, _currentCol) ?
		kSelectionCursor : kDefaultCursor;
	_engine->getCursor()->update(cursor);
	_engine->getCursor()->setVisible(true);
	return changed;
}

bool WebGridShiftPuzzle::isSolved() const {
	for (uint cell = 0; cell < kCellCount; ++cell) {
		if (_tileOrder[cell] != cell)
			return false;
	}
	return true;
}

bool WebGridShiftPuzzle::complete(uint completionFlag) {
	if (!_engine->getMilestones()->set(completionFlag, true,
			"web-grid-shift-puzzle"))
		return false;
	debugC(1, kDebugPuzzles,
		"Ripper: solved web grid shift puzzle milestone=%u "
		"current=%u,%u tiles=[%s]",
		completionFlag, _currentRow, _currentCol, orderString().c_str());
	return true;
}

bool WebGridShiftPuzzle::handleKeyword(uint16 command, uint completionFlag,
		Result &result) {
	if (command == 0)
		return false;
	if (command > 0xff) {
		_keywordProgress = 0;
		return false;
	}
	char character = command & 0xff;
	if (character >= 'A' && character <= 'Z')
		character += 'a' - 'A';
	if (character == kCompletionKeyword[_keywordProgress])
		++_keywordProgress;
	else
		_keywordProgress = 0;
	if (kCompletionKeyword[_keywordProgress] != '\0')
		return false;

	debugC(1, kDebugPuzzles,
		"Ripper: web grid shift solved by hidden keyword '%s'",
		kCompletionKeyword);
	result = complete(completionFlag) ? kSolved : kLoadFailed;
	return true;
}

void WebGridShiftPuzzle::playMoveCue() {
	const uint cue = _random.getRandomNumber(1) + 1;
	_engine->getMedia()->playSoundEffect(kAudioNames[cue], _audioHandles[cue]);
}

void WebGridShiftPuzzle::stopAudio() {
	for (uint cue = 0; cue < ARRAYSIZE(_audioHandles); ++cue)
		_engine->getMedia()->stopSoundEffect(_audioHandles[cue]);
}

Common::String WebGridShiftPuzzle::orderString() const {
	Common::String result;
	for (uint cell = 0; cell < kCellCount; ++cell) {
		if (cell != 0)
			result += ',';
		result += Common::String::format("%u", _tileOrder[cell]);
	}
	return result;
}

WebGridShiftPuzzle::Result WebGridShiftPuzzle::run(uint completionFlag) {
	// DispatchSceneEntryAction at 0x36892 maps action 19 to
	// RunWebGridShiftPuzzleScene at 0x2cdce.
	if (!loadResources() || !captureGrid())
		return kLoadFailed;

	CursorManager *cursor = _engine->getCursor();
	const uint savedSelectionIndex = cursor->getSelectionIndex();
	const bool savedCursorVisible = cursor->isVisible();
	const uint puzzleLevel =
		CLIP<uint>(_engine->getSettings()->getPuzzleLevel(), 1, 3);
	const uint autoMoveLimit = puzzleLevel * kAutoMovesPerLevel;

	_engine->getInput()->drainKeys();
	_engine->getInput()->discardMouseTransitions();
	cursor->setSelectionIndex(kDefaultCursor);
	cursor->dispatchSelectionIndexChange(kDefaultCursor);
	cursor->setVisible(false);
	renderOrder(_tileOrder);
	debugC(1, kDebugPuzzles,
		"Ripper: entered web grid shift puzzle function="
		"RunWebGridShiftPuzzleScene@0x2cdce milestone=%u difficulty=%u "
		"autoplayMoves=%u help=0x%x grid=5x5 cell=85x70 "
		"current=0,4 coordinates=physical",
		completionFlag, puzzleLevel, autoMoveLimit, kHelpSelectionTable);
	debugC(3, kDebugPuzzles,
		"Ripper: web grid shift hidden keyword='%s' previewCommand=0x%04x",
		kCompletionKeyword, kPreviewCommand);

	Result result = kExited;
	bool active = runAutoPlay(autoMoveLimit, completionFlag, result);
	while (active && !_engine->shouldQuit()) {
		if (_engine->getInput()->pollEvents()) {
			_engine->quitGame();
			break;
		}

		int targetCell = -1;
		uint16 command = 0;
		if (_engine->getInput()->hasPendingKey())
			command = _engine->getInput()->consumeKey();
		if (command == kEscapeCommand) {
			debugC(1, kDebugPuzzles,
				"Ripper: web grid shift exited by Escape");
			break;
		}
		if (command == kHelpCommand) {
			_keywordProgress = 0;
			debugC(1, kDebugPuzzles,
				"Ripper: web grid shift opening modal help table=0x%x",
				kHelpSelectionTable);
			if (!_engine->getModalDialog()->run(kHelpSelectionTable))
				warning("Ripper: web grid shift modal help failed");
			renderOrder(_tileOrder);
		} else if (command == kPreviewCommand) {
			_keywordProgress = 0;
			if (!showPreview())
				break;
		} else if (command == kUpCommand) {
			targetCell = targetForDirection(1);
		} else if (command == kDownCommand) {
			targetCell = targetForDirection(2);
		} else if (command == kLeftCommand) {
			targetCell = targetForDirection(3);
		} else if (command == kRightCommand) {
			targetCell = targetForDirection(4);
		} else if (handleKeyword(command, completionFlag, result)) {
			active = false;
		}
		if (!active)
			break;

		const MouseState mouse = _engine->getInput()->publishMouseState();
		const bool cursorChanged = updateCursor(mouse.position);
		if (targetCell < 0 &&
				(mouse.pressed & kMouseButtonLeft) != 0)
			targetCell = findCell(mouse.position);

		if (targetCell >= 0 &&
				shiftToCell(targetCell, false)) {
			const bool solved = isSolved();
			debugC(solved ? 1 : 2, kDebugPuzzles,
				"Ripper: web grid shift validation after move "
				"target=%d outcome=%s milestone=%u milestoneSet=%d",
				targetCell, solved ? "SOLVED" : "NOT_SOLVED",
				completionFlag,
				_engine->getMilestones()->isSet(completionFlag));
			if (solved) {
				result = complete(completionFlag) ? kSolved : kLoadFailed;
				active = false;
			}
		} else if (cursorChanged) {
			presentScreen();
		} else {
			// ServiceUiControlStateSelection runs every retail input tick.
			presentScreen();
		}
		g_system->delayMillis(10);
	}

	stopAudio();
	cursor->setVisible(false);
	if (!_engine->shouldQuit()) {
		_engine->getMedia()->fadePalette(false, kFadeSteps);
		g_system->fillScreen(0);
		presentScreen();
	}
	cursor->setSelectionIndex(savedSelectionIndex);
	cursor->dispatchSelectionIndexChange(savedSelectionIndex);
	cursor->refresh();
	cursor->setVisible(savedCursorVisible);
	_engine->getInput()->drainKeys();
	_engine->getInput()->discardMouseTransitions();
	debugC(result == kLoadFailed ? 2 : 1, kDebugPuzzles,
		"Ripper: left web grid shift puzzle result=%d milestone=%u "
		"milestoneSet=%d current=%u,%u solved=%d quit=%d",
		result, completionFlag,
		_engine->getMilestones()->isSet(completionFlag),
		_currentRow, _currentCol, isSolved(), _engine->shouldQuit());
	return result;
}

} // End of namespace Ripper
