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

#include "ripper/puzzles/keypad_sequence.h"

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

static const uint kExtraFrameCount = 4;
static const uint kKeyCount = 12;
static const uint kSequenceLength = 9;
static const int kSceneOriginY = 50;
static const int kSceneHeight = 300;
static const uint kDefaultCursor = 14;
static const uint kSelectionCursor = 16;
static const uint kExitCursor = 12;
static const uint kHelpSelectionTable = 0x1ac;
static const uint16 kEscapeCommand = 0x1b;
static const uint16 kHelpCommand = 0x3b00;
static const uint kDosTickMillis = 55;

static const int kCodeX = 142;
static const int kCodeY = kSceneOriginY;
static const int kBlinkX = 126;
static const int kBlinkY = 213 + kSceneOriginY;
static const int kSlotY = 200 + kSceneOriginY;
static const int kSlotX[kSequenceLength] = {
	169, 181, 193, 205, 217, 229, 241, 253, 265
};

// g_astKeypadButtonPositions at 0x84b8e stores scene-space Y/X pairs.
static const Common::Point kKeyPositions[kKeyCount] = {
	Common::Point(138,  38 + kSceneOriginY),
	Common::Point(188,  37 + kSceneOriginY),
	Common::Point(240,  36 + kSceneOriginY),
	Common::Point(141,  69 + kSceneOriginY),
	Common::Point(191,  68 + kSceneOriginY),
	Common::Point(241,  68 + kSceneOriginY),
	Common::Point(144,  99 + kSceneOriginY),
	Common::Point(193,  98 + kSceneOriginY),
	Common::Point(242,  98 + kSceneOriginY),
	Common::Point(146, 128 + kSceneOriginY),
	Common::Point(195, 128 + kSceneOriginY),
	Common::Point(244, 128 + kSceneOriginY)
};

// g_keypadSequenceSolutionDigits at 0x84b7c.
static const byte kSolution[kSequenceLength] = {
	4, 5, 0, 1, 4, 4, 2, 8, 6
};

static const char *const kOpenMedia = "kpopen.avi";
static const char *const kCloseMedia = "kpclose.avi";
static const char *const kPressAudio = "jb_loc0.wav";
static const char *const kReleaseAudio = "jb_loc1.wav";
static const char *const kEntryAudio = "jb_loc2.wav";
static const char *const kSuccessAudio = "jb_loc3.wav";
static const char *const kRejectAudio = "jb_loc4.wav";

static bool isExitRegion(const Common::Point &point) {
	return Common::Rect(0, kSceneOriginY, 100,
		kSceneOriginY + kSceneHeight).contains(point);
}

} // End of anonymous namespace

KeypadSequencePuzzle::KeypadSequencePuzzle(RipperEngine *engine) :
		_engine(engine), _slotMask(0), _lastBlinkMillis(0),
		_hoveredKey(-1), _pressedKey(-1), _hoveredControl(kHoverNone),
		_keypadOpen(false), _blinkLit(false) {
}

bool KeypadSequencePuzzle::loadFrame(const Common::String &path,
		BitmapAssetFrame &frame) {
	Common::ScopedPtr<Common::SeekableReadStream> stream(
		_engine->getResources()->createReadStreamForPath(path));
	BitmapAssetSequence sequence;
	if (!stream || !decodeBitmapAssetSequence(*stream, sequence) ||
			sequence.frames.empty()) {
		warning("Ripper: could not decode keypad bitmap '%s'", path.c_str());
		return false;
	}
	frame = Common::move(sequence.frames.front());
	return true;
}

bool KeypadSequencePuzzle::loadAssets() {
	_extraFrames.clear();
	for (uint frame = 0; frame < kExtraFrameCount; ++frame) {
		BitmapAssetFrame bitmap;
		const Common::String path =
			Common::String::format("kpextra%u.bbm", frame);
		if (!loadFrame(path, bitmap))
			return false;
		_extraFrames.push_back(Common::move(bitmap));
	}

	_keyFrames.clear();
	for (uint key = 0; key < kKeyCount; ++key) {
		BitmapAssetFrame bitmap;
		const Common::String path =
			Common::String::format("jb_key%u.bbm", key);
		if (!loadFrame(path, bitmap))
			return false;
		_keyFrames.push_back(Common::move(bitmap));
	}

	debugC(1, kDebugPuzzles,
		"Ripper: loaded keypad sequence assets extras=%u keys=%u "
		"open='%s' close='%s' audio=6",
		_extraFrames.size(), _keyFrames.size(), kOpenMedia, kCloseMedia);
	return true;
}

