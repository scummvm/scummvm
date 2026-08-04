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

#include "ripper/puzzles/eight_button_sequence.h"

#include "common/debug.h"
#include "common/ptr.h"
#include "common/stream.h"
#include "common/system.h"
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

static const uint kButtonCount = 8;
static const uint kSequenceLength = 6;
static const uint kAudioCueCount = 5;
static const int kSceneOriginY = 50;
static const int kSceneHeight = 300;
static const uint kDefaultCursor = 14;
static const uint kSelectionCursor = 16;
static const uint kExitCursor = 7;
static const uint kHelpSelectionTable = 0x1b2;
static const uint16 kEscapeCommand = 0x1b;
static const uint16 kHelpCommand = 0x3b00;
static const uint16 kSolvedCommand = 1;
static const uint16 kLoadFailedCommand = 2;
static const uint kDosTickMillis = 55;

static const char *const kBackgroundMedia = "xarmed.smk";
static const Common::Point kBackgroundPosition(248, 204);
static const Common::Point kSuccessDarkPosition(276, 241 + kSceneOriginY);

// g_astEightButtonInputPositions at 0x84e31 stores scene-space Y/X pairs.
static const Common::Point kButtonPositions[kButtonCount] = {
	Common::Point(230, 107 + kSceneOriginY),
	Common::Point(285, 107 + kSceneOriginY),
	Common::Point(330, 107 + kSceneOriginY),
	Common::Point(373, 107 + kSceneOriginY),
	Common::Point(239, 156 + kSceneOriginY),
	Common::Point(285, 155 + kSceneOriginY),
	Common::Point(329, 153 + kSceneOriginY),
	Common::Point(373, 152 + kSceneOriginY)
};

// g_astEightButtonFeedbackDirtyRegions at 0x84e51 also stores Y/X pairs.
static const Common::Point kFeedbackPositions[kSequenceLength] = {
	Common::Point(237, 40 + kSceneOriginY),
	Common::Point(265, 40 + kSceneOriginY),
	Common::Point(295, 40 + kSceneOriginY),
	Common::Point(325, 40 + kSceneOriginY),
	Common::Point(353, 40 + kSceneOriginY),
	Common::Point(386, 43 + kSceneOriginY)
};

// RunEightButtonSequencePuzzleScene at 0x4043d captures this exact region
// before drawing entry markers, then uses that backing to clear the row.
static Common::Rect feedbackBounds() {
	return Common::Rect(233, 36 + kSceneOriginY,
		233 + 180, 36 + kSceneOriginY + 30);
}

// g_awEightButtonSolution at 0x84e25.
static const byte kSolution[kSequenceLength] = {2, 2, 6, 4, 8, 1};

static const char *const kFeedbackNames[kSequenceLength] = {
	"dkp1.bbm", "dkp2.bbm", "dkp3.bbm",
	"dkp4.bbm", "dkp5.bbm", "dkpo.bbm"
};

static bool isExitPoint(const Common::Point &point) {
	return Common::Rect(0, kSceneOriginY, 100,
		kSceneOriginY + kSceneHeight).contains(point);
}

} // End of anonymous namespace

EightButtonSequencePuzzle::EightButtonSequencePuzzle(RipperEngine *engine) :
		_engine(engine), _completionFlag(0), _hoveredButton(-1),
		_result(kExited) {
}

bool EightButtonSequencePuzzle::loadFrame(const Common::String &path,
		BitmapAssetFrame &frame) {
	BitmapAssetSequence sequence;
	if (!_engine->getResources()->loadBitmapSequence(path, sequence) ||
			sequence.frames.empty()) {
		warning("Ripper: could not decode eight-button bitmap '%s'",
			path.c_str());
		return false;
	}
	frame = Common::move(sequence.frames.front());
	return true;
}

