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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/system.h"
#include "common/config-manager.h"
#include "common/events.h"
#include "common/str.h"

#include "audio/mixer.h"

#include "graphics/cursorman.h"

#include "scumm/scumm_v7.h"
#include "scumm/smush/smush_player.h"
#include "scumm/insane/rebel1/rebel.h"

namespace Scumm {

const int kRA1MainMenuItemCount = 6;
const int kRA1LevelSelectItemCount = 16;  // 15 levels + BACK
const int kRA1LevelSelectRowsPerCol = 8;
const int kRA1NumLevels = 15;
const int kRA1MenuAxisThreshold = Common::JOYAXIS_MAX / 2;
const int kRA1MenuLogicalWidth = 0x140;
const int kRA1MenuFrameX = 0x32;
const int kRA1MenuFrameW = 0xdc;
const int kRA1MenuFrameH = 0x0f;
const int kRA1MenuRowH = 0x0f;
const byte kRA1MenuFrameColor = 0xdf;
// Highlight-frame geometry shared by the render*Overlay drawing and the mouse hit-testing
// (clicking menu items is an extra feature, so the rects must stay in sync).
const int kRA1MainMenuFrameYBase    = 0x2c;  // frame Y = (item + 1) * kRA1MenuRowH + this
const int kRA1OptionsFrameYBase     = 0x1d;  // frame Y = (item + 1) * kRA1MenuRowH + this
const int kRA1LevelSelectFrameYBase = 0x2c;  // frame Y = row * kRA1MenuRowH + this
const int kRA1LevelSelectLeftX      = 20;
const int kRA1LevelSelectRightX     = 170;
const int kRA1LevelSelectColW       = 130;
const uint32 kRA1JoystickAxisEscGuardMs = 250;
const int kRA1GameplayMouseSettleJumpThreshold = 40;
const int kRA1GameplayMouseSettleRelativeThreshold = 40;
const int kRA1GameplayMouseSettleEdgeMargin = 16;
const int kRA1GameplayMouseMaxX = 319;
const int kRA1GameplayMouseMaxY = 199;
const uint32 kRA1GameplayMouseSettleExtendMs = 1000;
// Original picker traversal uses fixed max indices, not strlen(): passcodes
// stop at 0x1d and high-score names stop at 0x37.
const char kRA1TextEntryPickerChars[] = "^`_ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
const int kRA1PasscodePickerCount = 0x1d + 1;
const int kRA1NamePickerCount = 0x37 + 1;

static int getRebel1MenuAxisDirection(int16 axisValue) {
	if (axisValue >= kRA1MenuAxisThreshold)
		return 1;
	if (axisValue <= -kRA1MenuAxisThreshold)
		return -1;
	return 0;
}

static void setRebel1Volume(ScummEngine_v7 *vm, int &volume, int delta) {
	volume = CLIP<int>(volume + delta, 0, 127);
	const int mixerVolume = (volume * Audio::Mixer::kMaxChannelVolume) / 127;
	vm->_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, mixerVolume);
	vm->_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, mixerVolume);
	vm->_mixer->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, mixerVolume);
	ConfMan.setInt("music_volume", (volume * 256) / 127);
	ConfMan.setInt("sfx_volume", (volume * 256) / 127);
	ConfMan.setInt("speech_volume", (volume * 256) / 127);
}

static const char *getRebel1ActionName(Common::CustomEventType customType) {
	switch (customType) {
	case kScummActionInsaneUp:
		return "up";
	case kScummActionInsaneDown:
		return "down";
	case kScummActionInsaneLeft:
		return "left";
	case kScummActionInsaneRight:
		return "right";
	case kScummActionInsaneAttack:
		return "attack";
	case kScummActionInsaneSwitch:
		return "switch";
	case kScummActionInsaneBack:
		return "back";
	case kScummActionInsaneSkip:
		return "skip";
	default:
		return "other";
	}
}

static const char *getRebel1BackendAxisName(Common::CustomEventType customType) {
	switch (customType) {
	case kScummBackendActionRebel1AxisUp:
		return "stick-up";
	case kScummBackendActionRebel1AxisDown:
		return "stick-down";
	case kScummBackendActionRebel1AxisLeft:
		return "stick-left";
	case kScummBackendActionRebel1AxisRight:
		return "stick-right";
	default:
		return "other-axis";
	}
}

static int16 normalizeRebel1MappedAxisPosition(int16 axisPosition) {
	// Custom backend axis events are half-axis magnitudes. Keymapper takes
	// ABS(rawPosition), but int16 -32768 cannot be represented as +32768.
	return axisPosition == Common::JOYAXIS_MIN ? Common::JOYAXIS_MAX : axisPosition;
}

static int getRebel1MenuCenteredX(int textWidth) {
	return (kRA1MenuLogicalWidth - textWidth) / 2;
}

static const char *getRebel1TextEntryPickerChars(bool) {
	return kRA1TextEntryPickerChars;
}

static int getRebel1TextEntryPickerCount(bool passcodeMode) {
	return passcodeMode ? kRA1PasscodePickerCount : kRA1NamePickerCount;
}

static int getRebel1PasscodeDifficulty(int passwordIndex) {
	return (passwordIndex - 1) % 3;
}

static int getRebel1PasscodeStartLevel(int passwordIndex) {
	// Original main-menu control flow groups passcodes by difficulty:
	// 1-3 resume after chapter 3, 4-6 after chapter 6, 7-9 after chapter 10,
	// 10-12 after chapter 14, and 13-15 jump to the final ending sequence.
	switch ((passwordIndex - 1) / 3) {
	case 0:
		return 4;
	case 1:
		return 7;
	case 2:
		return 11;
	case 3:
		return 15;
	case 4:
		return kRA1NumLevels + 1;
	default:
		return 0;
	}
}

// 3DO Launchme_ARMv4 FUN_000092c4 compares the entered passcode against 45
// XOR-0xAA encoded 20-byte slots at 0x262FC. The caller's Ghidra disassembly at
// 0x12A58 maps those slots in three-code difficulty groups to the next chapter.
const char *const kRebel1ThreeDOPasswords[] = {
	"BOSSK", "BOTHAN", "BORDOK",
	"ENGRET", "HERGLIC", "SKYNX",
	"RALRRA", "LEENI", "DEFEL",
	"FRIJA", "THRAWN", "JEDGAR",
	"LAFRA", "LWYLL", "MADINE",
	"DERLIN", "MAZZIC", "TARKIN",
	"MOLTOK", "JULPA", "MOTHMA",
	"MORAG", "MORRT", "GLAYYD",
	"TANTISS", "MUFTAK", "OTTEGA",
	"OSWAFL", "RASKAR", "RISHII",
	"KLAATU", "JHOFF", "IZRINA",
	"IRENEZ", "ITHOR", "KARRDE",
	"LIANNA", "UMWAK", "VONZEL",
	"PAKKA", "ORLOK", "OSSUS",
	"NORVAL", "NKLLON", "MALANI"
};

int getRebel1ThreeDOPasscodeDifficulty(int passwordIndex) {
	return (passwordIndex - 1) % 3;
}

int getRebel1ThreeDOPasscodeStartLevel(int passwordIndex) {
	const int group = (passwordIndex - 1) / 3;
	if (group < 0 || group > 14)
		return 0;

	return group == 14 ? kRA1NumLevels + 1 : group + 2;
}

