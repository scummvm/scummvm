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

#include "ripper/scenes/gym_scene.h"

#include "common/debug.h"
#include "common/system.h"
#include "graphics/surface.h"

#include "ripper/cursor.h"
#include "ripper/detection.h"
#include "ripper/input.h"
#include "ripper/milestones.h"
#include "ripper/modal_dialog.h"
#include "ripper/ripper.h"

namespace Ripper {

namespace {

static const char *const kButtonBitmaps[2] = {
	"fd_gymb0.bbm", "fd_gymb1.bbm"
};

static const char *const kAudioCues[3] = {
	"gym0_w.wav", "gym1_w.wav", "gym2_w.wav"
};

static const int kButtonLeft[2] = {374, 373};
static const int kButtonTop[2] = {258 + 50, 277 + 50};
static const int kButtonWidth[2] = {23, 22};
static const int kButtonHeight[2] = {14, 13};
static const uint16 kButtonCommand[2] = {0x4800, 0x5000};

static const uint kDefaultCursor = 14;
static const uint kButtonCursor = 16;
static const uint kExitCursor = 7;
static const uint kHelpResource = 0x1a9;
static const uint kTransitionStopFrame = 0x26;
static const uint kIntroState = 4;
static const uint kDosTickMillis = 55;
static const uint kKeyboardPulseTicks = 3;

static const uint16 kEscapeCommand = 0x1b;
static const uint16 kHelpCommand = 0x3b00;
static const uint16 kFailureCommand = 0x7ffd;

static bool isButtonPoint(uint button, const Common::Point &point) {
	return button < ARRAYSIZE(kButtonLeft) &&
		point.x >= kButtonLeft[button] &&
		point.x < kButtonLeft[button] + kButtonWidth[button] &&
		point.y >= kButtonTop[button] &&
		point.y < kButtonTop[button] + kButtonHeight[button];
}

static bool isExitPoint(const Common::Point &point) {
	// RunGymSelectorScene at 0x3c64b builds the exit control at scene-space
	// (0, 0) with width 166 and height 300. The scene viewport starts at y=50.
	return point.x >= 0 && point.x < 166 && point.y >= 50 && point.y < 350;
}

} // End of anonymous namespace

GymScene::GymScene(RipperEngine *engine) : Scene(engine),
		_hoveredControl(-1), _pressedControl(-1), _currentState(0) {
}

bool GymScene::initialize() {
	for (uint button = 0; button < ARRAYSIZE(_buttonFrames); ++button) {
		BitmapAssetSequence sequence;
		if (!_engine->getResources()->loadBitmapSequence(
				kButtonBitmaps[button], sequence) || sequence.frames.empty()) {
			warning("Ripper: could not load gym selector bitmap '%s'",
				kButtonBitmaps[button]);
			return false;
		}
		_buttonFrames[button] = Common::move(sequence.frames.front());
		if (_buttonFrames[button].width != kButtonWidth[button] ||
				_buttonFrames[button].height != kButtonHeight[button]) {
			warning("Ripper: gym selector bitmap '%s' has invalid size=%ux%u",
				kButtonBitmaps[button], _buttonFrames[button].width,
				_buttonFrames[button].height);
			return false;
		}
	}

	debugC(1, kDebugPuzzles,
		"Ripper: loaded gym selector assets controls=2 audioCues=3 "
		"up=[374,308,23,14] down=[373,327,22,13] exit=[0,50,166,300]");
	return true;
}

bool GymScene::playControlCue(uint cue) {
	if (cue >= ARRAYSIZE(kAudioCues))
		return false;
	if (!_engine->getMedia()->playSoundEffect(kAudioCues[cue],
			_audioHandles[cue])) {
		warning("Ripper: could not start gym selector cue '%s'", kAudioCues[cue]);
		return false;
	}
	return true;
}

void GymScene::drawPressedControl() const {
	if (_pressedControl < 0 || _pressedControl >= 2)
		return;

	const uint control = (uint)_pressedControl;
	const BitmapAssetFrame &frame = _buttonFrames[control];
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1) {
		if (screen)
			g_system->unlockScreen();
		return;
	}
	for (uint sourceY = 0; sourceY < frame.height; ++sourceY) {
		byte *destination = (byte *)screen->getBasePtr(
			kButtonLeft[control], kButtonTop[control] + sourceY);
		const byte *source = frame.pixels.data() + sourceY * frame.width;
		for (uint sourceX = 0; sourceX < frame.width; ++sourceX) {
			if (source[sourceX] != frame.transparentColor)
				destination[sourceX] = source[sourceX];
		}
	}
	g_system->unlockScreen();
}

