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

#include "ripper/puzzles/stained_glass.h"

#include "audio/audiostream.h"
#include "audio/decoders/wave.h"
#include "common/debug.h"
#include "common/formats/ini-file.h"
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

static const uint kTileCount = 16;
static const uint kCueCount = 5;
static const uint kDefaultCursor = 14;
static const uint kSelectionCursor = 16;
static const uint kTransitionSteps = 16;
static const uint kTransitionStepMillis = 50;
static const uint kInitialPreviewMillis = 1250;
static const uint kSolvedPreviewMillis = 2500;
static const uint kFadeSteps = 9;
static const uint kAmbientVolume = 75;
static const uint16 kEscapeCommand = 0x1b;
static const uint16 kSpaceCommand = 0x20;
static const uint16 kHelpCommand = 0x3b00;
static const uint16 kSolvedPreviewCommand = 0x2300;
static const uint kHelpSelectionTable = 0x1b1;
static const char *const kLibraryName = "stglass.pl";
static const char *const kBackgroundName = "glass.pcx";
static const char *const kTransitionMaskName = "frame3.pcx";
static const char *const kAmbientName = "g-puzzle.wav";
static const char kCompletionKeyword[] = "headache";

// g_stainedGlassPuzzleTilePositionTable at 0x843f0 stores ordinary X/Y
// pairs. RunStainedGlassPuzzleScene at 0x2e366 cancels a 50-pixel scene
// presentation offset before creating the controls, so these coordinates are
// physical even when the preceding media used the scene viewport.
static const Common::Point kTilePositions[kTileCount] = {
	Common::Point(277, 5),
	Common::Point(228, 54), Common::Point(326, 54),
	Common::Point(179, 103), Common::Point(277, 103), Common::Point(375, 103),
	Common::Point(130, 152), Common::Point(228, 152),
	Common::Point(326, 152), Common::Point(424, 152),
	Common::Point(179, 201), Common::Point(277, 201), Common::Point(375, 201),
	Common::Point(228, 250), Common::Point(326, 250),
	Common::Point(277, 299)
};

static bool isPermutation(const uint *order) {
	bool seen[kTileCount] = {};
	for (uint index = 0; index < kTileCount; ++index) {
		if (order[index] >= kTileCount || seen[order[index]])
			return false;
		seen[order[index]] = true;
	}
	return true;
}

static Common::String orderString(const uint *order) {
	Common::String result;
	for (uint index = 0; index < kTileCount; ++index) {
		if (!result.empty())
			result += ',';
		result += Common::String::format("%u", order[index] + 1);
	}
	return result;
}

} // End of anonymous namespace

StainedGlassPuzzle::StainedGlassPuzzle(RipperEngine *engine) :
		_engine(engine), _random("ripper-stained-glass-puzzle"),
		_hoveredTile(-1), _keywordIndex(0) {
	for (uint row = 0; row < kTileCount; ++row) {
		_initialOrder[row] = row;
		_currentOrder[row] = row;
		_savedOrder[row] = row;
		for (uint column = 0; column < kTileCount; ++column)
			_moveTable[row][column] = -1;
	}
}

