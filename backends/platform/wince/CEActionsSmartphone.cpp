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
	bool is_kyra = (strncmp(gameid.c_str(), "kyra",4) == 0);
	bool is_samnmax = (gameid == "samnmax");
	bool is_cine = (gameid == "cine");
	bool is_touche = (gameid == "touche");
	bool is_agi = (gameid == "agi");
	bool is_parallaction = (gameid == "parallaction");
	bool is_lure = (gameid == "lure");
	bool is_feeble = (gameid == "feeble");
	bool is_drascula = (strncmp(gameid.c_str(), "drascula",8) == 0);
	bool is_tucker = (gameid == "tucker");
	bool is_groovie = (gameid == "groovie");
	bool is_tinsel = (gameid == "tinsel");
	bool is_cruise = (gameid == "cruise");

	GUI_Actions::initInstanceGame();

	// See if a right click mapping could be needed
	if (is_sword1 || is_sword2 || is_sky || is_queen || is_comi || is_gob || is_tinsel ||
			is_samnmax || is_cine || is_touche || is_parallaction || is_drascula || is_cruise)
		_right_click_needed = true;

	// Initialize keys for different actions
	// Save
	if (is_simon || is_sword2 || is_gob || is_kyra || is_feeble || is_groovie)
		_action_enabled[SMARTPHONE_ACTION_SAVE] = false;
	else if (is_queen) {
		_action_enabled[SMARTPHONE_ACTION_SAVE] = true;
		_key_action[SMARTPHONE_ACTION_SAVE].setKey(Common::ASCII_F5, SDLK_F5); // F1 key for FOTAQ
	} else if (is_sky) {
		_action_enabled[SMARTPHONE_ACTION_SAVE] = true;
		_key_action[SMARTPHONE_ACTION_SAVE].setKey(Common::ASCII_F5, SDLK_F5);
	} else if (is_cine || is_drascula || is_cruise) {
		_action_enabled[SMARTPHONE_ACTION_SAVE] = true;
		_key_action[SMARTPHONE_ACTION_SAVE].setKey(Common::ASCII_F10, SDLK_F10); //F10
	} else if (is_agi) {
		_action_enabled[SMARTPHONE_ACTION_SAVE] = true;
		_key_action[SMARTPHONE_ACTION_SAVE].setKey(Common::ASCII_ESCAPE, SDLK_ESCAPE);
	} else if (is_parallaction) {
		_action_enabled[SMARTPHONE_ACTION_SAVE] = true;
		_key_action[SMARTPHONE_ACTION_SAVE].setKey('s', SDLK_s);
	} else if (is_tinsel) {
		_action_enabled[SMARTPHONE_ACTION_SAVE] = true;
		_key_action[SMARTPHONE_ACTION_SAVE].setKey(Common::ASCII_F1, SDLK_F1);
	} else {
		_action_enabled[SMARTPHONE_ACTION_SAVE] = true;
		_key_action[SMARTPHONE_ACTION_SAVE].setKey(Common::ASCII_F5, SDLK_F5); // F5 key
	}
	// Skip
	_action_enabled[SMARTPHONE_ACTION_SKIP] = true;
	if (is_simon || is_sky || is_sword2 || is_queen || is_sword1 || is_gob || is_tinsel ||
			is_saga || is_kyra || is_touche || is_lure || is_feeble || is_drascula || is_tucker ||
			is_groovie || is_cruise)
		_key_action[SMARTPHONE_ACTION_SKIP].setKey(VK_ESCAPE);
	else
		_key_action[SMARTPHONE_ACTION_SKIP].setKey(KEY_ALL_SKIP);
	// Zone
	_action_enabled[SMARTPHONE_ACTION_ZONE] = true;
	// Multi function key
	_action_enabled[SMARTPHONE_ACTION_MULTI] = true;
	if (is_agi)
		_key_action[SMARTPHONE_ACTION_MULTI].setKey(SDLK_PAUSE); // agi: show predictive dialog
	else if (is_gob)
		_key_action[SMARTPHONE_ACTION_MULTI].setKey(Common::ASCII_F1, SDLK_F1); // bargon : F1 to start
	else if (gameid == "atlantis")
		_key_action[SMARTPHONE_ACTION_MULTI].setKey(0, SDLK_KP0); // fate of atlantis : Ins to sucker-punch
	else
		_key_action[SMARTPHONE_ACTION_MULTI].setKey('V', SDLK_v, KMOD_SHIFT); // FT cheat : shift-V
	// Bind keys
	_action_enabled[SMARTPHONE_ACTION_BINDKEYS] = true;
	// Disable double-tap right-click for convenience
	if (is_tinsel || is_cruise)
		if (!ConfMan.hasKey("no_doubletap_rightclick")) {
			ConfMan.setBool("no_doubletap_rightclick", true);
			ConfMan.flushToDisk();
		}
}


CEActionsSmartphone::~CEActionsSmartphone() {
}

bool CEActionsSmartphone::perform(GUI::ActionType action, bool pushed) {
	static bool keydialogrunning = false, quitdialog = false;

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
			case SMARTPHONE_ACTION_MULTI:
				EventsBuffer::simulateKey(&_key_action[action], false);
				return true;
		}
		return false;
	}

	switch (action) {
		case SMARTPHONE_ACTION_SAVE:
		case SMARTPHONE_ACTION_SKIP:
		case SMARTPHONE_ACTION_MULTI:
			if (action == SMARTPHONE_ACTION_SAVE && ConfMan.get("gameid") == "parallaction") {
				// FIXME: This is a temporary solution. The engine should handle its own menus.
				// Note that the user can accomplish this via the virtual keyboard as well, this is just for convenience
				GUI::MessageDialog alert("Do you want to load or save the game?", "Load", "Save");
				if (alert.runModal() == GUI::kMessageOK)
					_key_action[action].setKey(SDLK_l);
				else
					_key_action[action].setKey(SDLK_s);
			}
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
			if (!quitdialog) {
				quitdialog = true;
				GUI::MessageDialog alert("   Are you sure you want to quit ?   ", "Yes", "No");
				if (alert.runModal() == GUI::kMessageOK)
					_mainSystem->quit();
				quitdialog = false;
			}
			return true;
	}

	return false;
}
