#ifdef _WIN32_WCE

// Handle mapping of actions to hardware keys

#include "stdafx.h"
#include <assert.h>
#include <Winuser.h>
#include <sipapi.h>
#include <Aygshell.h>
#include <gx.h>

#include "gapi_keys.h"

#include "screen.h"

struct oneAction _actions[NUMBER_ACTIONS];
struct GXKeyList _keys;
pAction *_action_functions;

const char* ActionsText[] = {
	"Pause",
	"Save",
	"Quit",
	"Skip",
	"Hide",
	"Keyboard",
	"Sound",
	"Right click"
};

bool _typeExists(int x) {
	int i;

	for (i=0; i<NUMBER_ACTIONS; i++)
		if (_actions[i].action_type == x)
			return true;

	return false;
}


const char* getActionName(int action) {
	return ActionsText[action - 1];
}

void GAPIKeysInit(pAction *functions) {
	int i;
	GXOpenInput();
	for (i=0; i<NUMBER_ACTIONS; i++) {
		_actions[i].action_key = 0;
	}

	/* Default actions */

	_actions[0].action_type = ACTION_PAUSE;
	_actions[1].action_type = ACTION_SAVE;
	_actions[2].action_type = ACTION_QUIT;
	_actions[3].action_type = ACTION_SKIP;
	_actions[4].action_type = ACTION_HIDE;

	_action_functions = functions;
	GAPIKeysGetReference();
}

void GAPIKeysGetReference() {
	if(GetScreenMode()) {
		_keys = GXGetDefaultKeys(GX_LANDSCAPEKEYS);
	} else {
		_keys = GXGetDefaultKeys(GX_NORMALKEYS);
	}
}

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

const char* getGAPIKeyName(unsigned char key) {
	switch(key) {
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
		default:
			return "Not mapped";
	}
}

struct oneAction* getAction(int action) {
	return &_actions[action];
}

void processAction (short key) {
	int i;
	unsigned char GAPI_key;

	GAPI_key = getGAPIKeyMapping(key);
	if (!GAPI_key)
		return;
	
	for (i=0; i<NUMBER_ACTIONS; i++) 
		if (_actions[i].action_key == GAPI_key &&
			_actions[i].action_type != ACTION_NONE &&
			_action_functions[_actions[i].action_type - 1])
					_action_functions[_actions[i].action_type - 1]();
}

void clearActionKey (unsigned char key) {
	int i;

	for (i=0; i<NUMBER_ACTIONS; i++)
		if (_actions[i].action_key == key) {
			_actions[i].action_key = 0;
		}
}

const unsigned char* getActionKeys() {
	int i;
	static unsigned char actionKeys[NUMBER_ACTIONS];

	for (i=0; i<NUMBER_ACTIONS; i++)
		actionKeys[i] = _actions[i].action_key;

	return actionKeys;
}

const unsigned char* getActionTypes() {
	int i;
	static unsigned char actionTypes[NUMBER_ACTIONS];

	for (i=0; i<NUMBER_ACTIONS; i++)
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



void setActionKeys(unsigned char *actionKeys) {
	int i;

	for (i=0; i<NUMBER_ACTIONS; i++)
		_actions[i].action_key = actionKeys[i];
}

void setActionTypes(unsigned char *actionTypes) {
	int i;

	for (i=0; i<NUMBER_ACTIONS; i++)
		_actions[i].action_type = (ActionType)actionTypes[i];
}


#endif