static char normalizeRebel1PasscodeChar(char c) {
	if (c >= 'a' && c <= 'z')
		return c - ('a' - 'A');
	return c;
}

static RA1MenuCommand getRebel1MenuCommandFromAction(ScummAction action) {
	switch (action) {
	case kScummActionInsaneUp:
		return kRA1MenuCommandUp;
	case kScummActionInsaneDown:
		return kRA1MenuCommandDown;
	case kScummActionInsaneLeft:
		return kRA1MenuCommandLeft;
	case kScummActionInsaneRight:
		return kRA1MenuCommandRight;
	case kScummActionInsaneAttack:
		return kRA1MenuCommandAccept;
	case kScummActionInsaneSkip:
		return kRA1MenuCommandCancel;
	default:
		return kRA1MenuCommandNone;
	}
}

static RA1MenuCommand getRebel1MenuCommandFromKey(const Common::KeyState &kbd) {
	switch (kbd.keycode) {
	case Common::KEYCODE_UP:
	case Common::KEYCODE_w:
		return kRA1MenuCommandUp;
	case Common::KEYCODE_DOWN:
	case Common::KEYCODE_s:
		return kRA1MenuCommandDown;
	case Common::KEYCODE_LEFT:
	case Common::KEYCODE_a:
		return kRA1MenuCommandLeft;
	case Common::KEYCODE_RIGHT:
	case Common::KEYCODE_d:
		return kRA1MenuCommandRight;
	case Common::KEYCODE_RETURN:
	case Common::KEYCODE_KP_ENTER:
	case Common::KEYCODE_SPACE:
		return kRA1MenuCommandAccept;
	case Common::KEYCODE_ESCAPE:
		return kRA1MenuCommandCancel;
	case Common::KEYCODE_1:
		return kRA1MenuCommandSelect1;
	case Common::KEYCODE_2:
		return kRA1MenuCommandSelect2;
	case Common::KEYCODE_3:
		return kRA1MenuCommandSelect3;
	case Common::KEYCODE_4:
		return kRA1MenuCommandSelect4;
	case Common::KEYCODE_5:
		return kRA1MenuCommandSelect5;
	case Common::KEYCODE_6:
		return kRA1MenuCommandSelect6;
	default:
		return kRA1MenuCommandNone;
	}
}

static bool isRebel1TextEntryChar(bool passcodeMode, char c) {
	if (passcodeMode) {
		c = normalizeRebel1PasscodeChar(c);
		return c >= 'A' && c <= 'Z';
	}

	return c == ' ' || c == '_' ||
		(c >= 'A' && c <= 'Z') ||
		(c >= 'a' && c <= 'z');
}

static void appendRebel1TextEntryChar(char *buffer, int bufferSize, int maxChars, bool passcodeMode, char c) {
	if (!buffer || bufferSize <= 0)
		return;

	c = passcodeMode ? normalizeRebel1PasscodeChar(c) : c;
	const int len = strlen(buffer);
	if (len >= maxChars || len >= bufferSize - 1)
		return;

	buffer[len] = c;
	buffer[len + 1] = '\0';
}

static void drawRebel1MenuFrame(byte *dst, int pitch, int width, int height, int x, int y, int w) {
	if (!dst || width <= 0 || height <= 0)
		return;

	const int leftX = CLIP(x, 0, width - 1);
	const int rightX = CLIP(x + w - 1, 0, width - 1);
	const int topY = CLIP(y, 0, height - 1);
	const int bottomY = CLIP(y + kRA1MenuFrameH - 1, 0, height - 1);

	for (int px = leftX; px <= rightX; px++) {
		dst[topY * pitch + px] = kRA1MenuFrameColor;
		dst[bottomY * pitch + px] = kRA1MenuFrameColor;
	}
	for (int py = topY; py <= bottomY; py++) {
		dst[py * pitch + leftX] = kRA1MenuFrameColor;
		dst[py * pitch + rightX] = kRA1MenuFrameColor;
	}
}

int InsaneRebel1::getMainMenuItemCount() const {
	return _unlockAllLevels ? kRA1MainMenuItemCount : kRA1MainMenuItemCount - 1;
}

int InsaneRebel1::getMainMenuResultForSelection(int selection) const {
	if (selection < 0 || selection >= getMainMenuItemCount())
		return kRA1MainMenuItemCount;
	if (_unlockAllLevels || selection < 3)
		return selection + 1;
	return selection + 2;
}

void InsaneRebel1::setVirtualKeyboardVisible(bool visible) {
	if (!_vm->_system->hasFeature(OSystem::kFeatureVirtualKeyboard))
		return;

	if (_virtualKeyboardActive == visible)
		return;

	_vm->_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, visible);
	_virtualKeyboardActive = visible;
}

void InsaneRebel1::beginTextEntry(bool passcodeMode) {
	_textEntryActive = true;
	_textEntryPasscodeMode = passcodeMode;
	_textEntryDone = false;
	_textEntryCanceled = false;
	_textEntryPickerIndex = 3; // First letter in the original picker string.
	_textEntryPickerOffsetX = 0;
	// The DOS buffers include a leading '<' font marker and cap strlen() at 8.
	_textEntryMaxChars = 8;
	_textEntryBuffer[0] = '\0';

	if (passcodeMode && _maxChapterUnlocked > 0) {
		const char *password = getChapterCompletePassword(_maxChapterUnlocked);
		if (password) {
			Common::strlcpy(_textEntryBuffer, password, sizeof(_textEntryBuffer));
			_textEntryPickerIndex = 1;
		}
	}

	setVirtualKeyboardVisible(true);
}

void InsaneRebel1::finishTextEntry(bool canceled) {
	_textEntryCanceled = canceled;
	if (!canceled)
		_textEntryDone = true;
	_textEntryActive = false;
	setVirtualKeyboardVisible(false);
	_vm->_smushVideoShouldFinish = true;
}

void InsaneRebel1::selectTextEntryChar() {
	if (!_textEntryActive)
		return;

	const char *pickerChars = getRebel1TextEntryPickerChars(_textEntryPasscodeMode);
	const int pickerCount = getRebel1TextEntryPickerCount(_textEntryPasscodeMode);
	if (_textEntryPickerIndex < 0 || _textEntryPickerIndex >= pickerCount)
		return;

	const char ch = pickerChars[_textEntryPickerIndex];
	if (ch == '^') {
		const int len = strlen(_textEntryBuffer);
		if (len > 0)
			_textEntryBuffer[len - 1] = '\0';
	} else if (ch == '`') {
		finishTextEntry(false);
	} else if (ch == '_') {
		appendRebel1TextEntryChar(_textEntryBuffer, sizeof(_textEntryBuffer),
			_textEntryMaxChars, _textEntryPasscodeMode, ' ');
	} else {
		appendRebel1TextEntryChar(_textEntryBuffer, sizeof(_textEntryBuffer),
			_textEntryMaxChars, _textEntryPasscodeMode, ch);
	}
}