bool StainedGlassPuzzle::loadConfiguration() {
	const uint puzzleLevel = CLIP<uint>(
		_engine->getSettings()->getPuzzleLevel(), 1, 3);
	const Common::String name = Common::String::format(
		"stglass%u.ini", puzzleLevel);
	Common::ScopedPtr<Common::SeekableReadStream> stream(
		_engine->getResources()->createReadStreamForPath(name));
	Common::INIFile ini;
	if (!stream || !ini.loadFromStream(*stream)) {
		warning("Ripper: could not load stained glass puzzle configuration '%s'",
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
				warning("Ripper: stained glass configuration '%s' has invalid %s %s='%s'",
					name.c_str(), section.c_str(), key.c_str(), value.c_str());
				return false;
			}
			// LoadStainedGlassPuzzleMoveTable at 0x2da64 leaves a
			// destination unchanged when the INI lookup returns zero.
			_moveTable[row][destination] = source == 0 ? -1 : (int)source - 1;
			active = true;
		}
		if (active)
			++activeMoveRows;
	}

	uint configuredInitialPositions = 0;
	for (uint position = 0; position < kTileCount; ++position) {
		const Common::String key = Common::String::format(
			"Piece %u", position + 1);
		Common::String value;
		if (!ini.getKey(key, "Initial Positions", value))
			continue;
		const uint64 tile = value.asUint64();
		if (tile == 0 || tile > kTileCount) {
			warning("Ripper: stained glass configuration '%s' has invalid initial %s='%s'",
				name.c_str(), key.c_str(), value.c_str());
			return false;
		}
		_initialOrder[position] = (uint)tile - 1;
		++configuredInitialPositions;
	}
	if (configuredInitialPositions != 0 &&
			(configuredInitialPositions != kTileCount ||
			!isPermutation(_initialOrder))) {
		warning("Ripper: stained glass configuration '%s' has an incomplete or duplicate initial order",
			name.c_str());
		return false;
	}

	for (uint row = 0; row < kTileCount; ++row) {
		uint sourceSlots[kTileCount];
		for (uint destination = 0; destination < kTileCount; ++destination) {
			sourceSlots[destination] = _moveTable[row][destination] >= 0 ?
				(uint)_moveTable[row][destination] : destination;
		}
		if (!isPermutation(sourceSlots)) {
			warning("Ripper: stained glass configuration '%s' move row=%u is not a permutation",
				name.c_str(), row + 1);
			return false;
		}
	}

	debugC(2, kDebugPuzzles,
		"Ripper: loaded stained glass configuration path='%s' difficulty=%u "
		"moveRows=%u initial=[%s] coordinates=physical-x-y",
		name.c_str(), puzzleLevel, activeMoveRows,
		orderString(_initialOrder).c_str());
	return true;
}

bool StainedGlassPuzzle::loadPcx(const Common::String &name,
		BitmapAssetFrame &frame) {
	Common::ScopedPtr<Common::SeekableReadStream> stream(
		_library.createReadStreamForMember(name));
	if (!stream || !decodePcxAsset(*stream, frame)) {
		warning("Ripper: could not decode stained glass PCX '%s'",
			name.c_str());
		return false;
	}
	return true;
}

bool StainedGlassPuzzle::loadBitmap(const Common::String &name,
		BitmapAssetFrame &frame) {
	Common::ScopedPtr<Common::SeekableReadStream> stream(
		_library.createReadStreamForMember(name));
	if (!stream || !decodeBitmapAsset(*stream, frame)) {
		warning("Ripper: could not decode stained glass bitmap '%s'",
			name.c_str());
		return false;
	}
	return true;
}

bool StainedGlassPuzzle::loadAssets() {
	Common::ScopedPtr<Common::SeekableReadStream> stream(
		_engine->getResources()->createReadStreamForPath(kLibraryName));
	if (!stream || !_library.open(*stream, Common::Path(kLibraryName))) {
		warning("Ripper: could not open stained glass library '%s'",
			kLibraryName);
		return false;
	}
	if (!loadPcx(kBackgroundName, _background) ||
			!loadPcx(kTransitionMaskName, _transitionMask))
		return false;
	if (_background.width != kRipperScreenWidth ||
			_background.height != kRipperScreenHeight ||
			_background.palette.size() != kRipperPaletteByteCount ||
			_transitionMask.width != kRipperScreenWidth ||
			_transitionMask.height != kRipperScreenHeight) {
		warning("Ripper: stained glass PCX dimensions are invalid background=%ux%u mask=%ux%u",
			_background.width, _background.height,
			_transitionMask.width, _transitionMask.height);
		return false;
	}

	for (uint tile = 0; tile < kTileCount; ++tile) {
		if (!loadBitmap(Common::String::format("p%02u.bbm", tile + 1),
				_tiles[tile]))
			return false;
	}
	if (!loadBitmap("p17.bbm", _blankTile))
		return false;
	for (uint cue = 0; cue < kCueCount; ++cue) {
		const Common::String name = Common::String::format("glas%u.wav", cue);
		if (!_library.hasMember(name)) {
			warning("Ripper: stained glass audio '%s' is missing", name.c_str());
			return false;
		}
	}
	if (!_library.hasMember(kAmbientName)) {
		warning("Ripper: stained glass ambient audio '%s' is missing",
			kAmbientName);
		return false;
	}

	debugC(1, kDebugPuzzles,
		"Ripper: loaded stained glass assets library='%s' background=%ux%u "
		"transitionMask=%ux%u tiles=%u blank=%ux%u audio=%u ambient='%s'",
		kLibraryName, _background.width, _background.height,
		_transitionMask.width, _transitionMask.height, kTileCount,
		_blankTile.width, _blankTile.height, kCueCount, kAmbientName);
	return true;
}

