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

#include "ripper/puzzles/board_game.h"

#include "common/debug.h"
#include "common/ptr.h"
#include "common/stream.h"
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
#include "ripper/settings.h"
#include "ripper/toolbar.h"

namespace Ripper {

namespace {

static const char *const kLibraryName = "chess.gl";
static const char *const kBackgroundName = "board";
static const char *const kHitMapName = "boardtml";
static const char *const kSelectionShadingName = "boardpal";
static const uint kDefaultSelectionIndex = 0x13;
static const uint kDefaultCursor = 14;
static const uint kSelectionCursor = 16;
static const uint kExitCursor = 7;
static const uint kHelpSelectionTable = 0x1b4;
static const uint16 kEscapeCommand = 0x1b;
static const uint16 kHelpCommand = 0x3b00;
static const uint16 kScreenshotCommand = 0x4400;
static const byte kExitHitCode = 0x81;
static const char kCompletionKeyword[] = "aspirin";
static const uint kAudioCueCount = 5;
static const uint kTransformCue = 0;
static const uint kRemovalCue = 1;
static const uint kLongMoveCue = 2;
static const uint kShortMoveCue = 3;
static const uint kSelectionCue = 4;
static const uint kAnimationTickMillis = 10;
static const uint kShortMoveTicks = 100;
static const uint kLongMoveTicks = 200;
static const uint kRemovalLeadTicks = 50;
static const uint kShadingPaletteBytes = 0x300;
static const uint kShadingColorCubeBytes = 0x1000;
static const uint kShadingInfoBytes =
		kShadingPaletteBytes + kShadingColorCubeBytes;
static const uint kSelectedPieceEffectStrength = 0xa0;
static const uint kEffectStrengthRange = 0x100;
static const uint kPuzzleHelpSearchDepth = 3;
static const byte kPuzzleHelpLegalColor = 254;
static const byte kPuzzleHelpOptimalColor = 255;
static const uint kPuzzleHelpLegalDitherMask = 3;
static const uint kPuzzleHelpOptimalDitherMask = 1;

// The table at 0x40ab8 stores scene Y followed by X. These normalized
// physical points are the piece anchors used by RenderBoardState at 0x418c8.
static const Common::Point kCellAnchors[BoardGameModel::kCellCount] = {
	Common::Point(41, 220), Common::Point(95, 200),
	Common::Point(148, 181), Common::Point(199, 164),
	Common::Point(247, 145), Common::Point(294, 129),
	Common::Point(83, 240), Common::Point(140, 221),
	Common::Point(191, 200), Common::Point(243, 181),
	Common::Point(292, 164), Common::Point(338, 146),
	Common::Point(130, 261), Common::Point(184, 242),
	Common::Point(239, 221), Common::Point(294, 201),
	Common::Point(339, 183), Common::Point(386, 165),
	Common::Point(178, 286), Common::Point(234, 264),
	Common::Point(287, 242), Common::Point(338, 223),
	Common::Point(386, 203), Common::Point(437, 183),
	Common::Point(231, 312), Common::Point(287, 288),
	Common::Point(339, 266), Common::Point(389, 244),
	Common::Point(439, 223), Common::Point(485, 202),
	Common::Point(280, 334), Common::Point(337, 311),
	Common::Point(391, 288), Common::Point(441, 266),
	Common::Point(490, 244), Common::Point(537, 224),
	Common::Point(336, 361), Common::Point(393, 335),
	Common::Point(446, 312), Common::Point(495, 288),
	Common::Point(545, 266), Common::Point(591, 245)
};

// The piece-origin records at 0x84f14 are also stored Y/X. The first row is
// PART_A (positive), followed by PART_B (negative), with types one through
// five in each row.
static const Common::Point kPieceOrigins[2][5] = {
	{
		Common::Point(55, 123), Common::Point(76, 132),
		Common::Point(34, 109), Common::Point(35, 130),
		Common::Point(72, 186)
	},
	{
		Common::Point(30, 135), Common::Point(33, 98),
		Common::Point(85, 134), Common::Point(26, 121),
		Common::Point(39, 134)
	}
};

// RenderBoardState at 0x418c8 uses this fixed depth order from 0x40c08.
static const byte kRenderOrder[BoardGameModel::kCellCount] = {
	5, 4, 11, 3, 10, 17, 2, 9, 16, 23, 1, 8, 15, 22,
	29, 0, 7, 14, 21, 28, 35, 6, 13, 20, 27, 34, 41, 12,
	19, 26, 33, 40, 18, 25, 32, 39, 24, 31, 38, 30, 37, 36
};

static bool containsDestination(const Common::Array<int> &destinations,
		int destination) {
	for (uint i = 0; i < destinations.size(); ++i) {
		if (destinations[i] == destination)
			return true;
	}
	return false;
}

static Common::String moveString(const BoardGameModel::Move &move) {
	return Common::String::format("%d..%d", move.source, move.destination);
}

} // End of anonymous namespace

BoardGamePuzzle::DebugHelper::DebugHelper() :
		_enabled(false), _optimalDestination(-1) {
}

void BoardGamePuzzle::DebugHelper::reset(bool enabled) {
	_enabled = enabled;
	_optimalDestination = -1;
}

bool BoardGamePuzzle::DebugHelper::sync(const BoardGamePuzzle &puzzle) {
	const bool enabled = puzzle._engine->isPuzzleHelpEnabled();
	if (enabled == _enabled)
		return false;

	_enabled = enabled;
	selectionChanged(puzzle);
	debugC(2, kDebugPuzzles,
		"Ripper: board-game puzzle help enabled=%d source=%d legalDestinations=%u optimalDestination=%d depth=%u",
		_enabled, puzzle._selectedCell, puzzle._legalDestinations.size(),
		_optimalDestination, kPuzzleHelpSearchDepth);
	return true;
}

void BoardGamePuzzle::DebugHelper::selectionChanged(
		const BoardGamePuzzle &puzzle) {
	_optimalDestination = -1;
	if (!_enabled || puzzle._selectedCell < 0 ||
			puzzle._legalDestinations.empty())
		return;

	_optimalDestination = chooseOptimalDestination(puzzle);
	debugC(2, kDebugPuzzles,
		"Ripper: board-game puzzle help analyzed source=%d legalDestinations=%u optimalDestination=%d depth=%u legalColor=%u optimalColor=%u",
		puzzle._selectedCell, puzzle._legalDestinations.size(),
		_optimalDestination, kPuzzleHelpSearchDepth,
		kPuzzleHelpLegalColor, kPuzzleHelpOptimalColor);
}

int BoardGamePuzzle::DebugHelper::chooseOptimalDestination(
		const BoardGamePuzzle &puzzle) const {
	int bestDestination = -1;
	int bestScore = 0x7fffffff;
	for (uint destinationIndex = 0;
			destinationIndex < puzzle._legalDestinations.size();
			++destinationIndex) {
		const int destination =
			puzzle._legalDestinations[destinationIndex];
		BoardGameModel next = puzzle._model;
		const BoardGameModel::Move move(puzzle._selectedCell, destination);
		if (!next.applyMove(move))
			continue;
		const int score = puzzle.evaluatePosition(next,
			kPuzzleHelpSearchDepth - 1, -0x7fffffff, 0x7fffffff);
		debugC(3, kDebugPuzzles,
			"Ripper: board-game puzzle help candidate move=%s score=%d depth=%u",
			moveString(move).c_str(), score, kPuzzleHelpSearchDepth);
		// Positive pieces are the player side. The shared evaluator scores
		// negative positions positively, so the best player move minimizes.
		if (bestDestination < 0 || score < bestScore) {
			bestDestination = destination;
			bestScore = score;
		}
	}
	return bestDestination;
}

void BoardGamePuzzle::DebugHelper::draw(const BoardGamePuzzle &puzzle,
		byte *screen, uint pitch) const {
	if (!_enabled || puzzle._selectedCell < 0)
		return;

	bool highlighted[BoardGameModel::kOffBoardDestination + 1] = { false };
	for (uint destinationIndex = 0;
			destinationIndex < puzzle._legalDestinations.size();
			++destinationIndex) {
		const int destination =
			puzzle._legalDestinations[destinationIndex];
		if (destination >= 0 &&
				destination <= BoardGameModel::kOffBoardDestination)
			highlighted[destination] = true;
	}

	for (uint y = 0; y < puzzle._hitMap.height; ++y) {
		for (uint x = 0; x < puzzle._hitMap.width; ++x) {
			const byte code =
				puzzle._hitMap.pixels[y * puzzle._hitMap.width + x];
			if (code > BoardGameModel::kOffBoardDestination ||
					!highlighted[code])
				continue;
			const bool optimal = code == _optimalDestination;
			const uint ditherMask = optimal ? kPuzzleHelpOptimalDitherMask :
				kPuzzleHelpLegalDitherMask;
			if (((x + y) & ditherMask) != 0)
				continue;
			// ApplySharedDisplayPalettePatch at 0x205d0 guarantees the
			// interface red and yellow entries in every presentation palette.
			// Spatial dithering preserves the board without relying on
			// BOARDPAL, whose limited colors collapse arbitrary RGB tints.
			screen[y * pitch + x] = optimal ? kPuzzleHelpOptimalColor :
				kPuzzleHelpLegalColor;
		}
	}
}

BoardGamePuzzle::BoardGamePuzzle(RipperEngine *engine) :
		_engine(engine), _random("ripper-board-game"), _movingAnchor(0, 0),
		_selectedCell(-1), _hoveredCode(-1), _movingSource(-1), _movingPiece(0),
		_keywordIndex(0), _searchDepth(1), _savedSelectionIndex(0),
		_savedCursorVisible(true), _movingActive(false) {
}

bool BoardGamePuzzle::loadBitmap(const Common::String &name,
		BitmapAssetFrame &frame) {
	Common::ScopedPtr<Common::SeekableReadStream> stream(
		_library.createReadStreamForMember(name));
	if (!stream || !decodeBitmapAsset(*stream, frame)) {
		warning("Ripper: could not decode board-game bitmap '%s'", name.c_str());
		return false;
	}
	return true;
}

bool BoardGamePuzzle::loadSelectionShading() {
	Common::ScopedPtr<Common::SeekableReadStream> stream(
		_library.createReadStreamForMember(kSelectionShadingName));
	if (!stream || stream->size() != kShadingInfoBytes) {
		warning("Ripper: board-game shading '%s' has invalid size expected=%u actual=%lld",
			kSelectionShadingName, kShadingInfoBytes,
			stream ? (long long)stream->size() : -1LL);
		return false;
	}

	_selectionShading.resize(kShadingInfoBytes);
	if (stream->read(_selectionShading.data(), kShadingInfoBytes) !=
			kShadingInfoBytes) {
		warning("Ripper: could not read board-game shading '%s'",
			kSelectionShadingName);
		_selectionShading.clear();
		return false;
	}
	return true;
}

bool BoardGamePuzzle::loadAssets() {
	Common::ScopedPtr<Common::SeekableReadStream> stream(
		_engine->getResources()->createReadStreamForPath(kLibraryName));
	if (!stream || !_library.open(*stream, Common::Path(kLibraryName))) {
		warning("Ripper: could not open board-game library '%s'", kLibraryName);
		return false;
	}
	if (!loadBitmap(kBackgroundName, _background) ||
			!loadBitmap(kHitMapName, _hitMap) ||
			!loadSelectionShading())
		return false;
	if (_background.width != kRipperScreenWidth ||
			_background.height != kRipperScreenHeight ||
			_hitMap.width != kRipperScreenWidth ||
			_hitMap.height != kRipperScreenHeight ||
			_background.palette.size() < kRipperPaletteByteCount) {
		warning("Ripper: board-game assets have invalid geometry board=%ux%u hitMap=%ux%u colors=%u",
			_background.width, _background.height, _hitMap.width, _hitMap.height,
			_background.palette.size() / 3);
		return false;
	}
	for (uint side = 0; side < 2; ++side) {
		for (uint type = 0; type < 5; ++type) {
			const Common::String name = Common::String::format(
				"part_%c%02u", side == 0 ? 'a' : 'b', type);
			if (!loadBitmap(name, _pieces[side][type]))
				return false;
		}
	}

	debugC(1, kDebugPuzzles,
		"Ripper: loaded board-game assets library='%s' entries=%u board=%ux%u "
		"hitMap=%ux%u effect='%s' opacity=0x%x pieces=10 audio=CHESS0..CHESS4",
		kLibraryName, _library.getEntryCount(), _background.width,
		_background.height, _hitMap.width, _hitMap.height,
		kSelectionShadingName, kSelectedPieceEffectStrength);
	return true;
}

void BoardGamePuzzle::applyPalette() {
	Common::Array<byte> palette = _background.palette;
	_engine->applySharedPalettePatch(palette.data(),
		kRipperPaletteColorCount);
	_engine->getSettings()->applyVideoPalette(palette.data(),
		kRipperPaletteColorCount, true);
	g_system->getPaletteManager()->setPalette(palette.data(), 0,
		kRipperPaletteColorCount);
}

void BoardGamePuzzle::drawFrame(byte *screen, uint pitch,
		const BitmapAssetFrame &frame, int x, int y,
		bool translucent) const {
	for (uint sourceY = 0; sourceY < frame.height; ++sourceY) {
		const int destinationY = y + sourceY;
		if (destinationY < 0 || destinationY >= kRipperScreenHeight)
			continue;
		for (uint sourceX = 0; sourceX < frame.width; ++sourceX) {
			const int destinationX = x + sourceX;
			if (destinationX < 0 || destinationX >= kRipperScreenWidth)
				continue;
			const byte pixel = frame.pixels[sourceY * frame.width + sourceX];
			if (pixel != frame.transparentColor) {
				byte &destination = screen[destinationY * pitch + destinationX];
				destination = translucent ?
					blendSelectionPixel(pixel, destination) : pixel;
			}
		}
	}
}

byte BoardGamePuzzle::blendSelectionPixel(byte source,
		byte destination) const {
	// BOARDPAL's 0x1000-byte tail maps 12-bit RGB (rrrrggggbbbb) back
	// into its 256-color palette. The strength passed at 0x41876 grows
	// from 0 toward 0xff during AnimateBoardIntroFadeIn at 0x4365c, so
	// it is source opacity rather than a shade-table index.
	const byte *palette = _selectionShading.data();
	const byte *colorCube = palette + kShadingPaletteBytes;
	const uint inverseStrength =
		kEffectStrengthRange - kSelectedPieceEffectStrength;
	uint components[3];
	for (uint component = 0; component < ARRAYSIZE(components); ++component) {
		components[component] =
			(palette[source * 3 + component] * kSelectedPieceEffectStrength +
			 palette[destination * 3 + component] * inverseStrength) >> 8;
	}
	const uint colorCubeIndex = ((components[0] >> 2) << 8) |
		((components[1] >> 2) << 4) | (components[2] >> 2);
	return colorCube[colorCubeIndex];
}

void BoardGamePuzzle::drawPiece(byte *screen, uint pitch, int piece,
		const Common::Point &anchor, bool selected) const {
	if (piece == 0)
		return;
	const uint side = piece > 0 ? 0 : 1;
	const uint type = ABS(piece) - 1;
	const Common::Point position = anchor - kPieceOrigins[side][type];
	// RenderBoardPieceVisual at 0x4186c attaches BOARDPAL with strength 0xa0
	// when the cell matches the selected-source global at 0x84f68. The asset
	// stores a 0x300-byte palette followed by a 12-bit RGB color cube used
	// to quantize the source/background blend back into the board palette.
	drawFrame(screen, pitch, _pieces[side][type], position.x, position.y,
		selected);
}

void BoardGamePuzzle::render() {
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1 ||
			screen->w != kRipperScreenWidth || screen->h != kRipperScreenHeight) {
		if (screen)
			g_system->unlockScreen();
		return;
	}
	for (int y = 0; y < kRipperScreenHeight; ++y) {
		memcpy(screen->getBasePtr(0, y),
			_background.pixels.data() + y * kRipperScreenWidth,
			kRipperScreenWidth);
	}
	byte *pixels = (byte *)screen->getPixels();
	_debugHelper.draw(*this, pixels, screen->pitch);
	bool movingPiecePending = _movingActive;
	for (uint order = 0; order < ARRAYSIZE(kRenderOrder); ++order) {
		const int cell = kRenderOrder[order];
		const Common::Point &anchor = kCellAnchors[cell];
		if (movingPiecePending &&
				(_movingAnchor.y < anchor.y ||
				 (_movingAnchor.y == anchor.y && _movingAnchor.x < anchor.x))) {
			drawPiece(pixels, screen->pitch, _movingPiece, _movingAnchor);
			movingPiecePending = false;
		}
		if (_movingActive && cell == _movingSource)
			continue;
		const int piece = _model.pieceAt(cell);
		drawPiece(pixels, screen->pitch, piece, anchor,
			cell == _selectedCell);
	}
	if (movingPiecePending)
		drawPiece(pixels, screen->pitch, _movingPiece, _movingAnchor);
	g_system->unlockScreen();
	applyPalette();
	_engine->getCursor()->setVisible(true);
	presentScreen();
}

