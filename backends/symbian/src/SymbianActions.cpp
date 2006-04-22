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

#include "common/stdafx.h"
#include "backends/symbian/src/SymbianActions.h"

#include "gui/message.h"
#include "scumm/scumm.h"
#include "common/config-manager.h"

#include <sdl.h>

namespace GUI {

// SumthinWicked says: we either split our Actions like WinCE did with Pocket/Smartphone
// or we put them in this file separated by #ifdefs, this one is up to you, AnotherGuest :)
 
const Common::String actionNames[] = { 
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
	"Swap character",
	"Skip text",	
	"Pause", 
	"Quit"
};

#ifdef UIQ
static const int ACTIONS_DEFAULT[ACTION_LAST] = { 0, 0, 0, 0, 0x11a, 0x11b, SDLK_MENU, SDLK_PAGEDOWN, 0, 0,SDLK_PAGEUP,0,0};
#elif defined (S60)
const int ACTIONS_DEFAULT[ACTION_LAST] = { 0, 0, 0, 0, 0, 0, '*', '#', '9',0,0,0,0,0};
#elif defined (S90)
const int ACTIONS_DEFAULT[ACTION_LAST] = { 0, 0, 0, 0, 0, 0, SDLK_MENU, SDLK_ESCAPE, 0, 0,0,0,0,0};
#else
const int ACTIONS_DEFAULT[ACTION_LAST] = { 0, 0, 0, 0, 0x11a, 0x11b, SDLK_MENU, SDLK_ESCAPE, 0, 0,0,0,0,0};
#endif

// creator function according to Factory Pattern
void SymbianActions::init() {
	_instance = new SymbianActions();
}


Common::String SymbianActions::actionName(ActionType action) {
	return actionNames[action];
}

int SymbianActions::size() {
	return ACTION_LAST;
}

Common::String SymbianActions::domain() {
	return Common::ConfigManager::kApplicationDomain;
}

int SymbianActions::version() {
	return ACTION_VERSION;
}

SymbianActions::SymbianActions()
 : Actions() {
	int i;

	for (i = 0; i < ACTION_LAST; i++) {
		_action_mapping[i] = ACTIONS_DEFAULT[i];
		_action_enabled[i] = false;
	}

}

void SymbianActions::initInstanceMain(OSystem *mainSystem) {
	Actions::initInstanceMain(mainSystem);
	
	// Mouse Up
	_action_enabled[ACTION_UP] = true;

	// Mouse Down
	_action_enabled[ACTION_DOWN] = true;

	// Mouse Left
	_action_enabled[ACTION_LEFT] = true;

	// Mouse Right
	_action_enabled[ACTION_RIGHT] = true;

	// Left Click
	_action_enabled[ACTION_LEFTCLICK] = true;

	// Right Click
	_action_enabled[ACTION_RIGHTCLICK] = true;

	// Skip
	_action_enabled[ACTION_SKIP] = true;
	_key_action[ACTION_SKIP].setAscii(SDLK_ESCAPE);
}

void SymbianActions::initInstanceGame() {
	Common::String gameid(ConfMan.get("gameid"));
	bool is_simon = (strncmp(gameid.c_str(), "simon", 5) == 0);
	bool is_sky = (strncmp(gameid.c_str(), "sky", 3) == 0);
	bool is_queen = (strncmp(gameid.c_str(), "queen", 5) == 0);
	bool is_gob = (strncmp(gameid.c_str(), "gob", 3) == 0);
	bool is_ite = ((strncmp(gameid.c_str(), "ite", 3) == 0) ||
				  (strncmp(gameid.c_str(), "ihnm", 4) == 0));
	
	Actions::initInstanceGame();

	// See if a right click mapping could be needed
	if (is_sky || gameid == "samnmax" || is_gob)
		_right_click_needed = true;

	// Initialize keys for different actions
	// Save
	if (is_simon || is_gob) 
		_action_enabled[ACTION_SAVE] = false;
	else if (is_queen || is_ite) {
		_action_enabled[ACTION_SAVE] = true;
		_key_action[ACTION_SAVE].setAscii(SDLK_F1); // F1 key for FOTAQ and ITE
	} else if (is_sky) {
		_action_enabled[ACTION_SAVE] = true;
		_key_action[ACTION_SAVE].setAscii(63); 
	} else {
		_action_enabled[ACTION_SAVE] = true;
		_key_action[ACTION_SAVE].setAscii(SDLK_F5); // F5 key
	}

	// Swap character
	_action_enabled[ACTION_SWAPCHAR] = true;
	_key_action[ACTION_SWAPCHAR].setAscii('b'); // b

	// Zone
	_action_enabled[ACTION_ZONE] = true;

	// FT Cheat
	_action_enabled[ACTION_FT_CHEAT] = true;
	_key_action[ACTION_FT_CHEAT].setAscii(86); // shift-V

	// Skip text
	_action_enabled[ACTION_SKIP_TEXT] = true;
	if (is_queen) {
		_key_action[ACTION_SKIP_TEXT].setAscii(SDLK_SPACE);
	} else {
		_key_action[ACTION_SKIP_TEXT].setAscii(SDLK_PERIOD);
	}

	// Pause
	_key_action[ACTION_PAUSE].setAscii(' ');
	_action_enabled[ACTION_PAUSE] = true;

	// Quit
	_action_enabled[ACTION_QUIT] = true;
}


SymbianActions::~SymbianActions() {
}

bool SymbianActions::perform(ActionType /*action*/, bool /*pushed*/) {

	return false;
}

} // namespace GUI