void StainedGlassPuzzle::drawFrame(byte *screen, uint pitch,
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

bool StainedGlassPuzzle::buildPuzzleBacking() {
	if (_background.pixels.size() !=
			(uint32)kRipperScreenWidth * kRipperScreenHeight)
		return false;
	_puzzleBacking = _background.pixels;
	for (uint position = 0; position < kTileCount; ++position) {
		drawFrame(_puzzleBacking.data(), kRipperScreenWidth, _blankTile,
			kTilePositions[position].x, kTilePositions[position].y);
	}
	return true;
}

void StainedGlassPuzzle::applyPalette() {
	Common::Array<byte> palette = _background.palette;
	_engine->applySharedPalettePatch(palette.data(),
		kRipperPaletteColorCount);
	_engine->getSettings()->applyVideoPalette(palette.data(),
		kRipperPaletteColorCount, true);
	g_system->getPaletteManager()->setPalette(palette.data(), 0,
		kRipperPaletteColorCount);
}

void StainedGlassPuzzle::renderOrder(const uint *order) {
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1 ||
			screen->w != kRipperScreenWidth ||
			screen->h != kRipperScreenHeight) {
		if (screen)
			g_system->unlockScreen();
		return;
	}
	for (int y = 0; y < kRipperScreenHeight; ++y) {
		memcpy(screen->getBasePtr(0, y),
			_puzzleBacking.data() + y * kRipperScreenWidth,
			kRipperScreenWidth);
	}
	byte *pixels = (byte *)screen->getPixels();
	for (uint position = 0; position < kTileCount; ++position) {
		if (order[position] >= kTileCount)
			continue;
		drawFrame(pixels, screen->pitch, _tiles[order[position]],
			kTilePositions[position].x, kTilePositions[position].y);
	}
	g_system->unlockScreen();
	applyPalette();
	presentScreen();
}

void StainedGlassPuzzle::renderTransition(const uint *previousOrder,
		const uint *targetOrder, uint step) {
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1 ||
			screen->w != kRipperScreenWidth ||
			screen->h != kRipperScreenHeight) {
		if (screen)
			g_system->unlockScreen();
		return;
	}
	for (int y = 0; y < kRipperScreenHeight; ++y) {
		memcpy(screen->getBasePtr(0, y),
			_puzzleBacking.data() + y * kRipperScreenWidth,
			kRipperScreenWidth);
	}

	byte *pixels = (byte *)screen->getPixels();
	for (uint destination = 0; destination < kTileCount; ++destination) {
		const uint tile = targetOrder[destination];
		uint source = destination;
		for (uint candidate = 0; candidate < kTileCount; ++candidate) {
			if (previousOrder[candidate] == tile) {
				source = candidate;
				break;
			}
		}
		const int x = (kTilePositions[source].x *
			(int)(kTransitionSteps - step) +
			kTilePositions[destination].x * (int)step) /
			(int)kTransitionSteps;
		const int y = (kTilePositions[source].y *
			(int)(kTransitionSteps - step) +
			kTilePositions[destination].y * (int)step) /
			(int)kTransitionSteps;
		drawFrame(pixels, screen->pitch, _tiles[tile], x, y);
	}
	// AnimateStainedGlassPuzzleTileTransition at 0x2dc23 attaches the
	// FRAME3.PCX descriptor to the scratch update after composing the moving
	// tiles. It acts as a stencil rather than a color source: color-zero
	// openings expose the moving pieces, while all other pixels retain the
	// existing GLASS.PCX backdrop and lattice.
	for (uint y = 0; y < _transitionMask.height; ++y) {
		for (uint x = 0; x < _transitionMask.width; ++x) {
			const uint offset = y * _transitionMask.width + x;
			if (_transitionMask.pixels[offset] !=
					_transitionMask.transparentColor)
				pixels[y * screen->pitch + x] = _puzzleBacking[offset];
		}
	}
	g_system->unlockScreen();
	applyPalette();
	presentScreen();
}