bool InsaneRebel1::handleTextEntryAction(ScummAction action) {
	if (!_textEntryActive)
		return false;

	const int pickerCount = getRebel1TextEntryPickerCount(_textEntryPasscodeMode);
	switch (action) {
	case kScummActionInsaneLeft:
		_textEntryPickerIndex = (_textEntryPickerIndex + pickerCount - 1) % pickerCount;
		_textEntryPickerOffsetX = -7;
		return true;
	case kScummActionInsaneRight:
		_textEntryPickerIndex = (_textEntryPickerIndex + 1) % pickerCount;
		_textEntryPickerOffsetX = 7;
		return true;
	case kScummActionInsaneAttack:
		selectTextEntryChar();
		return true;
	case kScummActionInsaneSwitch:
	case kScummActionInsaneSkip:
		finishTextEntry(true);
		return true;
	default:
		return false;
	}
}

bool InsaneRebel1::handleTextEntryKey(const Common::Event &event) {
	if (!_textEntryActive || event.type != Common::EVENT_KEYDOWN)
		return false;

	const int pickerCount = getRebel1TextEntryPickerCount(_textEntryPasscodeMode);
	switch (event.kbd.keycode) {
	case Common::KEYCODE_LEFT:
		_textEntryPickerIndex = (_textEntryPickerIndex + pickerCount - 1) % pickerCount;
		_textEntryPickerOffsetX = -7;
		return true;
	case Common::KEYCODE_RIGHT:
		_textEntryPickerIndex = (_textEntryPickerIndex + 1) % pickerCount;
		_textEntryPickerOffsetX = 7;
		return true;
	case Common::KEYCODE_RETURN:
	case Common::KEYCODE_KP_ENTER:
		finishTextEntry(false);
		return true;
	case Common::KEYCODE_ESCAPE:
		finishTextEntry(true);
		return true;
	case Common::KEYCODE_BACKSPACE: {
		const int len = strlen(_textEntryBuffer);
		if (len > 0)
			_textEntryBuffer[len - 1] = '\0';
		return true;
	}
	case Common::KEYCODE_SPACE:
		appendRebel1TextEntryChar(_textEntryBuffer, sizeof(_textEntryBuffer),
			_textEntryMaxChars, _textEntryPasscodeMode, ' ');
		return true;
	default:
		break;
	}

	char ch = (char)event.kbd.ascii;
	if (ch == '\0')
		ch = (char)event.kbd.keycode;
	if (isRebel1TextEntryChar(_textEntryPasscodeMode, ch)) {
		appendRebel1TextEntryChar(_textEntryBuffer, sizeof(_textEntryBuffer),
			_textEntryMaxChars, _textEntryPasscodeMode, ch);
		return true;
	}

	return true;
}

bool InsaneRebel1::handleMenuCommand(RA1MenuCommand command) {
	if (!_menuActive || _highScoresActive || command == kRA1MenuCommandNone)
		return false;

	if (_textEntryActive)
		return false;

	if (_levelSelectActive) {
		int col = _levelSelectSel / kRA1LevelSelectRowsPerCol;
		int row = _levelSelectSel % kRA1LevelSelectRowsPerCol;

		switch (command) {
		case kRA1MenuCommandUp:
			row = (row + kRA1LevelSelectRowsPerCol - 1) % kRA1LevelSelectRowsPerCol;
			_levelSelectSel = col * kRA1LevelSelectRowsPerCol + row;
			return true;
		case kRA1MenuCommandDown:
			row = (row + 1) % kRA1LevelSelectRowsPerCol;
			_levelSelectSel = col * kRA1LevelSelectRowsPerCol + row;
			return true;
		case kRA1MenuCommandLeft:
			if (col > 0)
				_levelSelectSel -= kRA1LevelSelectRowsPerCol;
			return true;
		case kRA1MenuCommandRight:
			if (col < 1)
				_levelSelectSel += kRA1LevelSelectRowsPerCol;
			return true;
		case kRA1MenuCommandCancel:
			_levelSelectSel = kRA1LevelSelectItemCount - 1; // Back
			// fall through
		case kRA1MenuCommandAccept:
			_menuConfirmed = true;
			_vm->_smushVideoShouldFinish = true;
			return true;
		default:
			return false;
		}
	}

	if (_optionsActive) {
		switch (command) {
		case kRA1MenuCommandUp:
			_optionsSel = (_optionsSel + kOptionsItemCount - 1) % kOptionsItemCount;
			return true;
		case kRA1MenuCommandDown:
			_optionsSel = (_optionsSel + 1) % kOptionsItemCount;
			return true;
		case kRA1MenuCommandLeft:
			if (_optionsSel == 7) {
				setRebel1Volume(_vm, _optVolume, -5);
				applyAudioOptions();
			}
			return true;
		case kRA1MenuCommandRight:
			if (_optionsSel == 7) {
				setRebel1Volume(_vm, _optVolume, 5);
				applyAudioOptions();
			}
			return true;
		case kRA1MenuCommandCancel:
			_optionsSel = 0;
			// fall through
		case kRA1MenuCommandAccept:
			_menuConfirmed = true;
			_vm->_smushVideoShouldFinish = true;
			return true;
		default:
			return false;
		}
	}

	const int mainMenuItemCount = getMainMenuItemCount();

	switch (command) {
	case kRA1MenuCommandUp:
		_menuSelection = (_menuSelection + mainMenuItemCount - 1) % mainMenuItemCount;
		return true;
	case kRA1MenuCommandDown:
		_menuSelection = (_menuSelection + 1) % mainMenuItemCount;
		return true;
	case kRA1MenuCommandCancel:
		_menuSelection = mainMenuItemCount - 1;
		// fall through
	case kRA1MenuCommandAccept:
		_menuConfirmed = true;
		_vm->_smushVideoShouldFinish = true;
		return true;
	case kRA1MenuCommandSelect1:
	case kRA1MenuCommandSelect2:
	case kRA1MenuCommandSelect3:
	case kRA1MenuCommandSelect4:
	case kRA1MenuCommandSelect5:
	case kRA1MenuCommandSelect6: {
		const int selection = command - kRA1MenuCommandSelect1;
		if (selection >= mainMenuItemCount)
			return false;
		_menuSelection = selection;
		_menuConfirmed = true;
		_vm->_smushVideoShouldFinish = true;
		return true;
	}
	default:
		return false;
	}
}

bool InsaneRebel1::handleControllerMenuAction(ScummAction action) {
	if (!_menuActive || _highScoresActive)
		return false;

	if (_textEntryActive)
		return handleTextEntryAction(action);

	if (action == kScummActionInsaneSwitch || action == kScummActionInsaneSkip) {
		if (_levelSelectActive || _optionsActive)
			return handleMenuCommand(kRA1MenuCommandCancel);
		return true;
	}

	return handleMenuCommand(getRebel1MenuCommandFromAction(action));
}

bool InsaneRebel1::handleControllerMenuAxis(int16 oldAxisX, int16 oldAxisY) {
	if (!_menuActive || _highScoresActive)
		return false;

	// RA1 maps stick axes to backend axis events for analog gameplay.
	// Menus still need edge-triggered digital navigation from those same axes.
	// Match the original raw-input convention: positive Y is stick-down in
	// menus, and the Y-flip option reverses that interpretation.
	const int oldX = getRebel1MenuAxisDirection(oldAxisX);
	const int oldY = getRebel1MenuAxisDirection(oldAxisY);
	const int newX = getRebel1MenuAxisDirection(_joystickAxisX);
	const int newY = getRebel1MenuAxisDirection(_joystickAxisY);

	if (newY != oldY && newY != 0)
		return handleMenuCommand((newY > 0) != _optControlsYFlip ? kRA1MenuCommandDown : kRA1MenuCommandUp);
	if (newX != oldX && newX != 0)
		return handleMenuCommand(newX > 0 ? kRA1MenuCommandRight : kRA1MenuCommandLeft);

	return false;
}

