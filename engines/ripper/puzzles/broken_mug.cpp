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

#include "ripper/puzzles/broken_mug.h"

#include "common/archive.h"
#include "common/debug.h"
#include "common/system.h"
#include "common/util.h"
#include "graphics/paletteman.h"
#include "graphics/surface.h"
#include "video/smk_decoder.h"

#include "ripper/cursor.h"
#include "ripper/detection.h"
#include "ripper/input.h"
#include "ripper/media.h"
#include "ripper/ripper.h"
#include "ripper/script.h"
#include "ripper/wac.h"

namespace Ripper {

namespace {

static const uint kPieceCount = 9;
static const uint kPieceFrameCount = 32;
static const uint kFramesPerOrientation = 8;
static const uint kInitialOrientation = 2;
static const uint kPuzzleCursor = 16;
static const uint16 kDosF10Command = 0x4400;
static const uint32 kRotationFrameMillis = 55;
static const int kViewportLeft = 50;
static const int kViewportTop = 50;
static const int kViewportRight = 332;
static const int kViewportBottom = 400;
static const byte kViewportFillColor = 4;
static const uint kPalettePatchFirst = 10;
static const uint kPalettePatchCount = 140;
static const int kSolvedTolerance = 5;
static const uint kBrokenMugAvailableFlag = 0x47;
static const uint kBrokenMugCompletedFlag = 0x48;
static const int kCompletionX = 111;
static const int kCompletionY = 143;

static const int kInitialX[kPieceCount] = {
	122, 220, 216, 61, 54, 114, 160, 207, 230
};

static const int kInitialY[kPieceCount] = {
	258, 260, 94, 94, 254, 164, 123, 228, 155
};

static const int kSolvedDeltaX[kPieceCount] = {
	0, -6, 31, -2, 68, 5, 31, 64, 65
};

static const int kSolvedDeltaY[kPieceCount] = {
	0, -36, -2, 15, 16, -24, -2, 4, 55
};

} // End of anonymous namespace

BrokenMugPuzzle::BrokenMugPuzzle(RipperEngine *engine) : _engine(engine),
		_draggedPiece(-1), _rotationStep(-1), _nextRotationMillis(0),
		_releasePending(false) {
}

bool BrokenMugPuzzle::loadPiece(uint pieceIndex) {
	const Common::String name = Common::String::format("mug%u.smk", pieceIndex);
	Common::SeekableReadStream *stream =
		SearchMan.createReadStreamForMember(Common::Path(name));
	if (!stream) {
		warning("Ripper: unable to open broken mug asset '%s'", name.c_str());
		return false;
	}

	Video::SmackerDecoder decoder;
	if (!decoder.loadStream(stream) || decoder.getFrameCount() < kPieceFrameCount ||
		decoder.getPixelFormat().bytesPerPixel != 1) {
		warning("Ripper: invalid broken mug Smacker '%s'", name.c_str());
		return false;
	}

	Piece &piece = _pieces[pieceIndex];
	piece.frames.clear();
	piece.frames.reserve(kPieceFrameCount);
	Common::Array<byte> palette;
	for (uint frameIndex = 0; frameIndex < kPieceFrameCount; ++frameIndex) {
		const Graphics::Surface *surface = decoder.decodeNextFrame();
		if (!surface)
			return false;
		if (decoder.hasDirtyPalette() || palette.empty()) {
			palette.resize(256 * 3);
			memcpy(palette.data(), decoder.getPalette(), palette.size());
		}

		Frame frame;
		frame.width = surface->w;
		frame.height = surface->h;
		frame.pixels.resize(frame.width * frame.height);
		for (uint y = 0; y < frame.height; ++y)
			memcpy(frame.pixels.data() + y * frame.width,
				surface->getBasePtr(0, y), frame.width);
		frame.palette = palette;
		piece.frames.push_back(Common::move(frame));
	}

	piece.position = Common::Point(kInitialX[pieceIndex], kInitialY[pieceIndex]);
	piece.orientation = kInitialOrientation;
	debugC(2, kDebugWac,
		"Ripper: loaded broken mug piece=%u control=0x%x frames=%u size=%ux%u position=%d,%d orientation=%u",
		pieceIndex, 0x640 + pieceIndex, piece.frames.size(), piece.frames[0].width,
		piece.frames[0].height, piece.position.x, piece.position.y, piece.orientation);
	return true;
}

bool BrokenMugPuzzle::loadPieces() {
	_frontToBack.clear();
	for (uint pieceIndex = 0; pieceIndex < kPieceCount; ++pieceIndex) {
		if (!loadPiece(pieceIndex))
			return false;
		_frontToBack.push_back(pieceIndex);
	}
	_activePuzzlePalette = currentFrame(_pieces[kPieceCount - 1]).palette;
	return true;
}

bool BrokenMugPuzzle::captureBackground() {
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1 || screen->w != 640 || screen->h != 400) {
		if (screen)
			g_system->unlockScreen();
		return false;
	}