bool StainedGlassPuzzle::playCue(uint cue, bool loop) {
	if (cue >= kCueCount)
		return false;
	const Common::String name = Common::String::format("glas%u.wav", cue);
	Common::SeekableReadStream *stream =
		_library.createReadStreamForMember(name);
	if (!stream)
		return false;
	Audio::SeekableAudioStream *wavStream =
		Audio::makeWAVStream(stream, DisposeAfterUse::YES);
	if (!wavStream) {
		warning("Ripper: could not decode stained glass audio '%s'",
			name.c_str());
		return false;
	}

	Audio::Mixer *mixer = g_system->getMixer();
	mixer->stopHandle(_cueHandles[cue]);
	Audio::AudioStream *playbackStream = wavStream;
	if (loop)
		playbackStream = Audio::makeLoopingAudioStream(wavStream, 0);
	mixer->playStream(Audio::Mixer::kSFXSoundType, &_cueHandles[cue],
		playbackStream);
	debugC(3, kDebugPuzzles,
		"Ripper: stained glass played cue=%u path='%s' loop=%d",
		cue, name.c_str(), loop);
	return true;
}

bool StainedGlassPuzzle::playAmbient() {
	Common::SeekableReadStream *stream =
		_library.createReadStreamForMember(kAmbientName);
	if (!stream)
		return false;
	Audio::SeekableAudioStream *wavStream =
		Audio::makeWAVStream(stream, DisposeAfterUse::YES);
	if (!wavStream)
		return false;
	Audio::AudioStream *looping = Audio::makeLoopingAudioStream(wavStream, 0);
	const byte volume = (byte)(kAmbientVolume *
		Audio::Mixer::kMaxChannelVolume / 100);
	g_system->getMixer()->playStream(Audio::Mixer::kSFXSoundType,
		&_ambientHandle, looping, -1, volume);
	debugC(2, kDebugPuzzles,
		"Ripper: started stained glass ambient path='%s' loop=1 volume=%u",
		kAmbientName, kAmbientVolume);
	return true;
}

void StainedGlassPuzzle::stopAudio() {
	for (uint cue = 0; cue < kCueCount; ++cue)
		g_system->getMixer()->stopHandle(_cueHandles[cue]);
	g_system->getMixer()->stopHandle(_ambientHandle);
}

bool StainedGlassPuzzle::animateTo(const uint *targetOrder,
		const char *reason) {
	if (!isPermutation(targetOrder))
		return false;
	bool changed = false;
	for (uint position = 0; position < kTileCount; ++position) {
		if (_currentOrder[position] != targetOrder[position]) {
			changed = true;
			break;
		}
	}
	if (!changed) {
		renderOrder(_currentOrder);
		return true;
	}

	uint previousOrder[kTileCount];
	for (uint position = 0; position < kTileCount; ++position)
		previousOrder[position] = _currentOrder[position];
	const uint movementCue = _random.getRandomNumber(2) + 1;
	playCue(movementCue, true);
	_engine->getCursor()->setVisible(false);
	debugC(2, kDebugPuzzles,
		"Ripper: stained glass transition began reason='%s' steps=%u "
		"from=[%s] to=[%s] movementCue=%u frameMask=stencil",
		reason, kTransitionSteps, orderString(previousOrder).c_str(),
		orderString(targetOrder).c_str(), movementCue);

	for (uint step = 1; step <= kTransitionSteps; ++step) {
		if (_engine->getInput()->pollEvents()) {
			_engine->quitGame();
			return false;
		}
		renderTransition(previousOrder, targetOrder, step);
		g_system->delayMillis(kTransitionStepMillis);
	}
	for (uint position = 0; position < kTileCount; ++position)
		_currentOrder[position] = targetOrder[position];
	g_system->getMixer()->stopHandle(_cueHandles[movementCue]);
	const uint landingCue = _random.getRandomNumber(1) == 0 ? 0 : 4;
	playCue(landingCue);
	_engine->getInput()->drainKeys();
	_engine->getInput()->discardMouseTransitions();
	_engine->getCursor()->setVisible(true);
	renderOrder(_currentOrder);
	debugC(2, kDebugPuzzles,
		"Ripper: stained glass transition completed reason='%s' "
		"landingCue=%u order=[%s] solved=%u/%u",
		reason, landingCue, orderString(_currentOrder).c_str(),
		countSolved(), kTileCount);
	return true;
}

