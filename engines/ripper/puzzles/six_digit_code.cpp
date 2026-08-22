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

#include "ripper/puzzles/six_digit_code.h"

#include "common/debug.h"
#include "common/ptr.h"
#include "common/stream.h"
#include "common/system.h"
#include "graphics/surface.h"

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

static const uint kDigitCount = 6;
static const uint kNumberFrameCount = 10;
static const uint kControlCount = 12;
static const uint kEnterControl = 10;
static const uint kClearControl = 11;
static const uint kAudioCueCount = 5;
static const uint kDefaultCursor = 14;
static const uint kSelectionCursor = 16;
static const uint kExitCursor = 7;
static const uint kHelpSelectionTable = 0x1b7;
static const uint16 kEscapeCommand = 0x1b;
static const uint16 kHelpCommand = 0x3b00;
static const uint16 kClearCommand = 0x1300;
static const uint kDosTickMillis = 55;
static const int kSceneOriginY = 50;
static const int kSceneHeight = 300;

// g_awSixDigitCodeSolution at 0x84d50.
static const byte kSolution[kDigitCount] = {1, 8, 5, 6, 2, 1};

// g_astSixDigitControlLayouts at 0x84d5c stores scene-space Y/X/control-id
// triples. Entries 0..9 are the matching digit controls; 10 and 11 are Enter
// and clear.
static const Common::Point kControlPositions[kControlCount] = {
	Common::Point(346, 242 + kSceneOriginY),
	Common::Point(232, 217 + kSceneOriginY),
	Common::Point(261, 217 + kSceneOriginY),
	Common::Point(290, 217 + kSceneOriginY),
	Common::Point(318, 217 + kSceneOriginY),
	Common::Point(347, 217 + kSceneOriginY),
	Common::Point(232, 243 + kSceneOriginY),
	Common::Point(261, 243 + kSceneOriginY),
	Common::Point(290, 243 + kSceneOriginY),
	Common::Point(318, 243 + kSceneOriginY),
	Common::Point(223, 181 + kSceneOriginY),
	Common::Point(306, 181 + kSceneOriginY)
};

// g_astSixDigitDirtyRegions at 0x84da4 stores the six scene-space Y/X pairs.
static const Common::Point kDigitPositions[kDigitCount] = {
	Common::Point(261, 148 + kSceneOriginY),
	Common::Point(275, 148 + kSceneOriginY),
	Common::Point(289, 148 + kSceneOriginY),
	Common::Point(303, 148 + kSceneOriginY),
	Common::Point(317, 148 + kSceneOriginY),
	Common::Point(331, 148 + kSceneOriginY)
};

static bool isExitPoint(const Common::Point &point) {
	return Common::Rect(0, kSceneOriginY, 100,
		kSceneOriginY + kSceneHeight).contains(point);
}

} // End of anonymous namespace

SixDigitCodePuzzle::SixDigitCodePuzzle(RipperEngine *engine) :
		Puzzle(engine), _hoveredControl(-1) {
}

bool SixDigitCodePuzzle::loadFrame(const Common::String &path,
		BitmapAssetFrame &frame) {
	BitmapAssetSequence sequence;
	if (!_engine->getResources()->loadBitmapSequence(path, sequence) ||
			sequence.frames.empty()) {
		warning("Ripper: could not decode six-digit code bitmap '%s'",
			path.c_str());
		return false;
	}
	frame = Common::move(sequence.frames.front());
	return true;
}

bool SixDigitCodePuzzle::loadAssets() {
	_numberFrames.clear();
	for (uint digit = 0; digit < kNumberFrameCount; ++digit) {
		BitmapAssetFrame frame;
		if (!loadFrame(Common::String::format("bnknum%u.bbm", digit), frame))
			return false;
		_numberFrames.push_back(Common::move(frame));
	}

	_controlFrames.clear();
	for (uint control = 0; control < kControlCount; ++control) {
		BitmapAssetFrame frame;
		if (!loadFrame(Common::String::format("bnks%u.bbm", control), frame))
			return false;
		_controlFrames.push_back(Common::move(frame));
	}

	for (uint cue = 0; cue < kAudioCueCount; ++cue) {
		const Common::String path = Common::String::format("bnkwav%u.wav", cue);
		Common::ScopedPtr<Common::SeekableReadStream> stream(
			_engine->getResources()->createReadStreamForPath(path));
		if (!stream) {
			warning("Ripper: six-digit code audio '%s' is missing", path.c_str());
			return false;
		}
	}

	debugC(1, kDebugPuzzles,
		"Ripper: loaded six-digit code assets numbers=%u controls=%u audio=%u",
		_numberFrames.size(), _controlFrames.size(), kAudioCueCount);
	return true;
}

