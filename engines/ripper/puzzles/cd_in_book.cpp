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

#include "ripper/puzzles/cd_in_book.h"

#include "audio/audiostream.h"
#include "audio/decoders/wave.h"
#include "common/debug.h"
#include "common/ptr.h"
#include "common/stream.h"
#include "common/system.h"
#include "graphics/paletteman.h"
#include "graphics/surface.h"

#include "ripper/cursor.h"
#include "ripper/detection.h"
#include "ripper/input.h"
#include "ripper/milestones.h"
#include "ripper/modal_dialog.h"
#include "ripper/ripper.h"
#include "ripper/toolbar.h"

namespace Ripper {

namespace {

struct Position {
	int16 x;
	int16 y;
};

static const uint kButtonCount = 6;
static const uint kFeedbackMarkerCount = 8;
static const uint kSequenceLength = 6;
static const uint kAudioCueCount = 3;
static const uint kDefaultCursor = 14;
static const uint kButtonCursor = 16;
static const uint kExitCursor = 7;
static const int kSceneOriginY = 50;
static const int kSceneHeight = 300;
static const uint kDosTimerTickMillis = 55;
static const uint kFailedAttemptDelayTicks = 4;
static const uint16 kEscapeCommand = 0x1b;
static const uint16 kHelpCommand = 0x3b00;
static const uint kHelpSelectionTable = 0x1aa;

static const char *const kLibraryName = "cdinbook.pl";
static const char *const kBackgroundName = "up.pcx";
static const char *const kAudioNames[kAudioCueCount] = {
	"buton.wav", "butoff.wav", "buzer.wav"
};

// RunCdInBookButtonSequencePuzzleScene at 0x28427 reads these button
// coordinates from the twelve integers at 0x25d29. The stored Y coordinates
// are scene-relative and become physical screen coordinates after adding 50.
static const Position kButtonPositions[kButtonCount] = {
	{315, 57}, {372, 88}, {359, 153},
	{281, 173}, {246, 118}, {254, 66}
};

// The eight dirty-region origins at 0x25d59 identify the P07.BBM feedback
// markers. RIPPER.LE chooses GenerateRandomInt15() & 7 after each of the
// first five button presses, so the same marker may be removed more than once.
static const Position kFeedbackPositions[kFeedbackMarkerCount] = {
	{366, 262}, {372, 264}, {371, 269}, {378, 271},
	{364, 273}, {377, 276}, {356, 276}, {362, 278}
};

static const byte kAnswerSequence[kSequenceLength] = {1, 5, 5, 3, 6, 2};

enum AudioCue {
	kButtonOnCue,
	kButtonOffCue,
	kBuzzerCue
};

} // End of anonymous namespace

CdInBookPuzzle::CdInBookPuzzle(RipperEngine *engine) : Puzzle(engine),
		_random("ripper-cd-in-book-puzzle"), _pressedButton(-1),
		_hoveredButton(-1), _exitPressed(false) {
}

bool CdInBookPuzzle::loadBackground() {
	Common::ScopedPtr<Common::SeekableReadStream> stream(
		_library.createReadStreamForMember(kBackgroundName));
	if (!stream || !decodePcxAsset(*stream, _background)) {
		warning("Ripper: could not decode CD-in-book background '%s'", kBackgroundName);
		return false;
	}

	const uint paletteColors = _background.palette.size() / 3;
	if (_background.width != 640 || _background.height != kSceneHeight ||
			paletteColors < 256) {
		warning("Ripper: invalid CD-in-book background '%s' size=%dx%d colors=%u",
			kBackgroundName, _background.width, _background.height, paletteColors);
		return false;
	}

	_background.palette.resize(256 * 3);
	return true;
}

bool CdInBookPuzzle::loadBitmap(const Common::String &name, BitmapAssetFrame &frame) {
	Common::ScopedPtr<Common::SeekableReadStream> stream(
		_library.createReadStreamForMember(name));
	if (!stream || !decodeBitmapAsset(*stream, frame)) {
		warning("Ripper: could not decode CD-in-book bitmap '%s'", name.c_str());
		return false;
	}
	return true;
}

bool CdInBookPuzzle::loadAssets() {
	Common::ScopedPtr<Common::SeekableReadStream> stream(
		_engine->getResources()->createReadStreamForPath(kLibraryName));
	if (!stream || !_library.open(*stream, Common::Path(kLibraryName))) {
		warning("Ripper: could not open CD-in-book library '%s'", kLibraryName);
		return false;
	}
	if (!loadBackground())
		return false;

	for (uint button = 0; button < kButtonCount; ++button) {
		if (!loadBitmap(Common::String::format("p%02u.bbm", button + 1),
				_buttonFrames[button]))
			return false;
	}
	if (!loadBitmap("p07.bbm", _feedbackFrame))
		return false;
	for (uint cue = 0; cue < kAudioCueCount; ++cue) {
		if (!_library.hasMember(kAudioNames[cue])) {
			warning("Ripper: CD-in-book audio '%s' is missing", kAudioNames[cue]);
			return false;
		}
	}

	debugC(1, kDebugPuzzles,
		"Ripper: loaded CD-in-book assets library='%s' background=%ux%u buttons=%u "
		"feedback=%ux%u audio=%u",
		kLibraryName, _background.width, _background.height, kButtonCount,
		_feedbackFrame.width, _feedbackFrame.height, kAudioCueCount);
	return true;
}

bool CdInBookPuzzle::drawBackground() {
	if (_background.pixels.size() != (uint32)_background.width * _background.height ||
			_background.palette.size() != 256 * 3)
		return false;

	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1 ||
			screen->w < 640 || screen->h < kSceneOriginY + kSceneHeight) {
		if (screen)
			g_system->unlockScreen();
		return false;
	}
	for (uint y = 0; y < _background.height; ++y)
		memcpy(screen->getBasePtr(0, y + kSceneOriginY),
			_background.pixels.data() + y * _background.width, _background.width);
	g_system->unlockScreen();