void BoardGamePuzzle::clearSelectedPiece() {
	_selectedCell = -1;
	_legalDestinations.clear();
	_debugHelper.selectionChanged(*this);
}

int BoardGamePuzzle::hitCodeAt(const Common::Point &point) const {
	if (point.x < 0 || point.x >= _hitMap.width || point.y < 0 ||
			point.y >= _hitMap.height)
		return -1;
	return _hitMap.pixels[point.y * _hitMap.width + point.x];
}

void BoardGamePuzzle::updateCursor(const Common::Point &point) {
	const int hitCode = hitCodeAt(point);
	uint cursor = kDefaultCursor;
	if (hitCode == kExitHitCode) {
		cursor = kExitCursor;
	} else if (hitCode >= 0 && hitCode <= BoardGameModel::kOffBoardDestination) {
		if (_selectedCell >= 0 ?
				(hitCode == _selectedCell ||
				 containsDestination(_legalDestinations, hitCode)) :
				(hitCode < BoardGameModel::kCellCount &&
				 _model.pieceAt(hitCode) > 0))
			cursor = kSelectionCursor;
	}
	if (hitCode != _hoveredCode) {
		debugC(3, kDebugInput,
			"Ripper: board-game hover code=%d previous=%d point=%d,%d selected=%d cursor=%u",
			hitCode, _hoveredCode, point.x, point.y, _selectedCell, cursor);
		_hoveredCode = hitCode;
	}
	_engine->getCursor()->update(cursor);
	_engine->getCursor()->setVisible(true);
}