bool KeypadSequencePuzzle::prepareEntryDisplay() {
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1 ||
			screen->w < kRipperScreenWidth ||
			screen->h < kRipperScreenHeight) {
		if (screen)
			g_system->unlockScreen();
		return false;
	}
	for (int y = kSceneOriginY; y < kSceneOriginY + kSceneHeight; ++y)
		memset(screen->getBasePtr(0, y), 0, kRipperScreenWidth);
	g_system->unlockScreen();
	g_system->updateScreen();
	return _baseDisplay.capture();
}

bool KeypadSequencePuzzle::openKeypad() {
	_engine->getCursor()->setVisible(false);
	if (!_engine->getMedia()->playScene(kOpenMedia, 0, 0, false,
			false, false))
		return false;
	if (!_baseDisplay.capture())
		return false;

	_keypadOpen = true;
	resetEntry();
	_hoveredControl = kHoverNone;
	_hoveredKey = -1;
	_pressedKey = -1;
	_lastBlinkMillis = g_system->getMillis();
	_engine->getInput()->discardMouseTransitions();
	_engine->getCursor()->setVisible(true);
	render();
	debugC(1, kDebugPuzzles,
		"Ripper: opened keypad sequence presentation media='%s'",
		kOpenMedia);
	return true;
}

bool KeypadSequencePuzzle::closeKeypad() {
	_engine->getCursor()->setVisible(false);
	const bool result = _engine->getMedia()->playScene(kCloseMedia, 0, 0,
		false, false, false);
	_engine->getInput()->discardMouseTransitions();
	debugC(result ? 1 : 2, kDebugPuzzles,
		"Ripper: closed keypad sequence presentation media='%s' success=%d",
		kCloseMedia, result);
	return result;
}

void KeypadSequencePuzzle::drawFrame(byte *screen, uint pitch,
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

void KeypadSequencePuzzle::render() const {
	if (!_baseDisplay.restorePixels())
		return;

	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1) {
		if (screen)
			g_system->unlockScreen();
		return;
	}
	byte *pixels = (byte *)screen->getPixels();
	drawFrame(pixels, screen->pitch, _extraFrames[2], kCodeX, kCodeY);
	drawFrame(pixels, screen->pitch, _extraFrames[_blinkLit ? 1 : 3],
		kBlinkX, kBlinkY);
	if (_keypadOpen) {
		for (uint slot = 0; slot < kSequenceLength; ++slot) {
			if ((_slotMask & (1 << slot)) != 0)
				drawFrame(pixels, screen->pitch, _extraFrames[0],
					kSlotX[slot], kSlotY);
		}
		if (_pressedKey >= 0)
			drawFrame(pixels, screen->pitch, _keyFrames[_pressedKey],
				kKeyPositions[_pressedKey].x,
				kKeyPositions[_pressedKey].y);
	}
	g_system->unlockScreen();
	_baseDisplay.restorePalette();
	g_system->updateScreen();
}

int KeypadSequencePuzzle::findKey(const Common::Point &point) const {
	if (!_keypadOpen)
		return -1;
	for (uint key = 0; key < _keyFrames.size(); ++key) {
		const BitmapAssetFrame &frame = _keyFrames[key];
		const Common::Point &position = kKeyPositions[key];
		if (Common::Rect(position.x, position.y,
				position.x + frame.width,
				position.y + frame.height).contains(point))
			return key;
	}
	return -1;
}

KeypadSequencePuzzle::HoverControl KeypadSequencePuzzle::findControl(
		const Common::Point &point, int &key) const {
	key = findKey(point);
	if (key >= 0)
		return kHoverKey;
	if (Common::Rect(kCodeX, kCodeY,
			kCodeX + _extraFrames[2].width,
			kCodeY + _extraFrames[2].height).contains(point))
		return kHoverCode;
	if (isExitRegion(point))
		return kHoverExit;
	return kHoverNone;
}

