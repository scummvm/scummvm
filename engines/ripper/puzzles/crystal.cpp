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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ripper/puzzles/crystal.h"

#include "common/debug.h"
#include "common/file.h"
#include "common/formats/ini-file.h"
#include "common/system.h"
#include "graphics/paletteman.h"
#include "graphics/surface.h"

#include "ripper/cursor.h"
#include "ripper/detection.h"
#include "ripper/input.h"
#include "ripper/media.h"
#include "ripper/milestones.h"
#include "ripper/modal_dialog.h"
#include "ripper/ripper.h"

namespace Ripper {

namespace {

static const uint kPieceCount = 16;
static const uint kGridCellCount = 72;
static const uint kGridColumns = 9;
static const uint kRequiredCellCount = 10;
static const uint kDefaultCursor = 14;
static const uint kSelectionCursor = 16;
static const uint kEdgeCursor = 7;
static const int kSceneOriginY = 50;
static const int kCellWidth = 30;
static const int kCellHeight = 18;
static const int kTrayLeft = 183;
static const int kTrayTop = kSceneOriginY;
static const int kTrayRight = 333;
static const int kTrayBottom = 150;
static const int kEdgeTop = kSceneOriginY + 30;
static const int kEdgeBottom = kEdgeTop + 235;
static const int kHeadX = 328;
static const int kHeadY = kSceneOriginY;
static const uint kDefaultPuzzleLevel = 2;
static const uint kCrystalCompletionFadeSteps = 9;
static const uint kEasySeedDelayTicks = 0x12;
static const uint kDosTimerTickMillis = 55;
static const uint16 kHelpAction = 0x3b00;
static const uint kHelpSelectionTable = 0x19e;

static const int kTrayPieceY[kPieceCount] = {
	10, 16, 25, 18, 30, 44, 44, 36, 34, 63, 57, 54, 79, 76, 82, 63
};

static const int kTrayPieceX[kPieceCount] = {
	252, 212, 241, 266, 197, 219, 241, 267, 299, 218, 246, 271, 199, 234, 265, 289
};

static const int kGridRowY[8] = {
	106, 129, 151, 174, 196, 218, 241, 264
};

static const int kGridColumnX[kGridColumns] = {
	179, 212, 245, 277, 308, 340, 372, 404, 437
};

static const uint kSolvedCells[kRequiredCellCount] = {
	2, 9, 21, 25, 32, 37, 44, 48, 60, 62
};

static const char kCompletionKeyword[] = "pisces";

} // End of anonymous namespace

CrystalPuzzle::CrystalPuzzle(RipperEngine *engine) : Puzzle(engine),
		_random("ripper-crystal-puzzle"), _draggedPiece(-1), _hoveredPiece(-1),
		_keywordIndex(0) {
	for (uint piece = 0; piece < kPieceCount; ++piece)
		_pieceCells[piece] = -1;
	for (uint cell = 0; cell < kGridCellCount; ++cell)
		_cellPieces[cell] = -1;
}

bool CrystalPuzzle::captureBackground() {
	return _backgroundDisplay.capture();
}

void CrystalPuzzle::restoreBackground() const {
	_backgroundDisplay.restore();
}

bool CrystalPuzzle::loadPieceSet(const char *prefix,
		Common::Array<BitmapAssetFrame> &frames) {
	frames.clear();
	for (uint piece = 0; piece < kPieceCount; ++piece) {
		BitmapAssetSequence sequence;
		const Common::String name = Common::String::format("%s%u.bbm", prefix, piece);
		if (!_engine->getResources()->loadBitmapSequence(name, sequence) ||
				sequence.frames.empty())
			return false;
		frames.push_back(Common::move(sequence.frames.front()));
	}
	return true;
}

bool CrystalPuzzle::loadAssets() {
	if (!loadPieceSet("crysp", _placedFrames) ||
			!loadPieceSet("cryst", _trayFrames) ||
			!loadPieceSet("crysb", _blankingFrames))
		return false;

	_puzzlePalette = _blankingFrames.front().palette;
	debugC(1, kDebugPuzzles,
		"Ripper: loaded crystal puzzle assets placed=%u tray=%u blanking=%u",
		_placedFrames.size(), _trayFrames.size(), _blankingFrames.size());
	return true;
}

Common::Point CrystalPuzzle::trayPosition(uint piece) const {
	return Common::Point(kTrayPieceX[piece], kTrayPieceY[piece] + kSceneOriginY);
}

Common::Point CrystalPuzzle::gridPosition(uint cell, const BitmapAssetFrame &frame) const {
	const uint row = cell / kGridColumns;
	const uint column = cell % kGridColumns;
	return Common::Point(kGridColumnX[column] + (kCellWidth - frame.width) / 2,
		kGridRowY[row] + kSceneOriginY + (kCellHeight - frame.height) / 2);
}

void CrystalPuzzle::drawFrame(byte *screen, uint pitch, const BitmapAssetFrame &frame,
		int x, int y) const {
	for (uint sourceY = 0; sourceY < frame.height; ++sourceY) {
		const int destinationY = y + sourceY;
		if (destinationY < 0 || destinationY >= 400)
			continue;
		for (uint sourceX = 0; sourceX < frame.width; ++sourceX) {
			const int destinationX = x + sourceX;
			if (destinationX < 0 || destinationX >= 640)
				continue;
			const byte pixel = frame.pixels[sourceY * frame.width + sourceX];
			if (pixel != frame.transparentColor)
				screen[destinationY * pitch + destinationX] = pixel;
		}
	}
}

void CrystalPuzzle::render() {
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1) {
		if (screen)
			g_system->unlockScreen();
		return;
	}