bool BoardGamePuzzle::serviceKeyword(uint16 command) {
	if (command > 0xff)
		return false;
	char character = (char)command;
	if (character >= 'A' && character <= 'Z')
		character += 'a' - 'A';
	if (character == kCompletionKeyword[_keywordIndex])
		++_keywordIndex;
	else
		_keywordIndex = character == kCompletionKeyword[0] ? 1 : 0;
	if (_keywordIndex + 1 == ARRAYSIZE(kCompletionKeyword)) {
		_keywordIndex = 0;
		debugC(1, kDebugPuzzles,
			"Ripper: board-game completion keyword matched keyword='%s'",
			kCompletionKeyword);
		return true;
	}
	return false;
}

bool BoardGamePuzzle::playCue(uint cue) {
	if (cue >= kAudioCueCount)
		return false;
	const Common::String name = Common::String::format("chess%u", cue);
	Common::SeekableReadStream *stream =
		_library.createReadStreamForMember(name);
	if (!stream || !_engine->getMedia()->playSoundEffectStream(
			stream, name, _audioHandles[cue])) {
		warning("Ripper: could not play board-game cue '%s'", name.c_str());
		return false;
	}
	return true;
}

void BoardGamePuzzle::stopCue(uint cue) {
	if (cue < kAudioCueCount)
		_engine->getMedia()->stopSoundEffect(_audioHandles[cue]);
}