	_backgroundPixels.resize(screen->w * screen->h);
	for (int y = 0; y < screen->h; ++y)
		memcpy(_backgroundPixels.data() + y * screen->w, screen->getBasePtr(0, y), screen->w);
	g_system->unlockScreen();
	_backgroundPalette.resize(256 * 3);
	g_system->getPaletteManager()->grabPalette(_backgroundPalette.data(), 0, 256);
	return true;
}

void BrokenMugPuzzle::restoreBackground() const {
	if (_backgroundPixels.size() != 640 * 400 || _backgroundPalette.size() != 256 * 3)
		return;
	g_system->copyRectToScreen(_backgroundPixels.data(), 640, 0, 0, 640, 400);
	g_system->getPaletteManager()->setPalette(_backgroundPalette.data(), 0, 256);
	g_system->updateScreen();
}

const BrokenMugPuzzle::Frame &BrokenMugPuzzle::currentFrame(const Piece &piece) const {
	uint frameIndex;
	if (_rotationStep >= 0 && _draggedPiece >= 0 && &piece == &_pieces[_draggedPiece])
		frameIndex = (piece.orientation * kFramesPerOrientation + _rotationStep) % kPieceFrameCount;
	else
		frameIndex = (piece.orientation * kFramesPerOrientation + kPieceFrameCount - 1) %
			kPieceFrameCount;
	return piece.frames[frameIndex];
}

void BrokenMugPuzzle::drawPiece(byte *screen, uint pitch, const Piece &piece,
		const Frame &frame) const {
	for (uint sourceY = 0; sourceY < frame.height; ++sourceY) {
		const int destinationY = piece.position.y + sourceY;
		if (destinationY < 0 || destinationY >= 400)
			continue;
		for (uint sourceX = 0; sourceX < frame.width; ++sourceX) {
			const int destinationX = piece.position.x + sourceX;
			if (destinationX < 0 || destinationX >= 640)
				continue;
			const byte pixel = frame.pixels[sourceY * frame.width + sourceX];
			if (pixel != 0)
				screen[destinationY * pitch + destinationX] = pixel;
		}
	}
}

void BrokenMugPuzzle::render() {
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1) {
		if (screen)
			g_system->unlockScreen();
		return;
	}

	for (int y = 0; y < screen->h; ++y)
		memcpy(screen->getBasePtr(0, y), _backgroundPixels.data() + y * screen->w, screen->w);
	for (int y = kViewportTop; y < kViewportBottom; ++y)
		memset(screen->getBasePtr(kViewportLeft, y), kViewportFillColor,
			kViewportRight - kViewportLeft);
	for (int order = _frontToBack.size() - 1; order >= 0; --order) {
		const Piece &piece = _pieces[_frontToBack[order]];
		drawPiece((byte *)screen->getPixels(), screen->pitch, piece, currentFrame(piece));
	}
	g_system->unlockScreen();

	Common::Array<byte> palette = _backgroundPalette;
	if (_activePuzzlePalette.size() == 256 * 3) {
		memcpy(palette.data() + kPalettePatchFirst * 3,
			_activePuzzlePalette.data() + kPalettePatchFirst * 3,
			kPalettePatchCount * 3);
	}
	g_system->getPaletteManager()->setPalette(palette.data(), 0, 256);
	g_system->updateScreen();
}

