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

static const int kRA1LevelSelectItemCount = 16;  // 15 levels + BACK
static const int kRA1LevelSelectRowsPerCol = 8;
static const int kRA1NumLevels = 15;

bool InsaneRebel1::notifyEvent(const Common::Event &event) {
	if (event.type == Common::EVENT_MOUSEMOVE && !_mouseRecentering) {
		_activeInputSource = kInputSourceMouse;
	}

	if (event.type == Common::EVENT_CUSTOM_BACKEND_ACTION_AXIS) {
		_activeInputSource = kInputSourceJoystickAnalog;

		switch (event.customType) {
		case kScummBackendActionRebel1AxisUp:
			if (event.joystick.position == 0 && _joystickAxisY < 0)
				return true;
			_joystickAxisY = event.joystick.position;
			return true;
		case kScummBackendActionRebel1AxisDown:
			if (event.joystick.position == 0 && _joystickAxisY > 0)
				return true;
			_joystickAxisY = -event.joystick.position;
			return true;
		case kScummBackendActionRebel1AxisLeft:
			if (event.joystick.position == 0 && _joystickAxisX > 0)
				return true;
			_joystickAxisX = -event.joystick.position;
			return true;
		case kScummBackendActionRebel1AxisRight:
			if (event.joystick.position == 0 && _joystickAxisX < 0)
				return true;
			_joystickAxisX = event.joystick.position;
			return true;
		default:
			break;
		}
	}

	if (event.type == Common::EVENT_CUSTOM_ENGINE_ACTION_START ||
		event.type == Common::EVENT_CUSTOM_ENGINE_ACTION_END) {
		const bool pressed = (event.type == Common::EVENT_CUSTOM_ENGINE_ACTION_START);

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

		if (_menuActive && !_highScoresActive && pressed) {
			if (_levelSelectActive) {
				int col = _levelSelectSel / kRA1LevelSelectRowsPerCol;
				int row = _levelSelectSel % kRA1LevelSelectRowsPerCol;

				switch (event.customType) {
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
					break;
				}
			}

			if (_optionsActive) {
				switch (event.customType) {
				case kScummActionInsaneUp:
					_optionsSel = (_optionsSel + kOptionsItemCount - 1) % kOptionsItemCount;
					return true;
				case kScummActionInsaneDown:
					_optionsSel = (_optionsSel + 1) % kOptionsItemCount;
					return true;
				case kScummActionInsaneLeft:
					if (_optionsSel == 6) {
						_optVolume = MAX(0, _optVolume - 5);
						_vm->_mixer->setVolumeForSoundType(Audio::Mixer::kPlainSoundType,
							(_optVolume * Audio::Mixer::kMaxChannelVolume) / 127);
						ConfMan.setInt("music_volume", (_optVolume * 256) / 127);
						ConfMan.setInt("sfx_volume", (_optVolume * 256) / 127);
						ConfMan.setInt("speech_volume", (_optVolume * 256) / 127);
					}
					return true;
				case kScummActionInsaneRight:
					if (_optionsSel == 6) {
						_optVolume = MIN(127, _optVolume + 5);
						_vm->_mixer->setVolumeForSoundType(Audio::Mixer::kPlainSoundType,
							(_optVolume * Audio::Mixer::kMaxChannelVolume) / 127);
						ConfMan.setInt("music_volume", (_optVolume * 256) / 127);
						ConfMan.setInt("sfx_volume", (_optVolume * 256) / 127);
						ConfMan.setInt("speech_volume", (_optVolume * 256) / 127);
					}
					return true;
				case kScummActionInsaneAttack:
					_menuConfirmed = true;
					_vm->_smushVideoShouldFinish = true;
					return true;
				default:
					break;
				}
			}

			if (!_optionsActive && !_levelSelectActive) {
				switch (event.customType) {
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
					break;
				}
			}
		}

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
			if (_optionsSel == 6) {
				_optVolume = MAX(0, _optVolume - 5);
				_vm->_mixer->setVolumeForSoundType(Audio::Mixer::kPlainSoundType,
					(_optVolume * Audio::Mixer::kMaxChannelVolume) / 127);
				ConfMan.setInt("music_volume", (_optVolume * 256) / 127);
				ConfMan.setInt("sfx_volume", (_optVolume * 256) / 127);
				ConfMan.setInt("speech_volume", (_optVolume * 256) / 127);
			}
			return true;
		case Common::KEYCODE_RIGHT:
		case Common::KEYCODE_d:
			// Volume up when on volume row (row 6)
			if (_optionsSel == 6) {
				_optVolume = MIN(127, _optVolume + 5);
				_vm->_mixer->setVolumeForSoundType(Audio::Mixer::kPlainSoundType,
					(_optVolume * Audio::Mixer::kMaxChannelVolume) / 127);
				ConfMan.setInt("music_volume", (_optVolume * 256) / 127);
				ConfMan.setInt("sfx_volume", (_optVolume * 256) / 127);
				ConfMan.setInt("speech_volume", (_optVolume * 256) / 127);
			}
			return true;
		case Common::KEYCODE_RETURN:
		case Common::KEYCODE_KP_ENTER:
		case Common::KEYCODE_SPACE:
			_menuConfirmed = true;
			_vm->_smushVideoShouldFinish = true;
			return true;
		case Common::KEYCODE_ESCAPE:
			_optionsSel = kOptionsItemCount - 1;  // Back
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

	if (event.type == Common::EVENT_KEYDOWN && _player) {
		if (_interactiveVideoActive && !_menuActive &&
			event.kbd.keycode == Common::KEYCODE_ESCAPE) {
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
		drawTalkText((width - titleW) / 2, 10, "TOP PILOTS");

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
		static const char *kDiffNames[3] = { "EASY", "NORMAL", "HARD" };

		const int titleW = getTalkTextWidth("GAME OPTIONS");
		drawTalkText((width - titleW) / 2, 30, "GAME OPTIONS");

		// Build dynamic option strings for each row
		char diffLine[64], volLine[64];
		Common::sprintf_s(diffLine, "DIFFICULTY IS %s", kDiffNames[CLIP(_difficulty, 0, 2)]);
		Common::sprintf_s(volLine, "VOLUME AT %d PERCENT", (_optVolume * 100) / 127);

		const char *optItems[kOptionsItemCount] = {
			"EXIT MENU",
			_optMusicEnabled  ? "MUSIC IS ON"             : "MUSIC IS OFF",
			_optSfxEnabled    ? "SFX AND VOICE ARE ON"    : "SFX AND VOICE ARE OFF",
			_optTextEnabled   ? "DIALOGUE TEXT IS ON"      : "DIALOGUE TEXT IS OFF",
			_optControlsYFlip ? "CONTROLS ARE Y-FLIPPED"  : "CONTROLS ARE NORMAL",
			_turbulenceEnabled ? "TURBULENCE IS ON"        : "TURBULENCE IS OFF",
			volLine,
			diffLine,
			"BACK"
		};

		const int menuY = 44;
		const int rowH = 14;

		for (int i = 0; i < kOptionsItemCount; i++) {
			const int textW = getTalkTextWidth(optItems[i]);
			const int textX = (width - textW) / 2;
			const int y = menuY + i * rowH;
			drawTalkText(textX, y + 1, optItems[i]);

			if (i == _optionsSel) {
				byte highlightColor = ((_menuFrameCounter / 8) & 1) ? 248 : 240;
				int bracketWidth = textW + 12;
				int leftX = CLIP(textX - 6, 0, width - 1);
				int rightX = CLIP(leftX + bracketWidth, 0, width - 1);
				int topY = CLIP(y - 1, 0, height - 1);
				int bottomY = CLIP(y + rowH - 2, 0, height - 1);
				for (int x = leftX; x <= rightX; x++) {
					dst[topY * pitch + x] = highlightColor;
					dst[bottomY * pitch + x] = highlightColor;
				}
				for (int py = topY; py <= bottomY; py++) {
					dst[py * pitch + leftX] = highlightColor;
					dst[py * pitch + rightX] = highlightColor;
				}
			}
		}
		return;
	}

	if (_levelSelectActive) {
		// --- Level select submenu (two-column layout) ---
		const int titleW = getTalkTextWidth("LEVEL SELECT");
		drawTalkText((width - titleW) / 2, 30, "LEVEL SELECT");

		static const char *kLevelItems[kRA1LevelSelectItemCount] = {
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

		const int menuY = 50;
		const int rowH = 14;
		const int leftColX = 8;
		const int rightColX = width / 2 + 4;

		for (int i = 0; i < kRA1LevelSelectItemCount; i++) {
			const int col = i / kRA1LevelSelectRowsPerCol;
			const int row = i % kRA1LevelSelectRowsPerCol;
			const int textX = (col == 0) ? leftColX : rightColX;
			const int y = menuY + row * rowH;
			const int textW = getTalkTextWidth(kLevelItems[i]);

			drawTalkText(textX, y + 1, kLevelItems[i]);

			if (i == _levelSelectSel) {
				byte highlightColor = ((_menuFrameCounter / 8) & 1) ? 248 : 240;
				int bracketWidth = textW + 12;
				int leftX = CLIP(textX - 6, 0, width - 1);
				int rightX = CLIP(leftX + bracketWidth, 0, width - 1);
				int topY = CLIP(y - 1, 0, height - 1);
				int bottomY = CLIP(y + rowH - 2, 0, height - 1);
				for (int x = leftX; x <= rightX; x++) {
					dst[topY * pitch + x] = highlightColor;
					dst[bottomY * pitch + x] = highlightColor;
				}
				for (int py = topY; py <= bottomY; py++) {
					dst[py * pitch + leftX] = highlightColor;
					dst[py * pitch + rightX] = highlightColor;
				}
			}
		}
		return;
	}

	// --- Main menu ---
	static const char *kMenuItems[5] = {
		"START NEW GAME",
		"GAME OPTIONS",
		"LEVEL SELECT",
		"CONTINUE DEMO",
		"EXIT TO DOS"
	};

	// Center title
	const int titleW = getTitleTextWidth("MAIN MENU");
	const int titleX = (width - titleW) / 2;
	drawTitleText(titleX, 36, "MAIN MENU");

	// Draw menu items centered horizontally
	const int menuY = 60;
	const int rowH = 16;

	for (int i = 0; i < 5; i++) {
		const int textW = getTalkTextWidth(kMenuItems[i]);
		const int textX = (width - textW) / 2;
		const int y = menuY + i * rowH;

		drawTalkText(textX, y + 1, kMenuItems[i]);

		// Selection highlight box — flashing border (FUN_004292d0 pattern from RA2)
		if (i == _menuSelection) {
			// Flash between two palette colors every 8 frames
			byte highlightColor = ((_menuFrameCounter / 8) & 1) ? 248 : 240;

			int bracketWidth = textW + 12;
			int bracketHeight = rowH;
			int leftX = textX - 6;
			int rightX = leftX + bracketWidth;
			int topY = y - 1;
			int bottomY = y + bracketHeight - 2;

			// Clamp
			if (leftX < 0) leftX = 0;
			if (rightX >= width) rightX = width - 1;
			if (topY < 0) topY = 0;
			if (bottomY >= height) bottomY = height - 1;

			// Draw rectangle border (4 lines)
			for (int x = leftX; x <= rightX && x < width; x++) {
				if (topY >= 0 && topY < height)
					dst[topY * pitch + x] = highlightColor;
				if (bottomY >= 0 && bottomY < height)
					dst[bottomY * pitch + x] = highlightColor;
			}
			for (int py = topY; py <= bottomY && py < height; py++) {
				if (leftX >= 0 && leftX < width)
					dst[py * pitch + leftX] = highlightColor;
				if (rightX >= 0 && rightX < width)
					dst[py * pitch + rightX] = highlightColor;
			}
		}
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
			case 0: // EXIT MENU (same as BACK)
				_optionsActive = false;
				return;
			case 1: // Toggle music
				_optMusicEnabled = !_optMusicEnabled;
				_vm->_mixer->muteSoundType(Audio::Mixer::kMusicSoundType, !_optMusicEnabled);
				break;
			case 2: // Toggle SFX + Voice
				_optSfxEnabled = !_optSfxEnabled;
				_vm->_mixer->muteSoundType(Audio::Mixer::kSFXSoundType, !_optSfxEnabled);
				_vm->_mixer->muteSoundType(Audio::Mixer::kSpeechSoundType, !_optSfxEnabled);
				break;
			case 3: // Toggle dialogue text
				_optTextEnabled = !_optTextEnabled;
				ConfMan.setBool("subtitles", _optTextEnabled);
				break;
			case 4: // Toggle Y-flip controls
				_optControlsYFlip = !_optControlsYFlip;
				break;
			case 5: // Toggle turbulence
				_turbulenceEnabled = !_turbulenceEnabled;
				break;
			case 6: // Volume — adjusted via left/right in notifyEvent
				break;
			case 7: // Cycle difficulty
				_difficulty = (_difficulty + 1) % 3;
				loadTuningForLevel(0);
				break;
			case 8: // BACK
				_optionsActive = false;
				return;
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