bool BoardGamePuzzle::animateMove(const BoardGameModel::Move &move,
		int movingPiece, int capturedPiece, bool thinkingCursor) {
	const Common::Point start = kCellAnchors[move.source];
	Common::Point destination;
	uint durationTicks = kShortMoveTicks;
	if (move.destination == BoardGameModel::kOffBoardDestination) {
		const int row = move.source / BoardGameModel::kColumnCount;
		const int column = move.source % BoardGameModel::kColumnCount;
		const int referenceColumn = column == 0 ? 1 : column - 1;
		const Common::Point &reference =
			kCellAnchors[row * BoardGameModel::kColumnCount + referenceColumn];
		destination = start + (start - reference);
		debugC(3, kDebugPuzzles,
			"Ripper: board-game legal side exit source=%d piece=%d "
			"row=%d column=%d referenceColumn=%d delta=%d,%d",
			move.source, movingPiece, row, column, referenceColumn,
			destination.x - start.x, destination.y - start.y);
	} else {
		destination = kCellAnchors[move.destination];
		const int cellDistance = ABS(move.source - move.destination);
		if (cellDistance == 2 || cellDistance > 7)
			durationTicks = kLongMoveTicks;
	}

	const uint moveCue = durationTicks == kShortMoveTicks ?
		kShortMoveCue : kLongMoveCue;
	playCue(moveCue);
	_movingActive = true;
	_movingSource = move.source;
	_movingPiece = movingPiece;
	_movingAnchor = start;
	bool removalCueStarted = false;
	const uint32 startMillis = g_system->getMillis(true);
	const uint durationMillis = durationTicks * kAnimationTickMillis;
	debugC(2, kDebugPuzzles,
		"Ripper: board-game move animation started move=%s piece=%d "
		"ticks=%u cue=%u thinkingCursor=%d from=%d,%d to=%d,%d",
		moveString(move).c_str(), movingPiece, durationTicks, moveCue,
		thinkingCursor, start.x, start.y, destination.x, destination.y);
	debugC(3, kDebugPuzzles,
		"Ripper: board-game moving overlay depth order=y-then-x "
		"startY=%d startX=%d endY=%d endX=%d",
		start.y, start.x, destination.y, destination.x);

	while (!_engine->shouldQuit()) {
		if (_engine->getInput()->pollEvents()) {
			_engine->quitGame();
			break;
		}
		const uint elapsedMillis = MIN<uint32>(
			g_system->getMillis(true) - startMillis, durationMillis);
		_movingAnchor.x = start.x +
			(destination.x - start.x) * (int)elapsedMillis / (int)durationMillis;
		_movingAnchor.y = start.y +
			(destination.y - start.y) * (int)elapsedMillis / (int)durationMillis;
		if (capturedPiece != 0 && !removalCueStarted &&
				elapsedMillis + kRemovalLeadTicks * kAnimationTickMillis >= durationMillis) {
			playCue(kRemovalCue);
			removalCueStarted = true;
		}
		if (thinkingCursor)
			_engine->getCursor()->update(kDefaultSelectionIndex);
		render();
		if (elapsedMillis >= durationMillis)
			break;
		g_system->delayMillis(kAnimationTickMillis);
	}

	_movingActive = false;
	_movingSource = -1;
	_movingPiece = 0;
	debugC(2, kDebugPuzzles,
		"Ripper: board-game move animation completed move=%s quit=%d",
		moveString(move).c_str(), _engine->shouldQuit());
	return !_engine->shouldQuit();
}

