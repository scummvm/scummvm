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

#include "ripper/puzzles/horus_word.h"

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

static const uint kLetterCount = 26;
static const uint kWordLength = 5;
static const uint kAudioCueCount = 3;
static const uint kDefaultCursor = 14;
static const uint kSelectionCursor = 16;
static const uint kLoadingCursor = 19;
static const uint kHelpSelectionTable = 0x1b3;
static const uint16 kEscapeCommand = 0x1b;
static const uint16 kHelpCommand = 0x3b00;
static const uint kDosTickMillis = 55;
static const uint kWordSettleTicks = 0x24;
static const uint kShadingPaletteBytes = 0x300;
static const uint kShadingColorCubeBytes = 0x1000;
static const uint kShadingBytes =
	kShadingPaletteBytes + kShadingColorCubeBytes;
static const int kMaximumPhase = 0x19;
static const int kMinimumPulsePhase = 0x0e;
static const uint kPhaseOpacityStep = 10;
static const byte kPuzzleHelpFutureColor = 254;
static const byte kPuzzleHelpRecommendedColor = 255;
static const uint kPuzzleHelpFutureDitherMask = 7;
static const uint kPuzzleHelpRecommendedDitherMask = 3;
static const int kPuzzleHelpBorderWidth = 2;

static const char *const kLibraryName = "egypt.pl";
static const char *const kBackgroundName = "bkgrnd";
static const char *const kMaskName = "mask";
static const char *const kShadingName = "egypt";
static const char kTargetWord[] = "HORUS";

static const char *const kAudioNames[kAudioCueCount] = {
	"aztec0", "aztec2", "aztec3"
};

// g_horusWordPuzzleLetterControlOrigins at 0x3e82f stores these physical
// X/Y pairs. RunHorusWordPuzzleScene at 0x3f976 cancels the scene's 50-pixel
// presentation origin before creating controls 0x672 through 0x68b.
static const Common::Point kLetterPositions[kLetterCount] = {
	Common::Point(148, 136), Common::Point(146, 94),
	Common::Point(146, 51), Common::Point(181, 6),
	Common::Point(221, 7), Common::Point(257, 8),
	Common::Point(295, 9), Common::Point(333, 8),
	Common::Point(369, 8), Common::Point(406, 8),
	Common::Point(442, 51), Common::Point(441, 93),
	Common::Point(440, 136), Common::Point(439, 177),
	Common::Point(439, 218), Common::Point(437, 258),
	Common::Point(402, 299), Common::Point(367, 299),
	Common::Point(331, 299), Common::Point(295, 299),
	Common::Point(260, 299), Common::Point(224, 299),
	Common::Point(188, 299), Common::Point(150, 257),
	Common::Point(152, 218), Common::Point(149, 176)
};

// g_astHorusFallingLetterStates at 0x84dbc stores the five final letter
// anchors. Retail applies EGYPT.RTP at phase * 10 over this display region.
static const Common::Point kWordPositions[kWordLength] = {
	Common::Point(228, 347), Common::Point(264, 347),
	Common::Point(299, 347), Common::Point(334, 347),
	Common::Point(369, 347)
};

} // End of anonymous namespace

HorusWordPuzzle::DebugHelper::DebugHelper() :
		_enabled(false), _recommendedLetter(-1) {
}

void HorusWordPuzzle::DebugHelper::reset(bool enabled) {
	_enabled = enabled;
	_recommendedLetter = -1;
}

bool HorusWordPuzzle::DebugHelper::sync(
		const HorusWordPuzzle &puzzle) {
	const bool enabled = puzzle._engine->isPuzzleHelpEnabled();
	if (enabled == _enabled)
		return false;

	_enabled = enabled;
	stateChanged(puzzle);
	debugC(2, kDebugPuzzles,
		"Ripper: Horus word puzzle help enabled=%d recommendedLetter='%c' "
		"futureColor=%u recommendedColor=%u command=PUZZLE_HELP",
		_enabled, _recommendedLetter >= 0 ? 'A' + _recommendedLetter : '-',
		kPuzzleHelpFutureColor, kPuzzleHelpRecommendedColor);
	return true;
}

