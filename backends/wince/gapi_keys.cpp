/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001/2002 The ScummVM project
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

#ifdef _WIN32_WCE

// Handle mapping of actions to hardware keys

#include "stdafx.h"
#include <assert.h>

#if _WIN32_WCE < 300

#include <Wingdi.h>
#include <Winbase.h>
#include <Wtypes.h>

#endif

#include <Winuser.h>
#include <sipapi.h>
//#include <Aygshell.h>
#include <gx.h>
#include "dynamic_imports.h"

#include "gapi_keys.h"

#include "screen.h"

struct oneAction _actions[TOTAL_ACTIONS];
struct GXKeyList _portrait_keys;
struct GXKeyList _landscape_keys;
pAction *_action_functions;

void handleSelectGameUp(void);
void handleSelectGameDown(void);
void handleSelectGameButton(void);

const char* ActionsText[] = {
	"None",
	"Pause",
	"Save",
	"Quit",
	"Skip",
	"Hide",
	"Keyboard",
	"Sound",
	"Right click",
	"Cursor on/off",
	"Subtitles on/off",
	"Boss",
	"Free look"
};

bool _typeExists(unsigned int x) {
	int i;

	for (i=0; i<TOTAL_ACTIONS; i++)
		if (_actions[i].action_type == x)
			return true;

	return false;
}


const char* getActionName(int action) {
	return ActionsText[action];
}

void GAPIKeysInit() {
	dynamicGXOpenInput();
	GAPIKeysGetReference();
}

void GAPIKeysInitActions(pAction *functions) {
	int i;

	for (i=0; i<TOTAL_ACTIONS; i++) {
		_actions[i].action_key = 0;
	}

	/* Default actions */

	/*
	_actions[0].action_type = ACTION_PAUSE;
	_actions[1].action_type = ACTION_SAVE;
	_actions[2].action_type = ACTION_QUIT;
	_actions[3].action_type = ACTION_SKIP;
	_actions[4].action_type = ACTION_HIDE;
	*/

	_action_functions = functions;
	
}

void GAPIKeysGetReference() {
	/*
	if(GetScreenMode()) {
		_keys = GXGetDefaultKeys(GX_LANDSCAPEKEYS);
	} else {
		_keys = GXGetDefaultKeys(GX_NORMALKEYS);
	}
	*/

	_portrait_keys = dynamicGXGetDefaultKeys(GX_NORMALKEYS);
	_landscape_keys = dynamicGXGetDefaultKeys(GX_LANDSCAPEKEYS);
}

void GAPIKeysHandleSelect(int key) {
	if (key == _portrait_keys.vkUp)
		handleSelectGameUp();
	if (key == _portrait_keys.vkDown)
		handleSelectGameDown();
	if (key == _portrait_keys.vkA ||
		key == _portrait_keys.vkB ||
		key == _portrait_keys.vkC ||
		key == _portrait_keys.vkStart)
		handleSelectGameButton();
}

unsigned int GAPIKeysTranslate(unsigned int key) {
/*
	if (key == _landscape_keys.vkUp)
		return _portrait_keys.vkUp;
	if (key == _landscape_keys.vkDown)
		return _portrait_keys.vkDown;
	if (key == _landscape_keys.vkLeft)
		return _portrait_keys.vkLeft;
	if (key == _landscape_keys.vkRight)
		return _portrait_keys.vkRight;
	if (key == _landscape_keys.vkA)
		return _portrait_keys.vkA;
	if (key == _landscape_keys.vkB)
		return _portrait_keys.vkB;
	if (key == _landscape_keys.vkC)
		return _portrait_keys.vkC;
	if (key == _landscape_keys.vkStart)
		return _portrait_keys.vkStart;
*/
	return key;
}

/*
const unsigned char getGAPIKeyMapping(short key) {
	// first the standard GAPI controls
	if (key == _keys.vkA)
		return GAPI_KEY_VKA;
	
	if (key == _keys.vkB)
			return GAPI_KEY_VKB;
	
	if (key == _keys.vkC)
			return GAPI_KEY_VKC;
	
	if (key == _keys.vkStart)
			return GAPI_KEY_VKSTART;

	if (key == _keys.vkUp) 
			return GAPI_KEY_VKUP;

	if (key == _keys.vkDown)
			return GAPI_KEY_VKDOWN;

	if (key == _keys.vkLeft)
			return GAPI_KEY_VKLEFT;

	if (key == _keys.vkRight)
			return GAPI_KEY_VKRIGHT;

	switch (key) {
		// then the "unsupported" keys 
		case INTERNAL_KEY_CALENDAR:
			return GAPI_KEY_CALENDAR;
		case INTERNAL_KEY_CONTACTS:
			return GAPI_KEY_CONTACTS;
		case INTERNAL_KEY_INBOX:
			return GAPI_KEY_INBOX;
		case INTERNAL_KEY_ITASK:
			return GAPI_KEY_ITASK;
		default:
			return 0;
	}
}
*/

