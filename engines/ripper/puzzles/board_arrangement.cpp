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

#include "ripper/puzzles/board_arrangement.h"

#include "common/debug.h"
#include "common/ptr.h"
#include "common/stream.h"
#include "common/system.h"
#include "graphics/surface.h"
#include "image/pcx.h"

#include "ripper/cursor.h"
#include "ripper/detection.h"
#include "ripper/input.h"
#include "ripper/media.h"
#include "ripper/milestones.h"
#include "ripper/modal_dialog.h"
#include "ripper/ripper.h"
#include "ripper/toolbar.h"

namespace Ripper {

namespace {

static const uint kPieceCount = 8;
static const int kSceneOriginY = 50;
static const uint kDefaultCursor = 14;
static const uint kPieceCursor = 16;
static const uint kExitCursor = 7;
static const uint kHelpSelectionTable = 0x1ab;
static const uint kDosTickMillis = 55;
static const uint kInvalidDropPixelsPerTick = 15;

static const int kPreviewLeft = 243;
static const int kPreviewTop = 25 + kSceneOriginY;
static const int kInvalidPreviewLeft = 236;
static const int kInvalidPreviewRight = 530;
static const int kInvalidPreviewTop = kSceneOriginY;
static const int kInvalidPreviewBottom = 240 + kSceneOriginY;

static const int kDragLeft = 102;
static const int kDragTop = 13 + kSceneOriginY;
static const int kDragRight = 526;
static const int kDragBottom = 296 + kSceneOriginY;

static const int kExitWidth = 80;
static const int kExitRightLeft = 560;

static const uint16 kEscapeCommand = 0x1b;
static const uint16 kHelpCommand = 0x3b00;
static const uint16 kScreenshotCommand = 0x4400;

// g_astBoardArrangementRuntimePositions at 0x84b39 stores scene-space Y
// followed by X for each piece. ScummVM normalizes those pairs to physical
// screen points here.
static const Common::Point kInitialPositions[kPieceCount] = {
	Common::Point(103, 13 + kSceneOriginY),
	Common::Point(168, 13 + kSceneOriginY),
	Common::Point(103, 70 + kSceneOriginY),
	Common::Point(168, 70 + kSceneOriginY),
	Common::Point(103, 244 + kSceneOriginY),
	Common::Point(168, 126 + kSceneOriginY),
	Common::Point(103, 189 + kSceneOriginY),
	Common::Point(103, 126 + kSceneOriginY)
};

// g_boardArrangementPuzzleValidationPieceOrder at 0x374b1.
static const uint kValidationOrder[5] = {1, 0, 3, 6, 2};

static RipperEngine *sBoardStateOwner = nullptr;
static Common::Point sBoardPositions[kPieceCount];

} // End of anonymous namespace

BoardArrangementPuzzle::BoardArrangementPuzzle(RipperEngine *engine) :
		_engine(engine), _draggedPiece(-1), _hoveredPiece(-1) {
	for (uint piece = 0; piece < kPieceCount; ++piece) {
		_positions[piece] = kInitialPositions[piece];
		_zOrder[piece] = piece;
	}
}

bool BoardArrangementPuzzle::loadPcx(const Common::String &path,
		BitmapAssetFrame &frame) {
	Common::ScopedPtr<Common::SeekableReadStream> stream(
		_engine->getResources()->createReadStreamForPath(path));
	Image::PCXDecoder decoder;
	if (!stream || !decoder.loadStream(*stream)) {
		warning("Ripper: could not decode board arrangement PCX '%s'", path.c_str());
		return false;
	}

	const Graphics::Surface *surface = decoder.getSurface();
	if (!surface || surface->format.bytesPerPixel != 1 ||
			surface->w <= 0 || surface->h <= 0)
		return false;
	frame.width = surface->w;
	frame.height = surface->h;
	frame.transparentColor = 0;
	frame.pixels.resize((uint32)frame.width * frame.height);
	for (uint y = 0; y < frame.height; ++y)
		memcpy(frame.pixels.data() + y * frame.width,
			surface->getBasePtr(0, y), frame.width);

	const Graphics::Palette &palette = decoder.getPalette();
	frame.palette.resize(palette.size() * 3);
	if (!frame.palette.empty())
		memcpy(frame.palette.data(), palette.data(), frame.palette.size());
	return true;
}

bool BoardArrangementPuzzle::loadPieceSet(const char *prefix,
		Common::Array<BitmapAssetFrame> &frames) {
	frames.clear();
	for (uint piece = 0; piece < kPieceCount; ++piece) {
		BitmapAssetSequence sequence;
		const Common::String path = Common::String::format("%s%u.bbm", prefix, piece);
		if (!_engine->getResources()->loadBitmapSequence(path, sequence) ||
				sequence.frames.empty()) {
			warning("Ripper: could not load board arrangement piece '%s'",
				path.c_str());
			return false;
		}
		frames.push_back(Common::move(sequence.frames.front()));
	}
	return true;
}

bool BoardArrangementPuzzle::loadAssets() {
	if (!loadPieceSet("lg_bb", _largePieces) ||
			!loadPieceSet("sm_bb", _smallPieces) ||
			!loadPcx("bb_templ.pcx", _template))
		return false;

	if (_template.width != 640 || _template.height != 300) {
		warning("Ripper: board arrangement template has invalid size=%ux%u",
			_template.width, _template.height);
		return false;
	}
	for (uint piece = 0; piece < kPieceCount; ++piece) {
		if (_largePieces[piece].width > 273 ||
				_largePieces[piece].height > 203 ||
				_smallPieces[piece].width == 0 ||
				_smallPieces[piece].height == 0)
			return false;
	}

	Common::ScopedPtr<Common::SeekableReadStream> effect(
		_engine->getResources()->createReadStreamForPath("bboard.rtp"));
	if (!effect) {
		warning("Ripper: could not open board arrangement effect 'bboard.rtp'");
		return false;
	}

	debugC(1, kDebugPuzzles,
		"Ripper: loaded board arrangement assets pieces=%u preview=273x203 "
		"template=%ux%u effectBytes=%u audio='bb1.wav'",
		kPieceCount, _template.width, _template.height, (uint)effect->size());
	return true;
}

void BoardArrangementPuzzle::loadPersistentState() {
	if (sBoardStateOwner != _engine) {
		sBoardStateOwner = _engine;
		for (uint piece = 0; piece < kPieceCount; ++piece)
			sBoardPositions[piece] = kInitialPositions[piece];
	}
	for (uint piece = 0; piece < kPieceCount; ++piece)
		_positions[piece] = sBoardPositions[piece];
}

void BoardArrangementPuzzle::storePersistentState() const {
	for (uint piece = 0; piece < kPieceCount; ++piece)
		sBoardPositions[piece] = _positions[piece];
}

void BoardArrangementPuzzle::drawFrame(byte *screen, uint pitch,
		const BitmapAssetFrame &frame, int x, int y) const {
	for (uint sourceY = 0; sourceY < frame.height; ++sourceY) {
		const int destinationY = y + sourceY;
		if (destinationY < 0 || destinationY >= kRipperScreenHeight)
			continue;
		for (uint sourceX = 0; sourceX < frame.width; ++sourceX) {
			const int destinationX = x + sourceX;
			if (destinationX < 0 || destinationX >= kRipperScreenWidth)
				continue;
			const byte pixel = frame.pixels[sourceY * frame.width + sourceX];
			if (pixel != frame.transparentColor)
				screen[destinationY * pitch + destinationX] = pixel;
		}
	}
}

void BoardArrangementPuzzle::render() const {
	if (!_boardDisplay.restorePixels())
		return;

	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1) {
		if (screen)
			g_system->unlockScreen();
		return;
	}
	byte *pixels = (byte *)screen->getPixels();
	const int previewPiece = _draggedPiece >= 0 ? _draggedPiece : _hoveredPiece;
	if (previewPiece >= 0)
		drawFrame(pixels, screen->pitch, _largePieces[previewPiece],
			kPreviewLeft, kPreviewTop);

