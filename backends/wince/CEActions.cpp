/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2005 The ScummVM project
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

#include "common/stdafx.h"
#include "CEActions.h"
#include "CEActionsPocket.h"
#include "CEActionsSmartphone.h"

#include "gui/message.h"

#include "scumm/scumm.h"

#include "common/config-manager.h"


CEActions* CEActions::Instance() {		
	return _instance;		
}

CEActions::CEActions(GameDetector &detector) :
	_detector(&detector), _mapping_active(false), _initialized(false)
{
}


CEActions::~CEActions() {
}

void CEActions::init(GameDetector &detector) {
	if (!CEDevice::hasSmartphoneResolution())
		CEActionsPocket::init(detector);
//#ifdef WIN32_PLATFORM_WFSP
	else
		CEActionsSmartphone::init(detector);
//#endif
}

void CEActions::initInstanceMain(OSystem_WINCE3 *mainSystem) {
	_mainSystem = mainSystem;
}

void CEActions::initInstanceGame() {
	_instance->_initialized = true;
}


bool CEActions::initialized() {
	return _initialized;
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
	
	for (i=0; i<size(); i++) {
		if (_action_mapping[i] == keyCode && _action_enabled[i]) 
				return perform((ActionType)i, pushed);
	}

	return false;
}

bool CEActions::loadMapping() {
	const char *tempo;
	int current_version;
	int i;
	current_version = ConfMan.getInt("action_mapping_version", domain());
	if (current_version != version())
		return false;
	tempo = ConfMan.get("action_mapping", domain()).c_str();
	if (tempo && strlen(tempo)) {
		for (i=0; i<size(); i++) {
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
	ConfMan.set("action_mapping_version", version(), domain());
	for (i=0; i<size(); i++) {
		char x[4];
		sprintf(x, "%.4x ", _action_mapping[i]);
		strcat(tempo, x);
	}
	ConfMan.set("action_mapping", tempo, domain());
	ConfMan.flushToDisk();
	return true;
}

unsigned int CEActions::getMapping(ActionType action) {
	return _action_mapping[action];
}


void CEActions::setMapping(ActionType action, unsigned int keyCode) {
	int i;

	for (i=0; i<size(); i++) {
		if (_action_mapping[i] == keyCode)
			_action_mapping[i] = 0;
	}

	_action_mapping[action] = keyCode;
}


CEActions *CEActions::_instance = NULL;    