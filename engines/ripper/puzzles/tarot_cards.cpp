/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this program.
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

#include "ripper/puzzles/tarot_cards.h"

#include "common/debug.h"
#include "common/formats/ini-file.h"
#include "common/ptr.h"
#include "common/stream.h"
#include "common/system.h"
#include "graphics/paletteman.h"
#include "graphics/surface.h"

#include "ripper/cursor.h"
#include "ripper/detection.h"
#include "ripper/diagnostics/screen_presenter.h"
#include "ripper/input.h"
#include "ripper/ini.h"
#include "ripper/media.h"
#include "ripper/milestones.h"
#include "ripper/resources.h"
#include "ripper/ripper.h"
#include "ripper/scenes/ending_sequence.h"
#include "ripper/settings.h"
#include "ripper/toolbar.h"

namespace Ripper {

namespace {

static const uint kDefaultCursor = 14;
static const uint kSelectionCursor = 16;
static const uint kLoadingCursor = 19;
static const uint16 kEscapeCommand = 0x1b;
static const int kSceneOriginY = 50;
static const int kSceneHeight = 300;
static const uint kDosTickMillis = 55;
static const uint kDepartureStepCount = 10;
static const uint kDepartureStepMillis = kDosTickMillis;
static const char *const kLibraryName = "tarot.pl";
static const char *const kBackgroundName = "bkgrnd2";
static const char *const kMusicName = "reapsong";
static const char *const kSceneCueNames[2] = { "tarot0", "tarot2" };

// g_tarotCardSlotLayoutRows at 0x3e78d stores X/Y/card rows. The retail
// controller subtracts 90 from every Y coordinate before constructing
// controls 0x672 through 0x680.
static const Common::Point kSlotPositions[TarotCardsModel::kSlotCount] = {
	Common::Point(73, 158), Common::Point(162, 158),
	Common::Point(251, 158), Common::Point(337, 158),
	Common::Point(423, 158), Common::Point(508, 158),
	Common::Point(5, 14), Common::Point(75, 14),
	Common::Point(145, 14), Common::Point(215, 14),
	Common::Point(285, 14), Common::Point(354, 14),
	Common::Point(424, 14), Common::Point(495, 14),
	Common::Point(566, 14)
};

} // End of anonymous namespace

TarotCardsPuzzle::TarotCardsPuzzle(RipperEngine *engine) :
		Puzzle(engine), _hoveredSlot(-1), _timeLimitSeconds(0) {
	for (uint slot = 0; slot < TarotCardsModel::kSlotCount; ++slot)
		_departed[slot] = false;
}

bool TarotCardsPuzzle::loadConfiguration() {
	const uint puzzleLevel = CLIP<uint>(
		_engine->getSettings()->getPuzzleLevel(), 1, 3);
	const Common::String name = Common::String::format("card%u.ini",
		puzzleLevel);
	Common::SeekableReadStream *configStream =
		_engine->getResources()->createReadStreamForPath(name);
	if (!configStream) {
		configStream = _engine->getResources()->scripts().createReadStreamForMember(name);
		if (configStream) {
			debugC(2, kDebugResources,
				"Ripper: resolved tarot configuration '%s' through SCRIPT.PL",
				name.c_str());
		}
	}
	Common::ScopedPtr<Common::SeekableReadStream> stream(configStream);
	Common::INIFile ini;
	if (!stream || !ini.loadFromStream(*stream)) {
		warning("Ripper: could not load tarot configuration '%s'",
			name.c_str());
		return false;
	}
	_timeLimitSeconds = 0;
	readIniUint(ini, "start", "timelimit", _timeLimitSeconds);
	debugC(2, kDebugPuzzles,
		"Ripper: loaded tarot configuration path='%s' difficulty=%u timeLimitSeconds=%u",
		name.c_str(), puzzleLevel, _timeLimitSeconds);
	return true;
}

bool TarotCardsPuzzle::loadBitmap(const Common::String &name,
		BitmapAssetFrame &frame) {
	Common::ScopedPtr<Common::SeekableReadStream> stream(
		_library.createReadStreamForMember(name));
	if (!stream || !decodeBitmapAsset(*stream, frame)) {
		warning("Ripper: could not decode tarot bitmap '%s'", name.c_str());
		return false;
	}
	return true;
}

bool TarotCardsPuzzle::loadPcx(const Common::String &name,
		BitmapAssetFrame &frame) {
	Common::ScopedPtr<Common::SeekableReadStream> stream(
		_library.createReadStreamForMember(name));
	if (!stream || !decodePcxAsset(*stream, frame)) {
		warning("Ripper: could not decode tarot PCX '%s'", name.c_str());
		return false;
	}
	return true;
}

bool TarotCardsPuzzle::loadAssets() {
	Common::ScopedPtr<Common::SeekableReadStream> stream(
		_engine->getResources()->createReadStreamForPath(kLibraryName));
	if (!stream || !_library.open(*stream, Common::Path(kLibraryName))) {
		warning("Ripper: could not open tarot library '%s'", kLibraryName);
		return false;
	}
	if (!loadPcx(kBackgroundName, _background))
		return false;
	if (_background.width != kRipperScreenWidth ||
			_background.height != kSceneHeight ||
			_background.palette.size() < kRipperPaletteByteCount) {
		warning("Ripper: tarot background has invalid geometry=%ux%u colors=%u",
			_background.width, _background.height,
			_background.palette.size() / 3);
		return false;
	}

	for (uint card = 0; card < TarotCardsModel::kCardCount; ++card) {
		if (!loadBitmap(Common::String::format("si%u", card + 1),
				_smallCards[card]) ||
				!loadBitmap(Common::String::format("bi%u", card + 1),
					_largeCards[card]))
			return false;
	}
	if (_smallCards[0].width != 68 || _smallCards[0].height != 112 ||
			_largeCards[0].width != 118 || _largeCards[0].height != 198) {
		warning("Ripper: tarot card geometry is invalid small=%ux%u large=%ux%u",
			_smallCards[0].width, _smallCards[0].height,
			_largeCards[0].width, _largeCards[0].height);
		return false;
	}
	for (uint cue = 0; cue < ARRAYSIZE(kSceneCueNames); ++cue) {
		if (!_library.hasMember(kSceneCueNames[cue])) {
			warning("Ripper: tarot scene audio '%s' is missing",
				kSceneCueNames[cue]);
			return false;
		}
	}
	for (uint card = 0; card < TarotCardsModel::kCardCount; ++card) {
		if (!_library.hasMember(Common::String::format("snd%u", card))) {
			warning("Ripper: tarot card audio cue=%u is missing", card);
			return false;
		}
	}
	if (!_library.hasMember(kMusicName)) {
		warning("Ripper: tarot music '%s' is missing", kMusicName);
		return false;
	}

	debugC(1, kDebugPuzzles,
		"Ripper: loaded tarot assets library='%s' entries=%u background=%ux%u small=%ux%u large=%ux%u cards=%u audio=%u",
		kLibraryName, _library.getEntryCount(), _background.width,
		_background.height, _smallCards[0].width, _smallCards[0].height,
		_largeCards[0].width, _largeCards[0].height,
		TarotCardsModel::kCardCount,
		(uint)ARRAYSIZE(kSceneCueNames) + TarotCardsModel::kCardCount + 1);
	return true;
}

void TarotCardsPuzzle::applyPalette() const {
	Common::Array<byte> palette = _background.palette;
	_engine->applySharedPalettePatch(palette.data(),
		kRipperPaletteColorCount);
	_engine->getSettings()->applyVideoPalette(palette.data(),
		kRipperPaletteColorCount, true);
	g_system->getPaletteManager()->setPalette(palette.data(), 0,
		kRipperPaletteColorCount);
}

void TarotCardsPuzzle::drawFrame(byte *screen, uint pitch,
		const BitmapAssetFrame &frame, int x, int y) const {
	for (uint sourceY = 0; sourceY < frame.height; ++sourceY) {
		const int destinationY = y + sourceY;
		// InitializeTransientPresentationOverlay in
		// RunTarotCardPuzzleScene at 0x3eece clips the held card to logical
		// y=0..299. Keep every tarot bitmap inside the corresponding physical
		// scene page so dragging cannot dirty the retained interface bands.
		if (destinationY < kSceneOriginY ||
				destinationY >= kSceneOriginY + kSceneHeight)
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

void TarotCardsPuzzle::drawScaledFrame(byte *screen, uint pitch,
		const BitmapAssetFrame &frame, int x, int y, uint scaleNumerator,
		uint scaleDenominator) const {
	if (scaleNumerator == 0 || scaleDenominator == 0)
		return;
	const uint width = frame.width * scaleNumerator / scaleDenominator;
	const uint height = frame.height * scaleNumerator / scaleDenominator;
	for (uint destinationY = 0; destinationY < height; ++destinationY) {
		const int screenY = y + destinationY;
		if (screenY < kSceneOriginY ||
				screenY >= kSceneOriginY + kSceneHeight)
			continue;
		const uint sourceY = destinationY * scaleDenominator / scaleNumerator;
		for (uint destinationX = 0; destinationX < width; ++destinationX) {
			const int screenX = x + destinationX;
			if (screenX < 0 || screenX >= kRipperScreenWidth)
				continue;
			const uint sourceX = destinationX * scaleDenominator / scaleNumerator;
			const byte pixel = frame.pixels[sourceY * frame.width + sourceX];
			if (pixel != frame.transparentColor)
				screen[screenY * pitch + screenX] = pixel;
		}
	}
}

void TarotCardsPuzzle::render(const Common::Point &mousePosition) const {
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1 ||
			screen->w != kRipperScreenWidth ||
			screen->h != kRipperScreenHeight) {
		if (screen)
			g_system->unlockScreen();
		return;
	}
	for (int y = 0; y < kSceneHeight; ++y) {
		memcpy(screen->getBasePtr(0, y + kSceneOriginY),
			_background.pixels.data() + y * kRipperScreenWidth,
			kRipperScreenWidth);
	}
	byte *pixels = (byte *)screen->getPixels();
	for (uint slot = 0; slot < TarotCardsModel::kSlotCount; ++slot) {
		const uint card = _model.cardAt(slot);
		if (card == 0 || _departed[slot])
			continue;
		drawFrame(pixels, screen->pitch, _smallCards[card - 1],
			kSlotPositions[slot].x,
			kSlotPositions[slot].y + kSceneOriginY);
	}
	if (_model.heldCard() != 0) {
		const BitmapAssetFrame &held = _largeCards[_model.heldCard() - 1];
		drawFrame(pixels, screen->pitch, held,
			mousePosition.x - held.width / 2,
			mousePosition.y - held.height / 2);
	}
	g_system->unlockScreen();
	presentScreen();
}

void TarotCardsPuzzle::renderDeparture(uint slot, uint step) const {
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1) {
		if (screen)
			g_system->unlockScreen();
		return;
	}
	for (int y = 0; y < kSceneHeight; ++y) {
		memcpy(screen->getBasePtr(0, y + kSceneOriginY),
			_background.pixels.data() + y * kRipperScreenWidth,
			kRipperScreenWidth);
	}
	byte *pixels = (byte *)screen->getPixels();
	for (uint other = 0; other < TarotCardsModel::kSlotCount; ++other) {
		const uint card = _model.cardAt(other);
		if (card == 0 || _departed[other] || other == slot)
			continue;
		drawFrame(pixels, screen->pitch, _smallCards[card - 1],
			kSlotPositions[other].x,
			kSlotPositions[other].y + kSceneOriginY);
	}
	const uint card = _model.cardAt(slot);
	if (card != 0 && step < kDepartureStepCount) {
		const BitmapAssetFrame &frame = _smallCards[card - 1];
		const uint scale = kDepartureStepCount - step;
		const int x = kSlotPositions[slot].x +
			(frame.width * (int)step) / (2 * (int)kDepartureStepCount);
		const int y = kSlotPositions[slot].y + kSceneOriginY +
			(frame.height * (int)step) / (2 * (int)kDepartureStepCount);
		drawScaledFrame(pixels, screen->pitch, frame, x, y, scale,
			kDepartureStepCount);
	}
	g_system->unlockScreen();
	presentScreen();
}

bool TarotCardsPuzzle::animateRemainingCards() {
	for (uint slot = TarotCardsModel::kTargetSlotCount;
			slot < TarotCardsModel::kSlotCount; ++slot) {
		if (_model.cardAt(slot) == 0)
			continue;
		playSceneCue(1);
		for (uint step = 0; step <= kDepartureStepCount; ++step) {
			renderDeparture(slot, step);
			if (!serviceEngineEvents())
				return false;
			g_system->delayMillis(kDepartureStepMillis);
		}
		_departed[slot] = true;
		debugC(2, kDebugPuzzles,
			"Ripper: tarot cleanup removed slot=%u card=%u steps=%u",
			slot, _model.cardAt(slot), kDepartureStepCount + 1);
	}
	return !_engine->shouldQuit();
}

int TarotCardsPuzzle::findSlot(const Common::Point &point) const {
	for (uint slot = 0; slot < TarotCardsModel::kSlotCount; ++slot) {
		const Common::Point &position = kSlotPositions[slot];
		if (Common::Rect(position.x, position.y + kSceneOriginY,
				position.x + _smallCards[0].width,
				position.y + kSceneOriginY +
					_smallCards[0].height).contains(point))
			return slot;
	}
	return -1;
}

void TarotCardsPuzzle::updateCursor(const Common::Point &point) {
	const int hovered = findSlot(point);
	if (hovered == _hoveredSlot)
		return;
	debugC(3, kDebugPuzzles,
		"Ripper: tarot hover slot=%d previous=%d point=%d,%d",
		hovered, _hoveredSlot, point.x, point.y);
	_hoveredSlot = hovered;
	_engine->getCursor()->update(
		hovered >= 0 ? kSelectionCursor : kDefaultCursor);
}

bool TarotCardsPuzzle::playSceneCue(uint cue) {
	if (cue >= ARRAYSIZE(kSceneCueNames))
		return false;
	return _engine->getMedia()->playSoundEffectStream(
		_library.createReadStreamForMember(kSceneCueNames[cue]),
		kSceneCueNames[cue], _sceneCueHandles[cue], 100, false);
}

bool TarotCardsPuzzle::playCardCue(uint card) {
	if (card == 0 || card > TarotCardsModel::kCardCount)
		return false;
	const Common::String name = Common::String::format("snd%u", card - 1);
	return _engine->getMedia()->playSoundEffectStream(
		_library.createReadStreamForMember(name), name,
		_cardCueHandles[card - 1], 100, false);
}

void TarotCardsPuzzle::stopAudio() {
	stopAudioHandles(_sceneCueHandles, ARRAYSIZE(_sceneCueHandles));
	stopAudioHandles(_cardCueHandles, ARRAYSIZE(_cardCueHandles));
	_engine->getMedia()->stopSoundEffect(_musicHandle);
}

Common::String TarotCardsPuzzle::stateString() const {
	Common::String result;
	for (uint slot = 0; slot < TarotCardsModel::kSlotCount; ++slot) {
		if (slot != 0)
			result += ',';
		result += Common::String::format("%u", _model.cardAt(slot));
	}
	return result;
}

TarotCardsPuzzle::Result TarotCardsPuzzle::run(uint completionFlag) {
	// DispatchSceneEntryAction at 0x36892 routes action 23 to
	// RunTarotCardPuzzleScene at 0x3eece with the completion flag in EAX.
	if (!_incomingDisplay.capture())
		return kLoadFailed;

	_engine->getToolbar()->leave();
	_engine->getInput()->drainKeys();
	_engine->getInput()->discardMouseTransitions();
	_engine->getCursor()->setSelectionIndex(kLoadingCursor);
	_engine->getCursor()->dispatchSelectionIndexChange(kLoadingCursor);
	_engine->getCursor()->setVisible(true);
	if (!_engine->getMedia()->play("begin.avi", true) ||
			!loadConfiguration() || !loadAssets()) {
		_incomingDisplay.restore();
		return kLoadFailed;
	}

	applyPalette();
	_engine->getCursor()->setSelectionIndex(kDefaultCursor);
	_engine->getCursor()->dispatchSelectionIndexChange(kDefaultCursor);
	_engine->getCursor()->setVisible(true);
	const Common::Point initialMouse =
		_engine->getInput()->peekMouseState().position;
	render(initialMouse);
	const bool musicStarted = _engine->getMedia()->playSoundEffectStream(
		_library.createReadStreamForMember(kMusicName), kMusicName,
		_musicHandle, 45, true);
	const uint32 startTick = g_system->getMillis(true);
	const uint32 timeLimitMillis =
		_timeLimitSeconds * 18 * kDosTickMillis;
	debugC(1, kDebugPuzzles,
		"Ripper: entered tarot puzzle function=RunTarotCardPuzzleScene@0x3eece milestone=%u library='%s' controls=0x672..0x680 target=[8,3,9,7,2,1] timeLimitSeconds=%u music=%d",
		completionFlag, kLibraryName, _timeLimitSeconds, musicStarted);

	Result result = kExited;
	bool active = true;
	while (active && !_engine->shouldQuit()) {
		if (!serviceEngineEvents())
			break;
		while (_engine->getInput()->hasPendingKey()) {
			if (_engine->getInput()->consumeKey() == kEscapeCommand) {
				debugC(1, kDebugPuzzles,
					"Ripper: tarot puzzle exited by Escape held=%u state=[%s]",
					_model.heldCard(), stateString().c_str());
				active = false;
				break;
			}
		}
		if (!active)
			break;
		if (timeLimitMillis != 0 &&
				g_system->getMillis(true) - startTick > timeLimitMillis) {
			debugC(1, kDebugPuzzles,
				"Ripper: tarot puzzle timed out elapsedMs=%u limitMs=%u",
				g_system->getMillis(true) - startTick, timeLimitMillis);
			break;
		}

		const MouseState mouse = _engine->getInput()->publishMouseState();
		updateCursor(mouse.position);
		if ((mouse.pressed & kMouseButtonLeft) != 0) {
			const int slot = findSlot(mouse.position);
			if (slot >= 0) {
				const uint placedCard = _model.heldCard();
				const uint pickedCard = _model.cardAt(slot);
				if (placedCard != 0)
					playSceneCue(0);
				_model.swap(slot);
				if (pickedCard != 0)
					playCardCue(pickedCard);
				debugC(2, kDebugPuzzles,
					"Ripper: tarot swapped slot=%d placed=%u picked=%u held=%u state=[%s]",
					slot, placedCard, pickedCard, _model.heldCard(),
					stateString().c_str());
				render(mouse.position);
				if (_model.solved()) {
					debugC(1, kDebugPuzzles,
						"Ripper: solved tarot puzzle milestone=%u held=%u state=[%s]",
						completionFlag, _model.heldCard(),
						stateString().c_str());
					result = markSolved(completionFlag, "tarot-card-puzzle") ?
						kSolved : kLoadFailed;
					active = false;
				}
			}
		} else if (_model.heldCard() != 0) {
			render(mouse.position);
		}
		presentScreen();
		g_system->delayMillis(10);
	}

	_engine->getCursor()->setVisible(false);
	if (result == kSolved && !animateRemainingCards() &&
			!_engine->shouldQuit())
		result = kLoadFailed;
	stopAudio();
	if (result == kSolved && !_engine->shouldQuit()) {
		EndingSequence ending(_engine);
		if (!ending.run())
			result = kLoadFailed;
	} else if (result == kExited && !_engine->shouldQuit()) {
		if (!_engine->getMedia()->play("tarotdea.avi", true))
			result = kLoadFailed;
	}

	_engine->getCursor()->setSelectionIndex(0);
	_engine->getCursor()->dispatchSelectionIndexChange(0);
	_engine->getCursor()->setVisible(false);
	_engine->getInput()->drainKeys();
	_engine->getInput()->discardMouseTransitions();
	debugC(result == kLoadFailed ? 2 : 1, kDebugPuzzles,
		"Ripper: left tarot puzzle result=%d milestone=%u milestoneSet=%d held=%u state=[%s] quit=%d",
		result, completionFlag,
		_engine->getMilestones()->isSet(completionFlag),
		_model.heldCard(), stateString().c_str(), _engine->shouldQuit());
	return result;
}

} // End of namespace Ripper