int HorusWordPuzzle::DebugHelper::findRecommendedLetter(
		const HorusWordPuzzle &puzzle) const {
	if (!_enabled || puzzle._exitRequested || puzzle._wordSolved)
		return -1;

	uint slot = puzzle._selectedLetters.size();
	if (puzzle._wordComplete)
		slot = 0;
	if (slot >= kWordLength)
		return -1;
	const int letter = kTargetWord[slot] - 'A';
	if (!puzzle.isLetterSelected(letter))
		return letter;
	if (puzzle._wordComplete)
		return -1;
	for (uint targetSlot = 0; targetSlot < kWordLength; ++targetSlot) {
		const int recoveryLetter = kTargetWord[targetSlot] - 'A';
		if (!puzzle.isLetterSelected(recoveryLetter))
			return recoveryLetter;
	}
	return -1;
}

void HorusWordPuzzle::DebugHelper::stateChanged(
		const HorusWordPuzzle &puzzle) {
	const int recommendedLetter = findRecommendedLetter(puzzle);
	if (recommendedLetter == _recommendedLetter)
		return;

	_recommendedLetter = recommendedLetter;
	debugC(2, kDebugPuzzles,
		"Ripper: Horus word puzzle help analyzed word='%s' "
		"recommendedLetter='%c' target='%s'",
		puzzle.selectedWord().c_str(),
		_recommendedLetter >= 0 ? 'A' + _recommendedLetter : '-',
		kTargetWord);
}

void HorusWordPuzzle::DebugHelper::draw(
		const HorusWordPuzzle &puzzle, byte *screen, uint pitch) const {
	if (!_enabled || puzzle._exitRequested || puzzle._wordSolved)
		return;

	for (uint slot = 0; slot < kWordLength; ++slot) {
		const int letter = kTargetWord[slot] - 'A';
		if (puzzle.isLetterSelected(letter))
			continue;
		const bool recommended = letter == _recommendedLetter;
		const byte color = recommended ? kPuzzleHelpRecommendedColor :
			kPuzzleHelpFutureColor;
		const uint ditherMask = recommended ?
			kPuzzleHelpRecommendedDitherMask : kPuzzleHelpFutureDitherMask;
		const Common::Point &position = kLetterPositions[letter];
		const BitmapAssetFrame &button = puzzle._buttons[letter];
		const int left = MAX<int>(position.x, 0);
		const int top = MAX<int>(position.y, 0);
		const int right = MIN<int>(position.x + button.width,
			kRipperScreenWidth);
		const int bottom = MIN<int>(position.y + button.height,
			kRipperScreenHeight);
		for (int y = top; y < bottom; ++y) {
			for (int x = left; x < right; ++x) {
				const bool border = x - left < kPuzzleHelpBorderWidth ||
					right - x <= kPuzzleHelpBorderWidth ||
					y - top < kPuzzleHelpBorderWidth ||
					bottom - y <= kPuzzleHelpBorderWidth;
				if (border || ((x + y) & ditherMask) == 0)
					screen[y * pitch + x] = color;
			}
		}
	}
}

HorusWordPuzzle::HorusWordPuzzle(RipperEngine *engine) :
		Puzzle(engine), _hoveredLetter(-1), _lastAnimationTick(0),
		_settleDeadline(0), _wordComplete(false), _wordSolved(false),
		_exitRequested(false) {
}

bool HorusWordPuzzle::loadBitmap(const Common::String &name,
		BitmapAssetFrame &frame) {
	Common::ScopedPtr<Common::SeekableReadStream> stream(
		_library.createReadStreamForMember(name));
	if (!stream || !decodeBitmapAsset(*stream, frame)) {
		warning("Ripper: could not decode Horus word bitmap '%s'",
			name.c_str());
		return false;
	}
	return true;
}

bool HorusWordPuzzle::loadPcx(const Common::String &name,
		BitmapAssetFrame &frame) {
	Common::ScopedPtr<Common::SeekableReadStream> stream(
		_library.createReadStreamForMember(name));
	if (!stream || !decodePcxAsset(*stream, frame)) {
		warning("Ripper: could not decode Horus word PCX '%s'",
			name.c_str());
		return false;
	}
	return true;
}