	for (int zIndex = kPieceCount - 1; zIndex >= 0; --zIndex) {
		const uint piece = _zOrder[zIndex];
		if ((int)piece == _draggedPiece)
			continue;
		drawFrame(pixels, screen->pitch, _smallPieces[piece],
			_positions[piece].x, _positions[piece].y);
	}
	if (_draggedPiece >= 0) {
		drawFrame(pixels, screen->pitch, _smallPieces[_draggedPiece],
			_dragPosition.x, _dragPosition.y);
	}
	g_system->unlockScreen();
	_boardDisplay.restorePalette();
	g_system->updateScreen();
}

int BoardArrangementPuzzle::findPiece(const Common::Point &point,
		int *zIndex) const {
	for (uint index = 0; index < kPieceCount; ++index) {
		const uint piece = _zOrder[index];
		const BitmapAssetFrame &frame = _smallPieces[piece];
		const Common::Rect bounds(_positions[piece].x, _positions[piece].y,
			_positions[piece].x + frame.width,
			_positions[piece].y + frame.height);
		if (!bounds.contains(point))
			continue;
		if (zIndex)
			*zIndex = index;
		return piece;
	}
	return -1;
}

bool BoardArrangementPuzzle::isExitPoint(const Common::Point &point) const {
	return point.y >= kSceneOriginY && point.y < kSceneOriginY + 300 &&
		((point.x >= 0 && point.x < kExitWidth) ||
			(point.x >= kExitRightLeft && point.x < kRipperScreenWidth));
}

