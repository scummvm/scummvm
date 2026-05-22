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

#include "scumm/scumm_v7.h"
#include "scumm/smush/smush_player.h"
#include "scumm/insane/rebel1/rebel.h"

namespace Scumm {

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
const uint32 kRA1JoystickAxisEscGuardMs = 250;

static int getRebel1MenuAxisDirection(int16 axisValue) {
	if (axisValue >= kRA1MenuAxisThreshold)
		return 1;
	if (axisValue <= -kRA1MenuAxisThreshold)
		return -1;
	return 0;
}

static void setRebel1Volume(ScummEngine_v7 *vm, int &volume, int delta) {
	volume = CLIP<int>(volume + delta, 0, 127);
	vm->_mixer->setVolumeForSoundType(Audio::Mixer::kPlainSoundType,
		(volume * Audio::Mixer::kMaxChannelVolume) / 127);
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

bool InsaneRebel1::handleControllerMenuAction(ScummAction action) {
	if (!_menuActive || _highScoresActive)
		return false;

	if (_levelSelectActive) {
		int col = _levelSelectSel / kRA1LevelSelectRowsPerCol;
		int row = _levelSelectSel % kRA1LevelSelectRowsPerCol;

		switch (action) {
		case kScummActionInsaneUp:
			row = (row + kRA1LevelSelectRowsPerCol - 1) % kRA1LevelSelectRowsPerCol;
			_levelSelectSel = col * kRA1LevelSelectRowsPerCol + row;
			return true;
		case kScummActionInsaneDown:
			row = (row + 1) % kRA1LevelSelectRowsPerCol;
			_levelSelectSel = col * kRA1LevelSelectRowsPerCol + row;
			return true;
		case kScummActionInsaneLeft:
			if (col > 0)
				_levelSelectSel -= kRA1LevelSelectRowsPerCol;
			return true;
		case kScummActionInsaneRight:
			if (col < 1)
				_levelSelectSel += kRA1LevelSelectRowsPerCol;
			return true;
		case kScummActionInsaneAttack:
			_menuConfirmed = true;
			_vm->_smushVideoShouldFinish = true;
			return true;
		default:
			return false;
		}
	}

	if (_optionsActive) {
		switch (action) {
		case kScummActionInsaneUp:
			_optionsSel = (_optionsSel + kOptionsItemCount - 1) % kOptionsItemCount;
			return true;
		case kScummActionInsaneDown:
			_optionsSel = (_optionsSel + 1) % kOptionsItemCount;
			return true;
		case kScummActionInsaneLeft:
			if (_optionsSel == 6)
				setRebel1Volume(_vm, _optVolume, -5);
			return true;
		case kScummActionInsaneRight:
			if (_optionsSel == 6)
				setRebel1Volume(_vm, _optVolume, 5);
			return true;
		case kScummActionInsaneAttack:
			_menuConfirmed = true;
			_vm->_smushVideoShouldFinish = true;
			return true;
		default:
			return false;
		}
	}

	switch (action) {
	case kScummActionInsaneUp:
		_menuSelection = (_menuSelection + 4) % 5;
		return true;
	case kScummActionInsaneDown:
		_menuSelection = (_menuSelection + 1) % 5;
		return true;
	case kScummActionInsaneAttack:
		_menuConfirmed = true;
		_vm->_smushVideoShouldFinish = true;
		return true;
	default:
		return false;
	}
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
		return handleControllerMenuAction((newY > 0) != _optControlsYFlip ? kScummActionInsaneDown : kScummActionInsaneUp);
	if (newX != oldX && newX != 0)
		return handleControllerMenuAction(newX > 0 ? kScummActionInsaneRight : kScummActionInsaneLeft);

	return false;
}

bool InsaneRebel1::notifyEvent(const Common::Event &event) {
	// Global ScummVM dialogs pause the engine while their modal event loop runs.
	// Do not consume those mouse/key events as RA1 gameplay/menu input, or the
	// dialog buttons cannot receive clicks while an interactive video is active.
	if (_vm->isPaused())
		return false;

	if (event.type == Common::EVENT_MOUSEMOVE && !_mouseRecentering) {
		_activeInputSource = kInputSourceMouse;
	}

	if (event.type == Common::EVENT_JOYAXIS_MOTION) {
		_lastJoystickAxisEventTime = _vm->_system->getMillis();
		debug(1, "RA1 input raw-joy-axis: axis=%d pos=%d menu=%d gameplay=%d storedAxis=(%d,%d)",
			event.joystick.axis, event.joystick.position,
			_menuActive, _interactiveVideoActive && !_menuActive,
			_joystickAxisX, _joystickAxisY);
	}

	if (event.type == Common::EVENT_JOYBUTTON_DOWN || event.type == Common::EVENT_JOYBUTTON_UP) {
		debug(1, "RA1 input raw-joy-button: button=%d pressed=%d menu=%d gameplay=%d storedAxis=(%d,%d)",
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
				debug(1, "RA1 input mapped-axis ignored-reset: %s pos=0 current=(%d,%d)",
					getRebel1BackendAxisName(event.customType), _joystickAxisX, _joystickAxisY);
				return true;
			}
			_joystickAxisY = -axisPosition;
			debug(1, "RA1 input mapped-axis: %s pos=%d rawPos=%d old=(%d,%d) new=(%d,%d) menu=%d gameplay=%d",
				getRebel1BackendAxisName(event.customType), axisPosition, event.joystick.position,
				oldAxisX, oldAxisY, _joystickAxisX, _joystickAxisY,
				_menuActive, _interactiveVideoActive && !_menuActive);
			if (handleControllerMenuAxis(oldAxisX, oldAxisY))
				return true;
			return true;
		case kScummBackendActionRebel1AxisDown:
			if (event.joystick.position == 0 && _joystickAxisY < 0) {
				debug(1, "RA1 input mapped-axis ignored-reset: %s pos=0 current=(%d,%d)",
					getRebel1BackendAxisName(event.customType), _joystickAxisX, _joystickAxisY);
				return true;
			}
			_joystickAxisY = axisPosition;
			debug(1, "RA1 input mapped-axis: %s pos=%d rawPos=%d old=(%d,%d) new=(%d,%d) menu=%d gameplay=%d",
				getRebel1BackendAxisName(event.customType), axisPosition, event.joystick.position,
				oldAxisX, oldAxisY, _joystickAxisX, _joystickAxisY,
				_menuActive, _interactiveVideoActive && !_menuActive);
			if (handleControllerMenuAxis(oldAxisX, oldAxisY))
				return true;
			return true;
		case kScummBackendActionRebel1AxisLeft:
			if (event.joystick.position == 0 && _joystickAxisX > 0) {
				debug(1, "RA1 input mapped-axis ignored-reset: %s pos=0 current=(%d,%d)",
					getRebel1BackendAxisName(event.customType), _joystickAxisX, _joystickAxisY);
				return true;
			}
			_joystickAxisX = -axisPosition;
			debug(1, "RA1 input mapped-axis: %s pos=%d rawPos=%d old=(%d,%d) new=(%d,%d) menu=%d gameplay=%d",
				getRebel1BackendAxisName(event.customType), axisPosition, event.joystick.position,
				oldAxisX, oldAxisY, _joystickAxisX, _joystickAxisY,
				_menuActive, _interactiveVideoActive && !_menuActive);
			if (handleControllerMenuAxis(oldAxisX, oldAxisY))
				return true;
			return true;
		case kScummBackendActionRebel1AxisRight:
			if (event.joystick.position == 0 && _joystickAxisX < 0) {
				debug(1, "RA1 input mapped-axis ignored-reset: %s pos=0 current=(%d,%d)",
					getRebel1BackendAxisName(event.customType), _joystickAxisX, _joystickAxisY);
				return true;
			}
			_joystickAxisX = axisPosition;
			debug(1, "RA1 input mapped-axis: %s pos=%d rawPos=%d old=(%d,%d) new=(%d,%d) menu=%d gameplay=%d",
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

		debug(1, "RA1 input mapped-action: action=%s custom=%u pressed=%d menu=%d gameplay=%d storedAxis=(%d,%d) actionState(L,R,U,D)=(%d,%d,%d,%d)",
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

		if (_highScoresActive && pressed && event.customType == kScummActionInsaneAttack) {
			_vm->_smushVideoShouldFinish = true;
			return true;
		}

		if (pressed && handleControllerMenuAction((ScummAction)event.customType))
			return true;

		if (_interactiveVideoActive && !_menuActive && event.customType == kScummActionInsaneAttack) {
			_playerFired = pressed;
			return true;
		}
	}

	if (_menuActive && _levelSelectActive && event.type == Common::EVENT_KEYDOWN) {
		int col = _levelSelectSel / kRA1LevelSelectRowsPerCol;
		int row = _levelSelectSel % kRA1LevelSelectRowsPerCol;
		switch (event.kbd.keycode) {
		case Common::KEYCODE_UP:
		case Common::KEYCODE_w:
			row = (row + kRA1LevelSelectRowsPerCol - 1) % kRA1LevelSelectRowsPerCol;
			_levelSelectSel = col * kRA1LevelSelectRowsPerCol + row;
			return true;
		case Common::KEYCODE_DOWN:
		case Common::KEYCODE_s:
			row = (row + 1) % kRA1LevelSelectRowsPerCol;
			_levelSelectSel = col * kRA1LevelSelectRowsPerCol + row;
			return true;
		case Common::KEYCODE_LEFT:
		case Common::KEYCODE_a:
			if (col > 0)
				_levelSelectSel -= kRA1LevelSelectRowsPerCol;
			return true;
		case Common::KEYCODE_RIGHT:
		case Common::KEYCODE_d:
			if (col < 1)
				_levelSelectSel += kRA1LevelSelectRowsPerCol;
			return true;
		case Common::KEYCODE_RETURN:
		case Common::KEYCODE_KP_ENTER:
		case Common::KEYCODE_SPACE:
			_menuConfirmed = true;
			_vm->_smushVideoShouldFinish = true;
			return true;
		case Common::KEYCODE_ESCAPE:
			_levelSelectSel = kRA1LevelSelectItemCount - 1; // Back
			_menuConfirmed = true;
			_vm->_smushVideoShouldFinish = true;
			return true;
		default:
			break;
		}
	}

	if (_menuActive && _optionsActive && event.type == Common::EVENT_KEYDOWN) {
		switch (event.kbd.keycode) {
		case Common::KEYCODE_UP:
		case Common::KEYCODE_w:
			_optionsSel = (_optionsSel + kOptionsItemCount - 1) % kOptionsItemCount;
			return true;
		case Common::KEYCODE_DOWN:
		case Common::KEYCODE_s:
			_optionsSel = (_optionsSel + 1) % kOptionsItemCount;
			return true;
		case Common::KEYCODE_LEFT:
		case Common::KEYCODE_a:
			// Volume down when on volume row (row 6)
			if (_optionsSel == 6)
				setRebel1Volume(_vm, _optVolume, -5);
			return true;
		case Common::KEYCODE_RIGHT:
		case Common::KEYCODE_d:
			// Volume up when on volume row (row 6)
			if (_optionsSel == 6)
				setRebel1Volume(_vm, _optVolume, 5);
			return true;
		case Common::KEYCODE_RETURN:
		case Common::KEYCODE_KP_ENTER:
		case Common::KEYCODE_SPACE:
			_menuConfirmed = true;
			_vm->_smushVideoShouldFinish = true;
			return true;
		case Common::KEYCODE_ESCAPE:
			_optionsSel = 0;
			_menuConfirmed = true;
			_vm->_smushVideoShouldFinish = true;
			return true;
		default:
			break;
		}
	}

	if (_menuActive && !_optionsActive && !_levelSelectActive && event.type == Common::EVENT_KEYDOWN) {
		switch (event.kbd.keycode) {
		case Common::KEYCODE_UP:
		case Common::KEYCODE_w:
			_menuSelection = (_menuSelection + 4) % 5;
			return true;
		case Common::KEYCODE_DOWN:
		case Common::KEYCODE_s:
			_menuSelection = (_menuSelection + 1) % 5;
			return true;
		case Common::KEYCODE_RETURN:
		case Common::KEYCODE_KP_ENTER:
		case Common::KEYCODE_SPACE:
			_menuConfirmed = true;
			_vm->_smushVideoShouldFinish = true;
			return true;
		case Common::KEYCODE_1:
		case Common::KEYCODE_2:
		case Common::KEYCODE_3:
		case Common::KEYCODE_4:
		case Common::KEYCODE_5:
			_menuSelection = event.kbd.keycode - Common::KEYCODE_1;
			_menuConfirmed = true;
			_vm->_smushVideoShouldFinish = true;
			return true;
		case Common::KEYCODE_ESCAPE:
			_menuSelection = 4;
			_menuConfirmed = true;
			_vm->_smushVideoShouldFinish = true;
			return true;
		default:
			break;
		}
	}

	// Shooting: mouse button during interactive gameplay — FUN_1CCA0 (0x1CCA0)
	if (_interactiveVideoActive && !_menuActive) {
		if (event.type == Common::EVENT_LBUTTONDOWN) {
			_playerFired = true;
			return true;
		}
		if (event.type == Common::EVENT_LBUTTONUP) {
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

	if (event.type == Common::EVENT_MAINMENU && _interactiveVideoActive && !_menuActive) {
		const uint32 now = _vm->_system->getMillis();
		const uint32 elapsedSinceAxis = _lastJoystickAxisEventTime ? now - _lastJoystickAxisEventTime : 0xffffffffu;
		debug(1, "RA1 input mainmenu-event: gameplay=1 elapsedSinceAxis=%u storedAxis=(%d,%d)",
			elapsedSinceAxis, _joystickAxisX, _joystickAxisY);
		if (elapsedSinceAxis <= kRA1JoystickAxisEscGuardMs) {
			debug(1, "RA1 input ignored mainmenu event after recent joystick axis movement (%u ms)", elapsedSinceAxis);
			return true;
		}
	}

	if (event.type == Common::EVENT_KEYDOWN && _player) {
		if (_interactiveVideoActive && !_menuActive &&
			event.kbd.keycode == Common::KEYCODE_ESCAPE) {
			const uint32 now = _vm->_system->getMillis();
			const uint32 elapsedSinceAxis = _lastJoystickAxisEventTime ? now - _lastJoystickAxisEventTime : 0xffffffffu;
			debug(1, "RA1 input keydown-escape: gameplay=1 ascii=%d flags=0x%x repeat=%d elapsedSinceAxis=%u storedAxis=(%d,%d)",
				event.kbd.ascii, event.kbd.flags, event.kbdRepeat,
				elapsedSinceAxis, _joystickAxisX, _joystickAxisY);
			if (elapsedSinceAxis <= kRA1JoystickAxisEscGuardMs) {
				debug(1, "RA1 input ignored ESC after recent joystick axis movement (%u ms)", elapsedSinceAxis);
				return true;
			}

			debug("Rebel1: ESC pressed during gameplay - opening ScummVM menu");
			const bool wasPaused = _player->_paused;
			if (!wasPaused)
				_player->pause();
			_vm->openMainMenuDialog();
			if (!wasPaused)
				_player->unpause();
			return true;
		}

		if (_interactiveVideoActive && !_menuActive &&
			event.kbd.keycode == Common::KEYCODE_s &&
			event.kbd.hasFlags(Common::KBD_SHIFT)) {
			debug("Rebel1: Shift+S pressed - skipping gameplay section");
			_vm->_smushVideoShouldFinish = true;
			return true;
		}

		if (!_interactiveVideoActive && event.kbd.keycode == Common::KEYCODE_ESCAPE) {
			debug("Rebel1: ESC pressed - skipping cinematic");
			_vm->_smushVideoShouldFinish = true;
			return true;
		}
	}

	return false;
}

void InsaneRebel1::renderMainMenuOverlay(byte *dst, int pitch, int width, int height) {
	_menuFrameCounter++;
	auto makeTalkText = [](const char *text) {
		Common::String out("<");
		out += text;
		return out;
	};
	auto getTalkTextWidth = [&](const char *text) {
		Common::String styled = makeTalkText(text);
		return getFontBankStringWidth(styled.c_str());
	};
	auto drawTalkText = [&](int x, int y, const char *text) {
		Common::String styled = makeTalkText(text);
		drawFontBankString(dst, pitch, width, height, x, y, styled.c_str());
	};
	auto getTitleTextWidth = [&](const char *text) {
		return getFontBankStringWidth(text);
	};
	auto drawTitleText = [&](int x, int y, const char *text) {
		drawFontBankString(dst, pitch, width, height, x, y, text);
	};

	if (_highScoresActive) {
		// --- TOP PILOTS high score display ---
		// Original renders over O1SCORE.ANM. Title appears after frame 20,
		// entries fade in one per frame. We show all immediately.
		const int titleW = getTalkTextWidth("TOP PILOTS");
		drawTalkText(getRebel1MenuCenteredX(titleW), 10, "TOP PILOTS");

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
		return;
	}

	if (_optionsActive) {
		// --- Options submenu (matching original RunGameOptionsMenu) ---
		const char *kDiffNames[3] = { "EASY", "NORMAL", "HARD" };

		const int titleW = getTitleTextWidth("GAME OPTIONS");
		drawTitleText(getRebel1MenuCenteredX(titleW), 15, "GAME OPTIONS");

		// Build dynamic option strings for each row
		char diffLine[64], volLine[64];
		Common::sprintf_s(diffLine, "DIFFICULTY IS %s", kDiffNames[CLIP(_difficulty, 0, 2)]);
		Common::sprintf_s(volLine, "VOLUME AT %d PERCENT", (_optVolume * 100) / 127);

		const char *optItems[kOptionsItemCount] = {
			"EXIT MENU",
			_optRookieOneFemale ? "ROOKIE1 IS FEMALE" : "ROOKIE1 IS MALE",
			_optMusicEnabled  ? "MUSIC IS ON"             : "MUSIC IS OFF",
			_optSfxEnabled    ? "SFX AND VOICE ARE ON"    : "SFX AND VOICE ARE OFF",
			_optTextEnabled   ? "DIALOGUE TEXT IS ON"      : "DIALOGUE TEXT IS OFF",
			_optControlsYFlip ? "CONTROLS ARE Y-FLIPPED"  : "CONTROLS ARE NORMAL",
			volLine,
			diffLine
		};

		for (int i = 0; i < kOptionsItemCount; i++) {
			const int textW = getTalkTextWidth(optItems[i]);
			const int textX = getRebel1MenuCenteredX(textW);
			const int y = 0x2d + i * kRA1MenuRowH;
			drawTalkText(textX, y, optItems[i]);

			if (i == _optionsSel)
				drawRebel1MenuFrame(dst, pitch, width, height,
					kRA1MenuFrameX, (i + 1) * kRA1MenuRowH + 0x1d, kRA1MenuFrameW);
		}
		return;
	}

	if (_levelSelectActive) {
		// --- ScummVM level select submenu, styled like the original frontend menus ---
		const int titleW = getTitleTextWidth("LEVEL SELECT");
		drawTitleText(getRebel1MenuCenteredX(titleW), 15, "LEVEL SELECT");

		const char *kLevelItems[kRA1LevelSelectItemCount] = {
			" 1 TRAINING",
			" 2 ASTEROIDS",
			" 3 KOLAADOR",
			" 4 STAR DESTR",
			" 5 TATOOINE",
			" 6 AST CHASE",
			" 7 PROBES",
			" 8 WALKERS",
			" 9 TROOPERS",
			"10 TRANSPORT",
			"11 YAVIN",
			"12 TIE ATK",
			"13 DS SURFACE",
			"14 CANNON",
			"15 DS TRENCH",
			"BACK"
		};

		const int menuY = 0x2d;
		const int leftFrameX = 20;
		const int rightFrameX = 170;
		const int columnW = 130;

		for (int i = 0; i < kRA1LevelSelectItemCount; i++) {
			const int col = i / kRA1LevelSelectRowsPerCol;
			const int row = i % kRA1LevelSelectRowsPerCol;
			const int frameX = (col == 0) ? leftFrameX : rightFrameX;
			const int y = menuY + row * kRA1MenuRowH;
			const int textW = getTalkTextWidth(kLevelItems[i]);
			const int textX = frameX + (columnW - textW) / 2;

			drawTalkText(textX, y, kLevelItems[i]);

			if (i == _levelSelectSel)
				drawRebel1MenuFrame(dst, pitch, width, height,
					frameX, row * kRA1MenuRowH + 0x2c, columnW);
		}
		return;
	}

	// --- Main menu ---
	const char *kMenuItems[5] = {
		"START NEW GAME",
		"GAME OPTIONS",
		"LEVEL SELECT",
		"CONTINUE DEMO",
		"EXIT TO DOS"
	};

	// Center title
	const int titleW = getTitleTextWidth("MAIN MENU");
	const int titleX = getRebel1MenuCenteredX(titleW);
	drawTitleText(titleX, 30, "MAIN MENU");

	// Draw menu items centered horizontally
	for (int i = 0; i < 5; i++) {
		const int textW = getTalkTextWidth(kMenuItems[i]);
		const int textX = getRebel1MenuCenteredX(textW);
		const int y = 0x3c + i * kRA1MenuRowH;

		drawTalkText(textX, y, kMenuItems[i]);

		if (i == _menuSelection)
			drawRebel1MenuFrame(dst, pitch, width, height,
				kRA1MenuFrameX, (i + 1) * kRA1MenuRowH + 0x2c, kRA1MenuFrameW);
	}
}

int InsaneRebel1::runMainMenu() {
	debug(1, "InsaneRebel1: Main menu");

	_menuSelection = 0;
	while (!_vm->shouldQuit()) {
		_menuActive = true;
		_menuConfirmed = false;
		_menuFrameCounter = 0;
		clearVideoBuffer();
		playCinematic("OPEN/O1OPTION.ANM");
		_menuActive = false;

		if (_vm->shouldQuit())
			return 5;

		if (_menuConfirmed)
			return _menuSelection + 1;
	}

	return 5;
}

void InsaneRebel1::runOptionsMenu() {
	_optionsSel = 0;
	_optionsActive = true;

	while (!_vm->shouldQuit()) {
		_menuActive = true;
		_menuConfirmed = false;
		_menuFrameCounter = 0;
		clearVideoBuffer();
		playCinematic("OPEN/O1OPTION.ANM");
		_menuActive = false;

		if (_vm->shouldQuit())
			break;

		if (_menuConfirmed) {
			switch (_optionsSel) {
			case 0: // EXIT MENU
				_optionsActive = false;
				return;
			case 1: // Toggle Rookie One gender
				_optRookieOneFemale = !_optRookieOneFemale;
				break;
			case 2: // Toggle music
				_optMusicEnabled = !_optMusicEnabled;
				_vm->_mixer->muteSoundType(Audio::Mixer::kMusicSoundType, !_optMusicEnabled);
				break;
			case 3: // Toggle SFX + Voice
				_optSfxEnabled = !_optSfxEnabled;
				_vm->_mixer->muteSoundType(Audio::Mixer::kSFXSoundType, !_optSfxEnabled);
				_vm->_mixer->muteSoundType(Audio::Mixer::kSpeechSoundType, !_optSfxEnabled);
				break;
			case 4: // Toggle dialogue text
				_optTextEnabled = !_optTextEnabled;
				ConfMan.setBool("subtitles", _optTextEnabled);
				break;
			case 5: // Toggle Y-flip controls
				_optControlsYFlip = !_optControlsYFlip;
				break;
			case 6: // Volume — adjusted via left/right in notifyEvent
				break;
			case 7: // Cycle difficulty
				_difficulty = (_difficulty + 1) % 3;
				loadTuningForLevel(0);
				break;
			}
		}
	}
	_optionsActive = false;
}

int InsaneRebel1::runLevelSelectMenu() {
	_levelSelectSel = CLIP(_startLevel - 1, 0, kRA1NumLevels - 1);
	_levelSelectActive = true;

	while (!_vm->shouldQuit()) {
		_menuActive = true;
		_menuConfirmed = false;
		_menuFrameCounter = 0;
		clearVideoBuffer();
		playCinematic("OPEN/O1OPTION.ANM");
		_menuActive = false;

		if (_vm->shouldQuit())
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