bool BoardGamePuzzle::applyPlayerMove(int destination) {
	const BoardGameModel::Move move(_selectedCell, destination);
	const int movingPiece = _model.pieceAt(move.source);
	const int capturedPiece = destination < BoardGameModel::kCellCount ?
		_model.pieceAt(destination) : 0;
	stopCue(kSelectionCue);
	clearSelectedPiece();
	if (!animateMove(move, movingPiece, capturedPiece, false))
		return false;
	if (!_model.applyMove(move))
		return false;
	if (ABS(movingPiece) == 2 && destination < BoardGameModel::kCellCount &&
			ABS(_model.pieceAt(destination)) == 3)
		playCue(kTransformCue);
	debugC(2, kDebugPuzzles,
		"Ripper: board-game player move=%s piece=%d captured=%d nextSide=%d result=%d",
		moveString(move).c_str(), movingPiece, capturedPiece,
		_model.sideToMove(), _model.result());
	return true;
}

int BoardGamePuzzle::staticScore(const BoardGameModel &model) const {
	static const int kPieceValues[] = {0, 100000, 42, 84, 168, 210};
	int score = 0;
	for (int cell = 0; cell < BoardGameModel::kCellCount; ++cell) {
		const int piece = model.pieceAt(cell);
		if (piece == 0)
			continue;
		const int type = ABS(piece);
		const int row = cell / BoardGameModel::kColumnCount;
		const int progress = piece < 0 ? row : BoardGameModel::kRowCount - row - 1;
		const int value = kPieceValues[type] +
			((type == 2 || type == 3) ? progress * 5 : 0);
		score += piece < 0 ? value : -value;
	}
	return score;
}