bool EightButtonSequencePuzzle::loadAssets() {
	Common::ScopedPtr<Common::SeekableReadStream> background(
		_engine->getResources()->createReadStreamForPath(kBackgroundMedia));
	if (!background) {
		warning("Ripper: eight-button background media '%s' is missing",
			kBackgroundMedia);
		return false;
	}

	if (!loadFrame("xdark.bbm", _successDark))
		return false;

	_feedbackFrames.clear();
	for (uint slot = 0; slot < kSequenceLength; ++slot) {
		BitmapAssetFrame frame;
		if (!loadFrame(kFeedbackNames[slot], frame))
			return false;
		_feedbackFrames.push_back(Common::move(frame));
	}

	_buttonFrames.clear();
	for (uint button = 0; button < kButtonCount; ++button) {
		BitmapAssetFrame frame;
		if (!loadFrame(Common::String::format("xkp%u.bbm", button), frame))
			return false;
		_buttonFrames.push_back(Common::move(frame));
	}

	for (uint cue = 0; cue < kAudioCueCount; ++cue) {
		const Common::String path =
			Common::String::format("bnkwav%u.wav", cue);
		Common::ScopedPtr<Common::SeekableReadStream> stream(
			_engine->getResources()->createReadStreamForPath(path));
		if (!stream) {
			warning("Ripper: eight-button audio '%s' is missing", path.c_str());
			return false;
		}
	}

	debugC(1, kDebugPuzzles,
		"Ripper: loaded eight-button sequence assets background='%s' "
		"buttons=%u feedback=%u success=%ux%u audio=%u",
		kBackgroundMedia, _buttonFrames.size(), _feedbackFrames.size(),
		_successDark.width, _successDark.height, kAudioCueCount);
	return true;
}

bool EightButtonSequencePuzzle::captureRegion(const Common::Rect &bounds,
		Common::Array<byte> &pixels) {
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1 || bounds.left < 0 ||
			bounds.top < 0 || bounds.right > screen->w ||
			bounds.bottom > screen->h) {
		if (screen)
			g_system->unlockScreen();
		return false;
	}

	pixels.resize(bounds.width() * bounds.height());
	for (int y = 0; y < bounds.height(); ++y) {
		memcpy(pixels.data() + y * bounds.width(),
			screen->getBasePtr(bounds.left, bounds.top + y), bounds.width());
	}
	g_system->unlockScreen();
	return true;
}

bool EightButtonSequencePuzzle::captureBackings() {
	_buttonBackings.clear();
	_buttonBackings.resize(kButtonCount);
	for (uint button = 0; button < kButtonCount; ++button) {
		const BitmapAssetFrame &frame = _buttonFrames[button];
		const Common::Point &position = kButtonPositions[button];
		const Common::Rect bounds(position.x, position.y,
			position.x + frame.width, position.y + frame.height);
		if (!captureRegion(bounds, _buttonBackings[button]))
			return false;
	}
	return captureRegion(feedbackBounds(), _feedbackBacking);
}

void EightButtonSequencePuzzle::restoreRegion(byte *screen, uint pitch,
		const Common::Rect &bounds, const Common::Array<byte> &pixels) const {
	if (pixels.size() != (uint)(bounds.width() * bounds.height()))
		return;
	for (int y = 0; y < bounds.height(); ++y) {
		memcpy(screen + (bounds.top + y) * pitch + bounds.left,
			pixels.data() + y * bounds.width(), bounds.width());
	}
}

void EightButtonSequencePuzzle::drawFrame(byte *screen, uint pitch,
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

int EightButtonSequencePuzzle::findButton(const Common::Point &point) const {
	for (uint button = 0; button < _buttonFrames.size(); ++button) {
		const BitmapAssetFrame &frame = _buttonFrames[button];
		const Common::Point &position = kButtonPositions[button];
		if (Common::Rect(position.x, position.y,
				position.x + frame.width,
				position.y + frame.height).contains(point))
			return button;
	}
	return -1;
}

bool EightButtonSequencePuzzle::playCue(uint cue) {
	if (cue >= kAudioCueCount)
		return false;
	const Common::String path = Common::String::format("bnkwav%u.wav", cue);
	const bool result = _engine->getMedia()->playSoundEffect(
		path, _audioHandles[cue]);
	debugC(result ? 3 : 2, kDebugPuzzles,
		"Ripper: eight-button sequence audio cue=%u path='%s' success=%d",
		cue, path.c_str(), result);
	return result;
}

void EightButtonSequencePuzzle::updateHover(const Common::Point &point) {
	const int button = findButton(point);
	if (button == _hoveredButton) {
		uint cursor = kDefaultCursor;
		if (button >= 0)
			cursor = kSelectionCursor;
		else if (isExitPoint(point))
			cursor = kExitCursor;
		_engine->getCursor()->update(cursor);
		return;
	}

	Graphics::Surface *screen = g_system->lockScreen();
	if (screen && screen->format.bytesPerPixel == 1) {
		byte *pixels = (byte *)screen->getPixels();
		if (_hoveredButton >= 0) {
			const BitmapAssetFrame &oldFrame = _buttonFrames[_hoveredButton];
			const Common::Point &oldPosition = kButtonPositions[_hoveredButton];
			restoreRegion(pixels, screen->pitch,
				Common::Rect(oldPosition.x, oldPosition.y,
					oldPosition.x + oldFrame.width,
					oldPosition.y + oldFrame.height),
				_buttonBackings[_hoveredButton]);
		}
		if (button >= 0) {
			const Common::Point &position = kButtonPositions[button];
			drawFrame(pixels, screen->pitch, _buttonFrames[button],
				position.x, position.y);
		}
		g_system->unlockScreen();
		g_system->updateScreen();
	} else if (screen) {
		g_system->unlockScreen();
	}

	if (_hoveredButton >= 0)
		playCue(1);
	if (button >= 0)
		playCue(0);
	debugC(2, kDebugPuzzles,
		"Ripper: eight-button sequence hover button=%d previous=%d point=%d,%d",
		button >= 0 ? button + 1 : 0,
		_hoveredButton >= 0 ? _hoveredButton + 1 : 0,
		point.x, point.y);
	_hoveredButton = button;

	uint cursor = kDefaultCursor;
	if (button >= 0)
		cursor = kSelectionCursor;
	else if (isExitPoint(point))
		cursor = kExitCursor;
	_engine->getCursor()->update(cursor);
}

void EightButtonSequencePuzzle::drawFeedback(bool visible) {
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1) {
		if (screen)
			g_system->unlockScreen();
		return;
	}

	byte *pixels = (byte *)screen->getPixels();
	restoreRegion(pixels, screen->pitch, feedbackBounds(), _feedbackBacking);
	if (visible) {
		for (uint slot = 0; slot < _enteredButtons.size() &&
				slot < _feedbackFrames.size(); ++slot) {
			drawFrame(pixels, screen->pitch, _feedbackFrames[slot],
				kFeedbackPositions[slot].x, kFeedbackPositions[slot].y);
		}
	}
	g_system->unlockScreen();
	g_system->updateScreen();
}