int BrokenMugPuzzle::findPiece(const Common::Point &point) const {
	for (uint order = 0; order < _frontToBack.size(); ++order) {
		const uint pieceIndex = _frontToBack[order];
		const Piece &piece = _pieces[pieceIndex];
		const Frame &frame = currentFrame(piece);
		const int localX = point.x - piece.position.x;
		const int localY = point.y - piece.position.y;
		if (localX >= 0 && localY >= 0 && localX < frame.width && localY < frame.height &&
			frame.pixels[localY * frame.width + localX] != 0)
			return pieceIndex;
	}
	return -1;
}

void BrokenMugPuzzle::beginDrag(uint pieceIndex, const Common::Point &point) {
	_draggedPiece = pieceIndex;
	_dragOffset = point - _pieces[pieceIndex].position;
	_releasePending = false;
	for (uint order = 0; order < _frontToBack.size(); ++order) {
		if (_frontToBack[order] == pieceIndex) {
			_frontToBack.remove_at(order);
			break;
		}
	}
	_frontToBack.insert_at(0, pieceIndex);
	debugC(2, kDebugWac,
		"Ripper: broken mug drag began piece=%u control=0x%x point=%d,%d offset=%d,%d",
		pieceIndex, 0x640 + pieceIndex, point.x, point.y, _dragOffset.x, _dragOffset.y);
}

void BrokenMugPuzzle::updateDrag(const Common::Point &point) {
	const int pointerX = CLIP<int>(point.x, kViewportLeft, kViewportRight - 1);
	const int pointerY = CLIP<int>(point.y, kViewportTop, kViewportBottom - 1);
	_pieces[_draggedPiece].position = Common::Point(pointerX - _dragOffset.x,
		pointerY - _dragOffset.y);
}

void BrokenMugPuzzle::beginRotation() {
	_rotationStep = 0;
	_nextRotationMillis = g_system->getMillis(true) + kRotationFrameMillis;
	debugC(2, kDebugWac,
		"Ripper: broken mug rotation began piece=%d orientation=%u",
		_draggedPiece, _pieces[_draggedPiece].orientation);
}

bool BrokenMugPuzzle::updateRotation(uint32 now) {
	if (_rotationStep < 0 || now < _nextRotationMillis)
		return false;
	_activePuzzlePalette = currentFrame(_pieces[_draggedPiece]).palette;
	++_rotationStep;
	_nextRotationMillis = now + kRotationFrameMillis;
	if (_rotationStep >= (int)kFramesPerOrientation) {
		Piece &piece = _pieces[_draggedPiece];
		piece.orientation = (piece.orientation + 1) % 4;
		_rotationStep = -1;
		_activePuzzlePalette = currentFrame(piece).palette;
		debugC(2, kDebugWac,
			"Ripper: broken mug rotation completed piece=%d orientation=%u",
			_draggedPiece, piece.orientation);
	}
	return true;
}

void BrokenMugPuzzle::finishDrag() {
	const Piece &piece = _pieces[_draggedPiece];
	debugC(2, kDebugWac,
		"Ripper: broken mug drag completed piece=%d position=%d,%d orientation=%u",
		_draggedPiece, piece.position.x, piece.position.y, piece.orientation);
	_draggedPiece = -1;
	_releasePending = false;
}

bool BrokenMugPuzzle::isSolved() const {
	for (uint pieceIndex = 0; pieceIndex < kPieceCount; ++pieceIndex) {
		if (_pieces[pieceIndex].orientation != 0)
			return false;
	}
	const Common::Point anchor = _pieces[0].position;
	for (uint pieceIndex = 1; pieceIndex < kPieceCount; ++pieceIndex) {
		const int deltaX = _pieces[pieceIndex].position.x - anchor.x;
		const int deltaY = _pieces[pieceIndex].position.y - anchor.y;
		if (ABS(deltaX - kSolvedDeltaX[pieceIndex]) > kSolvedTolerance ||
			ABS(deltaY - kSolvedDeltaY[pieceIndex]) > kSolvedTolerance)
			return false;
	}
	return true;
}