void InsaneRebel1::openGameplayMainMenu() {
	if (!_player)
		return;

	const bool wasPaused = _player->_paused;
	if (!wasPaused)
		_player->pause();

	const bool restoreGamepad = _iosGamepadControllerState.isEnabled();
	restoreIOSGamepadController();
	_vm->openMainMenuDialog();
	if (restoreGamepad && _interactiveVideoActive && !_menuActive && !_vm->shouldQuit())
		enableIOSGamepadController();

	if (!wasPaused)
		_player->unpause();
}

// Extra feature, not in the original game: let the player navigate and
// activate the front-end menus with the mouse. Hovering highlights an item and a left
// click activates it (same as pressing accept). The item hit-rectangles mirror the
// highlight frames drawn by the render*Overlay() functions, so they share the
// kRA1*FrameYBase / kRA1LevelSelect* geometry constants. Returns true if consumed.
bool InsaneRebel1::handleMenuMouse(const Common::Event &event) {
	if (!_menuActive || _textEntryActive || _highScoresActive)
		return false;
	if (event.type != Common::EVENT_MOUSEMOVE && event.type != Common::EVENT_LBUTTONDOWN)
		return false;

	const int mx = event.mouse.x;
	const int my = event.mouse.y;
	int *selection = nullptr;
	int hit = -1;

	if (_levelSelectActive) {
		selection = &_levelSelectSel;
		for (int i = 0; i < kRA1LevelSelectItemCount; i++) {
			const int col = i / kRA1LevelSelectRowsPerCol;
			const int row = i % kRA1LevelSelectRowsPerCol;
			const int frameX = (col == 0) ? kRA1LevelSelectLeftX : kRA1LevelSelectRightX;
			const int frameY = row * kRA1MenuRowH + kRA1LevelSelectFrameYBase;
			if (mx >= frameX && mx < frameX + kRA1LevelSelectColW &&
				my >= frameY && my < frameY + kRA1MenuFrameH) {
				hit = i;
				break;
			}
		}
	} else if (_optionsActive) {
		selection = &_optionsSel;
		for (int i = 0; i < kOptionsItemCount; i++) {
			const int frameY = (i + 1) * kRA1MenuRowH + kRA1OptionsFrameYBase;
			if (mx >= kRA1MenuFrameX && mx < kRA1MenuFrameX + kRA1MenuFrameW &&
				my >= frameY && my < frameY + kRA1MenuFrameH) {
				hit = i;
				break;
			}
		}
	} else {
		selection = &_menuSelection;
		const int mainMenuItemCount = getMainMenuItemCount();
		for (int i = 0; i < mainMenuItemCount; i++) {
			const int frameY = (i + 1) * kRA1MenuRowH + kRA1MainMenuFrameYBase;
			if (mx >= kRA1MenuFrameX && mx < kRA1MenuFrameX + kRA1MenuFrameW &&
				my >= frameY && my < frameY + kRA1MenuFrameH) {
				hit = i;
				break;
			}
		}
	}

	if (hit < 0)
		return false;

	_activeInputSource = kInputSourceMouse;
	*selection = hit;

	// A left click activates the hovered item, just like pressing accept.
	if (event.type == Common::EVENT_LBUTTONDOWN)
		handleMenuCommand(kRA1MenuCommandAccept);

	return true;
}