void SixDigitCodePuzzle::drawFrame(byte *screen, uint pitch,
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

void SixDigitCodePuzzle::render(bool showDigits) const {
	if (!_baseDisplay.restorePixels())
		return;

	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1) {
		if (screen)
			g_system->unlockScreen();
		return;
	}

	byte *pixels = (byte *)screen->getPixels();
	if (showDigits) {
		for (uint slot = 0; slot < _enteredDigits.size() &&
				slot < kDigitCount; ++slot) {
			const byte digit = _enteredDigits[slot];
			drawFrame(pixels, screen->pitch, _numberFrames[digit],
				kDigitPositions[slot].x, kDigitPositions[slot].y);
		}
	}
	if (_hoveredControl >= 0) {
		drawFrame(pixels, screen->pitch, _controlFrames[_hoveredControl],
			kControlPositions[_hoveredControl].x,
			kControlPositions[_hoveredControl].y);
	}

	g_system->unlockScreen();
	_baseDisplay.restorePalette();
	presentScreen();
}

int SixDigitCodePuzzle::findControl(const Common::Point &point) const {
	for (uint control = 0; control < _controlFrames.size(); ++control) {
		const BitmapAssetFrame &frame = _controlFrames[control];
		const Common::Point &position = kControlPositions[control];
		if (Common::Rect(position.x, position.y,
				position.x + frame.width,
				position.y + frame.height).contains(point))
			return control;
	}
	return -1;
}

void SixDigitCodePuzzle::updateHover(const Common::Point &point) {
	const int control = findControl(point);
	if (control != _hoveredControl) {
		if (_hoveredControl >= 0)
			playCue(1);
		if (control >= 0)
			playCue(0);
		debugC(2, kDebugPuzzles,
			"Ripper: six-digit code hover control=%d previous=%d point=%d,%d",
			control, _hoveredControl, point.x, point.y);
		_hoveredControl = control;
		render();
	}

	uint cursor = kDefaultCursor;
	if (control >= 0)
		cursor = kSelectionCursor;
	else if (isExitPoint(point))
		cursor = kExitCursor;
	_engine->getCursor()->update(cursor);
}

bool SixDigitCodePuzzle::playCue(uint cue) {
	if (cue >= kAudioCueCount)
		return false;
	const Common::String path = Common::String::format("bnkwav%u.wav", cue);
	const bool result = _engine->getMedia()->playSoundEffect(
		path, _audioHandles[cue]);
	debugC(result ? 3 : 2, kDebugPuzzles,
		"Ripper: six-digit code audio cue=%u path='%s' success=%d",
		cue, path.c_str(), result);
	return result;
}

bool SixDigitCodePuzzle::waitTicks(uint ticks) {
	const uint32 target = g_system->getMillis() + ticks * kDosTickMillis;
	while (!_engine->shouldQuit() &&
			(int32)(target - g_system->getMillis()) > 0) {
		if (!serviceEngineEvents())
			return false;
		g_system->delayMillis(10);
	}
	return !_engine->shouldQuit();
}

Common::String SixDigitCodePuzzle::enteredCode() const {
	Common::String result;
	for (uint slot = 0; slot < _enteredDigits.size(); ++slot)
		result += (char)('0' + _enteredDigits[slot]);
	return result;
}

void SixDigitCodePuzzle::resetEntry(const char *reason) {
	debugC(2, kDebugPuzzles,
		"Ripper: six-digit code reset reason='%s' previous='%s'",
		reason, enteredCode().c_str());
	_enteredDigits.clear();
	playCue(3);
	render();
}

bool SixDigitCodePuzzle::validateEntry(uint completionFlag, Result &result) {
	bool solved = _enteredDigits.size() == kDigitCount;
	for (uint slot = 0; solved && slot < kDigitCount; ++slot)
		solved = _enteredDigits[slot] == kSolution[slot];
	debugC(solved ? 1 : 2, kDebugPuzzles,
		"Ripper: six-digit code validation entered='%s' outcome=%s",
		enteredCode().c_str(), solved ? "SOLVED" : "REJECTED");
	if (!solved) {
		resetEntry("rejected");
		return true;
	}

	playCue(2);
	for (uint phase = 0; phase < 5; ++phase) {
		if (!waitTicks(5))
			return false;
		const bool visible = (phase & 1) != 0;
		render(visible);
		debugC(3, kDebugPuzzles,
			"Ripper: six-digit code success flash phase=%u visible=%d",
			phase, visible);
	}
	if (!markSolved(completionFlag, "six-digit-code-puzzle")) {
		result = kLoadFailed;
		return false;
	}
	result = kSolved;
	return false;
}