int BoardGamePuzzle::evaluatePosition(const BoardGameModel &model, int depth,
		int alpha, int beta) const {
	if (model.result() < 0)
		return 1000000 + depth;
	if (model.result() > 0)
		return -1000000 - depth;
	if (depth == 0)
		return staticScore(model);

	Common::Array<BoardGameModel::Move> moves;
	model.legalMoves(moves, model.sideToMove() < 0);
	if (moves.empty()) {
		BoardGameModel passed = model;
		passed.passTurn();
		return evaluatePosition(passed, depth - 1, alpha, beta);
	}
	if (model.sideToMove() < 0) {
		int best = -0x7fffffff;
		for (uint move = 0; move < moves.size(); ++move) {
			BoardGameModel next = model;
			next.applyMove(moves[move]);
			best = MAX(best, evaluatePosition(next, depth - 1, alpha, beta));
			alpha = MAX(alpha, best);
			if (beta <= alpha)
				break;
		}
		return best;
	}

	int best = 0x7fffffff;
	for (uint move = 0; move < moves.size(); ++move) {
		BoardGameModel next = model;
		next.applyMove(moves[move]);
		best = MIN(best, evaluatePosition(next, depth - 1, alpha, beta));
		beta = MIN(beta, best);
		if (beta <= alpha)
			break;
	}
	return best;
}

BoardGameModel::Move BoardGamePuzzle::chooseAiMove() {
	Common::Array<BoardGameModel::Move> moves;
	_model.legalMoves(moves, true);
	if (moves.empty())
		return BoardGameModel::Move();

	Common::Array<BoardGameModel::Move> bestMoves;
	int bestScore = -0x7fffffff;
	for (uint move = 0; move < moves.size(); ++move) {
		BoardGameModel next = _model;
		next.applyMove(moves[move]);
		const int score = evaluatePosition(next,
			_searchDepth > 0 ? _searchDepth - 1 : 0,
			-0x7fffffff, 0x7fffffff);
		if (score > bestScore) {
			bestScore = score;
			bestMoves.clear();
			bestMoves.push_back(moves[move]);
		} else if (score == bestScore) {
			bestMoves.push_back(moves[move]);
		}
	}
	const uint choice = bestMoves.size() > 1 ?
		_random.getRandomNumber(bestMoves.size() - 1) : 0;
	debugC(2, kDebugPuzzles,
		"Ripper: board-game AI evaluated moves=%u bestScore=%d ties=%u depth=%u selected=%s",
		moves.size(), bestScore, bestMoves.size(), _searchDepth,
		moveString(bestMoves[choice]).c_str());
	return bestMoves[choice];
}

