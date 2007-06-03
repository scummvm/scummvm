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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "common/stdafx.h"
#include "CEActionsSmartphone.h"
#include "EventsBuffer.h"
#include "gui/message.h"
#include "scumm/scumm.h"
#include "common/config-manager.h"
#include "gui/KeysDialog.h"

#define		KEY_ALL_SKIP	3457

const String smartphoneActionNames[] = {
	"Up",
	"Down",
	"Left",
	"Right",
	"Left Click",
	"Right Click",
	"Save",
	"Skip",
	"Zone",
	"Multi Function",
	"Bind Keys",
	"Keyboard",
	"Rotate",
	"Quit"
};

const int ACTIONS_SMARTPHONE_DEFAULT[] = { SDLK_UP, SDLK_DOWN, SDLK_LEFT, SDLK_RIGHT, SDLK_F1, SDLK_F2, SDLK_F3, SDLK_ESCAPE, SDLK_9, SDLK_8, SDLK_F4, SDLK_RETURN, SDLK_5, SDLK_0 };

void CEActionsSmartphone::init() {
	_instance = new CEActionsSmartphone();
}


String CEActionsSmartphone::actionName(GUI::ActionType action) {
	return smartphoneActionNames[action];
}

int CEActionsSmartphone::size() {
	return SMARTPHONE_ACTION_LAST;
}

String CEActionsSmartphone::domain() {
	return ConfMan.kApplicationDomain;
}

int CEActionsSmartphone::version() {
	return SMARTPHONE_ACTION_VERSION;
}

CEActionsSmartphone::CEActionsSmartphone()
: GUI::Actions() {
	int i;

	for (i=0; i<SMARTPHONE_ACTION_LAST; i++) {
		_action_mapping[i] = ACTIONS_SMARTPHONE_DEFAULT[i];
		_action_enabled[i] = false;
	}

}

void CEActionsSmartphone::initInstanceMain(OSystem *mainSystem) {
	_CESystem = static_cast<OSystem_WINCE3*>(mainSystem);

	GUI_Actions::initInstanceMain(mainSystem);

	// These actions are always on
	_action_enabled[SMARTPHONE_ACTION_UP] = true;
	_action_enabled[SMARTPHONE_ACTION_DOWN] = true;
	_action_enabled[SMARTPHONE_ACTION_LEFT] = true;
	_action_enabled[SMARTPHONE_ACTION_RIGHT] = true;
	_action_enabled[SMARTPHONE_ACTION_LEFTCLICK] = true;
	_action_enabled[SMARTPHONE_ACTION_RIGHTCLICK] = true;
	_action_enabled[SMARTPHONE_ACTION_KEYBOARD] = true;
	_action_enabled[SMARTPHONE_ACTION_ROTATE] = true;
	_action_enabled[SMARTPHONE_ACTION_QUIT] = true;
	_action_enabled[SMARTPHONE_ACTION_BINDKEYS] = true;
}