bool StainedGlassPuzzle::waitForPreview(uint32 duration) {
	const uint32 deadline = g_system->getMillis() + duration;
	while (!_engine->shouldQuit() &&
			(int32)(g_system->getMillis() - deadline) < 0) {
		if (_engine->getInput()->pollEvents()) {
			_engine->quitGame();
			return false;
		}
		if (_engine->getInput()->hasPendingKey()) {
			_engine->getInput()->drainKeys();
			break;
		}
		const MouseState mouse = _engine->getInput()->publishMouseState();
		if ((mouse.pressed & kMouseButtonLeft) != 0)
			break;
		presentScreen();
		g_system->delayMillis(10);
	}
	_engine->getInput()->discardMouseTransitions();
	return !_engine->shouldQuit();
}

bool StainedGlassPuzzle::showInitialPreview() {
	uint solved[kTileCount];
	for (uint position = 0; position < kTileCount; ++position)
		solved[position] = position;
	debugC(2, kDebugPuzzles,
		"Ripper: stained glass initial preview durationMs=%u",
		kInitialPreviewMillis);
	return animateTo(solved, "initial-preview-solved") &&
		waitForPreview(kInitialPreviewMillis) &&
		animateTo(_initialOrder, "initial-preview-scramble");
}

bool StainedGlassPuzzle::showSolvedPreview() {
	for (uint position = 0; position < kTileCount; ++position)
		_savedOrder[position] = _currentOrder[position];
	uint solved[kTileCount];
	for (uint position = 0; position < kTileCount; ++position)
		solved[position] = position;
	debugC(2, kDebugPuzzles,
		"Ripper: stained glass solved preview durationMs=%u restore=[%s]",
		kSolvedPreviewMillis, orderString(_savedOrder).c_str());
	return animateTo(solved, "alt-h-solved-preview") &&
		waitForPreview(kSolvedPreviewMillis) &&
		animateTo(_savedOrder, "alt-h-restore");
}

uint StainedGlassPuzzle::countSolved() const {
	uint solved = 0;
	for (uint position = 0; position < kTileCount; ++position) {
		if (_currentOrder[position] == position)
			++solved;
	}
	return solved;
}

bool StainedGlassPuzzle::isSolved() const {
	return countSolved() == kTileCount;
}

int StainedGlassPuzzle::findTile(const Common::Point &point) const {
	for (uint position = 0; position < kTileCount; ++position) {
		const BitmapAssetFrame &frame = _tiles[position];
		const int x = point.x - kTilePositions[position].x;
		const int y = point.y - kTilePositions[position].y;
		if (x >= 0 && y >= 0 && x < frame.width && y < frame.height &&
				frame.pixels[y * frame.width + x] != frame.transparentColor)
			return position;
	}
	return -1;
}

void StainedGlassPuzzle::updateCursor(const Common::Point &point) {
	const int tile = findTile(point);
	if (tile != _hoveredTile) {
		debugC(3, kDebugPuzzles,
			"Ripper: stained glass hover tile=%d previous=%d point=%d,%d cursor=%u",
			tile, _hoveredTile, point.x, point.y,
			tile >= 0 ? kSelectionCursor : kDefaultCursor);
		_hoveredTile = tile;
	}
	_engine->getCursor()->update(
		tile >= 0 ? kSelectionCursor : kDefaultCursor);
}

bool StainedGlassPuzzle::applyMove(uint selectedTile) {
	if (selectedTile >= kTileCount)
		return false;
	uint targetOrder[kTileCount];
	for (uint destination = 0; destination < kTileCount; ++destination) {
		const int source = _moveTable[selectedTile][destination];
		targetOrder[destination] = source >= 0 ?
			_currentOrder[source] : _currentOrder[destination];
	}
	if (!isPermutation(targetOrder))
		return false;
	debugC(2, kDebugPuzzles,
		"Ripper: stained glass selected control=%u action=0x%x pointTable=physical-x-y",
		selectedTile + 1, 0x672 + selectedTile);
	return animateTo(targetOrder, "tile-selection");
}

bool StainedGlassPuzzle::serviceKeyword(uint16 command) {
	if (command > 0xff)
		return false;
	char character = (char)command;
	if (character >= 'A' && character <= 'Z')
		character += 'a' - 'A';
	if (character == kCompletionKeyword[_keywordIndex])
		++_keywordIndex;
	else
		_keywordIndex = 0;
	if (_keywordIndex + 1 == ARRAYSIZE(kCompletionKeyword)) {
		_keywordIndex = 0;
		debugC(1, kDebugPuzzles,
			"Ripper: stained glass completion keyword matched keyword='%s'",
			kCompletionKeyword);
		return true;
	}
	return false;
}

