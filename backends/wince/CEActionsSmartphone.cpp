/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2004 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */


#ifdef WIN32_PLATFORM_WFSP

#include "stdafx.h"
#include "CEActionsPocket.h"
#include "KeysBuffer.h"

#include "gui/message.h"

#include "scumm/scumm.h"

#include "common/config-manager.h"

const String smartphoneActionNames[] = { 
	"Up", 
	"Down", 
	"Left",
	"Right",
	"Left Click",
	"Right Click",
	"Save",
	"Skip",
	"Zone"
};

void CEActionsSmartphone::init(GameDetector &detector) {
	_instance = new CEActionsSmartphone(detector);
}


String CEActionsSmartphone::actionName(ActionType action) {
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

CEActionsSmartphone::CEActionsSmartphone(GameDetector &detector) :
	CEActions(detector)
{
	int i;

	for (i=0; i<SMARTPHONE_ACTION_LAST; i++) {
		_action_mapping[i] = 0;
		_action_enabled[i] = false;
	}

}

void CEActionsSmartphone::initInstance(OSystem_WINCE3 *mainSystem) {
{
	int i;
	bool is_simon = (strncmp(_detector->_targetName.c_str(), "simon", 5) == 0);
	bool is_sky = (_detector->_targetName == "sky");

	CEActions::initInstance(mainSystem);

	// See if a right click mapping could be needed
	if (is_sky || _detector->_targetName == "samnmax")
		_right_click_needed = true;

	// Initialize keys for different actions
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
	_action_enabled[ACTION_RIGHTCLICK] = true;
	// Save
	if (is_simon) 
		_action_enabled[SMARTPHONE_ACTION_SAVE] = false;
	else
	if (is_queen) {
		_action_enabled[SMARTPHONE_ACTION_SAVE] = true;
		_key_action[SMARTPHONE_ACTION_SAVE].setAscii(282); // F1 key
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
	if (is_simon || is_sky)
		_key_action[SMARTPHONE_ACTION_SKIP].setAscii(VK_ESCAPE);
	else
		_key_action[SMARTPHONE_ACTION_SKIP].setAscii(Scumm::KEY_ALL_SKIP);
	// Zone
	_key_action[SMARTPHONE_ACTION_ZONE] = true;
}


CEActionsSmartphone::~CEActionsSmartphone() {
}

bool CEActionsSmartphone::perform(ActionType action, bool pushed = true) {
	if (!pushed) {
		_mainSystem->clear_key_repeat();
		return true;
	}

	switch (action) {
		case SMARTPHONE_ACTION_SAVE:
		case SMARTPHONE_ACTION_SKIP:
			EventsBuffer::simulateKey(&_key_action[action]);
			return true;
		case SMARTPHONE_ACTION_RIGHTCLICK:
			_mainSystem->add_right_click();
			return true;
		case SMARTPHONE_ACTION_LEFTCLICK:
			_mainSystem->add_left_click();
			return true;
		case SMARTPHONE_ACTION_UP:
			_mainSystem->move_cursor_up();
			return true;
		case SMARTPHONE_ACTION_DOWN:
			_mainSystem->move_cursor_down();
			return true;
		case SMARTPHONE_ACTION_LEFT:
			_mainSystem->move_cursor_left();
			return true;
		case SMARTPHONE_ACTION_RIGHT:
			_mainSystem->move_cursor_right();
			return true;
		case SMARTPHONE_ACTION_ZONE:
			_mainSystem->switch_zone();
			return true;
	}

	return false;
}

#endif