bool GymScene::pulseKeyboardControl(uint control) {
	if (control >= ARRAYSIZE(_buttonFrames))
		return false;

	const BitmapAssetFrame &frame = _buttonFrames[control];
	Common::Array<byte> backing;
	backing.resize(frame.width * frame.height);
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1) {
		if (screen)
			g_system->unlockScreen();
		return false;
	}
	for (uint y = 0; y < frame.height; ++y) {
		memcpy(backing.data() + y * frame.width,
			screen->getBasePtr(kButtonLeft[control], kButtonTop[control] + y),
			frame.width);
	}
	g_system->unlockScreen();

	_pressedControl = control;
	if (!playControlCue(0))
		return false;
	drawPressedControl();
	presentScreen();
	g_system->delayMillis(kKeyboardPulseTicks * kDosTickMillis);

	screen = g_system->lockScreen();
	if (screen && screen->format.bytesPerPixel == 1) {
		for (uint y = 0; y < frame.height; ++y) {
			memcpy(screen->getBasePtr(kButtonLeft[control],
				kButtonTop[control] + y), backing.data() + y * frame.width,
				frame.width);
		}
	}
	if (screen)
		g_system->unlockScreen();
	_pressedControl = -1;
	if (!playControlCue(1))
		return false;
	presentScreen();
	return true;
}

void GymScene::updateCursor(const Common::Point &point) {
	int hovered = -1;
	uint cursor = kDefaultCursor;
	for (uint button = 0; button < ARRAYSIZE(_buttonFrames); ++button) {
		if (isButtonPoint(button, point)) {
			hovered = button;
			cursor = kButtonCursor;
			break;
		}
	}
	if (hovered < 0 && isExitPoint(point)) {
		hovered = 2;
		cursor = kExitCursor;
	}
	if (hovered != _hoveredControl) {
		debugC(2, kDebugInput,
			"Ripper: gym selector hover control=%d previous=%d point=%d,%d cursor=%u",
			hovered, _hoveredControl, point.x, point.y, cursor);
		_hoveredControl = hovered;
	}
	_engine->getCursor()->update(cursor);
	_engine->getCursor()->setVisible(true);
}

uint16 GymScene::service(uint frame) {
	if (frame == kTransitionStopFrame &&
			_engine->getMedia()->stopSoundEffect(_audioHandles[2])) {
		debugC(3, kDebugAudio,
			"Ripper: stopped gym selector transition cue at mediaFrame=%u state=%u",
			frame, _currentState);
	}

	const MouseState mouse = _engine->getInput()->publishMouseState();
	updateCursor(mouse.position);

	if (_pressedControl >= 0 && _hoveredControl != _pressedControl) {
		if (!playControlCue(1))
			return kFailureCommand;
		debugC(3, kDebugInput,
			"Ripper: cancelled gym selector control=%d point=%d,%d frame=%u",
			_pressedControl, mouse.position.x, mouse.position.y, frame);
		_pressedControl = -1;
	}

	if (_hoveredControl >= 0 && _hoveredControl < 2 &&
			_pressedControl < 0 &&
			((mouse.buttons | mouse.pressed) & kMouseButtonLeft) != 0) {
		_pressedControl = _hoveredControl;
		if (!playControlCue(0))
			return kFailureCommand;
		debugC(3, kDebugInput,
			"Ripper: pressed gym selector control=%d point=%d,%d frame=%u",
			_pressedControl, mouse.position.x, mouse.position.y, frame);
	}

	if ((mouse.released & kMouseButtonLeft) != 0 && _pressedControl >= 0 &&
			_pressedControl < (int)ARRAYSIZE(kButtonCommand)) {
		const int releasedControl = _pressedControl;
		if (!playControlCue(1))
			return kFailureCommand;
		_pressedControl = -1;
		debugC(2, kDebugInput,
			"Ripper: activated gym selector control=%d command=0x%04x "
			"point=%d,%d frame=%u transition=release",
			releasedControl, kButtonCommand[releasedControl],
			mouse.position.x, mouse.position.y, frame);
		return kButtonCommand[releasedControl];
	}

	if ((mouse.pressed & kMouseButtonLeft) != 0) {
		if (_hoveredControl == 2)
			return kEscapeCommand;
	}

	while (_engine->getInput()->hasPendingKey()) {
		const uint16 command = _engine->getInput()->consumeKey();
		if (command == kHelpCommand) {
			debugC(1, kDebugPuzzles,
				"Ripper: gym selector opening modal help resource=0x%x",
				kHelpResource);
			if (!_engine->getModalDialog()->run(kHelpResource))
				return kFailureCommand;
			updateCursor(_engine->getInput()->peekMouseState().position);
			continue;
		}
		if (command == kEscapeCommand)
			return command;
		for (uint button = 0; button < ARRAYSIZE(kButtonCommand); ++button) {
			if (command != kButtonCommand[button])
				continue;
			if (!pulseKeyboardControl(button))
				return kFailureCommand;
			debugC(2, kDebugInput,
				"Ripper: keyboard activated gym selector control=%u "
				"command=0x%04x frame=%u pulseTicks=%u",
				button, command, frame, kKeyboardPulseTicks);
			return command;
		}
	}

	drawPressedControl();
	return 0;
}