	for (int y = 0; y < screen->h; ++y)
		memcpy(screen->getBasePtr(0, y),
			_backgroundDisplay.pixels().data() + y * screen->w, screen->w);
	byte *pixels = (byte *)screen->getPixels();
	for (uint piece = 0; piece < kPieceCount; ++piece) {
		const Common::Point tray = trayPosition(piece);
		if (_pieceCells[piece] < 0 && (int)piece != _draggedPiece) {
			const BitmapAssetFrame &frame = _trayFrames[piece];
			drawFrame(pixels, screen->pitch, frame, tray.x, tray.y);
		} else {
			const BitmapAssetFrame &frame = _blankingFrames[piece];
			drawFrame(pixels, screen->pitch, frame, tray.x, tray.y);
		}

		if (_pieceCells[piece] >= 0) {
			const BitmapAssetFrame &frame = _placedFrames[piece];
			const Common::Point position = gridPosition(_pieceCells[piece], frame);
			drawFrame(pixels, screen->pitch, frame, position.x, position.y);
		}
	}
	if (_draggedPiece >= 0) {
		const BitmapAssetFrame &frame = _placedFrames[_draggedPiece];
		drawFrame(pixels, screen->pitch, frame, _dragPoint.x - frame.width / 2,
			_dragPoint.y - frame.height / 2);
	}
	g_system->unlockScreen();

	if (_puzzlePalette.size() == 256 * 3)
		g_system->getPaletteManager()->setPalette(_puzzlePalette.data(), 0, 256);
	presentScreen();
}

int CrystalPuzzle::findTrayPiece(const Common::Point &point) const {
	for (int piece = kPieceCount - 1; piece >= 0; --piece) {
		if (piece == _draggedPiece || _pieceCells[piece] >= 0)
			continue;
		const BitmapAssetFrame &frame = _blankingFrames[piece];
		const Common::Point position = trayPosition(piece);
		const int x = point.x - position.x;
		const int y = point.y - position.y;
		if (x >= 0 && y >= 0 && x < frame.width && y < frame.height &&
				frame.pixels[y * frame.width + x] != frame.transparentColor)
			return piece;
	}
	return -1;
}

int CrystalPuzzle::findGridCell(const Common::Point &point) const {
	for (uint row = 0; row < ARRAYSIZE(kGridRowY); ++row) {
		for (uint column = 0; column < kGridColumns; ++column) {
			if (Common::Rect(kGridColumnX[column], kGridRowY[row] + kSceneOriginY,
					kGridColumnX[column] + kCellWidth,
					kGridRowY[row] + kSceneOriginY + kCellHeight).contains(point))
				return row * kGridColumns + column;
		}
	}
	return -1;
}

int CrystalPuzzle::findPlacedPiece(const Common::Point &point) const {
	const int cell = findGridCell(point);
	return cell >= 0 ? _cellPieces[cell] : -1;
}

void CrystalPuzzle::playMovementCue() {
	const uint cue = _random.getRandomNumber(1);
	_engine->getMedia()->playSoundEffect(Common::String::format("crystal%u.wav", cue),
		_audioHandles[cue]);
}

void CrystalPuzzle::beginDrag(uint piece, const Common::Point &point) {
	if (_pieceCells[piece] >= 0) {
		_cellPieces[_pieceCells[piece]] = -1;
		_pieceCells[piece] = -1;
	}
	_draggedPiece = piece;
	_hoveredPiece = -1;
	_dragPoint = point;
	playMovementCue();
	debugC(2, kDebugPuzzles,
		"Ripper: crystal puzzle drag began piece=%u point=%d,%d",
		piece, point.x, point.y);
}