	Common::Array<byte> palette = _background.palette;
	_engine->applySharedPalettePatch(palette.data(), 256);
	g_system->getPaletteManager()->setPalette(palette.data(), 0, 256);
	presentScreen();
	return true;
}

void CdInBookPuzzle::drawFrame(const BitmapAssetFrame &frame, int x, int y) {
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1) {
		if (screen)
			g_system->unlockScreen();
		return;
	}

	byte *pixels = (byte *)screen->getPixels();
	for (uint sourceY = 0; sourceY < frame.height; ++sourceY) {
		const int destinationY = y + sourceY;
		if (destinationY < 0 || destinationY >= screen->h)
			continue;
		for (uint sourceX = 0; sourceX < frame.width; ++sourceX) {
			const int destinationX = x + sourceX;
			if (destinationX < 0 || destinationX >= screen->w)
				continue;
			const byte pixel = frame.pixels[sourceY * frame.width + sourceX];
			if (pixel != frame.transparentColor)
				pixels[destinationY * screen->pitch + destinationX] = pixel;
		}
	}
	g_system->unlockScreen();
	presentScreen();
}

bool CdInBookPuzzle::captureControls() {
	for (uint marker = 0; marker < kFeedbackMarkerCount; ++marker) {
		const Position &position = kFeedbackPositions[marker];
		if (!_feedbackBackings[marker].capture(Common::Rect(position.x,
				position.y + kSceneOriginY, position.x + _feedbackFrame.width,
				position.y + kSceneOriginY + _feedbackFrame.height)))
			return false;
	}
	drawFeedbackMarkers();

	for (uint button = 0; button < kButtonCount; ++button) {
		const Position &position = kButtonPositions[button];
		const BitmapAssetFrame &frame = _buttonFrames[button];
		if (!_buttonBackings[button].capture(Common::Rect(position.x,
				position.y + kSceneOriginY, position.x + frame.width,
				position.y + kSceneOriginY + frame.height)))
			return false;
	}
	return true;
}

void CdInBookPuzzle::drawFeedbackMarkers() {
	for (uint marker = 0; marker < kFeedbackMarkerCount; ++marker) {
		const Position &position = kFeedbackPositions[marker];
		drawFrame(_feedbackFrame, position.x, position.y + kSceneOriginY);
	}
}

void CdInBookPuzzle::removeFeedbackMarker(uint marker) {
	if (marker >= kFeedbackMarkerCount)
		return;
	_feedbackBackings[marker].restore(false, true);
	debugC(3, kDebugPuzzles,
		"Ripper: CD-in-book removed feedback marker=%u", marker);
}

void CdInBookPuzzle::setButtonPressed(uint button, bool pressed) {
	if (button >= kButtonCount)
		return;
	if (pressed) {
		const Position &position = kButtonPositions[button];
		drawFrame(_buttonFrames[button], position.x, position.y + kSceneOriginY);
	} else {
		_buttonBackings[button].restore(false, true);
	}
}