bool HorusWordPuzzle::loadAssets() {
	Common::ScopedPtr<Common::SeekableReadStream> stream(
		_engine->getResources()->createReadStreamForPath(kLibraryName));
	if (!stream || !_library.open(*stream, Common::Path(kLibraryName))) {
		warning("Ripper: could not open Horus word library '%s'",
			kLibraryName);
		return false;
	}
	if (!loadBitmap(kBackgroundName, _background) ||
			!loadBitmap(kMaskName, _mask))
		return false;
	if (_background.width != kRipperScreenWidth ||
			_background.height != kRipperScreenHeight ||
			_background.palette.size() < kRipperPaletteByteCount ||
			_mask.width != 189 || _mask.height != 50) {
		warning("Ripper: Horus word assets have invalid geometry "
			"background=%ux%u colors=%u mask=%ux%u",
			_background.width, _background.height,
			_background.palette.size() / 3, _mask.width, _mask.height);
		return false;
	}

	for (uint letter = 0; letter < kLetterCount; ++letter) {
		if (!loadBitmap(Common::String::format("but%u", letter + 1),
				_buttons[letter]) ||
				!loadPcx(Common::String::format("%c", 'a' + letter),
				_letters[letter]))
			return false;
	}

	Common::ScopedPtr<Common::SeekableReadStream> shadingStream(
		_library.createReadStreamForMember(kShadingName));
	if (!shadingStream || shadingStream->size() != kShadingBytes) {
		warning("Ripper: Horus word shading '%s' has invalid size "
			"expected=%u actual=%lld", kShadingName, kShadingBytes,
			shadingStream ? (long long)shadingStream->size() : -1LL);
		return false;
	}
	_shading.resize(kShadingBytes);
	if (shadingStream->read(_shading.data(), _shading.size()) !=
			_shading.size()) {
		warning("Ripper: could not read Horus word shading '%s'",
			kShadingName);
		return false;
	}

	for (uint cue = 0; cue < kAudioCueCount; ++cue) {
		if (!_library.hasMember(kAudioNames[cue])) {
			warning("Ripper: Horus word audio '%s' is missing",
				kAudioNames[cue]);
			return false;
		}
	}

	debugC(2, kDebugPuzzles,
		"Ripper: loaded Horus word assets library='%s' entries=%u "
		"background=%ux%u controls=%u letters=%u mask=%ux%u "
		"shadingBytes=%u audio=%u",
		kLibraryName, _library.getEntryCount(), _background.width,
		_background.height, kLetterCount, kLetterCount, _mask.width,
		_mask.height, _shading.size(), kAudioCueCount);
	return true;
}

void HorusWordPuzzle::applyPalette() const {
	Common::Array<byte> palette = _background.palette;
	_engine->applySharedPalettePatch(palette.data(),
		kRipperPaletteColorCount);
	_engine->getSettings()->applyVideoPalette(palette.data(),
		kRipperPaletteColorCount, true);
	g_system->getPaletteManager()->setPalette(palette.data(), 0,
		kRipperPaletteColorCount);
}

byte HorusWordPuzzle::blendPixel(byte source, byte destination,
		uint opacity) const {
	const byte *palette = _shading.data();
	const byte *colorCube = palette + kShadingPaletteBytes;
	const uint inverseOpacity = 256 - MIN<uint>(opacity, 255);
	uint components[3];
	for (uint component = 0; component < ARRAYSIZE(components); ++component) {
		components[component] =
			(palette[source * 3 + component] * opacity +
			 palette[destination * 3 + component] * inverseOpacity) >> 8;
	}
	const uint colorCubeIndex = ((components[0] >> 2) << 8) |
		((components[1] >> 2) << 4) | (components[2] >> 2);
	return colorCube[colorCubeIndex];
}

void HorusWordPuzzle::drawFrame(byte *screen, uint pitch,
		const BitmapAssetFrame &frame, int x, int y, int opacity) const {
	if (opacity == 0)
		return;
	for (uint sourceY = 0; sourceY < frame.height; ++sourceY) {
		const int destinationY = y + sourceY;
		if (destinationY < 0 || destinationY >= kRipperScreenHeight)
			continue;
		for (uint sourceX = 0; sourceX < frame.width; ++sourceX) {
			const int destinationX = x + sourceX;
			if (destinationX < 0 || destinationX >= kRipperScreenWidth)
				continue;
			const byte pixel = frame.pixels[sourceY * frame.width + sourceX];
			if (pixel == frame.transparentColor)
				continue;
			byte &destination =
				screen[destinationY * pitch + destinationX];
			destination = opacity < 0 ? pixel :
				blendPixel(pixel, destination, opacity);
		}
	}
}