bool InsaneRebel1::notifyEvent(const Common::Event &event) {
	// Global dialogs pause the engine while their modal event loop runs.
	// Do not consume those mouse/key events as RA1 gameplay/menu input, or the
	// dialog buttons cannot receive clicks while an interactive video is active.
	if (_vm->isPaused())
		return false;

	if (isTouchscreenActive() && !_interactiveVideoActive && !_menuActive &&
			event.type == Common::EVENT_LBUTTONDOWN) {
		_vm->_smushVideoShouldFinish = true;
		return true;
	}

	if (_interactiveVideoActive && !_menuActive && !isTouchscreenActive() &&
			getEffectiveGameOpcode() == 0x07 &&
			event.type == Common::EVENT_MOUSEMOVE && _gameplayMouseSettleUntil != 0) {
		const uint32 now = _vm->_system->getMillis();
		if (now < _gameplayMouseSettleUntil) {
			const int jumpX = event.mouse.x - _vm->_mouse.x;
			const int jumpY = event.mouse.y - _vm->_mouse.y;
			const bool largeAbsoluteJump =
				ABS(jumpX) >= kRA1GameplayMouseSettleJumpThreshold ||
				ABS(jumpY) >= kRA1GameplayMouseSettleJumpThreshold;
			const bool smallRelativeMove =
				ABS((int)event.relMouse.x) < kRA1GameplayMouseSettleRelativeThreshold &&
				ABS((int)event.relMouse.y) < kRA1GameplayMouseSettleRelativeThreshold;
			const bool nearWindowEdge =
				event.mouse.x <= kRA1GameplayMouseSettleEdgeMargin ||
				event.mouse.x >= kRA1GameplayMouseMaxX - kRA1GameplayMouseSettleEdgeMargin ||
				event.mouse.y <= kRA1GameplayMouseSettleEdgeMargin ||
				event.mouse.y >= kRA1GameplayMouseMaxY - kRA1GameplayMouseSettleEdgeMargin;

			if (largeAbsoluteJump && smallRelativeMove && nearWindowEdge) {
				const int recenterX = _vm->_mouse.x;
				const int recenterY = _vm->_mouse.y;
				_gameplayMouseSettleUntil = now + kRA1GameplayMouseSettleExtendMs;
				warpGameplayMouseNow(recenterX, recenterY);

				debugC(DEBUG_INSANE, "mouse settle: suppress pos=(%d,%d) rel=(%d,%d) current=(%d,%d) until=%u opcode=0x%X",
					event.mouse.x, event.mouse.y, event.relMouse.x, event.relMouse.y,
					_vm->_mouse.x, _vm->_mouse.y, _gameplayMouseSettleUntil,
					getEffectiveGameOpcode());
				return true;
			}
		}

		_gameplayMouseSettleUntil = 0;
	}

	if (event.type == Common::EVENT_MOUSEMOVE) {
		if (_gamepadAimActive && usesGamepadReticleAimForCurrentFrame() &&
				_interactiveVideoActive && !_menuActive &&
				(event.relMouse.x != 0 || event.relMouse.y != 0))
			_gamepadAimActive = false;
		_activeInputSource = kInputSourceMouse;
	}

	// Android direct touch reports taps as left-clicks. Treat those as mouse input
	// during gameplay so tap-to-aim-and-fire works even if a joystick event was last.
	if (isTouchscreenActive() && _interactiveVideoActive && !_menuActive &&
			event.type == Common::EVENT_LBUTTONDOWN)
		_activeInputSource = kInputSourceMouse;

	// Extra feature: mouse navigation/clicking of the RA1 front-end menus.
	if (handleMenuMouse(event))
		return true;

	if (event.type == Common::EVENT_JOYAXIS_MOTION) {
		_lastJoystickAxisEventTime = _vm->_system->getMillis();
		debugC(DEBUG_INSANE, "input raw-joy-axis: axis=%d pos=%d menu=%d gameplay=%d storedAxis=(%d,%d)",
			event.joystick.axis, event.joystick.position,
			_menuActive, _interactiveVideoActive && !_menuActive,
			_joystickAxisX, _joystickAxisY);
	}

	if (event.type == Common::EVENT_JOYBUTTON_DOWN || event.type == Common::EVENT_JOYBUTTON_UP) {
		debugC(DEBUG_INSANE, "input raw-joy-button: button=%d pressed=%d menu=%d gameplay=%d storedAxis=(%d,%d)",
			event.joystick.button, event.type == Common::EVENT_JOYBUTTON_DOWN,
			_menuActive, _interactiveVideoActive && !_menuActive,
			_joystickAxisX, _joystickAxisY);
	}

	if (event.type == Common::EVENT_CUSTOM_BACKEND_ACTION_AXIS) {
		_activeInputSource = kInputSourceJoystickAnalog;
		_lastJoystickAxisEventTime = _vm->_system->getMillis();
		const int16 oldAxisX = _joystickAxisX;
		const int16 oldAxisY = _joystickAxisY;
		const int16 axisPosition = normalizeRebel1MappedAxisPosition(event.joystick.position);

		switch (event.customType) {
		case kScummBackendActionRebel1AxisUp:
			if (event.joystick.position == 0 && _joystickAxisY > 0) {
				debugC(DEBUG_INSANE, "input mapped-axis ignored-reset: %s pos=0 current=(%d,%d)",
					getRebel1BackendAxisName(event.customType), _joystickAxisX, _joystickAxisY);
				return true;
			}
			_joystickAxisY = -axisPosition;
			debugC(DEBUG_INSANE, "input mapped-axis: %s pos=%d rawPos=%d old=(%d,%d) new=(%d,%d) menu=%d gameplay=%d",
				getRebel1BackendAxisName(event.customType), axisPosition, event.joystick.position,
				oldAxisX, oldAxisY, _joystickAxisX, _joystickAxisY,
				_menuActive, _interactiveVideoActive && !_menuActive);
			if (handleControllerMenuAxis(oldAxisX, oldAxisY))
				return true;
			return true;
		case kScummBackendActionRebel1AxisDown:
			if (event.joystick.position == 0 && _joystickAxisY < 0) {
				debugC(DEBUG_INSANE, "input mapped-axis ignored-reset: %s pos=0 current=(%d,%d)",
					getRebel1BackendAxisName(event.customType), _joystickAxisX, _joystickAxisY);
				return true;
			}
			_joystickAxisY = axisPosition;
			debugC(DEBUG_INSANE, "input mapped-axis: %s pos=%d rawPos=%d old=(%d,%d) new=(%d,%d) menu=%d gameplay=%d",
				getRebel1BackendAxisName(event.customType), axisPosition, event.joystick.position,
				oldAxisX, oldAxisY, _joystickAxisX, _joystickAxisY,
				_menuActive, _interactiveVideoActive && !_menuActive);
			if (handleControllerMenuAxis(oldAxisX, oldAxisY))
				return true;
			return true;
		case kScummBackendActionRebel1AxisLeft:
			if (event.joystick.position == 0 && _joystickAxisX > 0) {
				debugC(DEBUG_INSANE, "input mapped-axis ignored-reset: %s pos=0 current=(%d,%d)",
					getRebel1BackendAxisName(event.customType), _joystickAxisX, _joystickAxisY);
				return true;
			}
			_joystickAxisX = -axisPosition;
			debugC(DEBUG_INSANE, "input mapped-axis: %s pos=%d rawPos=%d old=(%d,%d) new=(%d,%d) menu=%d gameplay=%d",
				getRebel1BackendAxisName(event.customType), axisPosition, event.joystick.position,
				oldAxisX, oldAxisY, _joystickAxisX, _joystickAxisY,
				_menuActive, _interactiveVideoActive && !_menuActive);
			if (handleControllerMenuAxis(oldAxisX, oldAxisY))
				return true;
			return true;
		case kScummBackendActionRebel1AxisRight:
			if (event.joystick.position == 0 && _joystickAxisX < 0) {
				debugC(DEBUG_INSANE, "input mapped-axis ignored-reset: %s pos=0 current=(%d,%d)",
					getRebel1BackendAxisName(event.customType), _joystickAxisX, _joystickAxisY);
				return true;
			}
			_joystickAxisX = axisPosition;
			debugC(DEBUG_INSANE, "input mapped-axis: %s pos=%d rawPos=%d old=(%d,%d) new=(%d,%d) menu=%d gameplay=%d",
				getRebel1BackendAxisName(event.customType), axisPosition, event.joystick.position,
				oldAxisX, oldAxisY, _joystickAxisX, _joystickAxisY,
				_menuActive, _interactiveVideoActive && !_menuActive);
			if (handleControllerMenuAxis(oldAxisX, oldAxisY))
				return true;
			return true;
		default:
			break;
		}
	}

	if (event.type == Common::EVENT_CUSTOM_ENGINE_ACTION_START ||
		event.type == Common::EVENT_CUSTOM_ENGINE_ACTION_END) {
		const bool pressed = (event.type == Common::EVENT_CUSTOM_ENGINE_ACTION_START);

		debugC(DEBUG_INSANE, "input mapped-action: action=%s custom=%u pressed=%d menu=%d gameplay=%d storedAxis=(%d,%d) actionState(L,R,U,D)=(%d,%d,%d,%d)",
			getRebel1ActionName(event.customType), event.customType, pressed,
			_menuActive, _interactiveVideoActive && !_menuActive,
			_joystickAxisX, _joystickAxisY,
			_vm->getActionState(kScummActionInsaneLeft),
			_vm->getActionState(kScummActionInsaneRight),
			_vm->getActionState(kScummActionInsaneUp),
			_vm->getActionState(kScummActionInsaneDown));

		if (pressed &&
			(event.customType == kScummActionInsaneUp ||
			 event.customType == kScummActionInsaneDown ||
			 event.customType == kScummActionInsaneLeft ||
			 event.customType == kScummActionInsaneRight)) {
			_activeInputSource = kInputSourceJoystickDigital;
		}

		if (!pressed && event.customType == kScummActionInsaneSwitch)
			_playerSecondaryHeld = false;

		if (event.customType == kScummActionInsaneBack) {
			if (!pressed)
				return true;
			if (_player && !_interactiveVideoActive && !_menuActive) {
				_vm->_smushVideoShouldFinish = true;
				return true;
			}
			if (_player) {
				openGameplayMainMenu();
			}
			return true;
		}

		if (_highScoresActive && pressed &&
			(event.customType == kScummActionInsaneAttack ||
			 event.customType == kScummActionInsaneSwitch ||
			 event.customType == kScummActionInsaneSkip)) {
			_vm->_smushVideoShouldFinish = true;
			return true;
		}

		if (isTouchscreenActive() && !_interactiveVideoActive && !_menuActive && pressed &&
				event.customType == kScummActionInsaneSkip) {
			_vm->_smushVideoShouldFinish = true;
			return true;
		}

		if (pressed && handleControllerMenuAction((ScummAction)event.customType))
			return true;

		if (event.customType == kScummActionInsaneSkip) {
			if (!pressed)
				return true;
			if (!_interactiveVideoActive) {
				_vm->_smushVideoShouldFinish = true;
				return true;
			}
			return true;
		}

		if (_interactiveVideoActive && !_menuActive && event.customType == kScummActionInsaneAttack) {
			_playerFired = pressed;
			return true;
		}

		if (_interactiveVideoActive && !_menuActive && event.customType == kScummActionInsaneSwitch) {
			_playerSecondaryHeld = pressed;
			return true;
		}
	}

	if (_menuActive && _textEntryActive && event.type == Common::EVENT_KEYDOWN)
		return handleTextEntryKey(event);

	if (_menuActive && event.type == Common::EVENT_KEYDOWN &&
		handleMenuCommand(getRebel1MenuCommandFromKey(event.kbd)))
		return true;

	// Shooting: mouse button during interactive gameplay — FUN_1CCA0 (0x1CCA0)
	if (_interactiveVideoActive && !_menuActive) {
		if (event.type == Common::EVENT_LBUTTONDOWN) {
			_vm->_mouse.x = event.mouse.x;
			_vm->_mouse.y = event.mouse.y;
			_playerFired = true;
			return true;
		}
		if (event.type == Common::EVENT_LBUTTONUP) {
			_vm->_mouse.x = event.mouse.x;
			_vm->_mouse.y = event.mouse.y;
			_playerFired = false;
			return true;
		}
	}

	// High scores: any key or click dismisses
	if (_highScoresActive && (event.type == Common::EVENT_KEYDOWN ||
		event.type == Common::EVENT_LBUTTONDOWN)) {
		_vm->_smushVideoShouldFinish = true;
		return true;
	}

	if (event.type == Common::EVENT_MAINMENU) {
		if (_player && !_interactiveVideoActive && !_menuActive) {
			_vm->_smushVideoShouldFinish = true;
			return true;
		}

		if (_menuActive || _highScoresActive)
			return true;

		const uint32 now = _vm->_system->getMillis();
		const uint32 elapsedSinceAxis = _lastJoystickAxisEventTime ? now - _lastJoystickAxisEventTime : 0xffffffffu;

		if (_interactiveVideoActive && !_menuActive) {
			debugC(DEBUG_INSANE, "input mainmenu-event: gameplay=1 elapsedSinceAxis=%u storedAxis=(%d,%d)",
				elapsedSinceAxis, _joystickAxisX, _joystickAxisY);
			if (elapsedSinceAxis <= kRA1JoystickAxisEscGuardMs) {
				debugC(DEBUG_INSANE, "input ignored mainmenu event after recent joystick axis movement (%u ms)", elapsedSinceAxis);
				return true;
			}

			if (_player) {
				openGameplayMainMenu();
				return true;
			}
		}
	}

	if (event.type == Common::EVENT_KEYDOWN && _player) {
		if (_interactiveVideoActive && !_menuActive &&
			event.kbd.keycode == Common::KEYCODE_ESCAPE) {
			const uint32 now = _vm->_system->getMillis();
			const uint32 elapsedSinceAxis = _lastJoystickAxisEventTime ? now - _lastJoystickAxisEventTime : 0xffffffffu;
			debugC(DEBUG_INSANE, "input keydown-escape: gameplay=1 ascii=%d flags=0x%x repeat=%d elapsedSinceAxis=%u storedAxis=(%d,%d)",
				event.kbd.ascii, event.kbd.flags, event.kbdRepeat,
				elapsedSinceAxis, _joystickAxisX, _joystickAxisY);
			if (elapsedSinceAxis <= kRA1JoystickAxisEscGuardMs) {
				debugC(DEBUG_INSANE, "input ignored ESC after recent joystick axis movement (%u ms)", elapsedSinceAxis);
				return true;
			}

			openGameplayMainMenu();
			return true;
		}

		if (_interactiveVideoActive && !_menuActive &&
			event.kbd.keycode == Common::KEYCODE_s &&
			event.kbd.hasFlags(Common::KBD_SHIFT)) {
			_interactiveVideoCheatSkipped = true;
			_vm->_smushVideoShouldFinish = true;
			return true;
		}

		if (!_interactiveVideoActive && event.kbd.keycode == Common::KEYCODE_ESCAPE) {
			_vm->_smushVideoShouldFinish = true;
			return true;
		}
	}

	return false;
}

