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

#include "ripper/puzzles/kk_tile_match.h"

#include "common/debug.h"
#include "common/formats/ini-file.h"
#include "common/ptr.h"
#include "common/stream.h"
#include "common/system.h"
#include "graphics/surface.h"

#include "ripper/cursor.h"
#include "ripper/detection.h"
#include "ripper/display.h"
#include "ripper/input.h"
#include "ripper/media/media.h"
#include "ripper/media/smacker_decoder.h"
#include "ripper/milestones.h"
#include "ripper/modal/modal_dialog.h"
#include "ripper/ripper.h"
#include "ripper/settings.h"
#include "ripper/toolbar.h"

namespace Ripper {

namespace {

static const uint kTileCount = KkTileMatchModel::kTileCount;
static const uint kMaximumState = 13;
static const uint kCueCount = 3;
static const uint kDefaultCursor = 14;
static const uint kSelectionCursor = 16;
static const uint kHelpSelectionTable = 0x1b8;
static const uint16 kEscapeCommand = 0x1b;
static const uint16 kHelpCommand = 0x3b00;
static const uint kMoveSteps = 25;
static const uint kPlaybackStepMillis = 50;
static const uint kMismatchDelayMillis = 2500;
static const uint kPuzzleHelpSearchDepth = 6;
static const uint kPuzzleHelpSearchNodeLimit = 100000;
static const byte kPuzzleHelpRecommendedColor = 255;
static const uint kPuzzleHelpDitherMask = 3;
static const byte kAnimationTransparentColor = 0xff;
static const char *const kLibraryName = "kk.pl";
static const char *const kBackgroundName = "base";
static const char *const kClosedTileName = "big_s";
static const char kCompletionKeyword[] = "asparagus";

// Physical screen coordinates; the background fills the 640-by-400 screen.
static const Common::Point kTilePositions[kTileCount] = {
	Common::Point(49, 34), Common::Point(185, 34),
	Common::Point(321, 34), Common::Point(457, 34),
	Common::Point(49, 118), Common::Point(185, 118),
	Common::Point(321, 118), Common::Point(457, 118),
	Common::Point(49, 202), Common::Point(185, 202),
	Common::Point(321, 202), Common::Point(457, 202),
	Common::Point(49, 285), Common::Point(185, 285),
	Common::Point(321, 285), Common::Point(457, 285)
};

} // End of anonymous namespace

KkTileMatchPuzzle::DebugHelper::DebugHelper() :
		_enabled(false), _recommendedTile(-1), _solutionDepth(0) {
}

void KkTileMatchPuzzle::DebugHelper::reset(bool enabled) {
	_enabled = enabled;
	_recommendedTile = -1;
	_solutionDepth = 0;
}

bool KkTileMatchPuzzle::DebugHelper::sync(
		const KkTileMatchPuzzle &puzzle) {
	const bool enabled = puzzle._engine->isPuzzleHelpEnabled();
	if (enabled == _enabled)
		return false;

	_enabled = enabled;
	stateChanged(puzzle);
	debugC(2, kDebugPuzzles,
		"Ripper: KK tile-match puzzle help enabled=%d recommendedTile=%d solutionDepth=%u",
		_enabled, _recommendedTile < 0 ? -1 : _recommendedTile + 1,
		_solutionDepth);
	return true;
}

void KkTileMatchPuzzle::DebugHelper::stateChanging() {
	_recommendedTile = -1;
	_solutionDepth = 0;
}

void KkTileMatchPuzzle::DebugHelper::stateChanged(
		const KkTileMatchPuzzle &puzzle) {
	stateChanging();
	if (!_enabled)
		return;

	uint visitedStates = 0;
	_recommendedTile = findRecommendedTile(puzzle, _solutionDepth,
		visitedStates);
	debugC(2, kDebugPuzzles,
		"Ripper: KK tile-match puzzle help analyzed recommendedTile=%d "
		"solutionDepth=%u visitedStates=%u maxDepth=%u nodeLimit=%u color=%u",
		_recommendedTile < 0 ? -1 : _recommendedTile + 1,
		_solutionDepth, visitedStates, kPuzzleHelpSearchDepth,
		kPuzzleHelpSearchNodeLimit, kPuzzleHelpRecommendedColor);
}

uint KkTileMatchPuzzle::DebugHelper::SearchStateHash::operator()(
		const KkTileMatchModel &state) const {
	uint hash = 2166136261U;
	for (uint tile = 0; tile < kTileCount; ++tile) {
		hash ^= state.tileState(tile);
		hash *= 16777619U;
	}
	hash ^= state.activeMask() & 0xff;
	hash *= 16777619U;
	hash ^= state.activeMask() >> 8;
	hash *= 16777619U;
	return hash;
}

bool KkTileMatchPuzzle::DebugHelper::SearchStateEqual::operator()(
		const KkTileMatchModel &left, const KkTileMatchModel &right) const {
	if (left.activeMask() != right.activeMask())
		return false;
	for (uint tile = 0; tile < kTileCount; ++tile) {
		if (left.tileState(tile) != right.tileState(tile))
			return false;
	}
	return true;
}

int KkTileMatchPuzzle::DebugHelper::findRecommendedTile(
		const KkTileMatchPuzzle &puzzle, uint &solutionDepth,
		uint &visitedStates) const {
	typedef Common::HashMap<KkTileMatchModel, bool,
		SearchStateHash, SearchStateEqual> VisitedStateMap;
	Common::Array<SearchNode> nodes;
	VisitedStateMap visited;
	SearchNode root;
	root.state = puzzle._model;
	root.depth = 0;
	root.firstTile = -1;
	nodes.push_back(root);
	visited.setVal(root.state, true);

	for (uint nodeIndex = 0; nodeIndex < nodes.size(); ++nodeIndex) {
		const SearchNode node = nodes[nodeIndex];
		if (node.depth >= kPuzzleHelpSearchDepth)
			continue;
		for (uint tile = 0; tile < kTileCount; ++tile) {
			SearchNode next = node;
			const KkTileMatchModel::MatchResult result =
				next.state.applyClick(tile, puzzle._moveTable[tile]);
			next.depth = node.depth + 1;
			next.firstTile = node.depth == 0 ? tile : node.firstTile;
			if (result == KkTileMatchModel::kMatch) {
				solutionDepth = next.depth;
				visitedStates = nodes.size();
				return next.firstTile;
			}
			if (visited.contains(next.state))
				continue;
			visited.setVal(next.state, true);
			if (nodes.size() >= kPuzzleHelpSearchNodeLimit) {
				visitedStates = nodes.size();
				return -1;
			}
			nodes.push_back(next);
		}
	}

	visitedStates = nodes.size();
	return -1;
}

void KkTileMatchPuzzle::DebugHelper::draw(
		const KkTileMatchPuzzle &puzzle, byte *screen, uint pitch) const {
	if (!_enabled || _recommendedTile < 0 ||
			(uint)_recommendedTile >= kTileCount)
		return;

	const Common::Point &position = kTilePositions[_recommendedTile];
	IndexedBitmapRenderer::drawHintRect(screen, pitch,
		Common::Rect(position.x, position.y, position.x + puzzle._closedTile.width,
			position.y + puzzle._closedTile.height),
		Common::Rect(0, 0, kRipperScreenWidth, kRipperScreenHeight),
		kPuzzleHelpRecommendedColor, kPuzzleHelpDitherMask);
}

KkTileMatchPuzzle::KkTileMatchPuzzle(RipperEngine *engine) :
		Puzzle(engine), _random("ripper-kk-tile-match-puzzle"),
		_hoveredTile(-1), _keywordIndex(0) {
	for (uint row = 0; row < kTileCount; ++row) {
		_visibleTiles[row] = false;
		_frames[row] = 0;
		for (uint column = 0; column < kTileCount; ++column)
			_moveTable[row][column] = -1;
	}
}

bool KkTileMatchPuzzle::loadConfiguration() {
	const uint puzzleLevel = CLIP<uint>(
		_engine->getSettings()->getPuzzleLevel(), 1, 3);
	const Common::String name = Common::String::format(
		"kk%u.ini", puzzleLevel);
	Common::ScopedPtr<Common::SeekableReadStream> stream(
		_engine->getResources()->createReadStream(name, ResourceManager::kSourceScriptConfiguration));
	Common::INIFile ini;
	if (!stream || !ini.loadFromStream(*stream)) {
		warning("Ripper: could not load KK tile-match configuration '%s'",
			name.c_str());
		return false;
	}

	uint activeMoveRows = 0;
	for (uint row = 0; row < kTileCount; ++row) {
		const Common::String section = Common::String::format(
			"Piece %u", row + 1);
		bool active = false;
		for (uint destination = 0; destination < kTileCount; ++destination) {
			const Common::String key = Common::String::format(
				"Piece %u", destination + 1);
			Common::String value;
			if (!ini.getKey(key, section, value))
				continue;
			const uint64 source = value.asUint64();
			if (source > kTileCount) {
				warning("Ripper: KK tile-match configuration '%s' has invalid %s %s='%s'",
					name.c_str(), section.c_str(), key.c_str(), value.c_str());
				return false;
			}
			_moveTable[row][destination] = source == 0 ? -1 : (int)source - 1;
			active = active || source != 0;
		}
		if (active)
			++activeMoveRows;
	}

	uint configuredStates = 0;
	for (uint tile = 0; tile < kTileCount; ++tile) {
		const Common::String key = Common::String::format("Piece %u", tile + 1);
		Common::String value;
		if (!ini.getKey(key, "start", value))
			continue;
		const uint64 state = value.asUint64();
		if (state > kMaximumState) {
			warning("Ripper: KK tile-match configuration '%s' has invalid start %s='%s'",
				name.c_str(), key.c_str(), value.c_str());
			return false;
		}
		_model.setTileState(tile, (byte)state);
		if (state != 0)
			++configuredStates;
	}
	if (configuredStates == 0) {
		for (uint tile = 0; tile < kTileCount; ++tile)
			_model.setTileState(tile, 1);
	}

	debugC(2, kDebugPuzzles,
		"Ripper: loaded KK tile-match configuration path='%s' difficulty=%u "
		"moveRows=%u start=[%s]",
		name.c_str(), puzzleLevel, activeMoveRows, slotStateString().c_str());
	return true;
}

bool KkTileMatchPuzzle::loadAssets() {
	Common::ScopedPtr<Common::SeekableReadStream> stream(
		_engine->getResources()->createReadStreamForPath(kLibraryName));
	if (!stream || !_library.open(*stream, Common::Path(kLibraryName))) {
		warning("Ripper: could not open KK tile-match library '%s'",
			kLibraryName);
		return false;
	}
	if (!_library.loadBitmap(kBackgroundName, _background) ||
			!_library.loadBitmap(kClosedTileName, _closedTile))
		return false;
	if (_background.width != kRipperScreenWidth ||
			_background.height != kRipperScreenHeight ||
			_background.palette.size() != kRipperPaletteByteCount ||
			_closedTile.width == 0 || _closedTile.height == 0) {
		warning("Ripper: KK tile-match bitmap dimensions are invalid background=%ux%u tile=%ux%u",
			_background.width, _background.height,
			_closedTile.width, _closedTile.height);
		return false;
	}
	for (uint cue = 0; cue < kCueCount; ++cue) {
		const Common::String name = Common::String::format("chip%u", cue);
		if (!_library.hasMember(name)) {
			warning("Ripper: KK tile-match audio '%s' is missing", name.c_str());
			return false;
		}
	}

	debugC(1, kDebugPuzzles,
		"Ripper: loaded KK tile-match assets library='%s' entries=%u "
		"background=%ux%u closedTile=%ux%u audio=%u",
		kLibraryName, _library.getEntryCount(), _background.width,
		_background.height, _closedTile.width, _closedTile.height, kCueCount);
	return true;
}

bool KkTileMatchPuzzle::loadFlipAnimation(uint state) {
	if (state == 0 || state > kMaximumState)
		return false;
	FlipAnimation &animation = _flipAnimations[state - 1];
	if (animation.loaded)
		return !animation.frames.empty();
	animation.loaded = true;

	const Common::String name = Common::String::format("flip%u", state);
	Common::SeekableReadStream *stream = _library.createReadStreamForMember(name);
	Video::SmackerDecoder decoder;
	if (!stream || !decoder.loadStream(stream) ||
			decoder.getPixelFormat().bytesPerPixel != 1) {
		warning("Ripper: could not load KK tile-match animation '%s'",
			name.c_str());
		return false;
	}

	cacheSmackerFrames(decoder, animation.frames);
	if (animation.frames.size() < 2) {
		warning("Ripper: KK tile-match animation '%s' has only %u frames",
			name.c_str(), animation.frames.size());
		animation.frames.clear();
		return false;
	}

	debugC(2, kDebugPuzzles,
		"Ripper: loaded KK tile-match animation state=%u media='%s' "
		"frames=%u size=%ux%u midpoint=%u",
		state, name.c_str(), animation.frames.size(),
		animation.frames[0].width, animation.frames[0].height,
		animation.frames.size() / 2);
	return true;
}

void KkTileMatchPuzzle::drawSlot(byte *screen, uint pitch, const KkTileMatchModel &model,
		uint tile, uint frame, const Common::Point &position) const {
	if (!model.isActive(tile)) {
		IndexedBitmapRenderer::drawBitmap(screen, pitch, _closedTile,
			position.x, position.y,
			Common::Rect(0, 0, kRipperScreenWidth, kRipperScreenHeight));
		return;
	}
	const uint state = model.tileState(tile);
	if (state == 0 || state > kMaximumState)
		return;
	const FlipAnimation &animation = _flipAnimations[state - 1];
	if (frame < animation.frames.size()) {
		IndexedBitmapRenderer::drawBitmap(screen, pitch, animation.frames[frame],
			position.x, position.y,
			Common::Rect(0, 0, kRipperScreenWidth, kRipperScreenHeight),
			kAnimationTransparentColor);
	}
}

void KkTileMatchPuzzle::renderBoard() const {
	Graphics::Surface *surface = g_system->lockScreen();
	if (!surface || surface->format.bytesPerPixel != 1 ||
			surface->w != kRipperScreenWidth ||
			surface->h != kRipperScreenHeight) {
		if (surface)
			g_system->unlockScreen();
		return;
	}
	for (int y = 0; y < kRipperScreenHeight; ++y) {
		memcpy(surface->getBasePtr(0, y),
			_background.pixels.data() + y * kRipperScreenWidth,
			kRipperScreenWidth);
	}
	byte *screen = (byte *)surface->getPixels();
	for (uint tile = 0; tile < kTileCount; ++tile) {
		if (_visibleTiles[tile])
			drawSlot(screen, surface->pitch, _model, tile, _frames[tile], kTilePositions[tile]);
	}
	_debugHelper.draw(*this, screen, surface->pitch);
	g_system->unlockScreen();
	presentScreen();
}

void KkTileMatchPuzzle::renderMove(const KkTileMatchModel &snapshot, const uint *frames,
		uint selectedTile, uint step) const {
	Graphics::Surface *surface = g_system->lockScreen();
	if (!surface || surface->format.bytesPerPixel != 1 ||
			surface->w != kRipperScreenWidth ||
			surface->h != kRipperScreenHeight) {
		if (surface)
			g_system->unlockScreen();
		return;
	}
	for (int y = 0; y < kRipperScreenHeight; ++y) {
		memcpy(surface->getBasePtr(0, y),
			_background.pixels.data() + y * kRipperScreenWidth,
			kRipperScreenWidth);
	}

	byte *screen = (byte *)surface->getPixels();
	for (uint destination = 0; destination < kTileCount; ++destination) {
		if (_moveTable[selectedTile][destination] < 0 &&
				_visibleTiles[destination]) {
			drawSlot(screen, surface->pitch, snapshot, destination, frames[destination],
				kTilePositions[destination]);
		}
	}
	for (uint destination = 0; destination < kTileCount; ++destination) {
		const int source = _moveTable[selectedTile][destination];
		if (source < 0)
			continue;
		const Common::Point position(
			(kTilePositions[source].x * (int)(kMoveSteps - step) +
			 kTilePositions[destination].x * (int)step) / (int)kMoveSteps,
			(kTilePositions[source].y * (int)(kMoveSteps - step) +
			 kTilePositions[destination].y * (int)step) / (int)kMoveSteps);
		drawSlot(screen, surface->pitch, snapshot, source, frames[source], position);
	}
	g_system->unlockScreen();
	presentScreen();
}

bool KkTileMatchPuzzle::waitMillis(uint32 duration, bool interruptible) {
	const uint32 deadline = g_system->getMillis(true) + duration;
	while (!_engine->shouldQuit() &&
			(int32)(deadline - g_system->getMillis(true)) > 0) {
		if (!serviceEngineEvents())
			return false;
		if (interruptible &&
				(_engine->getInput()->hasPendingKey() ||
				 _engine->getInput()->peekMouseState().pressed != 0 ||
				 _engine->getInput()->peekMouseState().released != 0)) {
			_engine->getInput()->discardMouseTransitions();
			return true;
		}
		g_system->delayMillis(5);
	}
	return !_engine->shouldQuit();
}

bool KkTileMatchPuzzle::flashTiles() {
	uint order[kTileCount];
	for (uint tile = 0; tile < kTileCount; ++tile)
		order[tile] = tile;
	for (uint tile = 0; tile < 8; ++tile) {
		const uint swap = _random.getRandomNumber(kTileCount - 1);
		const uint saved = order[tile];
		order[tile] = order[swap];
		order[swap] = saved;
	}

	renderBoard();
	for (uint index = 0; index < kTileCount; ++index) {
		_visibleTiles[order[index]] = true;
		playCue(1);
		renderBoard();
		if (!waitMillis(kPlaybackStepMillis, false))
			return false;
	}
	debugC(2, kDebugPuzzles,
		"Ripper: KK tile-match initial tile flash completed tiles=%u stepMillis=%u",
		kTileCount, kPlaybackStepMillis);
	_engine->getInput()->discardMouseTransitions();
	return true;
}

bool KkTileMatchPuzzle::applyMove(uint selectedTile) {
	const KkTileMatchModel snapshot = _model;
	KkTileMatchModel target = snapshot;
	if (!target.applyMove(_moveTable[selectedTile]))
		return true;
	uint targetFrames[kTileCount];
	for (uint tile = 0; tile < kTileCount; ++tile) {
		const int source = _moveTable[selectedTile][tile];
		targetFrames[tile] = source >= 0 ? _frames[source] : _frames[tile];
	}

	playCue(0, true);
	_engine->getCursor()->setVisible(false);
	debugC(2, kDebugPuzzles,
		"Ripper: KK tile-match move animation started selected=%u steps=%u "
		"before=[%s]",
		selectedTile + 1, kMoveSteps, slotStateString().c_str());
	for (uint step = 1; step <= kMoveSteps; ++step) {
		renderMove(snapshot, _frames, selectedTile, step);
		if (!waitMillis(kPlaybackStepMillis, false)) {
			stopCue(0);
			return false;
		}
	}
	_model = target;
	for (uint tile = 0; tile < kTileCount; ++tile)
		_frames[tile] = targetFrames[tile];
	stopCue(0);
	playCue(1);
	_engine->getCursor()->setVisible(true);
	renderBoard();
	debugC(2, kDebugPuzzles,
		"Ripper: KK tile-match move animation completed selected=%u after=[%s]",
		selectedTile + 1, slotStateString().c_str());
	return true;
}

bool KkTileMatchPuzzle::openTile(uint tile) {
	if (_model.isActive(tile))
		return true;
	if (!loadFlipAnimation(_model.tileState(tile))) {
		warning("Ripper: KK tile-match tile=%u has invalid state=%u",
			tile + 1, _model.tileState(tile));
		return false;
	}
	FlipAnimation &animation = _flipAnimations[_model.tileState(tile) - 1];
	const uint midpoint = animation.frames.size() / 2;
	playCue(2);
	_model.reveal(tile);
	_engine->getCursor()->setVisible(false);
	debugC(2, kDebugPuzzles,
		"Ripper: KK tile-match opening tile=%u state=%u frames=0..%u",
		tile + 1, _model.tileState(tile), midpoint - 1);
	for (uint frame = 0; frame < midpoint; ++frame) {
		_frames[tile] = frame;
		renderBoard();
		if (!waitMillis(kPlaybackStepMillis, false))
			return false;
	}
	playCue(1);
	_engine->getCursor()->setVisible(true);
	_engine->getInput()->discardMouseTransitions();
	debugC(2, kDebugPuzzles,
		"Ripper: KK tile-match opened tile=%u state=%u active=%u",
		tile + 1, _model.tileState(tile), _model.activeTileCount());
	return true;
}

bool KkTileMatchPuzzle::closeActiveTiles() {
	for (uint tile = 0; tile < kTileCount; ++tile) {
		if (!_model.isActive(tile))
			continue;
		FlipAnimation &animation = _flipAnimations[_model.tileState(tile) - 1];
		playCue(2);
		_engine->getCursor()->setVisible(false);
		debugC(3, kDebugPuzzles,
			"Ripper: KK tile-match closing tile=%u state=%u frame=%u..%u",
			tile + 1, _model.tileState(tile), _frames[tile] + 1,
			animation.frames.size() - 1);
		for (uint frame = _frames[tile] + 1;
				frame < animation.frames.size(); ++frame) {
			_frames[tile] = frame;
			renderBoard();
			if (!waitMillis(kPlaybackStepMillis, false))
				return false;
		}
		_model.hide(tile);
		_frames[tile] = 0;
		playCue(1);
		renderBoard();
	}
	_engine->getCursor()->setVisible(true);
	_engine->getInput()->discardMouseTransitions();
	debugC(2, kDebugPuzzles,
		"Ripper: KK tile-match mismatch reset completed active=%u states=[%s]",
		_model.activeTileCount(), slotStateString().c_str());
	return true;
}

int KkTileMatchPuzzle::findTile(const Common::Point &point) const {
	for (uint tile = 0; tile < kTileCount; ++tile) {
		const Common::Rect bounds(kTilePositions[tile].x,
			kTilePositions[tile].y,
			kTilePositions[tile].x + _closedTile.width,
			kTilePositions[tile].y + _closedTile.height);
		if (bounds.contains(point))
			return tile;
	}
	return -1;
}

void KkTileMatchPuzzle::updateCursor(const Common::Point &point) {
	const int tile = findTile(point);
	if (tile != _hoveredTile) {
		debugC(3, kDebugInput,
			"Ripper: KK tile-match hover tile=%d previous=%d point=%d,%d cursor=%u",
			tile < 0 ? -1 : tile + 1,
			_hoveredTile < 0 ? -1 : _hoveredTile + 1,
			point.x, point.y, tile < 0 ? kDefaultCursor : kSelectionCursor);
		_hoveredTile = tile;
	}
	_engine->getCursor()->update(tile < 0 ? kDefaultCursor : kSelectionCursor);
	_engine->getCursor()->setVisible(true);
}

bool KkTileMatchPuzzle::serviceKeyword(uint16 command) {
	if (command > 0xff)
		return false;
	if (advanceKeyword(command, kCompletionKeyword, _keywordIndex, true)) {
		_keywordIndex = 0;
		debugC(1, kDebugPuzzles,
			"Ripper: KK tile-match completion keyword matched keyword='%s'",
			kCompletionKeyword);
		return true;
	}
	return false;
}

bool KkTileMatchPuzzle::playCue(uint cue, bool loop) {
	if (cue >= kCueCount)
		return false;
	const Common::String name = Common::String::format("chip%u", cue);
	Common::SeekableReadStream *stream =
		_library.createReadStreamForMember(name);
	if (!stream || !_engine->getMedia()->playSoundEffectStream(
			stream, name, _audioHandles[cue], 100, loop)) {
		warning("Ripper: could not play KK tile-match cue '%s'", name.c_str());
		return false;
	}
	debugC(3, kDebugPuzzles,
		"Ripper: KK tile-match played cue=%u path='%s' loop=%d",
		cue, name.c_str(), loop);
	return true;
}

void KkTileMatchPuzzle::stopCue(uint cue) {
	if (cue < kCueCount)
		_engine->getMedia()->stopSoundEffect(_audioHandles[cue]);
}

void KkTileMatchPuzzle::stopAudio() {
	stopAudioHandles(_audioHandles, ARRAYSIZE(_audioHandles));
}

Common::String KkTileMatchPuzzle::slotStateString() const {
	Common::String result;
	for (uint tile = 0; tile < kTileCount; ++tile) {
		if (tile != 0)
			result += ',';
		result += Common::String::format("%u%s", _model.tileState(tile),
			_model.isActive(tile) ? "*" : "");
	}
	return result;
}

KkTileMatchPuzzle::Result KkTileMatchPuzzle::run(uint completionFlag) {
	if (!_incomingDisplay.capture() || !loadConfiguration() || !loadAssets()) {
		_incomingDisplay.restore();
		return kLoadFailed;
	}

	_hoveredTile = -1;
	_keywordIndex = 0;
	_debugHelper.reset(_engine->isPuzzleHelpEnabled());
	_engine->getInput()->drainKeys();
	_engine->getInput()->discardMouseTransitions();
	_engine->getCursor()->setSelectionIndex(kDefaultCursor);
	_engine->getCursor()->dispatchSelectionIndexChange(kDefaultCursor);
	_engine->getCursor()->setVisible(true);
	_engine->installScenePalette(_background.palette);
	debugC(1, kDebugPuzzles,
		"Ripper: entered KK tile-match puzzle "
		"completionFlag=%u controls=%u help=0x%x states=[%s] puzzleHelp=%d",
		completionFlag, kTileCount, kHelpSelectionTable,
		slotStateString().c_str(),
		_engine->isPuzzleHelpEnabled());

	Result result = kExited;
	bool active = flashTiles();
	if (!active && !_engine->shouldQuit())
		result = kLoadFailed;
	if (active) {
		_debugHelper.stateChanged(*this);
		renderBoard();
	}
	while (active && !_engine->shouldQuit()) {
		if (!serviceEngineEvents())
			break;
		if (_debugHelper.sync(*this))
			renderBoard();

		while (_engine->getInput()->hasPendingKey()) {
			const uint16 command = _engine->getInput()->consumeKey();
			if (command == kEscapeCommand) {
				debugC(1, kDebugPuzzles,
					"Ripper: KK tile-match puzzle exited by Escape");
				active = false;
				break;
			}
			if (command == kHelpCommand) {
				debugC(1, kDebugPuzzles,
					"Ripper: KK tile-match puzzle opening modal help table=0x%x",
					kHelpSelectionTable);
				if (!_engine->getModalDialog()->run(kHelpSelectionTable))
					warning("Ripper: KK tile-match modal help failed");
				_engine->installScenePalette(_background.palette);
				renderBoard();
				continue;
			}
			if (serviceKeyword(command)) {
				if (!markSolved(completionFlag, "kk-tile-match-keyword")) {
					result = kLoadFailed;
				} else {
					result = kSolved;
				}
				active = false;
				break;
			}
		}
		if (!active)
			break;

		const MouseState mouse = _engine->getInput()->publishMouseState();
		updateCursor(mouse.position);
		if ((mouse.pressed & kMouseButtonLeft) != 0) {
			const int tile = findTile(mouse.position);
			if (tile >= 0) {
				_debugHelper.stateChanging();
				if (!applyMove(tile) || !openTile(tile)) {
					if (!_engine->shouldQuit())
						result = kLoadFailed;
					active = false;
				} else {
					const KkTileMatchModel::MatchResult match = _model.evaluate();
					if (match != KkTileMatchModel::kPending) {
						debugC(2, kDebugPuzzles,
							"Ripper: KK tile-match evaluated active=%u matchingFirst=%u states=[%s]",
							_model.activeTileCount(), _model.matchingActiveTileCount(), slotStateString().c_str());
						if (match == KkTileMatchModel::kMatch) {
							if (!markSolved(completionFlag, "kk-tile-match-puzzle")) {
								result = kLoadFailed;
							} else {
								result = kSolved;
							}
							debugC(1, kDebugPuzzles,
								"Ripper: solved KK tile-match puzzle milestone=%u states=[%s]",
								completionFlag, slotStateString().c_str());
							active = false;
						} else if (!waitMillis(kMismatchDelayMillis, true) ||
								!closeActiveTiles()) {
							if (!_engine->shouldQuit())
								result = kLoadFailed;
							active = false;
						}
					}
				}
				if (active) {
					_debugHelper.stateChanged(*this);
					renderBoard();
				}
			}
		}
		// Present unchanged frames too so the software cursor follows idle input ticks.
		presentScreen();
		g_system->delayMillis(10);
	}

	stopAudio();
	_engine->getCursor()->setSelectionIndex(0);
	_engine->getCursor()->dispatchSelectionIndexChange(0);
	_engine->getCursor()->refresh();
	_engine->getCursor()->setVisible(true);
	_engine->getInput()->drainKeys();
	_engine->getInput()->discardMouseTransitions();
	debugC(1, kDebugPuzzles,
		"Ripper: left KK tile-match puzzle result=%d milestone=%u "
		"milestoneSet=%d states=[%s] quit=%d",
		result, completionFlag,
		_engine->getMilestones()->isSet(completionFlag),
		slotStateString().c_str(), _engine->shouldQuit());
	return result;
}

} // End of namespace Ripper