void CEActionsSmartphone::initInstanceGame() {
	String gameid(ConfMan.get("gameid"));
	bool is_simon = (strncmp(gameid.c_str(), "simon", 5) == 0);
	bool is_sword1 = (gameid == "sword1");
	bool is_sword2 = (strcmp(gameid.c_str(), "sword2") == 0);
	bool is_queen = (gameid == "queen");
	bool is_sky = (gameid == "sky");
	bool is_comi = (strncmp(gameid.c_str(), "comi", 4) == 0);
	bool is_gob = (strncmp(gameid.c_str(), "gob", 3) == 0);
	bool is_saga = (gameid == "saga");
	bool is_kyra = (gameid == "kyra1");
	bool is_samnmax = (gameid == "samnmax");
	bool is_cine = (gameid == "cine");
	bool is_touche = (gameid == "touche");
	bool is_agi = (gameid == "agi");

	GUI_Actions::initInstanceGame();

	// See if a right click mapping could be needed
	if (is_sword1 || is_sword2 || is_sky || is_queen || is_comi || is_gob || is_samnmax || is_cine || is_touche)
		_right_click_needed = true;

	// Initialize keys for different actions
	// Save
	if (is_simon || is_sword2 || is_gob || is_kyra || is_touche)
		_action_enabled[SMARTPHONE_ACTION_SAVE] = false;
	else if (is_queen) {
		_action_enabled[SMARTPHONE_ACTION_SAVE] = true;
		_key_action[SMARTPHONE_ACTION_SAVE].setAscii(286); // F1 key for FOTAQ
	} else if (is_sky) {
		_action_enabled[SMARTPHONE_ACTION_SAVE] = true;
		_key_action[SMARTPHONE_ACTION_SAVE].setAscii(63);
	} else if (is_cine) {
		_action_enabled[SMARTPHONE_ACTION_SAVE] = true;
		_key_action[SMARTPHONE_ACTION_SAVE].setAscii(291); //F10
	} else if (is_agi) {
		_action_enabled[SMARTPHONE_ACTION_SAVE] = true;
		_key_action[SMARTPHONE_ACTION_SAVE].setAscii(SDLK_ESCAPE);
	} else {
		_action_enabled[SMARTPHONE_ACTION_SAVE] = true;
		_key_action[SMARTPHONE_ACTION_SAVE].setAscii(319); // F5 key
	}
	// Skip
	_action_enabled[SMARTPHONE_ACTION_SKIP] = true;
	if (is_simon || is_sky || is_sword2 || is_queen || is_sword1 || is_gob || is_saga || is_kyra || is_touche)
		_key_action[SMARTPHONE_ACTION_SKIP].setAscii(VK_ESCAPE);
	else
		_key_action[SMARTPHONE_ACTION_SKIP].setAscii(KEY_ALL_SKIP);
	// Zone
	_action_enabled[SMARTPHONE_ACTION_ZONE] = true;
	// Multi function key
	_action_enabled[SMARTPHONE_ACTION_FT_CHEAT] = true;
	if (is_agi)
		_key_action[SMARTPHONE_ACTION_FT_CHEAT].setAscii(SDLK_PAUSE);
	else
		_key_action[SMARTPHONE_ACTION_FT_CHEAT].setAscii(86); // shift-V
	// Bind keys
	_action_enabled[SMARTPHONE_ACTION_BINDKEYS] = true;
}


CEActionsSmartphone::~CEActionsSmartphone() {
}

bool CEActionsSmartphone::perform(GUI::ActionType action, bool pushed) {
	static bool keydialogrunning = false;

	if (!pushed) {
		switch (action) {
			case SMARTPHONE_ACTION_RIGHTCLICK:
				_CESystem->add_right_click(false);
				return true;
			case SMARTPHONE_ACTION_LEFTCLICK:
				_CESystem->add_left_click(false);
				return true;
			case SMARTPHONE_ACTION_SAVE:
			case SMARTPHONE_ACTION_SKIP:
			case SMARTPHONE_ACTION_FT_CHEAT:
				EventsBuffer::simulateKey(&_key_action[action], false);
				return true;
		}
		return false;
	}

	switch (action) {
		case SMARTPHONE_ACTION_SAVE:
		case SMARTPHONE_ACTION_SKIP:
		case SMARTPHONE_ACTION_FT_CHEAT:
			EventsBuffer::simulateKey(&_key_action[action], true);
			return true;
		case SMARTPHONE_ACTION_RIGHTCLICK:
			_CESystem->add_right_click(true);
			return true;
		case SMARTPHONE_ACTION_LEFTCLICK:
			_CESystem->add_left_click(true);
			return true;
		case SMARTPHONE_ACTION_UP:
			_CESystem->move_cursor_up();
			return true;
		case SMARTPHONE_ACTION_DOWN:
			_CESystem->move_cursor_down();
			return true;
		case SMARTPHONE_ACTION_LEFT:
			_CESystem->move_cursor_left();
			return true;
		case SMARTPHONE_ACTION_RIGHT:
			_CESystem->move_cursor_right();
			return true; 
		case SMARTPHONE_ACTION_ZONE:
			_CESystem->switch_zone();
			return true;
		case SMARTPHONE_ACTION_BINDKEYS:
			if (!keydialogrunning) {
				keydialogrunning = true;
				GUI::KeysDialog *keysDialog = new GUI::KeysDialog();
				keysDialog->runModal();
				delete keysDialog;
				keydialogrunning = false;
			}
			return true;
		case SMARTPHONE_ACTION_KEYBOARD:
			_CESystem->swap_smartphone_keyboard();
			return true;
		case SMARTPHONE_ACTION_ROTATE:
			_CESystem->smartphone_rotate_display();
			return true;
		case SMARTPHONE_ACTION_QUIT:
			{
				GUI::MessageDialog alert("   Are you sure you want to quit ?   ", "Yes", "No");
				if (alert.runModal() == GUI::kMessageOK)
					_mainSystem->quit();
				return true;
			}
	}

	return false;
}