int InsaneRebel1::getMenuTalkTextWidth(const char *text) {
	Common::String styled("<");
	styled += text;
	return getFontBankStringWidth(styled.c_str());
}

void InsaneRebel1::drawMenuTalkText(byte *dst, int pitch, int width, int height,
		int x, int y, const char *text) {
	Common::String styled("<");
	styled += text;
	drawFontBankString(dst, pitch, width, height, x, y, styled.c_str());
}

void InsaneRebel1::drawMenuTitleText(byte *dst, int pitch, int width, int height,
		int x, int y, const char *text) {
	drawFontBankString(dst, pitch, width, height, x, y, text);
}

void InsaneRebel1::renderHighScoresOverlay(byte *dst, int pitch, int width, int height) {
	// --- TOP PILOTS high score display ---
	// Original renders over O1SCORE.ANM. Title appears after frame 20,
	// entries fade in one per frame. We show all immediately.
	const int titleW = getMenuTalkTextWidth("TOP PILOTS");
	drawMenuTalkText(dst, pitch, width, height, getRebel1MenuCenteredX(titleW), 10, "TOP PILOTS");

	for (int i = 0; i < kHighScoreCount; i++) {
		const int y = 25 + i * 14;
		// Name (left side)
		drawFontBankString(dst, pitch, width, height, 40, y, _highScores[i].name);
		// Score + difficulty glyph (right side) — original format "<%ld %c"
		// Difficulty byte 0/1/2 + 0x7B = '{','|','}' tech font glyphs (easy/normal/hard)
		char scoreLine[32];
		Common::sprintf_s(scoreLine, "<%ld %c",
			(long)_highScores[i].score,
			(char)(_highScores[i].difficulty + 0x7B));
		drawFontBankString(dst, pitch, width, height, 220, y, scoreLine);
	}
}

void InsaneRebel1::renderOptionsOverlay(byte *dst, int pitch, int width, int height) {
	// --- Options submenu (matching original RunGameOptionsMenu) ---
	_optTextEnabled = ConfMan.getBool("subtitles");
	_optVolume = CLIP<int>(ConfMan.getInt("music_volume") / 2, 0, 127);

	const char *kDiffNames[3] = { "EASY", "NORMAL", "HARD" };

	const int titleW = getFontBankStringWidth("GAME OPTIONS");
	drawMenuTitleText(dst, pitch, width, height, getRebel1MenuCenteredX(titleW), 15, "GAME OPTIONS");

	// Build dynamic option strings for each row
	char diffLine[64], volLine[64];
	Common::sprintf_s(diffLine, "DIFFICULTY IS %s", kDiffNames[CLIP(_difficulty, 0, 2)]);
	Common::sprintf_s(volLine, "VOLUME AT %d PERCENT", (_optVolume * 100) / 127);

	const char *optItems[kOptionsItemCount] = {
		"EXIT MENU",
		_optRookieOneFemale ? "ROOKIE1 IS FEMALE" : "ROOKIE1 IS MALE",
		_optMusicEnabled  ? "MUSIC IS ON"             : "MUSIC IS OFF",
		_optSfxEnabled    ? "SFX AND VOICE ARE ON"    : "SFX AND VOICE ARE OFF",
		_optTextEnabled   ? "DIALOGUE TEXT IS ON"     : "DIALOGUE TEXT IS OFF",
		_optControlsYFlip ? "Y AXIS IS INVERTED"      : "Y AXIS IS NORMAL",
		_optRapidFire     ? "RAPID FIRE IS ON"        : "RAPID FIRE IS OFF",
		volLine,
		diffLine
	};

	for (int i = 0; i < kOptionsItemCount; i++) {
		const int textW = getMenuTalkTextWidth(optItems[i]);
		const int textX = getRebel1MenuCenteredX(textW);
		const int y = 0x2d + i * kRA1MenuRowH;
		drawMenuTalkText(dst, pitch, width, height, textX, y, optItems[i]);

		if (i == _optionsSel)
			drawRebel1MenuFrame(dst, pitch, width, height,
				kRA1MenuFrameX, (i + 1) * kRA1MenuRowH + kRA1OptionsFrameYBase, kRA1MenuFrameW);
	}
}