void HorusWordPuzzle::render() const {
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1) {
		if (screen)
			g_system->unlockScreen();
		return;
	}

	byte *pixels = (byte *)screen->getPixels();
	for (uint y = 0; y < _background.height; ++y) {
		memcpy(screen->getBasePtr(0, y),
			_background.pixels.data() + y * _background.width,
			_background.width);
	}
	if (_hoveredLetter >= 0 &&
			!isLetterSelected((uint)_hoveredLetter)) {
		drawFrame(pixels, screen->pitch, _buttons[_hoveredLetter],
			kLetterPositions[_hoveredLetter].x,
			kLetterPositions[_hoveredLetter].y);
	}
	for (uint slot = 0; slot < _selectedLetters.size(); ++slot) {
		if (_selectedLetters[slot] < 0 || _letterPhases[slot] < 0)
			continue;
		const uint opacity = MIN<uint>(
			_letterPhases[slot] * kPhaseOpacityStep, 255);
		drawFrame(pixels, screen->pitch,
			_letters[_selectedLetters[slot]], kWordPositions[slot].x,
			kWordPositions[slot].y, opacity);
	}
	_debugHelper.draw(*this, pixels, screen->pitch);

	g_system->unlockScreen();
	applyPalette();
	presentScreen();
}

bool HorusWordPuzzle::isLetterSelected(uint letter) const {
	for (uint slot = 0; slot < _selectedLetters.size(); ++slot) {
		if (_selectedLetters[slot] == (int)letter &&
				_letterPhases[slot] >= 0)
			return true;
	}
	return false;
}

int HorusWordPuzzle::findLetter(const Common::Point &point) const {
	for (uint letter = 0; letter < kLetterCount; ++letter) {
		if (isLetterSelected(letter))
			continue;
		const Common::Point &position = kLetterPositions[letter];
		const BitmapAssetFrame &button = _buttons[letter];
		if (Common::Rect(position.x, position.y,
				position.x + button.width,
				position.y + button.height).contains(point))
			return letter;
	}
	return -1;
}

void HorusWordPuzzle::updateHover(const Common::Point &point) {
	const int hovered = _exitRequested || _wordSolved ? -1 :
		findLetter(point);
	if (hovered == _hoveredLetter)
		return;
	debugC(2, kDebugPuzzles,
		"Ripper: Horus word hover letter=%d previous=%d point=%d,%d",
		hovered, _hoveredLetter, point.x, point.y);
	_hoveredLetter = hovered;
	_engine->getCursor()->update(
		hovered >= 0 ? kSelectionCursor : kDefaultCursor);
	render();
}

bool HorusWordPuzzle::playCue(uint cue, uint volume, bool loop) {
	if (cue >= kAudioCueCount)
		return false;
	Common::SeekableReadStream *stream =
		_library.createReadStreamForMember(kAudioNames[cue]);
	const bool result = _engine->getMedia()->playSoundEffectStream(
		stream, kAudioNames[cue], _audioHandles[cue], volume, loop);
	debugC(result ? 3 : 2, kDebugPuzzles,
		"Ripper: Horus word audio cue=%u name='%s' volume=%u "
		"loop=%d success=%d", cue, kAudioNames[cue], volume, loop,
		result);
	return result;
}

void HorusWordPuzzle::stopAudio() {
	stopAudioHandles(_audioHandles, ARRAYSIZE(_audioHandles));
}

Common::String HorusWordPuzzle::selectedWord() const {
	Common::String word;
	for (uint slot = 0; slot < _selectedLetters.size(); ++slot) {
		if (_selectedLetters[slot] >= 0)
			word += (char)('A' + _selectedLetters[slot]);
	}
	return word;
}

void HorusWordPuzzle::clearWord(const char *reason, bool playResetCue) {
	debugC(2, kDebugPuzzles,
		"Ripper: Horus word cleared reason='%s' previous='%s'",
		reason, selectedWord().c_str());
	_selectedLetters.clear();
	_letterPhases.clear();
	_letterTargets.clear();
	_wordComplete = false;
	_wordSolved = false;
	_settleDeadline = 0;
	_engine->getMedia()->stopSoundEffect(_audioHandles[0]);
	if (playResetCue)
		playCue(2);
	_debugHelper.stateChanged(*this);
}