bool BrokenMugPuzzle::playCompletionMedia(RipperEngine *engine) {
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1) {
		if (screen)
			g_system->unlockScreen();
		return false;
	}
	for (int y = kViewportTop; y < kViewportBottom; ++y)
		memset(screen->getBasePtr(kViewportLeft, y), kViewportFillColor,
			kViewportRight - kViewportLeft);
	g_system->unlockScreen();
	g_system->updateScreen();

	debugC(1, kDebugWac,
		"Ripper: playing completed mug presentation media='mug9.smk' audio='q_p_2.wav' position=%d,%d",
		kCompletionX, kCompletionY);
	if (!engine->getMedia()->playWacMedia("mug9.smk", kCompletionX, kCompletionY))
		return false;
	if (!engine->getMedia()->playBlockingAudio("q_p_2.wav"))
		warning("Ripper: broken mug completion audio failed; retaining completed presentation");
	return true;
}

bool BrokenMugPuzzle::completePuzzle() {
	if (!playCompletionMedia(_engine)) {
		warning("Ripper: broken mug completion media failed; flags unchanged");
		return false;
	}
	_engine->getScripts()->setMilestoneFlag(kBrokenMugAvailableFlag, "wac-broken-mug");
	_engine->getScripts()->setMilestoneFlag(kBrokenMugCompletedFlag, "wac-broken-mug");
	debugC(1, kDebugWac,
		"Ripper: completed broken mug puzzle availabilityFlag=0x%x completionFlag=0x%x",
		kBrokenMugAvailableFlag, kBrokenMugCompletedFlag);
	return true;
}

BrokenMugPuzzle::Result BrokenMugPuzzle::run() {
	if (!captureBackground() || !loadPieces()) {
		restoreBackground();
		return kLoadFailed;
	}

	debugC(1, kDebugWac,
		"Ripper: entered broken mug puzzle pieces=%u viewport=%d,%d,%d,%d initialOrientation=%u",
		kPieceCount, kViewportLeft, kViewportTop, kViewportRight - kViewportLeft,
		kViewportBottom - kViewportTop, kInitialOrientation);
	_engine->getInput()->discardMouseTransitions();
	_engine->getCursor()->update(kPuzzleCursor);
	render();
	if (!_engine->getMedia()->playBlockingAudio("q_p_1.wav"))
		warning("Ripper: broken mug introduction audio failed; continuing puzzle input");

	Result result = kExited;
	bool active = true;
	while (active && !_engine->shouldQuit()) {
		if (_engine->getInput()->pollEvents()) {
			_engine->quitGame();
			break;
		}
		while (_engine->getInput()->hasPendingKey()) {
			const uint16 command = _engine->getInput()->consumeKey();
			if (command == 0x1b || command == kDosF10Command) {
				debugC(1, kDebugWac,
					"Ripper: broken mug puzzle exited command=0x%x", command);
				active = false;
			}
		}

		const MouseState mouse = _engine->getInput()->publishMouseState();
		_engine->getCursor()->update(kPuzzleCursor);
		bool changed = false;
		if (_draggedPiece < 0) {
			if ((mouse.pressed & kMouseButtonLeft) != 0) {
				const int pieceIndex = findPiece(mouse.position);
				if (pieceIndex >= 0) {
					beginDrag(pieceIndex, mouse.position);
					changed = true;
				}
			}
		} else {
			updateDrag(mouse.position);
			changed = true;
			if ((mouse.pressed & kMouseButtonRight) != 0 && _rotationStep < 0)
				beginRotation();
			if ((mouse.released & kMouseButtonLeft) != 0)
				_releasePending = true;
			if (updateRotation(g_system->getMillis(true)))
				changed = true;
			if (_releasePending && _rotationStep < 0) {
				finishDrag();
				if (isSolved()) {
					debugC(1, kDebugWac, "Ripper: broken mug layout solved");
					result = completePuzzle() ? kSolved : kLoadFailed;
					active = false;
				}
			}
		}
		if (changed)
			render();
		_engine->getWac()->serviceIdleEffects();
		g_system->delayMillis(10);
	}

	restoreBackground();
	return result;
}

} // End of namespace Ripper