void EightButtonSequencePuzzle::drawSuccessDark() {
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1) {
		if (screen)
			g_system->unlockScreen();
		return;
	}
	drawFrame((byte *)screen->getPixels(), screen->pitch, _successDark,
		kSuccessDarkPosition.x, kSuccessDarkPosition.y);
	g_system->unlockScreen();
	g_system->updateScreen();
}

bool EightButtonSequencePuzzle::waitTicks(uint ticks) {
	const uint32 target = g_system->getMillis() + ticks * kDosTickMillis;
	while (!_engine->shouldQuit() &&
			(int32)(target - g_system->getMillis()) > 0) {
		if (_engine->getInput()->pollEvents()) {
			_engine->quitGame();
			return false;
		}
		g_system->delayMillis(10);
	}
	return !_engine->shouldQuit();
}

Common::String EightButtonSequencePuzzle::enteredSequence() const {
	Common::String result;
	for (uint index = 0; index < _enteredButtons.size(); ++index)
		result += (char)('0' + _enteredButtons[index]);
	return result;
}

uint16 EightButtonSequencePuzzle::pressButton(uint button) {
	if (button >= kButtonCount || _enteredButtons.size() >= kSequenceLength)
		return 0;

	_enteredButtons.push_back(button + 1);
	drawFeedback(true);
	debugC(2, kDebugPuzzles,
		"Ripper: eight-button sequence input button=%u entered='%s' count=%u/%u",
		button + 1, enteredSequence().c_str(), _enteredButtons.size(),
		kSequenceLength);
	if (_enteredButtons.size() != kSequenceLength)
		return 0;
	if (!waitTicks(2))
		return kEscapeCommand;

	bool solved = true;
	for (uint index = 0; index < kSequenceLength; ++index) {
		if (_enteredButtons[index] != kSolution[index]) {
			solved = false;
			break;
		}
	}
	debugC(solved ? 1 : 2, kDebugPuzzles,
		"Ripper: eight-button sequence validation entered='%s' outcome=%s",
		enteredSequence().c_str(), solved ? "SOLVED" : "REJECTED");
	if (!solved) {
		drawFeedback(false);
		playCue(3);
		_enteredButtons.clear();
		return 0;
	}

	playCue(2);
	drawSuccessDark();
	for (uint phase = 0; phase < 5; ++phase) {
		if (!waitTicks(5))
			return kEscapeCommand;
		drawFeedback((phase & 1) != 0);
		debugC(3, kDebugPuzzles,
			"Ripper: eight-button sequence success flash phase=%u visible=%d",
			phase, (phase & 1) != 0);
	}
	if (!_engine->getMilestones()->set(
			_completionFlag, true, "eight-button-sequence-puzzle")) {
		_result = kLoadFailed;
		return kLoadFailedCommand;
	}
	_result = kSolved;
	return kSolvedCommand;
}