void HorusWordPuzzle::beginFade(const char *reason) {
	debugC(2, kDebugPuzzles,
		"Ripper: Horus word fade started reason='%s' word='%s'",
		reason, selectedWord().c_str());
	for (uint slot = 0; slot < _letterTargets.size(); ++slot)
		_letterTargets[slot] = -1;
	_settleDeadline = 0;
	_engine->getMedia()->stopSoundEffect(_audioHandles[0]);
}

void HorusWordPuzzle::selectLetter(uint letter) {
	if (letter >= kLetterCount || isLetterSelected(letter) ||
			_wordSolved || _exitRequested)
		return;
	if (_wordComplete)
		clearWord("new-attempt");
	if (_selectedLetters.empty())
		playCue(0, 50, true);

	_selectedLetters.push_back(letter);
	_letterPhases.push_back(0);
	_letterTargets.push_back(kMaximumPhase);
	playCue(1);
	debugC(2, kDebugPuzzles,
		"Ripper: Horus word selected letter='%c' index=%u slot=%u/%u "
		"word='%s'", 'A' + letter, letter, _selectedLetters.size(),
		kWordLength, selectedWord().c_str());

	if (_selectedLetters.size() == kWordLength) {
		_wordComplete = true;
		_wordSolved = selectedWord() == kTargetWord;
		_settleDeadline = g_system->getMillis() +
			kWordSettleTicks * kDosTickMillis;
		debugC(_wordSolved ? 1 : 2, kDebugPuzzles,
			"Ripper: Horus word validation entered='%s' outcome=%s "
			"settleTicks=%u", selectedWord().c_str(),
			_wordSolved ? "SOLVED" : "REJECTED", kWordSettleTicks);
	}
	_debugHelper.stateChanged(*this);
	_hoveredLetter = -1;
	_engine->getCursor()->update(kDefaultCursor);
	render();
}

bool HorusWordPuzzle::hasVisibleLetters() const {
	for (uint slot = 0; slot < _letterPhases.size(); ++slot) {
		if (_letterPhases[slot] >= 0)
			return true;
	}
	return false;
}

bool HorusWordPuzzle::stepAnimation(uint32 now) {
	bool changed = false;
	while ((int32)(now - _lastAnimationTick) >= (int32)kDosTickMillis) {
		_lastAnimationTick += kDosTickMillis;
		for (uint slot = 0; slot < _letterPhases.size(); ++slot) {
			int &phase = _letterPhases[slot];
			int &target = _letterTargets[slot];
			if (phase < 0)
				continue;
			if (phase < target) {
				++phase;
			} else if (phase > target) {
				--phase;
				if (phase < 0) {
					playCue(2);
					_debugHelper.stateChanged(*this);
				}
			} else if (target == kMaximumPhase) {
				target = kMinimumPulsePhase;
			} else if (target == kMinimumPulsePhase) {
				target = kMaximumPhase;
			}
			changed = true;
		}
	}

	if (_settleDeadline != 0 &&
			(int32)(now - _settleDeadline) >= 0) {
		if (_wordSolved) {
			_settleDeadline = 0;
		} else {
			beginFade("rejected-timeout");
		}
	}

	if (!_wordSolved && !_exitRequested && _wordComplete &&
			!hasVisibleLetters()) {
		clearWord("rejected-fade-complete", false);
		changed = true;
	}
	if (changed) {
		debugC(3, kDebugPuzzles,
			"Ripper: Horus word animation word='%s' visible=%d "
			"phases=[%d,%d,%d,%d,%d]",
			selectedWord().c_str(), hasVisibleLetters(),
			_letterPhases.size() > 0 ? _letterPhases[0] : -1,
			_letterPhases.size() > 1 ? _letterPhases[1] : -1,
			_letterPhases.size() > 2 ? _letterPhases[2] : -1,
			_letterPhases.size() > 3 ? _letterPhases[3] : -1,
			_letterPhases.size() > 4 ? _letterPhases[4] : -1);
	}
	return changed;
}

