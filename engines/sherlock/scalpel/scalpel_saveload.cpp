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

#include "sherlock/scalpel/scalpel_fixed_text.h"
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

ScalpelSaveManager::ScalpelSaveManager(SherlockEngine *vm, const Common::String &target) :
	SaveManager(vm, target), _envMode(SAVEMODE_NONE) {

	_fixedTextExit = FIXED(LoadSave_Exit);
	_fixedTextLoad = FIXED(LoadSave_Load);
	_fixedTextSave  = FIXED(LoadSave_Save);
	_fixedTextUp = FIXED(LoadSave_Up);
	_fixedTextDown = FIXED(LoadSave_Down);
	_fixedTextQuit = FIXED(LoadSave_Quit);

	_hotkeyExit = toupper(_fixedTextExit[0]);
	_hotkeyLoad = toupper(_fixedTextLoad[0]);
	_hotkeySave = toupper(_fixedTextSave[0]);
	_hotkeyUp   = toupper(_fixedTextUp[0]);
	_hotkeyDown = toupper(_fixedTextDown[0]);
	_hotkeyQuit = toupper(_fixedTextQuit[0]);

	_hotkeysIndexed[0] = _hotkeyExit;
	_hotkeysIndexed[1] = _hotkeyLoad;
	_hotkeysIndexed[2] = _hotkeySave;
	_hotkeysIndexed[3] = _hotkeyUp;
	_hotkeysIndexed[4] = _hotkeyDown;
	_hotkeysIndexed[5] = _hotkeyQuit;

	_fixedTextQuitGameQuestion = FIXED(QuitGame_Question);
	_fixedTextQuitGameYes = FIXED(QuitGame_Yes);
	_fixedTextQuitGameNo = FIXED(QuitGame_No);

	_hotkeyQuitGameYes = toupper(_fixedTextQuitGameYes[0]);
	_hotkeyQuitGameNo  = toupper(_fixedTextQuitGameNo[0]);
}

int ScalpelSaveManager::identifyUserButton(int key) {
	for (uint16 hotkeyNr = 0; hotkeyNr < sizeof(_hotkeysIndexed); hotkeyNr++) {
		if (key == _hotkeysIndexed[hotkeyNr])
			return hotkeyNr;
	}
	return -1;
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
		ENV_POINTS[0][2], _fixedTextExit);
	screen.makeButton(Common::Rect(ENV_POINTS[1][0], CONTROLS_Y, ENV_POINTS[1][1], CONTROLS_Y + 10),
		ENV_POINTS[1][2], _fixedTextLoad);
	screen.makeButton(Common::Rect(ENV_POINTS[2][0], CONTROLS_Y, ENV_POINTS[2][1], CONTROLS_Y + 10),
		ENV_POINTS[2][2], _fixedTextSave);
	screen.makeButton(Common::Rect(ENV_POINTS[3][0], CONTROLS_Y, ENV_POINTS[3][1], CONTROLS_Y + 10),
		ENV_POINTS[3][2], _fixedTextUp);
	screen.makeButton(Common::Rect(ENV_POINTS[4][0], CONTROLS_Y, ENV_POINTS[4][1], CONTROLS_Y + 10),
		ENV_POINTS[4][2], _fixedTextDown);
	screen.makeButton(Common::Rect(ENV_POINTS[5][0], CONTROLS_Y, ENV_POINTS[5][1], CONTROLS_Y + 10),
		ENV_POINTS[5][2], _fixedTextQuit);

	if (!_savegameIndex)
		screen.buttonPrint(Common::Point(ENV_POINTS[3][2], CONTROLS_Y), COMMAND_NULL, 0, _fixedTextUp);

	if (_savegameIndex == MAX_SAVEGAME_SLOTS - ONSCREEN_FILES_COUNT)
		screen.buttonPrint(Common::Point(ENV_POINTS[4][2], CONTROLS_Y), COMMAND_NULL, 0, _fixedTextDown);

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

	screen.buttonPrint(Common::Point(ENV_POINTS[0][2], CONTROLS_Y), color, 1, _fixedTextExit);

	if ((btnIndex == 1) || ((_envMode == SAVEMODE_LOAD) && (btnIndex != 2)))
		screen.buttonPrint(Common::Point(ENV_POINTS[1][2], CONTROLS_Y), COMMAND_HIGHLIGHTED, true, _fixedTextLoad);
	else
		screen.buttonPrint(Common::Point(ENV_POINTS[1][2], CONTROLS_Y), COMMAND_FOREGROUND, true, _fixedTextLoad);

	if ((btnIndex == 2) || ((_envMode == SAVEMODE_SAVE) && (btnIndex != 1)))
		screen.buttonPrint(Common::Point(ENV_POINTS[2][2], CONTROLS_Y), COMMAND_HIGHLIGHTED, true, _fixedTextSave);
	else
		screen.buttonPrint(Common::Point(ENV_POINTS[2][2], CONTROLS_Y), COMMAND_FOREGROUND, true, _fixedTextSave);

	if (btnIndex == 3 && _savegameIndex)
		screen.buttonPrint(Common::Point(ENV_POINTS[3][2], CONTROLS_Y), COMMAND_HIGHLIGHTED, true, _fixedTextUp);
	else
		if (_savegameIndex)
			screen.buttonPrint(Common::Point(ENV_POINTS[3][2], CONTROLS_Y), COMMAND_FOREGROUND, true, _fixedTextUp);

	if ((btnIndex == 4) && (_savegameIndex < MAX_SAVEGAME_SLOTS - 5))
		screen.buttonPrint(Common::Point(ENV_POINTS[4][2], CONTROLS_Y), COMMAND_HIGHLIGHTED, true, _fixedTextDown);
	else if (_savegameIndex < (MAX_SAVEGAME_SLOTS - 5))
		screen.buttonPrint(Common::Point(ENV_POINTS[4][2], CONTROLS_Y), COMMAND_FOREGROUND, true, _fixedTextDown);

	color = (btnIndex == 5) ? COMMAND_HIGHLIGHTED : COMMAND_FOREGROUND;
	screen.buttonPrint(Common::Point(ENV_POINTS[5][2], CONTROLS_Y), color, 1, _fixedTextQuit);
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
		screen.buttonPrint(Common::Point(ENV_POINTS[3][2], CONTROLS_Y), color, 1, _fixedTextUp);

		color = (_savegameIndex == (MAX_SAVEGAME_SLOTS - 5)) ? COMMAND_NULL : COMMAND_FOREGROUND;
		screen.buttonPrint(Common::Point(ENV_POINTS[4][2], CONTROLS_Y), color, 1, _fixedTextDown);

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

	screen.buttonPrint(Common::Point(ENV_POINTS[0][2], CONTROLS_Y), COMMAND_NULL, true, _fixedTextExit);
	screen.buttonPrint(Common::Point(ENV_POINTS[1][2], CONTROLS_Y), COMMAND_NULL, true, _fixedTextLoad);
	screen.buttonPrint(Common::Point(ENV_POINTS[2][2], CONTROLS_Y), COMMAND_NULL, true, _fixedTextSave);
	screen.buttonPrint(Common::Point(ENV_POINTS[3][2], CONTROLS_Y), COMMAND_NULL, true, _fixedTextUp);
	screen.buttonPrint(Common::Point(ENV_POINTS[4][2], CONTROLS_Y), COMMAND_NULL, true, _fixedTextDown);
	screen.buttonPrint(Common::Point(ENV_POINTS[5][2], CONTROLS_Y), COMMAND_NULL, true, _fixedTextQuit);

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