int CdInBookPuzzle::findButton(const Common::Point &point) const {
	for (uint button = 0; button < kButtonCount; ++button) {
		const Position &position = kButtonPositions[button];
		const BitmapAssetFrame &frame = _buttonFrames[button];
		if (Common::Rect(position.x, position.y + kSceneOriginY,
				position.x + frame.width,
				position.y + kSceneOriginY + frame.height).contains(point))
			return button;
	}
	return -1;
}

bool CdInBookPuzzle::isExitRegion(const Common::Point &point) const {
	return point.y >= kSceneOriginY && point.y < kSceneOriginY + kSceneHeight &&
		(point.x < 140 || point.x >= 500);
}

void CdInBookPuzzle::updateCursor(const Common::Point &point) {
	const int hoveredButton = findButton(point);
	uint cursor = kDefaultCursor;
	if (hoveredButton >= 0)
		cursor = kButtonCursor;
	else if (isExitRegion(point))
		cursor = kExitCursor;

	if (hoveredButton != _hoveredButton) {
		debugC(3, kDebugPuzzles,
			"Ripper: CD-in-book hover button=%d previous=%d point=%d,%d cursor=%u",
			hoveredButton, _hoveredButton, point.x, point.y, cursor);
		_hoveredButton = hoveredButton;
	}
	_engine->getCursor()->update(cursor);
}

bool CdInBookPuzzle::playCue(uint cue) {
	if (cue >= kAudioCueCount)
		return false;
	Common::SeekableReadStream *stream =
		_library.createReadStreamForMember(kAudioNames[cue]);
	if (!stream)
		return false;
	Audio::SeekableAudioStream *wavStream =
		Audio::makeWAVStream(stream, DisposeAfterUse::YES);
	if (!wavStream) {
		warning("Ripper: could not decode CD-in-book audio '%s'", kAudioNames[cue]);
		return false;
	}

	Audio::Mixer *mixer = g_system->getMixer();
	mixer->stopHandle(_audioHandles[cue]);
	mixer->playStream(Audio::Mixer::kSFXSoundType, &_audioHandles[cue], wavStream);
	debugC(3, kDebugPuzzles,
		"Ripper: CD-in-book played audio cue=%u path='%s'", cue, kAudioNames[cue]);
	return true;
}

void CdInBookPuzzle::stopAudio() {
	for (uint cue = 0; cue < kAudioCueCount; ++cue)
		g_system->getMixer()->stopHandle(_audioHandles[cue]);
}

bool CdInBookPuzzle::waitMillis(uint32 duration) {
	const uint32 target = g_system->getMillis() + duration;
	while (!_engine->shouldQuit() && (int32)(g_system->getMillis() - target) < 0) {
		if (!serviceEngineEvents())
			return false;
		g_system->delayMillis(10);
	}
	return !_engine->shouldQuit();
}

bool CdInBookPuzzle::waitForCue(uint cue) {
	if (cue >= kAudioCueCount)
		return false;
	while (!_engine->shouldQuit() &&
			g_system->getMixer()->isSoundHandleActive(_audioHandles[cue])) {
		if (!serviceEngineEvents())
			return false;
		g_system->delayMillis(10);
	}
	return !_engine->shouldQuit();
}

bool CdInBookPuzzle::resetFailedAttempt() {
	if (!waitMillis(kFailedAttemptDelayTicks * kDosTimerTickMillis))
		return false;
	if (!playCue(kBuzzerCue))
		return false;
	drawFeedbackMarkers();
	debugC(2, kDebugPuzzles,
		"Ripper: CD-in-book reset failed six-button attempt markers=%u",
		kFeedbackMarkerCount);
	if (!waitForCue(kBuzzerCue))
		return false;
	_exitPressed = false;
	_engine->getInput()->discardMouseTransitions();
	return true;
}