HorusWordPuzzle::Result HorusWordPuzzle::run(uint completionFlag) {
	// DispatchSceneEntryAction at 0x36892 routes action 20 to
	// RunHorusWordPuzzleScene at 0x3f976 with the completion flag in EAX.
	if (!_incomingDisplay.capture())
		return kLoadFailed;

	_engine->getToolbar()->leave();
	_engine->getInput()->drainKeys();
	_engine->getInput()->discardMouseTransitions();
	_engine->getCursor()->setSelectionIndex(kLoadingCursor);
	_engine->getCursor()->dispatchSelectionIndexChange(kLoadingCursor);
	_engine->getCursor()->setVisible(true);
	if (!loadAssets()) {
		_incomingDisplay.restore();
		return kLoadFailed;
	}

	_selectedLetters.clear();
	_letterPhases.clear();
	_letterTargets.clear();
	_hoveredLetter = -1;
	_settleDeadline = 0;
	_wordComplete = false;
	_wordSolved = false;
	_exitRequested = false;
	_lastAnimationTick = g_system->getMillis();
	_debugHelper.reset(_engine->isPuzzleHelpEnabled());
	_debugHelper.stateChanged(*this);
	_engine->getCursor()->setSelectionIndex(kDefaultCursor);
	_engine->getCursor()->dispatchSelectionIndexChange(kDefaultCursor);
	render();
	debugC(1, kDebugPuzzles,
		"Ripper: entered Horus word puzzle function="
		"RunHorusWordPuzzleScene@0x3f976 milestone=%u library='%s' "
		"controls=%u slots=%u help=0x%x puzzleHelp=%d",
		completionFlag, kLibraryName, kLetterCount, kWordLength,
		kHelpSelectionTable, _engine->isPuzzleHelpEnabled());
	debugC(3, kDebugPuzzles,
		"Ripper: Horus word retail target='%s' controls=0x672..0x68b",
		kTargetWord);

	Result result = kExited;
	bool active = true;
	while (active && !_engine->shouldQuit()) {
		if (!serviceEngineEvents())
			break;

		while (_engine->getInput()->hasPendingKey()) {
			const uint16 command = _engine->getInput()->consumeKey();
			if (command == kEscapeCommand && !_exitRequested) {
				_exitRequested = true;
				_debugHelper.stateChanged(*this);
				_hoveredLetter = -1;
				_engine->getCursor()->update(kDefaultCursor);
				beginFade("escape");
				debugC(1, kDebugPuzzles,
					"Ripper: Horus word puzzle exit requested by Escape");
				break;
			}
			if (command == kHelpCommand && !_exitRequested &&
					!_wordSolved) {
				debugC(1, kDebugPuzzles,
					"Ripper: Horus word puzzle opening modal help table=0x%x",
					kHelpSelectionTable);
				if (!_engine->getModalDialog()->run(kHelpSelectionTable))
					warning("Ripper: Horus word modal help failed");
				render();
			}
		}
		if (_debugHelper.sync(*this))
			render();

		const uint32 now = g_system->getMillis();
		if (stepAnimation(now))
			render();
		if (_wordSolved && _settleDeadline == 0) {
			if (!_engine->getMilestones()->set(completionFlag, true,
					"horus-word-puzzle")) {
				result = kLoadFailed;
			} else {
				result = kSolved;
			}
			active = false;
			continue;
		}
		if (_exitRequested) {
			if (!hasVisibleLetters())
				active = false;
		} else if (!_wordSolved) {
			const MouseState mouse =
				_engine->getInput()->publishMouseState();
			updateHover(mouse.position);
			if ((mouse.pressed & kMouseButtonLeft) != 0 &&
					_hoveredLetter >= 0)
				selectLetter(_hoveredLetter);
		}

		presentScreen();
		g_system->delayMillis(10);
	}

	stopAudio();
	_engine->getCursor()->setSelectionIndex(0);
	_engine->getCursor()->dispatchSelectionIndexChange(0);
	_engine->getCursor()->refresh();
	_engine->getCursor()->setVisible(true);
	_incomingDisplay.restore();
	_engine->getInput()->drainKeys();
	_engine->getInput()->discardMouseTransitions();
	debugC(result == kLoadFailed ? 2 : 1, kDebugPuzzles,
		"Ripper: left Horus word puzzle result=%d milestone=%u "
		"milestoneSet=%d word='%s' exitRequested=%d quit=%d",
		result, completionFlag,
		_engine->getMilestones()->isSet(completionFlag),
		selectedWord().c_str(), _exitRequested, _engine->shouldQuit());
	return result;
}

} // End of namespace Ripper