bool BoardArrangementPuzzle::updateCursor(const Common::Point &point) {
	int hovered = -1;
	uint cursor = kDefaultCursor;
	if (_draggedPiece >= 0) {
		hovered = _draggedPiece;
		cursor = kPieceCursor;
	} else {
		hovered = findPiece(point);
		if (hovered >= 0)
			cursor = kPieceCursor;
		else if (isExitPoint(point))
			cursor = kExitCursor;
	}
	const bool changed = hovered != _hoveredPiece;
	if (changed) {
		debugC(2, kDebugInput,
			"Ripper: board arrangement hover piece=%d previous=%d point=%d,%d cursor=%u",
			hovered, _hoveredPiece, point.x, point.y, cursor);
		_hoveredPiece = hovered;
	}
	_engine->getCursor()->update(cursor);
	_engine->getCursor()->setVisible(true);
	return changed;
}

void BoardArrangementPuzzle::beginDrag(uint piece, int zIndex) {
	_draggedPiece = piece;
	_dragOrigin = _positions[piece];
	_dragPosition = _positions[piece];
	_hoveredPiece = piece;
	const BitmapAssetFrame &frame = _smallPieces[piece];
	const Common::Point center(_positions[piece].x + frame.width / 2,
		_positions[piece].y + frame.height / 2);
	_engine->getInput()->warpMousePosition(center);
	if (zIndex != 0 &&
			!_engine->getMedia()->playSoundEffect("bb1.wav", _moveAudioHandle))
		warning("Ripper: could not start board arrangement move cue");
	debugC(2, kDebugPuzzles,
		"Ripper: board arrangement drag began piece=%u zIndex=%d "
		"origin=%d,%d pointerCenter=%d,%d",
		piece, zIndex, _dragOrigin.x, _dragOrigin.y, center.x, center.y);
}

void BoardArrangementPuzzle::updateDrag(const Common::Point &point) {
	if (_draggedPiece < 0)
		return;
	const BitmapAssetFrame &frame = _smallPieces[_draggedPiece];
	_dragPosition.x = CLIP<int>(point.x - frame.width / 2,
		kDragLeft, kDragRight - frame.width);
	_dragPosition.y = CLIP<int>(point.y - frame.height / 2,
		kDragTop, kDragBottom - frame.height);
}

bool BoardArrangementPuzzle::isInvalidDrop(const Common::Point &position,
		const BitmapAssetFrame &frame) const {
	return position.y > kInvalidPreviewTop &&
		position.x > kInvalidPreviewLeft &&
		position.y + frame.height < kInvalidPreviewBottom &&
		position.x + frame.width < kInvalidPreviewRight;
}

void BoardArrangementPuzzle::animateInvalidDrop(const Common::Point &target) {
	const Common::Point start = _dragPosition;
	const int dx = target.x - start.x;
	const int dy = target.y - start.y;
	const uint distance = MAX(ABS(dx), ABS(dy));
	const uint steps = MAX<uint>(1,
		(distance + kInvalidDropPixelsPerTick - 1) / kInvalidDropPixelsPerTick);
	for (uint step = 1; step <= steps && !_engine->shouldQuit(); ++step) {
		_dragPosition.x = start.x + dx * (int)step / (int)steps;
		_dragPosition.y = start.y + dy * (int)step / (int)steps;
		render();
		if (step != steps)
			g_system->delayMillis(kDosTickMillis);
	}
	debugC(2, kDebugPuzzles,
		"Ripper: board arrangement rejected preview-window drop piece=%d "
		"from=%d,%d to=%d,%d steps=%u",
		_draggedPiece, start.x, start.y, target.x, target.y, steps);
}

