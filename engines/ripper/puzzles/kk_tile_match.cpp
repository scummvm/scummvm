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
#include "graphics/paletteman.h"
#include "graphics/surface.h"
#include "video/smk_decoder.h"

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

static const uint kTileCount = 16;
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
static const byte kAnimationTransparentColor = 0xff;
static const char *const kLibraryName = "kk.pl";
static const char *const kBackgroundName = "base";
static const char *const kClosedTileName = "big_s";
static const char kCompletionKeyword[] = "asparagus";

// g_kkPuzzleTilePositionPairs at 0x84474 contains scene Y followed by X.
// RunKkTileMatchPuzzleScene at 0x2fa31 cancels the active 50-pixel scene
// origin before it creates the 16 controls, so these are physical points.
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

KkTileMatchPuzzle::KkTileMatchPuzzle(RipperEngine *engine) :
		_engine(engine), _random("ripper-kk-tile-match-puzzle"),
		_hoveredTile(-1), _keywordIndex(0) {
	for (uint row = 0; row < kTileCount; ++row) {
		_visibleTiles[row] = false;
		for (uint column = 0; column < kTileCount; ++column)
			_moveTable[row][column] = -1;
	}
}

bool KkTileMatchPuzzle::loadConfiguration() {
	const uint puzzleLevel = CLIP<uint>(
		_engine->getSettings()->getPuzzleLevel(), 1, 3);
	const Common::String name = Common::String::format(
		"kk%u.ini", puzzleLevel);
	Common::SeekableReadStream *configStream =
		_engine->getResources()->createReadStreamForPath(name);
	if (!configStream) {
		configStream = _engine->getResources()->scripts().createReadStreamForMember(name);
		if (configStream) {
			debugC(2, kDebugResources,
				"Ripper: resolved KK tile-match configuration '%s' through SCRIPT.PL",
				name.c_str());
		}
	}
	Common::ScopedPtr<Common::SeekableReadStream> stream(configStream);
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
		_slots[tile].state = (uint)state;
		if (state != 0)
			++configuredStates;
	}
	if (configuredStates == 0) {
		for (uint tile = 0; tile < kTileCount; ++tile)
			_slots[tile].state = 1;
	}

	debugC(2, kDebugPuzzles,
		"Ripper: loaded KK tile-match configuration path='%s' difficulty=%u "
		"moveRows=%u start=[%s]",
		name.c_str(), puzzleLevel, activeMoveRows, slotStateString().c_str());
	return true;
}

