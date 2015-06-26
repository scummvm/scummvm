/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "sherlock/sherlock.h"
#include "sherlock/scalpel/settings.h"
#include "sherlock/scalpel/scalpel_screen.h"
#include "sherlock/scalpel/scalpel_user_interface.h"
#include "sherlock/scalpel/scalpel.h"

namespace Sherlock {

namespace Scalpel {

static const int SETUP_POINTS[12][4]  = {
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

static const char *const SETUP_STRS0[2] = { "off", "on" };
static const char *const SETUP_STRS1[2] = { "Directly", "by Pixel" };
static const char *const SETUP_STRS2[2] = { "Left", "Right" };
static const char *const SETUP_STRS3[2] = { "Appear", "Slide" };
static const char *const SETUP_STRS5[2] = { "Left", "Right" };
static const char *const SETUP_NAMES[12] = {
	"Exit", "M", "V", "S", "B", "New Font Style", "J", "Calibrate Joystick", "F", "W", "P", "K"
};

/*----------------------------------------------------------------*/

void Settings::drawInteface(bool flag) {
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
			SHERLOCK_SCREEN_HEIGHT - 2), INV_BACKGROUND);
	}

	screen.makeButton(Common::Rect(SETUP_POINTS[0][0], SETUP_POINTS[0][1], SETUP_POINTS[0][2], SETUP_POINTS[0][1] + 10),
		SETUP_POINTS[0][3] - screen.stringWidth("Exit") / 2, "Exit");

	tempStr = Common::String::format("Music %s", SETUP_STRS0[music._musicOn]);
	screen.makeButton(Common::Rect(SETUP_POINTS[1][0], SETUP_POINTS[1][1], SETUP_POINTS[1][2], SETUP_POINTS[1][1] + 10),
		SETUP_POINTS[1][3] - screen.stringWidth(tempStr) / 2, tempStr);

	tempStr = Common::String::format("Voices %s", SETUP_STRS0[sound._voices]);
	screen.makeButton(Common::Rect(SETUP_POINTS[2][0], SETUP_POINTS[2][1], SETUP_POINTS[2][2], SETUP_POINTS[2][1] + 10),
		SETUP_POINTS[2][3] - screen.stringWidth(tempStr) / 2, tempStr);

	tempStr = Common::String::format("Sound Effects %s", SETUP_STRS0[sound._digitized]);
	screen.makeButton(Common::Rect(SETUP_POINTS[3][0], SETUP_POINTS[3][1], SETUP_POINTS[3][2], SETUP_POINTS[3][1] + 10),
		SETUP_POINTS[3][3] - screen.stringWidth(tempStr) / 2, tempStr);

	tempStr = Common::String::format("Auto Help %s", SETUP_STRS5[ui._helpStyle]);
	screen.makeButton(Common::Rect(SETUP_POINTS[4][0], SETUP_POINTS[4][1], SETUP_POINTS[4][2], SETUP_POINTS[4][1] + 10),
		SETUP_POINTS[4][3] - screen.stringWidth(tempStr) / 2, tempStr);
	screen.makeButton(Common::Rect(SETUP_POINTS[5][0], SETUP_POINTS[5][1], SETUP_POINTS[5][2], SETUP_POINTS[5][1] + 10),
		SETUP_POINTS[5][3] - screen.stringWidth("New Font Style") / 2, "New Font Style");

	// WORKAROUND: We don't support the joystick in ScummVM, so draw the next two buttons as disabled
	tempStr = "Joystick Off";
	screen.makeButton(Common::Rect(SETUP_POINTS[6][0], SETUP_POINTS[6][1], SETUP_POINTS[6][2], SETUP_POINTS[6][1] + 10),
		SETUP_POINTS[6][3] - screen.stringWidth(tempStr) / 2, tempStr);
	screen.buttonPrint(Common::Point(SETUP_POINTS[6][3], SETUP_POINTS[6][1]), COMMAND_NULL, false, tempStr);

	tempStr = "Calibrate Joystick";
	screen.makeButton(Common::Rect(SETUP_POINTS[7][0], SETUP_POINTS[7][1], SETUP_POINTS[7][2], SETUP_POINTS[7][1] + 10),
		SETUP_POINTS[7][3] - screen.stringWidth(tempStr) / 2, tempStr);
	screen.buttonPrint(Common::Point(SETUP_POINTS[7][3], SETUP_POINTS[7][1]), COMMAND_NULL, false, tempStr);

	tempStr = Common::String::format("Fade %s", screen._fadeStyle ? "by Pixel" : "Directly");
	screen.makeButton(Common::Rect(SETUP_POINTS[8][0], SETUP_POINTS[8][1], SETUP_POINTS[8][2], SETUP_POINTS[8][1] + 10),
		SETUP_POINTS[8][3] - screen.stringWidth(tempStr) / 2, tempStr);

	tempStr = Common::String::format("Windows %s", ui._slideWindows ? "Slide" : "Appear");
	screen.makeButton(Common::Rect(SETUP_POINTS[9][0], SETUP_POINTS[9][1], SETUP_POINTS[9][2], SETUP_POINTS[9][1] + 10),
		SETUP_POINTS[9][3] - screen.stringWidth(tempStr) / 2, tempStr);

	tempStr = Common::String::format("Portraits %s", SETUP_STRS0[people._portraitsOn]);
	screen.makeButton(Common::Rect(SETUP_POINTS[10][0], SETUP_POINTS[10][1], SETUP_POINTS[10][2], SETUP_POINTS[10][1] + 10),
		SETUP_POINTS[10][3] - screen.stringWidth(tempStr) / 2, tempStr);