bool GymScene::playStateMedia(uint state, uint16 &command) {
	const Common::String path = Common::String::format("fd_gym%u.smk", state);
	command = 0;
	const bool played = _engine->getMedia()->playPuzzleSequence(
		path, 0, this, &command);
	debugC(played ? 2 : 1, kDebugPuzzles,
		"Ripper: gym selector media='%s' state=%u command=0x%04x result=%d",
		path.c_str(), state, command, played);
	return played;
}

void GymScene::stopAllAudio() {
	for (uint cue = 0; cue < ARRAYSIZE(_audioHandles); ++cue)
		stopAudio(_audioHandles[cue]);
}

GymScene::Result GymScene::run(uint completionFlag) {
	if (!initialize())
		return kLoadFailed;

	prepare("gym-selector-entry", kDefaultCursor, true);
	debugC(1, kDebugPuzzles,
		"Ripper: entered gym selector scene function=RunGymSelectorScene@0x3c64b "
		"milestone=%u initialState=0 introState=4 help=0x%x",
		completionFlag, kHelpResource);

	Result result = kExited;
	bool selectedTarget = false;
	uint16 command = 0;
	_currentState = 0;

	while (command == 0 && !_engine->shouldQuit()) {
		if (!playStateMedia(kIntroState, command)) {
			if (!_engine->shouldQuit())
				result = kLoadFailed;
			break;
		}
		_currentState = kIntroState;
		if (command == kFailureCommand) {
			result = kLoadFailed;
			break;
		}
	}

	while (result != kLoadFailed && command != 0 &&
			command != kEscapeCommand && !_engine->shouldQuit()) {
		if (command == kButtonCommand[0]) {
			if (_currentState == kIntroState || _currentState == 3)
				_currentState = 0;
			else
				++_currentState;
		} else if (command == kButtonCommand[1]) {
			if (_currentState == 0)
				_currentState = 3;
			else
				--_currentState;
		}

		if (!playControlCue(2)) {
			result = kLoadFailed;
			break;
		}
		if (!playStateMedia(_currentState, command)) {
			if (!_engine->shouldQuit())
				result = kLoadFailed;
			break;
		}
		stopAudio(_audioHandles[2]);
		if (command == kFailureCommand) {
			result = kLoadFailed;
			break;
		}
		if (_currentState == 2) {
			if (!_engine->getMilestones()->set(
					completionFlag, true, "gym-selector")) {
				result = kLoadFailed;
				break;
			}
			selectedTarget = true;
			result = kSolved;
			debugC(1, kDebugPuzzles,
				"Ripper: gym selector reached target state=2 milestone=%u",
				completionFlag);
		}
	}

	stopAllAudio();
	_engine->getCursor()->setSelectionIndex(0);
	finish("gym-selector-exit", 0, true);
	debugC(result == kLoadFailed ? 2 : 1, kDebugPuzzles,
		"Ripper: left gym selector scene result=%d milestone=%u state=%u "
		"command=0x%04x selectedTarget=%d quit=%d",
		result, completionFlag, _currentState, command, selectedTarget,
		_engine->shouldQuit());
	return result;
}

} // End of namespace Ripper