void InsaneRebel1::renderLevelSelectOverlay(byte *dst, int pitch, int width, int height) {
	// --- Extra level select submenu, styled like the original frontend menus ---
	const int titleW = getFontBankStringWidth("LEVEL SELECT");
	drawMenuTitleText(dst, pitch, width, height, getRebel1MenuCenteredX(titleW), 15, "LEVEL SELECT");

	const char *const kLevelItems[kRA1NumLevels] = {
		"1  TRAINING  ",
		"2  ASTEROIDS ",
		"3  KOLAADOR  ",
		"4  STAR DESTR",
		"5  TATOOINE  ",
		"6  AST CHASE ",
		"7  PROBES    ",
		"8  WALKERS   ",
		"9  TROOPERS  ",
		"10 TRANSPORT ",
		"11 YAVIN     ",
		"12 TIE ATK   ",
		"13 DS SURFACE",
		"14 CANNON    ",
		"15 DS TRENCH "
	};
	const char *const kBackItem = "BACK";

	const int menuY = 0x2d;
	const int leftFrameX = kRA1LevelSelectLeftX;
	const int rightFrameX = kRA1LevelSelectRightX;
	const int columnW = kRA1LevelSelectColW;
	int levelTextW = 0;

	for (int i = 0; i < kRA1NumLevels; i++)
		levelTextW = MAX(levelTextW, getMenuTalkTextWidth(kLevelItems[i]));

	for (int i = 0; i < kRA1LevelSelectItemCount; i++) {
		const int col = i / kRA1LevelSelectRowsPerCol;
		const int row = i % kRA1LevelSelectRowsPerCol;
		const int frameX = (col == 0) ? leftFrameX : rightFrameX;
		const int y = menuY + row * kRA1MenuRowH;
		const bool levelItem = i < kRA1NumLevels;
		const char *text = levelItem ? kLevelItems[i] : kBackItem;
		const int textW = levelItem ? levelTextW : getMenuTalkTextWidth(text);
		const int textX = frameX + (columnW - textW) / 2;

		drawMenuTalkText(dst, pitch, width, height, textX, y, text);

		if (i == _levelSelectSel)
			drawRebel1MenuFrame(dst, pitch, width, height,
				frameX, row * kRA1MenuRowH + kRA1LevelSelectFrameYBase, columnW);
	}
}

void InsaneRebel1::renderMainMenuItems(byte *dst, int pitch, int width, int height) {
	// --- Main menu ---
	const char *const kMenuItems[kRA1MainMenuItemCount] = {
		"START NEW GAME",
		"GAME OPTIONS",
		"ENTER PASSCODE",
		"LEVEL SELECT",
		"CONTINUE DEMO",
		"EXIT TO DOS"
	};
	const char *const kMenuItemsLocked[kRA1MainMenuItemCount - 1] = {
		"START NEW GAME",
		"GAME OPTIONS",
		"ENTER PASSCODE",
		"CONTINUE DEMO",
		"EXIT TO DOS"
	};
	const char *const *menuItems = _unlockAllLevels ? kMenuItems : kMenuItemsLocked;
	const int mainMenuItemCount = getMainMenuItemCount();

	// Center title
	const int titleW = getFontBankStringWidth("MAIN MENU");
	const int titleX = getRebel1MenuCenteredX(titleW);
	drawMenuTitleText(dst, pitch, width, height, titleX, 30, "MAIN MENU");

	// Draw menu items centered horizontally
	for (int i = 0; i < mainMenuItemCount; i++) {
		const int textW = getMenuTalkTextWidth(menuItems[i]);
		const int textX = getRebel1MenuCenteredX(textW);
		const int y = 0x3c + i * kRA1MenuRowH;

		drawMenuTalkText(dst, pitch, width, height, textX, y, menuItems[i]);

		if (i == _menuSelection)
			drawRebel1MenuFrame(dst, pitch, width, height,
				kRA1MenuFrameX, (i + 1) * kRA1MenuRowH + kRA1MainMenuFrameYBase, kRA1MenuFrameW);
	}
}

void InsaneRebel1::renderMainMenuOverlay(byte *dst, int pitch, int width, int height) {
	_menuFrameCounter++;

	// The menus are mouse-clickable, so keep the default
	// arrow cursor visible. SmushPlayer::play() hides the system cursor for the whole
	// video (and re-hides it every video), so re-assert visibility each rendered frame
	// while a menu overlay is on screen. The arrow bitmap/palette is set in
	// playMenuBackground(); gameplay hides it again via captureInteractiveVideoInput().
	CursorMan.showMouse(true);

	if (_textEntryActive) {
		renderTextEntryOverlay(dst, pitch, width, height);
		return;
	}

	if (_highScoresActive) {
		renderHighScoresOverlay(dst, pitch, width, height);
		return;
	}

	if (_optionsActive) {
		renderOptionsOverlay(dst, pitch, width, height);
		return;
	}

	if (_levelSelectActive) {
		renderLevelSelectOverlay(dst, pitch, width, height);
		return;
	}

	renderMainMenuItems(dst, pitch, width, height);
}

void InsaneRebel1::renderTextEntryOverlay(byte *dst, int pitch, int width, int height) {
	auto drawCenteredTalkText = [&](int y, const char *text) {
		const int textW = getMenuTalkTextWidth(text);
		drawMenuTalkText(dst, pitch, width, height, getRebel1MenuCenteredX(textW), y, text);
	};
	auto drawCenteredRawChar = [&](int centerX, int y, char ch) {
		char text[2] = { ch, '\0' };
		const int textW = getFontBankStringWidth(text);
		drawFontBankString(dst, pitch, width, height, centerX - textW / 2, y, text);
	};

	if (_textEntryPasscodeMode) {
		drawCenteredTalkText(0x4b, "ENTER PASSCODE");
		drawCenteredTalkText(0x5f, _textEntryBuffer);
	} else {
		char scoreText[40];
		Common::sprintf_s(scoreText, "SCORE: %ld", (long)_score);
		drawCenteredTalkText(0x37, scoreText);
		drawCenteredTalkText(0x4b, "NEW HIGH SCORE");
		drawCenteredTalkText(0x5f, _textEntryBuffer);
	}

	const char *pickerChars = getRebel1TextEntryPickerChars(_textEntryPasscodeMode);
	const int pickerCount = getRebel1TextEntryPickerCount(_textEntryPasscodeMode);
	const int prevIndex = (_textEntryPickerIndex + pickerCount - 1) % pickerCount;
	const int nextIndex = (_textEntryPickerIndex + 1) % pickerCount;
	drawCenteredRawChar(0x91 + _textEntryPickerOffsetX, 0x6e, pickerChars[prevIndex]);
	drawCenteredRawChar(0xa0 + _textEntryPickerOffsetX, 0x6e, pickerChars[_textEntryPickerIndex]);
	drawCenteredRawChar(0xaf + _textEntryPickerOffsetX, 0x6e, pickerChars[nextIndex]);
	_textEntryPickerOffsetX = 0;
}