void BoardArrangementPuzzle::bringPieceToFront(uint piece) {
	uint current = 0;
	while (current < kPieceCount && _zOrder[current] != piece)
		++current;
	if (current == 0 || current >= kPieceCount)
		return;
	for (uint index = current; index > 0; --index)
		_zOrder[index] = _zOrder[index - 1];
	_zOrder[0] = piece;
}

void BoardArrangementPuzzle::finishDrag() {
	if (_draggedPiece < 0)
		return;
	const uint piece = _draggedPiece;
	const BitmapAssetFrame &frame = _smallPieces[piece];
	if (isInvalidDrop(_dragPosition, frame))
		animateInvalidDrop(_dragOrigin);
	_positions[piece] = _dragPosition;
	bringPieceToFront(piece);
	_draggedPiece = -1;
	_hoveredPiece = piece;
	debugC(2, kDebugPuzzles,
		"Ripper: board arrangement drag completed piece=%u position=%d,%d",
		piece, _positions[piece].x, _positions[piece].y);
}

bool BoardArrangementPuzzle::isSolved(bool logPairs) const {
	bool solved = true;
	for (uint order = 0; order + 1 < ARRAYSIZE(kValidationOrder); ++order) {
		const uint first = kValidationOrder[order];
		const uint second = kValidationOrder[order + 1];
		const BitmapAssetFrame &firstFrame = _smallPieces[first];
		const BitmapAssetFrame &secondFrame = _smallPieces[second];
		const int minimumSecondTop =
			_positions[first].y + firstFrame.height;
		const int firstRight = _positions[first].x + firstFrame.width;
		const int secondRight = _positions[second].x + secondFrame.width;
		const bool verticalPass =
			_positions[second].y >= minimumSecondTop;
		const bool horizontalPass =
			firstRight >= _positions[second].x &&
			secondRight >= _positions[first].x;
		if (logPairs) {
			debugC(2, kDebugPuzzles,
				"Ripper: board arrangement pair=%u->%u status=%s "
				"vertical=%s nextTop=%d minimumTop=%d horizontal=%s "
				"firstX=%d..%d secondX=%d..%d",
				first, second,
				verticalPass && horizontalPass ? "PASS" : "FAIL",
				verticalPass ? "PASS" : "FAIL",
				_positions[second].y, minimumSecondTop,
				horizontalPass ? "PASS" : "FAIL",
				_positions[first].x, firstRight,
				_positions[second].x, secondRight);
		}
		if (!verticalPass || !horizontalPass)
			solved = false;
	}
	return solved;
}

bool BoardArrangementPuzzle::complete(uint completionFlag) {
	if (_engine->getMilestones()->isSet(completionFlag))
		return true;
	if (!_engine->getMilestones()->set(completionFlag, true,
			"board-arrangement-puzzle"))
		return false;
	debugC(1, kDebugPuzzles,
		"Ripper: solved board arrangement puzzle milestone=%u order=1,0,3,6,2",
		completionFlag);
	if (!_engine->getMedia()->playBlockingAudio("q_p_16.wav"))
		warning("Ripper: board arrangement completion audio failed");
	return true;
}