void StainedGlassPuzzle::clearDisplay() {
	_engine->getMedia()->fadePalette(false, kFadeSteps);
	g_system->fillScreen(0);
	presentScreen();
}

StainedGlassPuzzle::Result StainedGlassPuzzle::run(uint completionFlag) {
	// RunStainedGlassPuzzleScene at 0x2e366 owns STGLASS.PL, loads the move
	// table from STGLASSn.INI, and creates controls 0x672..0x681. It begins
	// from identity order, previews the solved image, then animates to the
	// configured initial permutation.
	if (!_incomingDisplay.capture() || !loadConfiguration() || !loadAssets() ||
			!buildPuzzleBacking()) {
		_incomingDisplay.restore();
		return kLoadFailed;
	}

	for (uint position = 0; position < kTileCount; ++position)
		_currentOrder[position] = position;
	_hoveredTile = -1;
	_keywordIndex = 0;
	_engine->getInput()->drainKeys();
	_engine->getInput()->discardMouseTransitions();
	_engine->getCursor()->setSelectionIndex(kDefaultCursor);
	_engine->getCursor()->dispatchSelectionIndexChange(kDefaultCursor);
	_engine->getCursor()->setVisible(true);
	renderOrder(_currentOrder);
	if (!playAmbient())
		warning("Ripper: could not start stained glass ambient audio");
	debugC(1, kDebugPuzzles,
		"Ripper: entered stained glass puzzle function=RunStainedGlassPuzzleScene@0x2e366 "
		"completionFlag=%u controls=%u help=0x%x keyword='%s' order=[%s]",
		completionFlag, kTileCount, kHelpSelectionTable,
		kCompletionKeyword, orderString(_currentOrder).c_str());

	Result result = kExited;
	bool active = showInitialPreview();
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
					"Ripper: stained glass puzzle exited by Escape");
				active = false;
				break;
			}
			if (command == kHelpCommand) {
				debugC(1, kDebugPuzzles,
					"Ripper: stained glass puzzle opening modal help table=0x%x",
					kHelpSelectionTable);
				if (!_engine->getModalDialog()->run(kHelpSelectionTable))
					warning("Ripper: stained glass puzzle modal help failed");
				renderOrder(_currentOrder);
				continue;
			}
			if (command == kSpaceCommand) {
				if (!showInitialPreview()) {
					result = kLoadFailed;
					active = false;
				}
				continue;
			}
			if (command == kSolvedPreviewCommand) {
				if (!showSolvedPreview()) {
					result = kLoadFailed;
					active = false;
				}
				continue;
			}
			if (serviceKeyword(command)) {
				if (!_engine->getMilestones()->set(completionFlag, true,
						"stained-glass-keyword")) {
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
				if (!applyMove(tile)) {
					result = kLoadFailed;
					active = false;
				} else if (isSolved()) {
					if (!_engine->getMilestones()->set(completionFlag, true,
							"stained-glass-puzzle")) {
						result = kLoadFailed;
					} else {
						result = kSolved;
					}
					debugC(1, kDebugPuzzles,
						"Ripper: solved stained glass puzzle milestone=%u order=[%s]",
						completionFlag, orderString(_currentOrder).c_str());
					active = false;
				}
			}
		}
		presentScreen();
		g_system->delayMillis(10);
	}

	stopAudio();
	clearDisplay();
	_engine->getCursor()->setSelectionIndex(0);
	_engine->getCursor()->dispatchSelectionIndexChange(0);
	_engine->getCursor()->refresh();
	_engine->getCursor()->setVisible(true);
	_engine->getInput()->drainKeys();
	_engine->getInput()->discardMouseTransitions();
	debugC(result == kLoadFailed ? 2 : 1, kDebugPuzzles,
		"Ripper: left stained glass puzzle result=%d milestone=%u "
		"milestoneSet=%d order=[%s] quit=%d",
		result, completionFlag,
		_engine->getMilestones()->isSet(completionFlag),
		orderString(_currentOrder).c_str(), _engine->shouldQuit());
	return result;
}

} // End of namespace Ripper