bool SixDigitCodePuzzle::activateControl(uint control, uint completionFlag,
		Result &result) {
	if (control < kNumberFrameCount) {
		if (_enteredDigits.size() < kDigitCount) {
			_enteredDigits.push_back(control);
			render();
			debugC(2, kDebugPuzzles,
				"Ripper: six-digit code input digit=%u entered='%s' count=%u/%u",
				control, enteredCode().c_str(), _enteredDigits.size(), kDigitCount);
		}
		return true;
	}
	if (control == kEnterControl)
		return validateEntry(completionFlag, result);
	if (control == kClearControl) {
		resetEntry("clear-control");
		return true;
	}
	return true;
}

void SixDigitCodePuzzle::stopAudio() {
	stopAudioHandles(_audioHandles, ARRAYSIZE(_audioHandles));
}

SixDigitCodePuzzle::Result SixDigitCodePuzzle::run(uint completionFlag) {
	// DispatchSceneEntryAction at 0x36892 maps action 21 to
	// RunSixDigitCodePuzzleScene at 0x3e913.
	if (!loadAssets() || !_baseDisplay.capture())
		return kLoadFailed;

	_engine->getToolbar()->leave();
	_engine->getInput()->drainKeys();
	_engine->getInput()->discardMouseTransitions();
	_engine->getCursor()->setSelectionIndex(kDefaultCursor);
	_engine->getCursor()->dispatchSelectionIndexChange(kDefaultCursor);
	_engine->getCursor()->setVisible(true);
	_enteredDigits.clear();
	_hoveredControl = -1;
	render();
	debugC(1, kDebugPuzzles,
		"Ripper: entered six-digit code puzzle function="
		"RunSixDigitCodePuzzleScene@0x3e913 milestone=%u help=0x%x "
		"controls=%u digits=%u",
		completionFlag, kHelpSelectionTable, kControlCount, kDigitCount);
	debugC(3, kDebugPuzzles,
		"Ripper: six-digit code retail solution=185621");

	Result result = kExited;
	bool active = true;
	while (active && !_engine->shouldQuit()) {
		if (!serviceEngineEvents())
			break;

		while (_engine->getInput()->hasPendingKey()) {
			const uint16 command = _engine->getInput()->consumeKey();
			if (command == kEscapeCommand) {
				active = false;
				break;
			}
			if (command == kHelpCommand) {
				debugC(1, kDebugPuzzles,
					"Ripper: six-digit code opening modal help table=0x%x",
					kHelpSelectionTable);
				if (!_engine->getModalDialog()->run(kHelpSelectionTable))
					warning("Ripper: six-digit code modal help failed");
				render();
				continue;
			}
			if (command >= '0' && command <= '9') {
				active = activateControl(command - '0', completionFlag, result);
			} else if (command == 0xd) {
				active = activateControl(kEnterControl, completionFlag, result);
			} else if (command == kClearCommand) {
				active = activateControl(kClearControl, completionFlag, result);
			}
			if (!active)
				break;
		}
		if (!active)
			break;

		const MouseState mouse = _engine->getInput()->publishMouseState();
		updateHover(mouse.position);
		if ((mouse.pressed & kMouseButtonLeft) != 0) {
			if (_hoveredControl >= 0) {
				active = activateControl(_hoveredControl, completionFlag, result);
			} else if (isExitPoint(mouse.position)) {
				debugC(1, kDebugPuzzles,
					"Ripper: six-digit code exited by left-edge control point=%d,%d",
					mouse.position.x, mouse.position.y);
				active = false;
			}
		}
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
	debugC(result == kLoadFailed ? 2 : 1, kDebugPuzzles,
		"Ripper: left six-digit code puzzle result=%d milestone=%u "
		"milestoneSet=%d entered='%s' quit=%d",
		result, completionFlag,
		_engine->getMilestones()->isSet(completionFlag),
		enteredCode().c_str(), _engine->shouldQuit());
	return result;
}

} // End of namespace Ripper