BoardArrangementPuzzle::Result BoardArrangementPuzzle::run(uint completionFlag) {
	if (!_incomingDisplay.capture() || !loadAssets())
		return kLoadFailed;

	loadPersistentState();
	_engine->getToolbar()->leave();
	_engine->getInput()->drainKeys();
	_engine->getInput()->discardMouseTransitions();
	_engine->getCursor()->setSelectionIndex(kDefaultCursor);
	_engine->getCursor()->dispatchSelectionIndexChange(kDefaultCursor);
	if (!_engine->getMedia()->displayScenePcx("bb_bg.pcx") ||
			!_boardDisplay.capture()) {
		_incomingDisplay.restore();
		return kLoadFailed;
	}
	debugC(1, kDebugPuzzles,
		"Ripper: entered board arrangement puzzle function=RunBoardArrangementPuzzleScene@0x39d8f "
		"milestone=%u help=0x%x dragBounds=[%d,%d,%d,%d] validation=1,0,3,6,2",
		completionFlag, kHelpSelectionTable, kDragLeft, kDragTop,
		kDragRight, kDragBottom);
	debugC(1, kDebugPuzzles,
		"Ripper: board arrangement rules topToBottom=1,0,3,6,2 "
		"nextTop>=previousBottom horizontal=touch-or-overlap "
		"maximumVerticalGap=none ignoredPieces=4,5,7");

	Result result = _engine->getMilestones()->isSet(completionFlag) ?
		kSolved : kExited;
	render();
	bool active = true;
	while (active && !_engine->shouldQuit()) {
		if (_engine->getInput()->pollEvents()) {
			_engine->quitGame();
			break;
		}

		if (_draggedPiece < 0) {
			while (_engine->getInput()->hasPendingKey()) {
				const uint16 command = _engine->getInput()->consumeKey();
				if (command == kEscapeCommand) {
					active = false;
					break;
				}
				if (command == kHelpCommand) {
					debugC(1, kDebugPuzzles,
						"Ripper: board arrangement opening modal help table=0x%x",
						kHelpSelectionTable);
					if (!_engine->getModalDialog()->run(kHelpSelectionTable))
						warning("Ripper: board arrangement modal help failed");
					render();
					continue;
				}
				if (command == kScreenshotCommand || command == 0x10)
					g_system->saveScreenshot();
			}
		}
		if (!active)
			break;

		const MouseState mouse = _engine->getInput()->publishMouseState();
		bool redraw = updateCursor(mouse.position);
		if (_draggedPiece >= 0) {
			updateDrag(mouse.position);
			redraw = true;
			if ((mouse.released & kMouseButtonLeft) != 0 ||
					(mouse.buttons & kMouseButtonLeft) == 0) {
				const int droppedPiece = _draggedPiece;
				finishDrag();
				const bool arrangementSolved = isSolved(true);
				if (arrangementSolved) {
					result = complete(completionFlag) ? kSolved : kLoadFailed;
					if (result == kLoadFailed)
						active = false;
				}
				debugC(arrangementSolved ? 1 : 2, kDebugPuzzles,
					"Ripper: board arrangement validation after drop piece=%d "
					"outcome=%s milestone=%u milestoneSet=%d",
					droppedPiece, arrangementSolved ? "SOLVED" : "NOT_SOLVED",
					completionFlag,
					_engine->getMilestones()->isSet(completionFlag));
				redraw = true;
			}
		} else if ((mouse.pressed & kMouseButtonLeft) != 0) {
			int zIndex = -1;
			const int piece = findPiece(mouse.position, &zIndex);
			if (piece >= 0) {
				beginDrag(piece, zIndex);
				redraw = true;
			} else if (isExitPoint(mouse.position)) {
				active = false;
			}
		}
		if (redraw)
			render();
		else {
			// RunBoardArrangementPuzzleScene at 0x39d8f services
			// ServiceUiControlStateSelection on every idle loop while the
			// selection presentation is active. Submit unchanged puzzle frames
			// too so ScummVM's software cursor tracks those input ticks.
			g_system->updateScreen();
		}
		g_system->delayMillis(10);
	}

	storePersistentState();
	_engine->getMedia()->stopSoundEffect(_moveAudioHandle);
	_incomingDisplay.restore();
	_engine->getCursor()->setSelectionIndex(0);
	_engine->getCursor()->dispatchSelectionIndexChange(0);
	_engine->getCursor()->refresh();
	_engine->getCursor()->setVisible(true);
	_engine->getInput()->drainKeys();
	_engine->getInput()->discardMouseTransitions();
	const bool arrangementSolved = isSolved(false);
	debugC(result == kLoadFailed ? 2 : 1, kDebugPuzzles,
		"Ripper: left board arrangement puzzle result=%d "
		"arrangementSolved=%d milestone=%u milestoneSet=%d quit=%d",
		result, arrangementSolved, completionFlag,
		_engine->getMilestones()->isSet(completionFlag),
		_engine->shouldQuit());
	return result;
}

} // End of namespace Ripper