void CrystalPuzzle::placeDraggedPiece(uint cell) {
	_cellPieces[cell] = _draggedPiece;
	_pieceCells[_draggedPiece] = cell;
	debugC(2, kDebugPuzzles,
		"Ripper: crystal puzzle placed piece=%d cell=%u row=%u column=%u",
		_draggedPiece, cell, cell / kGridColumns, cell % kGridColumns);
	_draggedPiece = -1;
	playMovementCue();
}

void CrystalPuzzle::returnDraggedPieceToTray() {
	debugC(2, kDebugPuzzles, "Ripper: crystal puzzle returned piece=%d to tray",
		_draggedPiece);
	_draggedPiece = -1;
	playMovementCue();
}

bool CrystalPuzzle::isSolved() const {
	uint occupied = 0;
	for (uint cell = 0; cell < kGridCellCount; ++cell) {
		if (_cellPieces[cell] >= 0)
			++occupied;
	}
	if (occupied != kRequiredCellCount)
		return false;
	for (uint index = 0; index < kRequiredCellCount; ++index) {
		if (_cellPieces[kSolvedCells[index]] < 0)
			return false;
	}
	return true;
}

uint CrystalPuzzle::readPuzzleLevel() const {
	Common::File iniFile;
	Common::INIFile ini;
	Common::String value;
	uint puzzleLevel = kDefaultPuzzleLevel;
	if (iniFile.open("ripper.ini") && ini.loadFromStream(iniFile) &&
			ini.getKey("puzzle level", "game", value)) {
		const uint64 configuredLevel = value.asUint64();
		if (configuredLevel >= 1 && configuredLevel <= 3)
			puzzleLevel = (uint)configuredLevel;
	}
	debugC(2, kDebugPuzzles, "Ripper: crystal puzzle difficulty level=%u",
		puzzleLevel);
	return puzzleLevel;
}

void CrystalPuzzle::seedInitialPieces(uint puzzleLevel) {
	if (puzzleLevel == 1) {
		_pieceCells[0] = 9;
		_cellPieces[9] = 0;
		playMovementCue();
		debugC(2, kDebugPuzzles,
			"Ripper: crystal puzzle seeded piece=0 cell=9 difficulty=1");
		render();
		g_system->delayMillis(kEasySeedDelayTicks * kDosTimerTickMillis);
		_pieceCells[1] = 44;
		_cellPieces[44] = 1;
		playMovementCue();
		debugC(2, kDebugPuzzles,
			"Ripper: crystal puzzle seeded piece=1 cell=44 difficulty=1");
	} else if (puzzleLevel == 2) {
		_pieceCells[0] = 21;
		_cellPieces[21] = 0;
		playMovementCue();
		debugC(2, kDebugPuzzles,
			"Ripper: crystal puzzle seeded piece=0 cell=21 difficulty=2");
	}
}

bool CrystalPuzzle::complete(uint completionFlag) {
	if (!markSolved(completionFlag, "crystal-puzzle"))
		return false;
	_engine->getMedia()->playSoundEffect("crystal2.wav", _audioHandles[2]);
	_engine->getMedia()->playSoundEffect("crystal3.wav", _audioHandles[3]);
	_engine->getCursor()->setVisible(false);
	debugC(1, kDebugPuzzles,
		"Ripper: completed crystal puzzle milestone=%u head='cryshead.smk' "
		"position=%d,%d solution='crysolve.avi'",
		completionFlag, kHeadX, kHeadY);
	if (!_engine->getMedia()->play("cryshead.smk", false, kHeadX, kHeadY))
		warning("Ripper: could not play crystal puzzle completion header");
	_engine->getMedia()->fadePalette(false, kCrystalCompletionFadeSteps);
	if (!_engine->getMedia()->play("crysolve.avi", false))
		warning("Ripper: could not play crystal puzzle solution video");
	return true;
}

