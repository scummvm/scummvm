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

#include "sherlock/sherlock.h"
#include "sherlock/scalpel/settings.h"
#include "sherlock/scalpel/scalpel_screen.h"
#include "sherlock/scalpel/scalpel_user_interface.h"
#include "sherlock/scalpel/scalpel_fixed_text.h"
#include "sherlock/scalpel/scalpel.h"

namespace Sherlock {

namespace Scalpel {

static const int SETUP_POINTS_INTL[12][4]  = {
	{ 4, 154, 101, 53 },		// Exit
	{ 4, 165, 101, 53 },		// Music Toggle
	{ 219, 165, 316, 268 },		// Voice Toggle
	{ 103, 165, 217, 160 },		// Sound Effects Toggle
	{ 219, 154, 316, 268 },		// Help Button Left/Right
	{ 103, 154, 217, 160 },		// New Font Style
	{ 4, 187, 101, 53 },		// Joystick Toggle
	{ 103, 187, 217, 160 },		// Calibrate Joystick
	{ 219, 176, 316, 268 },		// Fade Style
	{ 103, 176, 217, 160 },		// Window Open Style
	{ 4, 176, 101, 53 }, 		// Portraits Toggle
	{ 219, 187, 316, 268 }		// _key Pad Accel. Toggle
};

// Different from original to accommodate hotkeys
static const int SETUP_POINTS_ZH[12][4]  = {
	{ 3, 159, 73, 38 },		// Exit // OK
	{ 3, 178, 73, 38 },		// Music Toggle // OK
	{ 0, 0, 0, 0 },		// Voice Toggle
	{ 74, 178, 145, 109 },		// Sound Effects Toggle // OK
	{ 146, 178, 233, 191 },		// Help Button Left/Right // OK
	{ 0, 0, 0, 0 },		// New Font Style // OK
	{ 0, 0, 0, 0 },		// Joystick Toggle // OK
	{ 0, 0, 0, 0 },		// Calibrate Joystick // OK
	{ 234, 159, 317, 276 },		// Fade Style // OK
	{ 146, 159, 233, 191 },		// Window Open Style // OK
	{ 74, 159, 145, 109 }, 		// Portraits Toggle // OK
	{ 234, 178, 317, 276 }		// _key Pad Accel. Toggle
};

/*----------------------------------------------------------------*/

bool Settings::doesButtonExist(int num) const {
	if (_vm->getLanguage() == Common::Language::ZH_TWN)
		return num != 2 && num != 5 && num != 6 && num != 7;
	return true;
}

void Settings::makeButtonNumDisabled(int num, const Common::String &s) {
	if (!doesButtonExist(num))
		return;

	ScalpelScreen &screen = *(ScalpelScreen *)_vm->_screen;
	makeButtonNum(num, s);
	screen.buttonPrint(getButtonTextPoint(num), COMMAND_NULL, false, s);
}

Common::Rect Settings::getButtonRect(int num) const {
	if (_vm->getLanguage() == Common::Language::ZH_TWN) {
		return Common::Rect(SETUP_POINTS_ZH[num][0], SETUP_POINTS_ZH[num][1],
				    SETUP_POINTS_ZH[num][2], SETUP_POINTS_ZH[num][1] + 19);
	} else {
		return Common::Rect(SETUP_POINTS_INTL[num][0], SETUP_POINTS_INTL[num][1],
				    SETUP_POINTS_INTL[num][2], SETUP_POINTS_INTL[num][1] + 10);
	}
}

void Settings::makeButtonNum(int num, const Common::String &s) {
	if (!doesButtonExist(num))
		return;

	ScalpelScreen &screen = *(ScalpelScreen *)_vm->_screen;
	screen.makeButton(getButtonRect(num), getButtonTextPoint(num), s);
}

Common::Point Settings::getButtonTextPoint(int num) const {
	if (_vm->getLanguage() == Common::Language::ZH_TWN) {
		return Common::Point(SETUP_POINTS_ZH[num][3], SETUP_POINTS_ZH[num][1] + 2);
	} else {
		return Common::Point(SETUP_POINTS_INTL[num][3], SETUP_POINTS_INTL[num][1]);
	}
}

void Settings::drawInterface(bool flag) {
	People &people = *_vm->_people;
	ScalpelScreen &screen = *(ScalpelScreen *)_vm->_screen;
	Sound &sound = *_vm->_sound;
	Music &music = *_vm->_music;
	UserInterface &ui = *_vm->_ui;
	Common::String tempStr;

	if (!flag) {
		screen._backBuffer1.fillRect(Common::Rect(0, CONTROLS_Y1, SHERLOCK_SCREEN_WIDTH, CONTROLS_Y1 + 1), BORDER_COLOR);
		screen._backBuffer1.fillRect(Common::Rect(0, CONTROLS_Y1 + 1, 2, SHERLOCK_SCREEN_HEIGHT), BORDER_COLOR);
		screen._backBuffer1.fillRect(Common::Rect(SHERLOCK_SCREEN_WIDTH - 2, CONTROLS_Y1 + 1, SHERLOCK_SCREEN_WIDTH,
			SHERLOCK_SCREEN_HEIGHT), BORDER_COLOR);
		screen._backBuffer1.hLine(0, SHERLOCK_SCREEN_HEIGHT - 1, SHERLOCK_SCREEN_WIDTH - 1, BORDER_COLOR);
		screen._backBuffer1.fillRect(Common::Rect(2, CONTROLS_Y1 + 1, SHERLOCK_SCREEN_WIDTH - 2,
			SHERLOCK_SCREEN_HEIGHT - 1), INV_BACKGROUND);
	}

	tempStr = FIXED(Settings_Exit);
	_hotkeyExit = toupper(tempStr.firstChar());
	makeButtonNum(0, tempStr);

	if (music._musicOn) {
		tempStr = FIXED(Settings_MusicOn);
	} else {
		tempStr = FIXED(Settings_MusicOff);
	}
	_hotkeyMusic = toupper(tempStr.firstChar());
	makeButtonNum(1, tempStr);

	if (people._portraitsOn) {
		tempStr = FIXED(Settings_PortraitsOn);
	} else {
		tempStr = FIXED(Settings_PortraitsOff);
	}
	_hotkeyPortraits = toupper(tempStr.firstChar());
	makeButtonNum(10, tempStr);

	// WORKAROUND: We don't support the joystick in ScummVM, so draw the next two buttons as disabled
	tempStr = FIXED(Settings_JoystickOff);
	makeButtonNumDisabled(6, tempStr);

	tempStr = FIXED(Settings_NewFontStyle);
	_hotkeyNewFontStyle = toupper(tempStr.firstChar());
	makeButtonNum(5, tempStr);

	if (sound._digitized) {
		tempStr = FIXED(Settings_SoundEffectsOn);
	} else {
		tempStr = FIXED(Settings_SoundEffectsOff);
	}
	_hotkeySoundEffects = toupper(tempStr.firstChar());
	makeButtonNum(3, tempStr);

	if (ui._slideWindows) {
		tempStr = FIXED(Settings_WindowsSlide);
	} else {
		tempStr = FIXED(Settings_WindowsAppear);
	}
	_hotkeyWindows = toupper(tempStr.firstChar());
	makeButtonNum(9, tempStr);

	tempStr = FIXED(Settings_CalibrateJoystick);
	makeButtonNumDisabled(7, tempStr);

	if (ui._helpStyle) {
		tempStr = FIXED(Settings_AutoHelpRight);
	} else {
		tempStr = FIXED(Settings_AutoHelpLeft);
	}
	_hotkeyAutoHelp = toupper(tempStr.firstChar());
	makeButtonNum(4, tempStr);

	if (sound._voices) {
		tempStr = FIXED(Settings_VoicesOn);
	} else {
		tempStr = FIXED(Settings_VoicesOff);
	}
	_hotkeyVoices = toupper(tempStr.firstChar());
	makeButtonNum(2, tempStr);

	if (screen._fadeStyle) {
		tempStr = FIXED(Settings_FadeByPixel);
	} else {
		tempStr = FIXED(Settings_FadeDirectly);
	}
	_hotkeyFade = toupper(tempStr.firstChar());
	makeButtonNum(8, tempStr);

	tempStr = FIXED(Settings_KeyPadSlow);
	makeButtonNumDisabled(11, tempStr);

	_hotkeysIndexed[0] = _hotkeyExit;
	_hotkeysIndexed[1] = _hotkeyMusic;
	_hotkeysIndexed[2] = _hotkeyVoices;
	_hotkeysIndexed[3] = _hotkeySoundEffects;
	_hotkeysIndexed[4] = _hotkeyAutoHelp;
	_hotkeysIndexed[5] = _hotkeyNewFontStyle;
	_hotkeysIndexed[8] = _hotkeyFade;
	_hotkeysIndexed[9] = _hotkeyWindows;
	_hotkeysIndexed[10] = _hotkeyPortraits;

	// Show the window immediately, or slide it on-screen
	if (!flag) {
		if (!ui._slideWindows) {
			screen.slamRect(Common::Rect(0, CONTROLS_Y1, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT));
		} else {
			ui.summonWindow(true, CONTROLS_Y1);
		}

		ui._windowOpen = true;
	} else {
		screen.slamRect(Common::Rect(0, CONTROLS_Y1, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT));
	}
}

int Settings::drawButtons(const Common::Point &pt, int _key) {
	Events &events = *_vm->_events;
	People &people = *_vm->_people;
	ScalpelScreen &screen = *(ScalpelScreen *)_vm->_screen;
	Music &music = *_vm->_music;
	Sound &sound = *_vm->_sound;
	UserInterface &ui = *_vm->_ui;
	int found = -1;
	byte color;
	Common::String tempStr;

	for (int idx = 0; idx < 12; ++idx) {
		if (!doesButtonExist(idx))
			continue;
		if ((getButtonRect(idx).contains(pt) && (events._pressed || events._released))
				|| (_key == toupper(_hotkeysIndexed[idx]))) {
			found = idx;
			color = COMMAND_HIGHLIGHTED;
		} else {
			color = COMMAND_FOREGROUND;
		}

		// Print the button text
		switch (idx) {
		case 0:
			tempStr = FIXED(Settings_Exit);
			break;
		case 1:
			if (music._musicOn) {
				tempStr = FIXED(Settings_MusicOn);
			} else {
				tempStr = FIXED(Settings_MusicOff);
			}
			break;
		case 2:
			if (sound._voices) {
				tempStr = FIXED(Settings_VoicesOn);
			} else {
				tempStr = FIXED(Settings_VoicesOff);
			}
			break;
		case 3:
			if (sound._digitized) {
				tempStr = FIXED(Settings_SoundEffectsOn);
			} else {
				tempStr = FIXED(Settings_SoundEffectsOff);
			}
			break;
		case 4:
			if (ui._helpStyle) {
				tempStr = FIXED(Settings_AutoHelpRight);
			} else {
				tempStr = FIXED(Settings_AutoHelpLeft);
			}
			break;
		case 5:
			tempStr = FIXED(Settings_NewFontStyle);
			break;
		case 6:
			// Joystick Off - disabled in ScummVM
			continue;
		case 7:
			// Calibrate Joystick - disabled in ScummVM
			continue;
		case 8:
			if (screen._fadeStyle) {
				tempStr = FIXED(Settings_FadeByPixel);
			} else {
				tempStr = FIXED(Settings_FadeDirectly);
			}
			break;
		case 9:
			if (ui._slideWindows) {
				tempStr = FIXED(Settings_WindowsSlide);
			} else {
				tempStr = FIXED(Settings_WindowsAppear);
			}
			break;
		case 10:
			if (people._portraitsOn) {
				tempStr = FIXED(Settings_PortraitsOn);
			} else {
				tempStr = FIXED(Settings_PortraitsOff);
			}
			break;
		case 11:
			// Key Pad Slow - disabled in ScummVM
			continue;
		default:
			continue;
		}
		screen.buttonPrint(getButtonTextPoint(idx), color, true, tempStr);
	}

	return found;
}

void Settings::show(SherlockEngine *vm) {
	Events &events = *vm->_events;
	People &people = *vm->_people;
	Scene &scene = *vm->_scene;
	Screen &screen = *vm->_screen;
	Sound &sound = *vm->_sound;
	Music &music = *vm->_music;
	Talk &talk = *vm->_talk;
	ScalpelUserInterface &ui = *(ScalpelUserInterface *)vm->_ui;
	bool updateConfig = false;

	assert(vm->getGameID() == GType_SerratedScalpel);
	Settings settings(vm);
	settings.drawInterface(false);

	do {
		if (ui._menuCounter)
			ui.whileMenuCounter();

		int found = -1;
		ui._key = -1;

		scene.doBgAnim();
		if (talk._talkToAbort)
			return;

		events.setButtonState();
		Common::Point pt = events.mousePos();

		if (events._pressed || events._released || events.kbHit()) {
			ui.clearInfo();
			ui._key = -1;

			if (events.kbHit()) {
				Common::KeyState keyState = events.getKey();
				ui._key = toupper(keyState.ascii);

				if (ui._key == Common::KEYCODE_RETURN || ui._key == Common::KEYCODE_SPACE) {
					events._pressed = false;
					events._oldButtons = 0;
					ui._keyPress = '\0';
					events._released = true;
				}
			}

			// Handle highlighting button under mouse
			found = settings.drawButtons(pt, ui._key);
		}

		if ((found == 0 && events._released) || (ui._key == settings._hotkeyExit || ui._key == Common::KEYCODE_ESCAPE))
			// Exit
			break;

		if ((found == 1 && events._released) || ui._key == settings._hotkeyMusic) {
			// Toggle music
			music._musicOn = !music._musicOn;
			if (!music._musicOn)
				music.stopMusic();
			else
				music.startSong();

			updateConfig = true;
			settings.drawInterface(true);
		}

		if ((found == 2 && events._released) || ui._key == settings._hotkeyVoices) {
			sound._voices = !sound._voices;
			updateConfig = true;
			settings.drawInterface(true);
		}

		if ((found == 3 && events._released) || ui._key == settings._hotkeySoundEffects) {
			// Toggle sound effects
			sound._digitized = !sound._digitized;
			updateConfig = true;
			settings.drawInterface(true);
		}

		if ((found == 4 && events._released) || ui._key == settings._hotkeyAutoHelp) {
			// Help button style
			ui._helpStyle = !ui._helpStyle;
			updateConfig = true;
			settings.drawInterface(true);
		}

		if ((found == 5 && events._released) || ui._key == settings._hotkeyNewFontStyle) {
			// New font style
			int fontNum = screen.fontNumber() + 1;
			if (fontNum == 3)
				fontNum = 0;

			screen.setFont(fontNum);
			updateConfig = true;
			settings.drawInterface(true);
		}

		if ((found == 8 && events._released) || ui._key == settings._hotkeyFade) {
			// Toggle fade style
			screen._fadeStyle = !screen._fadeStyle;
			updateConfig = true;
			settings.drawInterface(true);
		}

		if ((found == 9 && events._released) || ui._key == settings._hotkeyWindows) {
			// Window style
			ui._slideWindows = !ui._slideWindows;
			updateConfig = true;
			settings.drawInterface(true);
		}

		if ((found == 10 && events._released) || ui._key == settings._hotkeyPortraits) {
			// Toggle portraits being shown
			people._portraitsOn = !people._portraitsOn;
			updateConfig = true;
			settings.drawInterface(true);
		}
	} while (!vm->shouldQuit());

	ui.banishWindow();

	if (updateConfig)
		vm->saveConfig();

	ui._keyPress = '\0';
	ui._keyboardInput = false;
	ui._windowBounds.top = CONTROLS_Y1;
	ui._key = -1;
}

} // End of namespace Scalpel

} // End of namespace Sherlock