	tempStr = "Key Pad Slow";
	screen.makeButton(Common::Rect(SETUP_POINTS[11][0], SETUP_POINTS[11][1], SETUP_POINTS[11][2], SETUP_POINTS[11][1] + 10),
		SETUP_POINTS[11][3] - screen.stringWidth(tempStr) / 2, tempStr);
	screen.buttonPrint(Common::Point(SETUP_POINTS[11][3], SETUP_POINTS[11][1]), COMMAND_NULL, false, tempStr);

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
		if ((pt.x > SETUP_POINTS[idx][0] && pt.x < SETUP_POINTS[idx][2] && pt.y > SETUP_POINTS[idx][1]
				&& pt.y < (SETUP_POINTS[idx][1] + 10) && (events._pressed || events._released))
				|| (_key == SETUP_NAMES[idx][0])) {
			found = idx;
			color = COMMAND_HIGHLIGHTED;
		} else {
			color = COMMAND_FOREGROUND;
		}

		// Print the button text
		switch (idx) {
		case 1:
			tempStr = Common::String::format("Music %s", SETUP_STRS0[music._musicOn]);
			screen.buttonPrint(Common::Point(SETUP_POINTS[idx][3], SETUP_POINTS[idx][1]), color, true, tempStr);
			break;
		case 2:
			tempStr = Common::String::format("Voices %s", SETUP_STRS0[sound._voices]);
			screen.buttonPrint(Common::Point(SETUP_POINTS[idx][3], SETUP_POINTS[idx][1]), color, true, tempStr);
			break;
		case 3:
			tempStr = Common::String::format("Sound Effects %s", SETUP_STRS0[sound._digitized]);
			screen.buttonPrint(Common::Point(SETUP_POINTS[idx][3], SETUP_POINTS[idx][1]), color, true, tempStr);
			break;
		case 4:
			tempStr = Common::String::format("Auto Help %s", SETUP_STRS2[ui._helpStyle]);
			screen.buttonPrint(Common::Point(SETUP_POINTS[idx][3], SETUP_POINTS[idx][1]), color, true, tempStr);
			break;
		case 6:
			tempStr = "Joystick Off";
			screen.buttonPrint(Common::Point(SETUP_POINTS[idx][3], SETUP_POINTS[idx][1]), COMMAND_NULL, true, tempStr);
			break;
		case 7:
			tempStr = "Calibrate Joystick";
			screen.buttonPrint(Common::Point(SETUP_POINTS[idx][3], SETUP_POINTS[idx][1]), COMMAND_NULL, true, tempStr);
			break;
		case 8:
			tempStr = Common::String::format("Fade %s", SETUP_STRS1[screen._fadeStyle]);
			screen.buttonPrint(Common::Point(SETUP_POINTS[idx][3], SETUP_POINTS[idx][1]), color, true, tempStr);
			break;
		case 9:
			tempStr = Common::String::format("Windows %s", SETUP_STRS3[ui._slideWindows]);
			screen.buttonPrint(Common::Point(SETUP_POINTS[idx][3], SETUP_POINTS[idx][1]), color, true, tempStr);
			break;
		case 10:
			tempStr = Common::String::format("Portraits %s", SETUP_STRS0[people._portraitsOn]);
			screen.buttonPrint(Common::Point(SETUP_POINTS[idx][3], SETUP_POINTS[idx][1]), color, true, tempStr);
			break;
		case 11:
			tempStr = "Key Pad Slow";
			screen.buttonPrint(Common::Point(SETUP_POINTS[idx][3], SETUP_POINTS[idx][1]), COMMAND_NULL, true, tempStr);
			break;
		default:
			screen.buttonPrint(Common::Point(SETUP_POINTS[idx][3], SETUP_POINTS[idx][1]), color, true, SETUP_NAMES[idx]);
			break;
		}
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
	settings.drawInteface(false);

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
				ui._key = toupper(keyState.keycode);

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

		if ((found == 0 && events._released) || (ui._key == 'E' || ui._key == Common::KEYCODE_ESCAPE))
			// Exit
			break;

		if ((found == 1 && events._released) || ui._key == 'M') {
			// Toggle music
			music._musicOn = !music._musicOn;
			if (!music._musicOn)
				music.stopMusic();
			else
				music.startSong();

			updateConfig = true;
			settings.drawInteface(true);
		}

		if ((found == 2 && events._released) || ui._key == 'V') {
			sound._voices = !sound._voices;
			updateConfig = true;
			settings.drawInteface(true);
		}

		if ((found == 3 && events._released) || ui._key == 'S') {
			// Toggle sound effects
			sound._digitized = !sound._digitized;
			updateConfig = true;
			settings.drawInteface(true);
		}

		if ((found == 4 && events._released) || ui._key == 'A') {
			// Help button style
			ui._helpStyle = !ui._helpStyle;
			updateConfig = true;
			settings.drawInteface(true);
		}

		if ((found == 5 && events._released) || ui._key == 'N') {
			// New font style
			int fontNum = screen.fontNumber() + 1;
			if (fontNum == 3)
				fontNum = 0;

			screen.setFont(fontNum);
			updateConfig = true;
			settings.drawInteface(true);
		}

		if ((found == 8 && events._released) || ui._key == 'F') {
			// Toggle fade style
			screen._fadeStyle = !screen._fadeStyle;
			updateConfig = true;
			settings.drawInteface(true);
		}

		if ((found == 9 && events._released) || ui._key == 'W') {
			// Window style
			ui._slideWindows = !ui._slideWindows;
			updateConfig = true;
			settings.drawInteface(true);
		}

		if ((found == 10 && events._released) || ui._key == 'P') {
			// Toggle portraits being shown
			people._portraitsOn = !people._portraitsOn;
			updateConfig = true;
			settings.drawInteface(true);
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