uint16 EightButtonSequencePuzzle::service(uint frame) {
	if (_engine->getInput()->pollEvents()) {
		_engine->quitGame();
		return kEscapeCommand;
	}

	while (_engine->getInput()->hasPendingKey()) {
		const uint16 command = _engine->getInput()->consumeKey();
		if (command == kEscapeCommand)
			return kEscapeCommand;
		if (command == kHelpCommand) {
			debugC(1, kDebugPuzzles,
				"Ripper: eight-button sequence opening modal help table=0x%x",
				kHelpSelectionTable);
			if (!_engine->getModalDialog()->run(kHelpSelectionTable))
				warning("Ripper: eight-button sequence modal help failed");
			updateHover(_engine->getInput()->peekMouseState().position);
			return MediaSequenceCallback::kContinueRefreshPalette;
		}
	}

	const MouseState mouse = _engine->getInput()->publishMouseState();
	updateHover(mouse.position);
	if ((mouse.pressed & kMouseButtonLeft) != 0) {
		if (_hoveredButton >= 0)
			return pressButton(_hoveredButton);
		if (isExitPoint(mouse.position)) {
			debugC(1, kDebugPuzzles,
				"Ripper: eight-button sequence exited by left-edge control point=%d,%d",
				mouse.position.x, mouse.position.y);
			return kEscapeCommand;
		}
	}

	debugC(11, kDebugPuzzles,
		"Ripper: serviced eight-button sequence media frame=%u entered=%u hover=%d",
		frame, _enteredButtons.size(),
		_hoveredButton >= 0 ? _hoveredButton + 1 : 0);
	return 0;
}

void EightButtonSequencePuzzle::stopAudio() {
	for (uint cue = 0; cue < kAudioCueCount; ++cue)
		_engine->getMedia()->stopSoundEffect(_audioHandles[cue]);
}

EightButtonSequencePuzzle::Result EightButtonSequencePuzzle::run(
		uint completionFlag) {
	// DispatchSceneEntryAction at 0x36892 maps action 61 to
	// RunEightButtonSequencePuzzleScene at 0x4043d.
	if (!loadAssets() || !captureBackings())
		return kLoadFailed;

	_completionFlag = completionFlag;
	_hoveredButton = -1;
	_enteredButtons.clear();
	_result = kExited;
	_engine->getToolbar()->leave();
	_engine->getInput()->drainKeys();
	_engine->getInput()->discardMouseTransitions();
	_engine->getCursor()->setSelectionIndex(kDefaultCursor);
	_engine->getCursor()->dispatchSelectionIndexChange(kDefaultCursor);
	_engine->getCursor()->setVisible(true);
	debugC(1, kDebugPuzzles,
		"Ripper: entered eight-button sequence puzzle function="
		"RunEightButtonSequencePuzzleScene@0x4043d milestone=%u "
		"help=0x%x controls=%u sequenceLength=%u media='%s' position=%d,%d",
		completionFlag, kHelpSelectionTable, kButtonCount, kSequenceLength,
		kBackgroundMedia, kBackgroundPosition.x,
		kBackgroundPosition.y + kSceneOriginY);
	debugC(3, kDebugPuzzles,
		"Ripper: eight-button sequence retail solution=226481");

	uint16 command = 0;
	Common::SeekableReadStream *stream =
		_engine->getResources()->createReadStreamForPath(kBackgroundMedia);
	if (!_engine->getMedia()->playPuzzleSequenceStream(stream,
			kBackgroundMedia, kBackgroundPosition.x, kBackgroundPosition.y,
			0, this, &command)) {
		_result = kLoadFailed;
	} else if (command == kLoadFailedCommand) {
		_result = kLoadFailed;
	}

	stopAudio();
	_engine->getCursor()->setSelectionIndex(0);
	_engine->getCursor()->dispatchSelectionIndexChange(0);
	_engine->getCursor()->refresh();
	_engine->getCursor()->setVisible(true);
	_engine->getInput()->drainKeys();
	_engine->getInput()->discardMouseTransitions();
	debugC(_result == kLoadFailed ? 2 : 1, kDebugPuzzles,
		"Ripper: left eight-button sequence puzzle result=%d command=0x%04x "
		"milestone=%u milestoneSet=%d entered='%s' quit=%d",
		_result, command, completionFlag,
		_engine->getMilestones()->isSet(completionFlag),
		enteredSequence().c_str(), _engine->shouldQuit());
	return _result;
}

} // End of namespace Ripper
