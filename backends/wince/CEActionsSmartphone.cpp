/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2006 The ScummVM project
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

//#define SIMU_SMARTPHONE 1

//#ifdef WIN32_PLATFORM_WFSP

#include "common/stdafx.h"
#include "CEActionsSmartphone.h"
#include "EventsBuffer.h"

#include "gui/message.h"

#include "scumm/scumm.h"

#include "common/config-manager.h"

#include "gui/KeysDialog.h"

#ifdef _WIN32_WCE
#define		KEY_ALL_SKIP	3457
#endif

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
	"FT Cheat",
	"Bind Keys"
};

#ifdef SIMU_SMARTPHONE
const int ACTIONS_SMARTPHONE_DEFAULT[] = { 0x111, 0x112, 0x114, 0x113, 0x11a, 0x11b, VK_LWIN, VK_ESCAPE, VK_F8, 0, VK_RETURN };
#else
const int ACTIONS_SMARTPHONE_DEFAULT[] = { '4', '6', '8', '2', 0x11a, 0x11b, '0', VK_ESCAPE, '9', 0, VK_RETURN };
#endif

void CEActionsSmartphone::init(const Common::String &gameid) {
	_instance = new CEActionsSmartphone(gameid);
}


String CEActionsSmartphone::actionName(GUI::ActionType action) {
	return smartphoneActionNames[action];
}

int CEActionsSmartphone::size() {
	return SMARTPHONE_ACTION_LAST;
}

String CEActionsSmartphone::domain() {
	return "smartphone";
}

int CEActionsSmartphone::version() {
	return SMARTPHONE_ACTION_VERSION;
}

CEActionsSmartphone::CEActionsSmartphone(const Common::String &gameid) :
GUI::Actions(gameid)
{
	int i;

	for (i=0; i<SMARTPHONE_ACTION_LAST; i++) {
		_action_mapping[i] = ACTIONS_SMARTPHONE_DEFAULT[i];
		_action_enabled[i] = false;
	}

}

void CEActionsSmartphone::initInstanceMain(OSystem *mainSystem) {
	_CESystem = static_cast<OSystem_WINCE3*>(mainSystem);

	GUI_Actions::initInstanceMain(mainSystem);
	// Mouse Up
	_action_enabled[SMARTPHONE_ACTION_UP] = true;
	// Mouse Down
	_action_enabled[SMARTPHONE_ACTION_DOWN] = true;
	// Mouse Left
	_action_enabled[SMARTPHONE_ACTION_LEFT] = true;
	// Mouse Right
	_action_enabled[SMARTPHONE_ACTION_RIGHT] = true;
	// Left Click
	_action_enabled[SMARTPHONE_ACTION_LEFTCLICK] = true;
	// Right Click
	_action_enabled[SMARTPHONE_ACTION_RIGHTCLICK] = true;
}

void CEActionsSmartphone::initInstanceGame() {
	bool is_simon = (strncmp(_gameid.c_str(), "simon", 5) == 0);
	bool is_sky = (_gameid == "sky");
	bool is_queen = (_gameid == "queen");
	bool is_gob = (strncmp(_gameid.c_str(), "gob", 3) == 0);
	bool is_ite = ((strncmp(_gameid.c_str(), "ite", 3) == 0) ||
				  (strncmp(_gameid.c_str(), "ihnm", 4) == 0));

	GUI_Actions::initInstanceGame();

	// See if a right click mapping could be needed
	if (is_sky || _gameid == "samnmax" || is_gob)
		_right_click_needed = true;

	// Initialize keys for different actions
	// Save
	if (is_simon || is_gob)
		_action_enabled[SMARTPHONE_ACTION_SAVE] = false;
	else
	if (is_queen || is_ite) {
		_action_enabled[SMARTPHONE_ACTION_SAVE] = true;
		_key_action[SMARTPHONE_ACTION_SAVE].setAscii(286); // F1 key for FOTAQ and ITE
	}
	else
	if (is_sky) {
		_action_enabled[SMARTPHONE_ACTION_SAVE] = true;
		_key_action[SMARTPHONE_ACTION_SAVE].setAscii(63);
	}
	else {
		_action_enabled[SMARTPHONE_ACTION_SAVE] = true;
		_key_action[SMARTPHONE_ACTION_SAVE].setAscii(319); // F5 key
	}
	// Skip
	_action_enabled[SMARTPHONE_ACTION_SKIP] = true;
	if (is_simon || is_sky || is_gob || is_ite)
		_key_action[SMARTPHONE_ACTION_SKIP].setAscii(VK_ESCAPE);
	else
		_key_action[SMARTPHONE_ACTION_SKIP].setAscii(KEY_ALL_SKIP);
	// Zone
	_action_enabled[SMARTPHONE_ACTION_ZONE] = true;
	// FT Cheat
	_action_enabled[SMARTPHONE_ACTION_FT_CHEAT] = true;
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
	}

	return false;
}

//#endif