bool KkTileMatchPuzzle::loadBitmap(const Common::String &name,
		BitmapAssetFrame &frame) {
	Common::ScopedPtr<Common::SeekableReadStream> stream(
		_library.createReadStreamForMember(name));
	if (!stream || !decodeBitmapAsset(*stream, frame)) {
		warning("Ripper: could not decode KK tile-match bitmap '%s'",
			name.c_str());
		return false;
	}
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
	if (!loadBitmap(kBackgroundName, _background) ||
			!loadBitmap(kClosedTileName, _closedTile))
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

	animation.frames.reserve(decoder.getFrameCount());
	while (!decoder.endOfVideo()) {
		const Graphics::Surface *surface = decoder.decodeNextFrame();
		if (!surface)
			break;
		AnimationFrame frame;
		frame.width = surface->w;
		frame.height = surface->h;
		frame.pixels.resize((uint32)frame.width * frame.height);
		for (uint y = 0; y < frame.height; ++y) {
			memcpy(frame.pixels.data() + y * frame.width,
				surface->getBasePtr(0, y), frame.width);
		}
		animation.frames.push_back(Common::move(frame));
	}
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

void KkTileMatchPuzzle::applyPalette() const {
	Common::Array<byte> palette = _background.palette;
	_engine->getToolbar()->applySharedPalettePatch(palette.data(),
		kRipperPaletteColorCount);
	_engine->getSettings()->applyVideoPalette(palette.data(),
		kRipperPaletteColorCount, true);
	g_system->getPaletteManager()->setPalette(palette.data(), 0,
		kRipperPaletteColorCount);
}

void KkTileMatchPuzzle::drawBitmap(byte *screen, uint pitch,
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

void KkTileMatchPuzzle::drawAnimationFrame(byte *screen, uint pitch,
		const AnimationFrame &frame, int x, int y) const {
	for (uint sourceY = 0; sourceY < frame.height; ++sourceY) {
		const int destinationY = y + sourceY;
		if (destinationY < 0 || destinationY >= kRipperScreenHeight)
			continue;
		for (uint sourceX = 0; sourceX < frame.width; ++sourceX) {
			const int destinationX = x + sourceX;
			if (destinationX < 0 || destinationX >= kRipperScreenWidth)
				continue;
			const byte pixel = frame.pixels[sourceY * frame.width + sourceX];
			if (pixel != kAnimationTransparentColor)
				screen[destinationY * pitch + destinationX] = pixel;
		}
	}
}

void KkTileMatchPuzzle::drawSlot(byte *screen, uint pitch, const Slot &slot,
		const Common::Point &position) const {
	if (!slot.active) {
		drawBitmap(screen, pitch, _closedTile, position.x, position.y);
		return;
	}
	if (slot.state == 0 || slot.state > kMaximumState)
		return;
	const FlipAnimation &animation = _flipAnimations[slot.state - 1];
	if (slot.frame < animation.frames.size()) {
		drawAnimationFrame(screen, pitch, animation.frames[slot.frame],
			position.x, position.y);
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
			drawSlot(screen, surface->pitch, _slots[tile], kTilePositions[tile]);
	}
	g_system->unlockScreen();
	presentScreen();
}

void KkTileMatchPuzzle::renderMove(const Slot *snapshot,
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
			drawSlot(screen, surface->pitch, snapshot[destination],
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
		drawSlot(screen, surface->pitch, snapshot[source], position);
	}
	g_system->unlockScreen();
	presentScreen();
}

bool KkTileMatchPuzzle::waitMillis(uint32 duration, bool interruptible) {
	const uint32 deadline = g_system->getMillis(true) + duration;
	while (!_engine->shouldQuit() &&
			(int32)(deadline - g_system->getMillis(true)) > 0) {
		if (_engine->getInput()->pollEvents()) {
			_engine->quitGame();
			return false;
		}
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
	bool moved = false;
	Slot snapshot[kTileCount];
	Slot target[kTileCount];
	for (uint tile = 0; tile < kTileCount; ++tile) {
		snapshot[tile] = _slots[tile];
		target[tile] = _slots[tile];
		const int source = _moveTable[selectedTile][tile];
		if (source >= 0) {
			target[tile] = snapshot[source];
			moved = true;
		}
	}
	if (!moved)
		return true;

	playCue(0, true);
	_engine->getCursor()->setVisible(false);
	debugC(2, kDebugPuzzles,
		"Ripper: KK tile-match move animation started selected=%u steps=%u "
		"before=[%s]",
		selectedTile + 1, kMoveSteps, slotStateString().c_str());
	for (uint step = 1; step <= kMoveSteps; ++step) {
		renderMove(snapshot, selectedTile, step);
		if (!waitMillis(kPlaybackStepMillis, false)) {
			stopCue(0);
			return false;
		}
	}
	for (uint tile = 0; tile < kTileCount; ++tile)
		_slots[tile] = target[tile];
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
	if (_slots[tile].active)
		return true;
	if (!loadFlipAnimation(_slots[tile].state)) {
		warning("Ripper: KK tile-match tile=%u has invalid state=%u",
			tile + 1, _slots[tile].state);
		return false;
	}
	FlipAnimation &animation = _flipAnimations[_slots[tile].state - 1];
	const uint midpoint = animation.frames.size() / 2;
	playCue(2);
	_slots[tile].active = true;
	_engine->getCursor()->setVisible(false);
	debugC(2, kDebugPuzzles,
		"Ripper: KK tile-match opening tile=%u state=%u frames=0..%u",
		tile + 1, _slots[tile].state, midpoint - 1);
	for (uint frame = 0; frame < midpoint; ++frame) {
		_slots[tile].frame = frame;
		renderBoard();
		if (!waitMillis(kPlaybackStepMillis, false))
			return false;
	}
	playCue(1);
	_engine->getCursor()->setVisible(true);
	_engine->getInput()->discardMouseTransitions();
	debugC(2, kDebugPuzzles,
		"Ripper: KK tile-match opened tile=%u state=%u active=%u",
		tile + 1, _slots[tile].state, activeTileCount());
	return true;
}

bool KkTileMatchPuzzle::closeActiveTiles() {
	for (uint tile = 0; tile < kTileCount; ++tile) {
		if (!_slots[tile].active)
			continue;
		FlipAnimation &animation = _flipAnimations[_slots[tile].state - 1];
		playCue(2);
		_engine->getCursor()->setVisible(false);
		debugC(3, kDebugPuzzles,
			"Ripper: KK tile-match closing tile=%u state=%u frame=%u..%u",
			tile + 1, _slots[tile].state, _slots[tile].frame + 1,
			animation.frames.size() - 1);
		for (uint frame = _slots[tile].frame + 1;
				frame < animation.frames.size(); ++frame) {
			_slots[tile].frame = frame;
			renderBoard();
			if (!waitMillis(kPlaybackStepMillis, false))
				return false;
		}
		_slots[tile].active = false;
		_slots[tile].frame = 0;
		playCue(1);
		renderBoard();
	}
	_engine->getCursor()->setVisible(true);
	_engine->getInput()->discardMouseTransitions();
	debugC(2, kDebugPuzzles,
		"Ripper: KK tile-match mismatch reset completed active=%u states=[%s]",
		activeTileCount(), slotStateString().c_str());
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
	for (uint cue = 0; cue < kCueCount; ++cue)
		stopCue(cue);
}

uint KkTileMatchPuzzle::activeTileCount() const {
	uint count = 0;
	for (uint tile = 0; tile < kTileCount; ++tile) {
		if (_slots[tile].active)
			++count;
	}
	return count;
}

uint KkTileMatchPuzzle::matchingActiveTileCount() const {
	uint referenceState = 0;
	uint count = 0;
	for (uint tile = 0; tile < kTileCount; ++tile) {
		if (!_slots[tile].active)
			continue;
		if (referenceState == 0)
			referenceState = _slots[tile].state;
		if (_slots[tile].state == referenceState)
			++count;
	}
	return count;
}

Common::String KkTileMatchPuzzle::slotStateString() const {
	Common::String result;
	for (uint tile = 0; tile < kTileCount; ++tile) {
		if (tile != 0)
			result += ',';
		result += Common::String::format("%u%s", _slots[tile].state,
			_slots[tile].active ? "*" : "");
	}
	return result;
}

KkTileMatchPuzzle::Result KkTileMatchPuzzle::run(uint completionFlag) {
	// DispatchSceneEntryAction at 0x36892 routes action 25 to
	// RunKkTileMatchPuzzleScene at 0x2fa31. The retail controller owns KK.PL,
	// reads KK%d.INI, and creates controls 0x672..0x681 over this 4x4 grid.
	if (!_incomingDisplay.capture() || !loadConfiguration() || !loadAssets()) {
		_incomingDisplay.restore();
		return kLoadFailed;
	}

	_hoveredTile = -1;
	_keywordIndex = 0;
	_engine->getInput()->drainKeys();
	_engine->getInput()->discardMouseTransitions();
	_engine->getCursor()->setSelectionIndex(kDefaultCursor);
	_engine->getCursor()->dispatchSelectionIndexChange(kDefaultCursor);
	_engine->getCursor()->setVisible(true);
	applyPalette();
	debugC(1, kDebugPuzzles,
		"Ripper: entered KK tile-match puzzle function=RunKkTileMatchPuzzleScene@0x2fa31 "
		"completionFlag=%u controls=%u help=0x%x keyword='%s' states=[%s]",
		completionFlag, kTileCount, kHelpSelectionTable,
		kCompletionKeyword, slotStateString().c_str());

	Result result = kExited;
	bool active = flashTiles();
	if (!active && !_engine->shouldQuit())
		result = kLoadFailed;
	while (active && !_engine->shouldQuit()) {
		if (_engine->getInput()->pollEvents()) {
			_engine->quitGame();
			break;
		}

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
				applyPalette();
				renderBoard();
				continue;
			}
			if (serviceKeyword(command)) {
				if (!_engine->getMilestones()->set(completionFlag, true,
						"kk-tile-match-keyword")) {
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
				if (!applyMove(tile) || !openTile(tile)) {
					if (!_engine->shouldQuit())
						result = kLoadFailed;
					active = false;
				} else if (activeTileCount() >= 3) {
					const uint matching = matchingActiveTileCount();
					debugC(2, kDebugPuzzles,
						"Ripper: KK tile-match evaluated active=%u matchingFirst=%u states=[%s]",
						activeTileCount(), matching, slotStateString().c_str());
					if (matching == 3) {
						if (!_engine->getMilestones()->set(completionFlag, true,
								"kk-tile-match-puzzle")) {
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
		}
		g_system->delayMillis(10);
	}

	stopAudio();
	_engine->getCursor()->setSelectionIndex(0);
	_engine->getCursor()->dispatchSelectionIndexChange(0);
	_engine->getCursor()->refresh();
	_engine->getCursor()->setVisible(true);
	_engine->getInput()->drainKeys();
	_engine->getInput()->discardMouseTransitions();
	debugC(result == kLoadFailed ? 2 : 1, kDebugPuzzles,
		"Ripper: left KK tile-match puzzle result=%d milestone=%u "
		"milestoneSet=%d states=[%s] quit=%d",
		result, completionFlag,
		_engine->getMilestones()->isSet(completionFlag),
		slotStateString().c_str(), _engine->shouldQuit());
	return result;
}

} // End of namespace Ripper