bool BoardGamePuzzle::runAiTurn() {
	_engine->getCursor()->dispatchSelectionIndexChange(kDefaultSelectionIndex);
	presentScreen();
	const BoardGameModel::Move move = chooseAiMove();
	if (move.source < 0) {
		_model.passTurn();
		debugC(2, kDebugPuzzles,
			"Ripper: board-game AI passed no legal moves nextSide=%d",
			_model.sideToMove());
		return true;
	}
	const int movingPiece = _model.pieceAt(move.source);
	const int capturedPiece = move.destination < BoardGameModel::kCellCount ?
		_model.pieceAt(move.destination) : 0;
	if (!animateMove(move, movingPiece, capturedPiece, true))
		return false;
	if (!_model.applyMove(move))
		return false;
	if (ABS(movingPiece) == 2 &&
			move.destination < BoardGameModel::kCellCount &&
			ABS(_model.pieceAt(move.destination)) == 3)
		playCue(kTransformCue);
	debugC(2, kDebugPuzzles,
		"Ripper: board-game AI move=%s piece=%d captured=%d nextSide=%d result=%d",
		moveString(move).c_str(), movingPiece, capturedPiece,
		_model.sideToMove(), _model.result());
	return true;
}

BoardGamePuzzle::Result BoardGamePuzzle::finishResult(uint completionFlag) {
	Result result = kExited;
	if (_model.result() > 0) {
		if (!_engine->getMilestones()->set(
				completionFlag, true, "board-game-puzzle"))
			result = kLoadFailed;
		else
			result = kSolved;
	}
	debugC(1, kDebugPuzzles,
		"Ripper: board-game terminal resultCode=%d milestone=%u milestoneSet=%d",
		_model.result(), completionFlag,
		_engine->getMilestones()->isSet(completionFlag));
	return result;
}