bool KeypadSequencePuzzle::updateCursor(const Common::Point &point) {
	int key = -1;
	const HoverControl control = findControl(point, key);
	const bool changed = control != _hoveredControl || key != _hoveredKey;
	if (changed) {
		debugC(2, kDebugInput,
			"Ripper: keypad sequence hover control=%d key=%d point=%d,%d",
			control, key, point.x, point.y);
		_hoveredControl = control;
		_hoveredKey = key;
	}

	uint cursor = kDefaultCursor;
	if (control == kHoverKey || control == kHoverCode)
		cursor = kSelectionCursor;
	else if (control == kHoverExit)
		cursor = kExitCursor;
	_engine->getCursor()->update(cursor);
	_engine->getCursor()->setVisible(true);
	return changed;
}

bool KeypadSequencePuzzle::serviceBlink() {
	const uint32 now = g_system->getMillis();
	const uint32 interval =
		(_blinkLit ? 3 : 2) * kDosTickMillis;
	if (now - _lastBlinkMillis < interval)
		return false;
	_blinkLit = !_blinkLit;
	_lastBlinkMillis = now;
	render();
	debugC(3, kDebugPuzzles,
		"Ripper: keypad sequence blink phase=%d intervalMs=%u",
		_blinkLit, interval);
	return true;
}

bool KeypadSequencePuzzle::waitFeedbackTicks(uint ticks) {
	const uint32 target = g_system->getMillis() + ticks * kDosTickMillis;
	while (!_engine->shouldQuit() &&
			(int32)(target - g_system->getMillis()) > 0) {
		if (_engine->getInput()->pollEvents()) {
			_engine->quitGame();
			return false;
		}
		serviceBlink();
		g_system->delayMillis(10);
	}
	return !_engine->shouldQuit();
}

void KeypadSequencePuzzle::animateClear(bool success) {
	_engine->getMedia()->playSoundEffect(
		success ? kSuccessAudio : kRejectAudio, _feedbackAudioHandle);
	if (success) {
		for (uint slot = 0; slot < kSequenceLength; ++slot) {
			_slotMask &= ~(1 << slot);
			render();
			if (!waitFeedbackTicks(2))
				break;
		}
	} else {
		for (int slot = (int)kSequenceLength - 1; slot >= 0; --slot) {
			if ((_slotMask & (1 << slot)) == 0)
				continue;
			_slotMask &= ~(1 << slot);
			render();
			if (!waitFeedbackTicks(2))
				break;
		}
	}
	_enteredDigits.clear();
	_slotMask = 0;
	render();
	debugC(2, kDebugPuzzles,
		"Ripper: keypad sequence feedback completed outcome=%s",
		success ? "success" : "reject");
}

bool KeypadSequencePuzzle::pressKey(uint key, uint completionFlag,
		Result &result) {
	if (key >= kKeyCount)
		return false;

	_pressedKey = key;
	_engine->getMedia()->playSoundEffect(kPressAudio, _pressAudioHandle);
	render();
	waitFeedbackTicks(1);
	_pressedKey = -1;
	_engine->getMedia()->playSoundEffect(kReleaseAudio, _releaseAudioHandle);

	// Controls 0x67d (EX) and 0x67f (RV) both run the retail reject/clear sweep.
	if (key == 9 || key == 11) {
		debugC(2, kDebugPuzzles,
			"Ripper: keypad sequence clear key=%u entered=%u",
			key, _enteredDigits.size());
		animateClear(false);
		return true;
	}

	const byte digit = key == 10 ? 0 : key + 1;
	if (_enteredDigits.size() >= kSequenceLength)
		return true;
	_enteredDigits.push_back(digit);
	_slotMask |= 1 << (_enteredDigits.size() - 1);
	render();
	debugC(2, kDebugPuzzles,
		"Ripper: keypad sequence input key=%u digit=%u entered='%s' count=%u",
		key, digit, enteredSequence().c_str(), _enteredDigits.size());

	if (_enteredDigits.size() != kSequenceLength)
		return true;

	bool solved = true;
	for (uint index = 0; index < kSequenceLength; ++index) {
		if (_enteredDigits[index] != kSolution[index]) {
			solved = false;
			break;
		}
	}
	debugC(solved ? 1 : 2, kDebugPuzzles,
		"Ripper: keypad sequence validation entered='%s' outcome=%s",
		enteredSequence().c_str(), solved ? "SOLVED" : "REJECTED");
	if (!solved) {
		animateClear(false);
		return true;
	}

	animateClear(true);
	if (!_engine->getMilestones()->set(completionFlag, true,
			"keypad-sequence-puzzle")) {
		result = kLoadFailed;
		return false;
	}
	result = kSolved;
	return false;
}