CdInBookPuzzle::Result CdInBookPuzzle::run(uint completionFlag) {
	// RunCdInBookButtonSequencePuzzleScene at 0x28427 owns CDINBOOK.PL,
	// controls 0x672..0x677, the six-entry answer at 0x25d99, and the supplied
	// completion flag. Escape and either side control leave that flag clear.
	IndexedDisplaySnapshot incomingDisplay;
	if (!incomingDisplay.capture())
		return kLoadFailed;
	if (!loadAssets() || !drawBackground() || !captureControls()) {
		incomingDisplay.restore();
		return kLoadFailed;
	}

	_engine->getInput()->drainKeys();
	_engine->getInput()->discardMouseTransitions();
	_engine->getCursor()->setSelectionIndex(kDefaultCursor);
	_engine->getCursor()->dispatchSelectionIndexChange(kDefaultCursor);
	_engine->getCursor()->refresh();
	debugC(1, kDebugPuzzles,
		"Ripper: entered CD-in-book puzzle completionFlag=%u buttons=%u "
		"sequenceLength=%u feedbackMarkers=%u",
		completionFlag, kButtonCount, kSequenceLength, kFeedbackMarkerCount);

	Result result = kExited;
	uint attemptLength = 0;
	uint correctEntries = 0;
	bool active = true;
	while (active && !_engine->shouldQuit()) {
		if (!serviceEngineEvents())
			break;
		while (_engine->getInput()->hasPendingKey()) {
			const uint16 command = _engine->getInput()->consumeKey();
			if (command == kEscapeCommand) {
				debugC(1, kDebugPuzzles, "Ripper: CD-in-book puzzle exited by Escape");
				active = false;
				break;
			}
			if (command == kHelpCommand) {
				debugC(1, kDebugPuzzles,
					"Ripper: CD-in-book puzzle opening modal help table=0x%x",
					kHelpSelectionTable);
				if (!_engine->getModalDialog()->run(kHelpSelectionTable))
					warning("Ripper: CD-in-book modal help failed");
			}
		}
		if (!active)
			break;

		const MouseState mouse = _engine->getInput()->publishMouseState();
		updateCursor(mouse.position);
		presentScreen();
		if ((mouse.pressed & kMouseButtonLeft) != 0) {
			const int button = findButton(mouse.position);
			if (button >= 0) {
				_pressedButton = button;
				setButtonPressed(button, true);
				playCue(kButtonOnCue);
				debugC(3, kDebugPuzzles,
					"Ripper: CD-in-book pressed button=%u control=0x%x position=%u",
					button + 1, 0x672 + button, attemptLength);
			} else {
				_exitPressed = isExitRegion(mouse.position);
			}
		}
		if ((mouse.released & kMouseButtonLeft) != 0) {
			if (_exitPressed) {
				if (isExitRegion(mouse.position)) {
					debugC(1, kDebugPuzzles,
						"Ripper: CD-in-book puzzle exited by side control point=%d,%d",
						mouse.position.x, mouse.position.y);
					active = false;
				}
				_exitPressed = false;
			}
			if (_pressedButton >= 0) {
				const uint button = _pressedButton;
				setButtonPressed(button, false);
				playCue(kButtonOffCue);
				_pressedButton = -1;
				if (findButton(mouse.position) == (int)button) {
					if (button + 1 == kAnswerSequence[attemptLength])
						++correctEntries;
					++attemptLength;
					debugC(2, kDebugPuzzles,
						"Ripper: CD-in-book accepted button=%u attempt=%u/%u matches=%u",
						button + 1, attemptLength, kSequenceLength, correctEntries);

					if (attemptLength < kSequenceLength) {
						removeFeedbackMarker(_random.getRandomNumber(
							kFeedbackMarkerCount - 1));
					} else if (correctEntries == kSequenceLength) {
						if (!markSolved(completionFlag, "cd-in-book-puzzle")) {
							result = kLoadFailed;
						} else {
							result = kSolved;
							debugC(1, kDebugPuzzles,
								"Ripper: solved CD-in-book puzzle milestone=%u",
								completionFlag);
						}
						active = false;
					} else {
						debugC(2, kDebugPuzzles,
							"Ripper: CD-in-book rejected sequence matches=%u/%u",
							correctEntries, kSequenceLength);
						attemptLength = 0;
						correctEntries = 0;
						if (!resetFailedAttempt()) {
							if (!_engine->shouldQuit())
								result = kLoadFailed;
							active = false;
						}
					}
				}
			}
		}
		g_system->delayMillis(10);
	}

	if (_pressedButton >= 0)
		setButtonPressed(_pressedButton, false);
	stopAudio();
	_engine->getCursor()->update(0);
	_engine->getInput()->discardMouseTransitions();
	debugC(1, kDebugPuzzles,
		"Ripper: left CD-in-book puzzle result=%d milestone=%u attemptLength=%u",
		result, completionFlag, attemptLength);
	return result;
}

} // End of namespace Ripper