BoardGamePuzzle::Result BoardGamePuzzle::run(uint completionFlag) {
	if (!_incomingDisplay.capture() || !loadAssets())
		return kLoadFailed;

	_savedSelectionIndex = _engine->getCursor()->getSelectionIndex();
	_savedCursorVisible = _engine->getCursor()->isVisible();
	_searchDepth = CLIP<uint>(_engine->getSettings()->getPuzzleLevel(), 1, 3);
	_model.reset();
	_debugHelper.reset(_engine->isPuzzleHelpEnabled());
	clearSelectedPiece();
	_hoveredCode = -1;
	_movingSource = -1;
	_movingPiece = 0;
	_movingActive = false;
	_keywordIndex = 0;
	_engine->getToolbar()->leave();
	_engine->getInput()->drainKeys();
	_engine->getInput()->discardMouseTransitions();
	_engine->getCursor()->setSelectionIndex(kDefaultSelectionIndex);
	_engine->getCursor()->dispatchSelectionIndexChange(kDefaultSelectionIndex);
	_engine->getCursor()->setVisible(true);
	render();
	debugC(1, kDebugPuzzles,
		"Ripper: entered board-game puzzle function=RunBoardGameScene@0x436c0 "
		"milestone=%u library='%s' cells=%u side=%d difficulty=%u help=0x%x keyword='%s' puzzleHelp=%d",
		completionFlag, kLibraryName, BoardGameModel::kCellCount,
		_model.sideToMove(), _searchDepth, kHelpSelectionTable,
		kCompletionKeyword, _engine->isPuzzleHelpEnabled());

	Result result = kExited;
	bool active = true;
	while (active && !_engine->shouldQuit()) {
		if (_engine->getInput()->pollEvents()) {
			_engine->quitGame();
			break;
		}
		if (_debugHelper.sync(*this))
			render();

		while (_engine->getInput()->hasPendingKey()) {
			const uint16 command = _engine->getInput()->consumeKey();
			// HandleBoardPlayerTurnInput at 0x42c1a first cancels an
			// active source choice when any keyboard command is pending;
			// the outer loop then interprets that same command.
			if (_selectedCell >= 0) {
				debugC(2, kDebugPuzzles,
					"Ripper: board-game selection cancelled by keyboard source=%d command=0x%04x",
					_selectedCell, command);
				clearSelectedPiece();
				stopCue(kSelectionCue);
				render();
			}
			if (command == kEscapeCommand) {
				debugC(1, kDebugPuzzles,
					"Ripper: board-game puzzle exited by Escape resultCode=7");
				active = false;
				break;
			}
			if (command == kHelpCommand) {
				debugC(1, kDebugPuzzles,
					"Ripper: board-game puzzle opening modal help table=0x%x",
					kHelpSelectionTable);
				if (!_engine->getModalDialog()->run(kHelpSelectionTable))
					warning("Ripper: board-game modal help failed");
				render();
				continue;
			}
			if (command == kScreenshotCommand || command == 0x10) {
				g_system->saveScreenshot();
				continue;
			}
			if (serviceKeyword(command)) {
				// RunBoardGameScene at 0x437bb stores result code 1;
				// its common cleanup path records the supplied milestone.
				_model.setResult(1);
				break;
			}
		}
		if (!active)
			break;
		if (_model.result() != 0) {
			result = finishResult(completionFlag);
			break;
		}

		if (_model.sideToMove() < 0) {
			if (!runAiTurn()) {
				result = kLoadFailed;
				break;
			}
			render();
			g_system->delayMillis(10);
		} else {
			const MouseState mouse = _engine->getInput()->publishMouseState();
			updateCursor(mouse.position);
			if ((mouse.pressed & kMouseButtonRight) != 0 &&
					_selectedCell >= 0) {
				debugC(2, kDebugPuzzles,
					"Ripper: board-game selection cancelled source=%d",
					_selectedCell);
				clearSelectedPiece();
				stopCue(kSelectionCue);
				render();
			} else if ((mouse.pressed & kMouseButtonLeft) != 0) {
				const int hitCode = hitCodeAt(mouse.position);
				if (hitCode == kExitHitCode) {
					debugC(1, kDebugPuzzles,
						"Ripper: board-game puzzle exited through control=0x81 resultCode=10");
					active = false;
				} else if (_selectedCell >= 0 &&
						hitCode == _selectedCell) {
					// HandleBoardPlayerTurnInput at 0x42c1a clears the
					// selected-source global before resolving the second
					// click. A click on the source skips the move call and
					// reaches the common opaque redraw.
					debugC(2, kDebugPuzzles,
						"Ripper: board-game deselected source=%d piece=%d",
						_selectedCell, _model.pieceAt(_selectedCell));
					clearSelectedPiece();
					stopCue(kSelectionCue);
					render();
				} else if (_selectedCell >= 0 &&
						containsDestination(_legalDestinations, hitCode)) {
					if (!applyPlayerMove(hitCode)) {
						result = kLoadFailed;
						active = false;
					} else {
						render();
					}
				} else if (_selectedCell < 0 && hitCode >= 0 &&
						hitCode < BoardGameModel::kCellCount &&
						_model.pieceAt(hitCode) > 0) {
					_selectedCell = hitCode;
					_model.legalDestinations(_selectedCell,
						_legalDestinations);
					_debugHelper.selectionChanged(*this);
					playCue(kSelectionCue);
					debugC(2, kDebugPuzzles,
						"Ripper: board-game selected source=%d piece=%d legalDestinations=%u",
						_selectedCell, _model.pieceAt(_selectedCell),
						_legalDestinations.size());
					debugC(3, kDebugPuzzles,
						"Ripper: board-game selected-piece effect resource='%s' opacity=0x%x quantization=12-bit-rgb",
						kSelectionShadingName, kSelectedPieceEffectStrength);
					render();
				}
			}
		}

		if (_model.result() != 0) {
			result = finishResult(completionFlag);
			active = false;
		}
		presentScreen();
		g_system->delayMillis(10);
	}

	for (uint cue = 0; cue < kAudioCueCount; ++cue)
		stopCue(cue);
	_incomingDisplay.restore();
	_engine->getCursor()->setSelectionIndex(_savedSelectionIndex);
	_engine->getCursor()->dispatchSelectionIndexChange(_savedSelectionIndex);
	_engine->getCursor()->refresh();
	_engine->getCursor()->setVisible(_savedCursorVisible);
	_engine->getInput()->drainKeys();
	_engine->getInput()->discardMouseTransitions();
	debugC(result == kLoadFailed ? 2 : 1, kDebugPuzzles,
		"Ripper: left board-game puzzle result=%d boardResult=%d milestone=%u milestoneSet=%d quit=%d",
		result, _model.result(), completionFlag,
		_engine->getMilestones()->isSet(completionFlag),
		_engine->shouldQuit());
	return result;
}

} // End of namespace Ripper