void CrystalPuzzle::updateCursor(const Common::Point &point) {
	uint cursor = kDefaultCursor;
	int hoveredPiece = -1;
	const char *hoverSource = "none";
	if (_draggedPiece >= 0) {
		cursor = kSelectionCursor;
	} else {
		hoveredPiece = findTrayPiece(point);
		if (hoveredPiece >= 0) {
			hoverSource = "tray";
			cursor = kSelectionCursor;
		} else {
			hoveredPiece = findPlacedPiece(point);
			if (hoveredPiece >= 0) {
				hoverSource = "grid";
				cursor = kSelectionCursor;
			}
		}
	}
	if (cursor == kDefaultCursor && point.y >= kEdgeTop && point.y < kEdgeBottom &&
			((point.x >= 20 && point.x < 150) ||
			(point.x >= 500 && point.x < 630)))
		cursor = kEdgeCursor;

	if (hoveredPiece != _hoveredPiece) {
		if (hoveredPiece >= 0) {
			debugC(2, kDebugPuzzles,
				"Ripper: crystal puzzle hover piece=%d source=%s point=%d,%d cursor=%u",
				hoveredPiece, hoverSource, point.x, point.y, cursor);
		} else if (_hoveredPiece >= 0) {
			debugC(2, kDebugPuzzles,
				"Ripper: crystal puzzle hover cleared piece=%d point=%d,%d cursor=%u",
				_hoveredPiece, point.x, point.y, cursor);
		}
		_hoveredPiece = hoveredPiece;
	}
	_engine->getCursor()->update(cursor);
}

CrystalPuzzle::Result CrystalPuzzle::run(uint completionFlag) {
	if (!captureBackground() || !loadAssets()) {
		restoreBackground();
		return kLoadFailed;
	}

	debugC(1, kDebugPuzzles,
		"Ripper: entered crystal puzzle pieces=%u grid=%ux%u completionFlag=%u keyword='%s'",
		kPieceCount, kGridColumns, ARRAYSIZE(kGridRowY), completionFlag,
		kCompletionKeyword);
	_engine->getInput()->discardMouseTransitions();
	seedInitialPieces(readPuzzleLevel());
	render();

	Result result = kExited;
	bool active = true;
	while (active && !_engine->shouldQuit()) {
		if (!serviceEngineEvents())
			break;
		while (_engine->getInput()->hasPendingKey()) {
			const uint16 command = _engine->getInput()->consumeKey();
			if (command == 0x1b) {
				debugC(1, kDebugPuzzles, "Ripper: crystal puzzle exited by Escape");
				active = false;
				break;
			}
			if (command == kHelpAction) {
				debugC(1, kDebugPuzzles,
					"Ripper: crystal puzzle opening modal help table=0x%x",
					kHelpSelectionTable);
				if (!_engine->getModalDialog()->run(kHelpSelectionTable))
					warning("Ripper: crystal puzzle modal help failed");
				render();
				continue;
			}
			char character = command & 0xff;
			if (character >= 'A' && character <= 'Z')
				character += 'a' - 'A';
			if (character == kCompletionKeyword[_keywordIndex])
				++_keywordIndex;
			else
				_keywordIndex = 0;
			if (kCompletionKeyword[_keywordIndex] == '\0') {
				debugC(1, kDebugPuzzles,
					"Ripper: crystal puzzle solved by hidden keyword");
				result = complete(completionFlag) ? kSolved : kLoadFailed;
				active = false;
				break;
			}
		}
		if (!active)
			break;

		const MouseState mouse = _engine->getInput()->publishMouseState();
		bool changed = false;
		if (_draggedPiece >= 0) {
			_dragPoint = mouse.position;
			changed = true;
			if ((mouse.pressed & kMouseButtonLeft) != 0) {
				const int cell = findGridCell(mouse.position);
				if (cell >= 0 && _cellPieces[cell] < 0) {
					placeDraggedPiece(cell);
					changed = true;
					if (isSolved()) {
						debugC(1, kDebugPuzzles,
							"Ripper: crystal puzzle solved by grid layout");
						result = complete(completionFlag) ? kSolved : kLoadFailed;
						active = false;
					}
				} else if (Common::Rect(kTrayLeft, kTrayTop, kTrayRight,
						kTrayBottom).contains(mouse.position)) {
					returnDraggedPieceToTray();
					changed = true;
				}
			}
		} else if ((mouse.pressed & kMouseButtonLeft) != 0) {
			int piece = findTrayPiece(mouse.position);
			if (piece < 0)
				piece = findPlacedPiece(mouse.position);
			if (piece >= 0) {
				beginDrag(piece, mouse.position);
				changed = true;
			}
		}
		updateCursor(mouse.position);
		if (changed && active)
			render();
		presentScreen();
		g_system->delayMillis(10);
	}

	for (uint cue = 0; cue < ARRAYSIZE(_audioHandles); ++cue)
		_engine->getMedia()->stopSoundEffect(_audioHandles[cue]);
	restoreBackground();
	_engine->getCursor()->update(0);
	return result;
}

} // End of namespace Ripper
