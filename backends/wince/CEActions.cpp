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


#include "stdafx.h"
#include "CEActions.h"
#include "KeysBuffer.h"

#include "gui/message.h"

#include "scumm/scumm.h"

#include "common/config-manager.h"

const String actionNames[] = { 
	"none", 
	"Pause", 
	"Save", 
	"Quit",
	"Skip",
	"Hide",
	"Keyboard",
	"Sound",
	"Right click",
	"Cursor",
	"Free look"
};

CEActions* CEActions::Instance() {		
	return _instance;		
}

String CEActions::actionName(ActionType action) {
	return actionNames[action];
}

int CEActions::size() {
	return ACTION_LAST - 1;
}

CEActions::CEActions(OSystem_WINCE3 *mainSystem, GameDetector &detector) :
	_mainSystem(mainSystem), _mapping_active(false), _right_click_needed(false),
	_hide_toolbar_needed(false) 
{
	int i;
	bool is_simon = (strncmp(detector._targetName.c_str(), "simon", 5) == 0);
	bool is_sword1 = (detector._targetName == "sword1");
	bool is_sword2 = (strcmp(detector._targetName.c_str(), "sword2") == 0);
	bool is_queen = (detector._targetName == "queen");
	bool is_sky = (detector._targetName == "sky");

	for (i=0; i<ACTION_LAST; i++)
		_action_mapping[i] = 0;

	// See if a right click mapping could be needed
	if (is_sword1 || is_sword2 || is_sky || is_queen || strncmp(detector._targetName.c_str(), "comi", 4) == 0 ||
		detector._targetName == "samnmax")
		_right_click_needed = true;

	// See if a "hide toolbar" mapping could be needed
	if (is_sword1 || is_sword2 || strncmp(detector._targetName.c_str(), "comi", 4) == 0)
		_hide_toolbar_needed = true;

	// Initialize keys for different actions
	// Pause
	_key_action[ACTION_PAUSE].setAscii(VK_SPACE);
	_action_enabled[ACTION_PAUSE] = true;
	// Save
	if (is_simon || is_sword2) 
		_action_enabled[ACTION_SAVE] = false;
	else
	if (is_queen) {
		_action_enabled[ACTION_SAVE] = true;
		_key_action[ACTION_SAVE].setAscii(282); // F1 key
	}
	else
	if (is_sky) {
		_action_enabled[ACTION_SAVE] = true;
		_key_action[ACTION_SAVE].setAscii(63); 
	}
	else {
		_action_enabled[ACTION_SAVE] = true;
		_key_action[ACTION_SAVE].setAscii(319); // F5 key
	}
	// Quit
	_action_enabled[ACTION_QUIT] = true;
	// Skip
	_action_enabled[ACTION_SKIP] = true;
	if (is_simon || is_sky || is_sword2 || is_queen || is_sword1)
		_key_action[ACTION_SKIP].setAscii(VK_ESCAPE);
	else
		_key_action[ACTION_SKIP].setAscii(Scumm::KEY_ALL_SKIP);
	// Hide
	_action_enabled[ACTION_HIDE] = true;
	// Keyboard 
	_action_enabled[ACTION_KEYBOARD] = true;
	// Sound
	_action_enabled[ACTION_SOUND] = true;
	// RightClick
	_action_enabled[ACTION_RIGHTCLICK] = true;
	// Cursor
	_action_enabled[ACTION_CURSOR] = true;
	// Freelook
	_action_enabled[ACTION_FREELOOK] = true;
}


CEActions::~CEActions() {
}

void CEActions::init(OSystem_WINCE3 *mainSystem, GameDetector &detector) {
	_instance = new CEActions(mainSystem, detector);
}

bool CEActions::perform(ActionType action) {
	switch (action) {
		case ACTION_PAUSE:
		case ACTION_SAVE:
		case ACTION_SKIP:
			KeysBuffer::Instance()->simulate(&_key_action[action]);
			return true;
		case ACTION_KEYBOARD:
			_mainSystem->swap_panel();
			return true;
		case ACTION_HIDE:
			_mainSystem->swap_panel_visibility();
			return true;
		case ACTION_SOUND:
			_mainSystem->swap_sound_master();
			return true;
		case ACTION_RIGHTCLICK:
			_mainSystem->add_right_click();
			return true;
		case ACTION_CURSOR:
			_mainSystem->swap_mouse_visibility();
			return true;
		case ACTION_QUIT:
			GUI::MessageDialog alert("Do you want to quit ?", "Yes", "No");
			if (alert.runModal() == 1)
				_mainSystem->quit();
			return true;
	}
	return false;
}

bool CEActions::isActive(ActionType action) {
	return false;
}

bool CEActions::isEnabled(ActionType action) {
	return _action_enabled[action];
}

void CEActions::beginMapping(bool start) {
	_mapping_active = start;
}

bool CEActions::mappingActive() {
	return _mapping_active;
}

bool CEActions::performMapped(unsigned int keyCode, bool pushed) {
	int i;
	
	for (i=0; i<ACTION_LAST; i++) {
		if (_action_mapping[i] == keyCode) {
			if (pushed)
				return perform((ActionType)(i + 1));
			else
				return true;
		}
	}

	return false;
}

bool CEActions::loadMapping() {
	const char *tempo;
	int version;
	int i;
	version = ConfMan.getInt("CE_mapping_version");
	if (version != ACTIONS_VERSION)
		return false;
	tempo = ConfMan.get("CE_mapping").c_str();
	if (tempo && strlen(tempo)) {
		for (i=0; i<ACTION_LAST; i++) {
			char x[6];
			int j;
			memset(x, 0, sizeof(x));
			memcpy(x, tempo + 5 * i, 4);
			sscanf(x, "%x", &j);
			_action_mapping[i] = j;
		}
		return true;
	}
	else
		return false;
}

bool CEActions::saveMapping() {
	char tempo[200];
	int i;
	tempo[0] = '\0';
	ConfMan.set("CE_mapping_version", ACTIONS_VERSION);
	for (i=0; i<ACTION_LAST; i++) {
		char x[4];
		sprintf(x, "%.4x ", _action_mapping[i]);
		strcat(tempo, x);
	}
	ConfMan.set("CE_mapping", tempo);
	ConfMan.flushToDisk();
	return true;
}

unsigned int CEActions::getMapping(ActionType action) {
	return _action_mapping[action - 1];
}


void CEActions::setMapping(ActionType action, unsigned int keyCode) {
	int i;

	for (i=0; i<ACTION_LAST; i++) {
		if (_action_mapping[i] == keyCode)
			_action_mapping[i] = 0;
	}

	_action_mapping[action - 1] = keyCode;
}

bool CEActions::needsRightClickMapping() {
	if (!_right_click_needed)
		return false;
	else
		return (_action_mapping[ACTION_RIGHTCLICK] == 0);
}

bool CEActions::needsHideToolbarMapping() {
	if (!_hide_toolbar_needed)
		return false;
	else
		return (_action_mapping[ACTION_HIDE] == 0);
}

CEActions *CEActions::_instance = NULL;    