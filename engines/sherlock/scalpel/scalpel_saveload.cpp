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

#include "sherlock/scalpel/scalpel_saveload.h"
#include "sherlock/scalpel/scalpel_screen.h"
#include "sherlock/scalpel/scalpel.h"

namespace Sherlock {

namespace Scalpel {

const int ENV_POINTS[6][3] = {
	{ 41, 80, 61 },		// Exit
	{ 81, 120, 101 },	// Load
	{ 121, 160, 141 },	// Save
	{ 161, 200, 181 },	// Up
	{ 201, 240, 221 },	// Down
	{ 241, 280, 261 }	// Quit
};

/*----------------------------------------------------------------*/

ScalpelSaveManager::ScalpelSaveManager(SherlockEngine *vm, const Common::String &target) : SaveManager(vm, target) {
}

void ScalpelSaveManager::drawInterface() {
	ScalpelScreen &screen = *(ScalpelScreen *)_vm->_screen;
	UserInterface &ui = *_vm->_ui;

	// Create a list of savegame slots
	createSavegameList();

	screen._backBuffer1.fillRect(Common::Rect(0, CONTROLS_Y, SHERLOCK_SCREEN_WIDTH, CONTROLS_Y + 10), BORDER_COLOR);
	screen._backBuffer1.fillRect(Common::Rect(0, CONTROLS_Y + 10, 2, SHERLOCK_SCREEN_HEIGHT), BORDER_COLOR);
	screen._backBuffer1.fillRect(Common::Rect(318, CONTROLS_Y + 10, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT), BORDER_COLOR);
	screen._backBuffer1.fillRect(Common::Rect(0, 199, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT), BORDER_COLOR);
	screen._backBuffer1.fillRect(Common::Rect(2, CONTROLS_Y + 10, SHERLOCK_SCREEN_WIDTH - 2, SHERLOCK_SCREEN_HEIGHT - 2), INV_BACKGROUND);

	screen.makeButton(Common::Rect(ENV_POINTS[0][0], CONTROLS_Y, ENV_POINTS[0][1], CONTROLS_Y + 10),
		ENV_POINTS[0][2] - screen.stringWidth("Exit") / 2, "Exit");
	screen.makeButton(Common::Rect(ENV_POINTS[1][0], CONTROLS_Y, ENV_POINTS[1][1], CONTROLS_Y + 10),
		ENV_POINTS[1][2] - screen.stringWidth("Load") / 2, "Load");
	screen.makeButton(Common::Rect(ENV_POINTS[2][0], CONTROLS_Y, ENV_POINTS[2][1], CONTROLS_Y + 10),
		ENV_POINTS[2][2] - screen.stringWidth("Save") / 2, "Save");
	screen.makeButton(Common::Rect(ENV_POINTS[3][0], CONTROLS_Y, ENV_POINTS[3][1], CONTROLS_Y + 10),
		ENV_POINTS[3][2] - screen.stringWidth("Up") / 2, "Up");
	screen.makeButton(Common::Rect(ENV_POINTS[4][0], CONTROLS_Y, ENV_POINTS[4][1], CONTROLS_Y + 10),
		ENV_POINTS[4][2] - screen.stringWidth("Down") / 2, "Down");
	screen.makeButton(Common::Rect(ENV_POINTS[5][0], CONTROLS_Y, ENV_POINTS[5][1], CONTROLS_Y + 10),
		ENV_POINTS[5][2] - screen.stringWidth("Quit") / 2, "Quit");

	if (!_savegameIndex)
		screen.buttonPrint(Common::Point(ENV_POINTS[3][2], CONTROLS_Y), COMMAND_NULL, 0, "Up");

	if (_savegameIndex == MAX_SAVEGAME_SLOTS - ONSCREEN_FILES_COUNT)
		screen.buttonPrint(Common::Point(ENV_POINTS[4][2], CONTROLS_Y), COMMAND_NULL, 0, "Down");

	for (int idx = _savegameIndex; idx < _savegameIndex + ONSCREEN_FILES_COUNT; ++idx) {
		screen.gPrint(Common::Point(6, CONTROLS_Y + 11 + (idx - _savegameIndex) * 10),
			INV_FOREGROUND, "%d.", idx + 1);
		screen.gPrint(Common::Point(24, CONTROLS_Y + 11 + (idx - _savegameIndex) * 10),
			INV_FOREGROUND, "%s", _savegames[idx].c_str());
	}

	if (!ui._slideWindows) {
		screen.slamRect(Common::Rect(0, CONTROLS_Y, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT));
	} else {
		ui.summonWindow();
	}

	_envMode = SAVEMODE_NONE;
}

int ScalpelSaveManager::getHighlightedButton() const {
	Common::Point pt = _vm->_events->mousePos();

	for (int idx = 0; idx < 6; ++idx) {
		if (pt.x > ENV_POINTS[idx][0] && pt.x < ENV_POINTS[idx][1] && pt.y > CONTROLS_Y
				&& pt.y < (CONTROLS_Y + 10))
			return idx;
	}

	return -1;
}

void ScalpelSaveManager::highlightButtons(int btnIndex) {
	ScalpelScreen &screen = *(ScalpelScreen *)_vm->_screen;
	byte color = (btnIndex == 0) ? COMMAND_HIGHLIGHTED : COMMAND_FOREGROUND;

	screen.buttonPrint(Common::Point(ENV_POINTS[0][2], CONTROLS_Y), color, 1, "Exit");

	if ((btnIndex == 1) || ((_envMode == SAVEMODE_LOAD) && (btnIndex != 2)))
		screen.buttonPrint(Common::Point(ENV_POINTS[1][2], CONTROLS_Y), COMMAND_HIGHLIGHTED, true, "Load");
	else
		screen.buttonPrint(Common::Point(ENV_POINTS[1][2], CONTROLS_Y), COMMAND_FOREGROUND, true, "Load");

	if ((btnIndex == 2) || ((_envMode == SAVEMODE_SAVE) && (btnIndex != 1)))
		screen.buttonPrint(Common::Point(ENV_POINTS[2][2], CONTROLS_Y), COMMAND_HIGHLIGHTED, true, "Save");
	else
		screen.buttonPrint(Common::Point(ENV_POINTS[2][2], CONTROLS_Y), COMMAND_FOREGROUND, true, "Save");

	if (btnIndex == 3 && _savegameIndex)
		screen.buttonPrint(Common::Point(ENV_POINTS[3][2], CONTROLS_Y), COMMAND_HIGHLIGHTED, true, "Up");
	else
		if (_savegameIndex)
			screen.buttonPrint(Common::Point(ENV_POINTS[3][2], CONTROLS_Y), COMMAND_FOREGROUND, true, "Up");

	if ((btnIndex == 4) && (_savegameIndex < MAX_SAVEGAME_SLOTS - 5))
		screen.buttonPrint(Common::Point(ENV_POINTS[4][2], CONTROLS_Y), COMMAND_HIGHLIGHTED, true, "Down");
	else if (_savegameIndex < (MAX_SAVEGAME_SLOTS - 5))
		screen.buttonPrint(Common::Point(ENV_POINTS[4][2], CONTROLS_Y), COMMAND_FOREGROUND, true, "Down");

	color = (btnIndex == 5) ? COMMAND_HIGHLIGHTED : COMMAND_FOREGROUND;
	screen.buttonPrint(Common::Point(ENV_POINTS[5][2], CONTROLS_Y), color, 1, "Quit");
}

bool ScalpelSaveManager::checkGameOnScreen(int slot) {
	ScalpelScreen &screen = *(ScalpelScreen *)_vm->_screen;

	// Check if it's already on-screen
	if (slot != -1 && (slot < _savegameIndex || slot >= (_savegameIndex + ONSCREEN_FILES_COUNT))) {
		_savegameIndex = slot;

		screen._backBuffer1.fillRect(Common::Rect(3, CONTROLS_Y + 11, SHERLOCK_SCREEN_WIDTH - 2,
			SHERLOCK_SCREEN_HEIGHT - 1), INV_BACKGROUND);

		for (int idx = _savegameIndex; idx < (_savegameIndex + 5); ++idx) {
			screen.gPrint(Common::Point(6, CONTROLS_Y + 11 + (idx - _savegameIndex) * 10),
				INV_FOREGROUND, "%d.", idx + 1);
			screen.gPrint(Common::Point(24, CONTROLS_Y + 11 + (idx - _savegameIndex) * 10),
				INV_FOREGROUND, "%s", _savegames[idx].c_str());
		}

		screen.slamRect(Common::Rect(3, CONTROLS_Y + 11, 318, SHERLOCK_SCREEN_HEIGHT));

		byte color = !_savegameIndex ? COMMAND_NULL : COMMAND_FOREGROUND;
		screen.buttonPrint(Common::Point(ENV_POINTS[3][2], CONTROLS_Y), color, 1, "Up");

		color = (_savegameIndex == (MAX_SAVEGAME_SLOTS - 5)) ? COMMAND_NULL : COMMAND_FOREGROUND;
		screen.buttonPrint(Common::Point(ENV_POINTS[4][2], CONTROLS_Y), color, 1, "Down");

		return true;
	}

	return false;
}

bool ScalpelSaveManager::promptForDescription(int slot) {
	Events &events = *_vm->_events;
	Scene &scene = *_vm->_scene;
	ScalpelScreen &screen = *(ScalpelScreen *)_vm->_screen;
	Talk &talk = *_vm->_talk;
	int xp, yp;
	bool flag = false;

	screen.buttonPrint(Common::Point(ENV_POINTS[0][2], CONTROLS_Y), COMMAND_NULL, true, "Exit");
	screen.buttonPrint(Common::Point(ENV_POINTS[1][2], CONTROLS_Y), COMMAND_NULL, true, "Load");
	screen.buttonPrint(Common::Point(ENV_POINTS[2][2], CONTROLS_Y), COMMAND_NULL, true, "Save");
	screen.buttonPrint(Common::Point(ENV_POINTS[3][2], CONTROLS_Y), COMMAND_NULL, true, "Up");
	screen.buttonPrint(Common::Point(ENV_POINTS[4][2], CONTROLS_Y), COMMAND_NULL, true, "Down");
	screen.buttonPrint(Common::Point(ENV_POINTS[5][2], CONTROLS_Y), COMMAND_NULL, true, "Quit");

	Common::String saveName = _savegames[slot];
	if (isSlotEmpty(slot)) {
		// It's an empty slot, so start off with an empty save name
		saveName = "";

		yp = CONTROLS_Y + 12 + (slot - _savegameIndex) * 10;
		screen.vgaBar(Common::Rect(24, yp, 85, yp + 9), INV_BACKGROUND);
	}

	screen.print(Common::Point(6, CONTROLS_Y + 12 + (slot - _savegameIndex) * 10), TALK_FOREGROUND, "%d.", slot + 1);
	screen.print(Common::Point(24, CONTROLS_Y + 12 + (slot - _savegameIndex) * 10), TALK_FOREGROUND, "%s", saveName.c_str());
	xp = 24 + screen.stringWidth(saveName);
	yp = CONTROLS_Y + 12 + (slot - _savegameIndex) * 10;

	int done = 0;
	do {
		while (!_vm->shouldQuit() && !events.kbHit()) {
			scene.doBgAnim();

			if (talk._talkToAbort)
				return false;

			// Allow event processing
			events.pollEventsAndWait();
			events.setButtonState();

			flag = !flag;
			if (flag)
				screen.vgaBar(Common::Rect(xp, yp - 1, xp + 8, yp + 9), INV_FOREGROUND);
			else
				screen.vgaBar(Common::Rect(xp, yp - 1, xp + 8, yp + 9), INV_BACKGROUND);
		}
		if (_vm->shouldQuit())
			return false;

		// Get the next keypress
		Common::KeyState keyState = events.getKey();

		if (keyState.keycode == Common::KEYCODE_BACKSPACE && saveName.size() > 0) {
			// Delete character of save name
			screen.vgaBar(Common::Rect(xp - screen.charWidth(saveName.lastChar()), yp - 1,
				xp + 8, yp + 9), INV_BACKGROUND);
			xp -= screen.charWidth(saveName.lastChar());
			screen.vgaBar(Common::Rect(xp, yp - 1, xp + 8, yp + 9), INV_FOREGROUND);
			saveName.deleteLastChar();
		
		} else if (keyState.keycode == Common::KEYCODE_RETURN && saveName.compareToIgnoreCase(EMPTY_SAVEGAME_SLOT)) {
			done = 1;

		} else if (keyState.keycode == Common::KEYCODE_ESCAPE) {
			screen.vgaBar(Common::Rect(xp, yp - 1, xp + 8, yp + 9), INV_BACKGROUND);
			done = -1;
		
		} else if (keyState.ascii >= ' ' && keyState.ascii <= 'z' && saveName.size() < 50
				&& (xp + screen.charWidth(keyState.ascii)) < 308) {
			char c = (char)keyState.ascii;

			screen.vgaBar(Common::Rect(xp, yp - 1, xp + 8, yp + 9), INV_BACKGROUND);
			screen.print(Common::Point(xp, yp), TALK_FOREGROUND, "%c", c);
			xp += screen.charWidth(c);
			screen.vgaBar(Common::Rect(xp, yp - 1, xp + 8, yp + 9), INV_FOREGROUND);
			saveName += c;
		}
	} while (!done);

	if (done == 1) {
		// Enter key perssed
		_savegames[slot] = saveName;
	} else {
		done = 0;
		_envMode = SAVEMODE_NONE;
		highlightButtons(-1);
	}

	return done == 1;
}

} // End of namespace Scalpel

} // End of namespace Sherlock