void InsaneRebel1::playMenuBackground() {
	_menuActive = true;
	_menuConfirmed = false;
	_menuFrameCounter = 0;
	// Show the built-in arrow pointer for the mouse-clickable menus. Set it once
	// here; renderMainMenuOverlay() re-asserts showMouse() each frame because the SMUSH
	// player forces the cursor off while a video plays. disableCursorPalette(false) ensures
	// the arrow's own CLUT palette is used rather than the game palette.
	CursorMan.disableCursorPalette(false);
	CursorMan.setDefaultArrowCursor();
	clearVideoBuffer();
	playCinematic("OPEN/O1OPTION.ANM");
	_menuActive = false;
}

bool InsaneRebel1::runTextEntryMenuLoop() {
	while (!shouldAbortGameFlow() && !_textEntryDone && !_textEntryCanceled)
		playMenuBackground();

	setVirtualKeyboardVisible(false);
	return !shouldAbortGameFlow() && !_textEntryCanceled;
}

int InsaneRebel1::runMainMenu() {
	_menuSelection = 0;
	while (!shouldAbortGameFlow()) {
		playMenuBackground();

		if (shouldAbortGameFlow())
			return kRA1MainMenuItemCount;

		if (_menuConfirmed)
			return getMainMenuResultForSelection(_menuSelection);
	}

	return kRA1MainMenuItemCount;
}

int InsaneRebel1::runPasscodeEntryDialog() {
	beginTextEntry(true);

	if (!runTextEntryMenuLoop())
		return 0;

	for (int i = 1; i <= kRA1NumLevels; i++) {
		const char *password = getChapterCompletePassword(i);
		if (password && !scumm_stricmp(_textEntryBuffer, password)) {
			const int targetLevel = getRebel1PasscodeStartLevel(i);
			if (targetLevel == 0)
				return 0;

			_difficulty = getRebel1PasscodeDifficulty(i);
			_maxChapterUnlocked = MAX<int16>(_maxChapterUnlocked, i);
			if (targetLevel <= kRA1NumLevels)
				_startLevel = targetLevel;
			debugC(DEBUG_INSANE, "passcode accepted: slot=%d password=%s difficulty=%d target=%d",
				i, password, _difficulty, targetLevel);
			return targetLevel;
		}
	}

	for (int i = 1; i <= (int)ARRAYSIZE(kRebel1ThreeDOPasswords); i++) {
		const char *password = kRebel1ThreeDOPasswords[i - 1];
		if (!scumm_stricmp(_textEntryBuffer, password)) {
			const int targetLevel = getRebel1ThreeDOPasscodeStartLevel(i);
			if (targetLevel == 0)
				return 0;

			_difficulty = getRebel1ThreeDOPasscodeDifficulty(i);
			if (targetLevel <= kRA1NumLevels)
				_startLevel = targetLevel;
			debugC(DEBUG_INSANE, "3DO passcode accepted: slot=%d password=%s difficulty=%d target=%d",
				i, password, _difficulty, targetLevel);
			return targetLevel;
		}
	}

	debugC(DEBUG_INSANE, "passcode rejected: '%s'", _textEntryBuffer);
	return 0;
}

bool InsaneRebel1::runHighScoreNameEntry() {
	int slot = 0;
	while (slot < kHighScoreCount && _highScores[slot].score >= _score)
		slot++;
	if (slot >= kHighScoreCount)
		return false;

	for (int i = kHighScoreCount - 1; i > slot; i--)
		_highScores[i] = _highScores[i - 1];

	_highScores[slot].score = _score;
	_highScores[slot].difficulty = _difficulty;
	Common::strlcpy(_highScores[slot].name, "<", sizeof(_highScores[slot].name));
	_highScoreEntryIndex = slot;

	beginTextEntry(false);
	runTextEntryMenuLoop();

	Common::String storedName("<");
	storedName += _textEntryBuffer;
	Common::strlcpy(_highScores[slot].name, storedName.c_str(), sizeof(_highScores[slot].name));
	_highScores[slot].difficulty = _difficulty;
	_highScoreEntryIndex = -1;
	debugC(DEBUG_INSANE, "high score inserted: slot=%d name=%s score=%ld difficulty=%d",
		slot, _highScores[slot].name, (long)_highScores[slot].score, _highScores[slot].difficulty);
	return true;
}

void InsaneRebel1::runOptionsMenu() {
	_optionsSel = 0;
	_optionsActive = true;

	while (!shouldAbortGameFlow()) {
		playMenuBackground();

		if (shouldAbortGameFlow())
			break;

		if (_menuConfirmed) {
			switch (_optionsSel) {
			case 0: // EXIT MENU
				_optionsActive = false;
				return;
			case 1: // Toggle Rookie One gender
				_optRookieOneFemale = !_optRookieOneFemale;
				_player->_smushAudioTable[4] = _optRookieOneFemale ? 1 : 0;
				break;
			case 2: // Toggle music
				_optMusicEnabled = !_optMusicEnabled;
				applyAudioOptions();
				break;
			case 3: // Toggle SFX + Voice
				_optSfxEnabled = !_optSfxEnabled;
				applyAudioOptions();
				break;
			case 4: // Toggle dialogue text
				_optTextEnabled = !ConfMan.getBool("subtitles");
				ConfMan.setBool("subtitles", _optTextEnabled);
				break;
			case 5: // Toggle Y-flip controls
				_optControlsYFlip = !_optControlsYFlip;
				break;
			case 6: // Toggle held-fire shooting
				_optRapidFire = !_optRapidFire;
				break;
			case 7: // Volume — adjusted via left/right in notifyEvent
				break;
			case 8: // Cycle difficulty
				_difficulty = (_difficulty + 1) % 3;
				loadTuningForLevel(0);
				break;
			}
		}
	}
	_optionsActive = false;
}

int InsaneRebel1::runLevelSelectMenu() {
	if (!_unlockAllLevels)
		return 0;

	_levelSelectSel = CLIP(_startLevel - 1, 0, kRA1NumLevels - 1);
	_levelSelectActive = true;

	while (!shouldAbortGameFlow()) {
		playMenuBackground();

		if (shouldAbortGameFlow())
			break;

		if (_menuConfirmed) {
			if (_levelSelectSel < kRA1NumLevels) {
				_levelSelectActive = false;
				return _levelSelectSel + 1;  // 1-based level number
			}
			// BACK
			_levelSelectActive = false;
			return 0;
		}
	}
	_levelSelectActive = false;
	return 0;
}

void InsaneRebel1::showHighScores() {
	// Original plays O1SCORE.ANM with TOP PILOTS overlay, dismissable by any key.
	_highScoresActive = true;
	_menuActive = true;
	_menuFrameCounter = 0;
	clearVideoBuffer();
	playCinematic("OPEN/O1SCORE.ANM");
	_menuActive = false;
	_highScoresActive = false;
}

} // End of namespace Scumm