const char* getGAPIKeyName(unsigned int key) {
	static char key_name[50];

	if (!key)
		return "Not mapped";
	if (key == (unsigned int)_portrait_keys.vkA)
		return "Button A";
	if (key == (unsigned int)_portrait_keys.vkB)
		return "Button B";
	if (key == (unsigned int)_portrait_keys.vkC)
		return "Button C";
	if (key == (unsigned int)_portrait_keys.vkStart)
		return "Button Start";
	if (key == (unsigned int)_portrait_keys.vkUp)
		return "Pad Up";
	if (key == (unsigned int)_portrait_keys.vkDown)
		return "Pad Down";
	if (key == (unsigned int)_portrait_keys.vkLeft)
		return "Pad Left";
	if (key == (unsigned int)_portrait_keys.vkRight)
		return "Pad Right";
	if (key == INTERNAL_KEY_CALENDAR)
		return "Button Calendar";
	if (key == INTERNAL_KEY_CONTACTS)
		return "Button Contacts";
	if (key == INTERNAL_KEY_INBOX)
		return "Button Inbox";
	if (key == INTERNAL_KEY_ITASK)
		return "Button ITask";
	sprintf(key_name, "Key %.4x", key);
		return key_name;

/*
	switch(key) {
		case 0:
			return "Not mapped";
		case GAPI_KEY_VKA:
			return "Button A";
		case GAPI_KEY_VKB:
			return "Button B";
		case GAPI_KEY_VKC:
			return "Button C";
		case GAPI_KEY_VKSTART:
			return "Button Start";
		case GAPI_KEY_CALENDAR:
			return "Button Calendar";
		case GAPI_KEY_CONTACTS:
			return "Button Contacts";
		case GAPI_KEY_INBOX:
			return "Button Inbox";
		case GAPI_KEY_ITASK:
			return "Button ITask";
		case GAPI_KEY_VKUP:
			return "Pad Up";
		case GAPI_KEY_VKDOWN:
			return "Pad Down";
		case GAPI_KEY_VKLEFT:
			return "Pad Left";
		case GAPI_KEY_VKRIGHT:
			return "Pad Right";
		default:
			sprintf(key_name, "Unknown key %.4x", key);
			return key_name;
	}
*/
}

struct oneAction* getAction(int action) {
	return &_actions[action];
}

bool processAction (unsigned int key) {
	int i;
	/*
	unsigned char GAPI_key;

	GAPI_key = getGAPIKeyMapping(key);
	if (!GAPI_key)
		return;
	*/

	for (i=0; i<TOTAL_ACTIONS; i++) {
		//if (_actions[i].action_key == GAPI_key &&
		/*
		if (_actions[i].action_key == key &&
			_actions[i].action_type != ACTION_NONE &&
			_action_functions[_actions[i].action_type - 1]) {
					_action_functions[_actions[i].action_type - 1]();
					return true;
		}
		*/
		
		if (_actions[i].action_key && _actions[i].action_key == key) {
			_action_functions[i]();
			return true;
		}
	}


	return false;
}

void clearActionKey (unsigned char key) {
	int i;

	for (i=0; i<TOTAL_ACTIONS; i++)
		if (_actions[i].action_key == key) {
			_actions[i].action_key = 0;
		}
}

const unsigned int* getActionKeys() {
	int i;
	static unsigned int actionKeys[TOTAL_ACTIONS];

	for (i=0; i<TOTAL_ACTIONS; i++)
		actionKeys[i] = _actions[i].action_key;

	return actionKeys;
}

const unsigned char* getActionTypes() {
	int i;
	static unsigned char actionTypes[TOTAL_ACTIONS];

	for (i=0; i<TOTAL_ACTIONS; i++)
		actionTypes[i] = _actions[i].action_type;

	return actionTypes;

}


void setNextType(int action) {
	int start = _actions[action].action_type;
	int current = start;
	for (;;) {
		current++;
		if (current == start)
			return;
		if (current > TOTAL_ACTIONS)
			current = 1;
		if (!_typeExists(current)) {
				_actions[action].action_type = current;
				return;
		}
	}
}

void setPreviousType(int action) {
	int start = _actions[action].action_type;
	int current = start;
	for (;;) {
		current--;
		if (current == start)
			return;
		if (current <= 0)
			current = TOTAL_ACTIONS;
		if (!_typeExists(current)) {
				_actions[action].action_type = current;
				return;
		}
	}
}



void setActionKeys(unsigned int *actionKeys) {
	int i;

	for (i=0; i<TOTAL_ACTIONS; i++)
		_actions[i].action_key = actionKeys[i];
}

void setActionTypes(unsigned char *actionTypes) {
	int i;

	for (i=0; i<TOTAL_ACTIONS; i++)
		_actions[i].action_type = (ActionType)actionTypes[i];
}


#endif