void KeypadSequencePuzzle::resetEntry() {
	_enteredDigits.clear();
	_slotMask = 0;
	_blinkLit = false;
	_lastBlinkMillis = g_system->getMillis();
}

Common::String KeypadSequencePuzzle::enteredSequence() const {
	Common::String result;
	for (uint index = 0; index < _enteredDigits.size(); ++index)
		result += (char)('0' + _enteredDigits[index]);
	return result;
}

void KeypadSequencePuzzle::stopAudio() {
	_engine->getMedia()->stopSoundEffect(_pressAudioHandle);
	_engine->getMedia()->stopSoundEffect(_releaseAudioHandle);
	_engine->getMedia()->stopSoundEffect(_feedbackAudioHandle);
}

KeypadSequencePuzzle::Result KeypadSequencePuzzle::run(uint completionFlag) {
	// DispatchSceneEntryAction at 0x36892 maps action 34 to
	// RunKeypadSequencePuzzleScene at 0x3bd30.
	if (!loadAssets() || !prepareEntryDisplay())
		return kLoadFailed;

	_engine->getToolbar()->leave();
	_engine->getInput()->drainKeys();
	_engine->getInput()->discardMouseTransitions();
	_engine->getCursor()->setSelectionIndex(kDefaultCursor);
	_engine->getCursor()->dispatchSelectionIndexChange(kDefaultCursor);
	_engine->getCursor()->setVisible(true);
	resetEntry();
	render();
	debugC(1, kDebugPuzzles,
		"Ripper: entered keypad sequence puzzle function="
		"RunKeypadSequencePuzzleScene@0x3bd30 milestone=%u "
		"help=0x%x controls=12 sequenceLength=9",
		completionFlag, kHelpSelectionTable);
	debugC(3, kDebugPuzzles,
		"Ripper: keypad sequence retail solution=450144286");

	Result result = kExited;
	bool active = true;
	while (active && !_engine->shouldQuit()) {
		if (_engine->getInput()->pollEvents()) {
			_engine->quitGame();
			break;
		}

		while (_engine->getInput()->hasPendingKey()) {
			const uint16 command = _engine->getInput()->consumeKey();
			if (command == kEscapeCommand) {
				if (_keypadOpen && !closeKeypad())
					result = kLoadFailed;
				active = false;
				break;
			}
			if (command == kHelpCommand) {
				debugC(1, kDebugPuzzles,
					"Ripper: keypad sequence opening modal help table=0x%x",
					kHelpSelectionTable);
				if (!_engine->getModalDialog()->run(kHelpSelectionTable))
					warning("Ripper: keypad sequence modal help failed");
				render();
			}
		}
		if (!active)
			break;

		const MouseState mouse = _engine->getInput()->publishMouseState();
		bool redraw = updateCursor(mouse.position);
		if ((mouse.pressed & kMouseButtonLeft) != 0) {
			int key = -1;
			const HoverControl control = findControl(mouse.position, key);
			if (control == kHoverKey) {
				if (!pressKey(key, completionFlag, result))
					active = false;
				redraw = true;
			} else if (control == kHoverCode) {
				if (_keypadOpen) {
					if (!closeKeypad())
						result = kLoadFailed;
					active = false;
				} else {
					_engine->getMedia()->playSoundEffect(
						kEntryAudio, _feedbackAudioHandle);
					if (!openKeypad()) {
						result = kLoadFailed;
						active = false;
					}
				}
				redraw = true;
			} else if (control == kHoverExit) {
				if (_keypadOpen && !closeKeypad())
					result = kLoadFailed;
				active = false;
			}
		}
		if (!active)
			break;
		if (!serviceBlink() && redraw)
			render();
		else if (!redraw)
			g_system->updateScreen();
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
		"Ripper: left keypad sequence puzzle result=%d milestone=%u "
		"milestoneSet=%d opened=%d quit=%d",
		result, completionFlag,
		_engine->getMilestones()->isSet(completionFlag),
		_keypadOpen, _engine->shouldQuit());
	return result;
}

} // End of namespace Ripper
