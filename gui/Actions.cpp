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
 * $Header$
 *
 */

#include "stdafx.h"
#include "gui/Actions.h"
#include "gui/message.h"
#include "scumm/scumm.h"
#include "common/config-manager.h"

#ifdef _WIN32_WCE
	#include "backends/wince/CEActionsPocket.h"
	#include "backends/wince/CEActionsSmartphone.h"
#elif defined(__SYMBIAN32__)
	#include "backends/epoc/src/SymbianActions.h"
#endif

namespace GUI {

Actions* Actions::Instance() {
	return _instance;
}

Actions::Actions(GameDetector &detector) :
	_detector(&detector), _mapping_active(false), _initialized(false)
{
}


Actions::~Actions() {
}

// call the correct object creator function according to the Factory Pattern
void Actions::init(GameDetector &detector) {
#ifdef _WIN32_WCE
	// For WinCE: now use software + Factory pattern to create correct objects
	if (!CEDevice::isSmartphone())
		CEActionsPocket::init(detector);
	else
		CEActionsSmartphone::init(detector);
#elif defined(__SYMBIAN32__)
	SymbianActions::init(detector);
#endif
}

void Actions::initInstanceMain(OSystem *mainSystem) {
	_mainSystem = mainSystem;
}

void Actions::initInstanceGame() {
	_instance->_initialized = true;
}


bool Actions::initialized() {
	return _initialized;
}

bool Actions::isActive(ActionType action) {
	return false;
}

bool Actions::isEnabled(ActionType action) {
	return _action_enabled[action];
}

void Actions::beginMapping(bool start) {
	_mapping_active = start;
}

bool Actions::mappingActive() {
	return _mapping_active;
}

bool Actions::performMapped(unsigned int keyCode, bool pushed) {
	int i;

	for (i=0; i<size(); i++) {
		if (_action_mapping[i] == keyCode && _action_enabled[i])
				return perform((ActionType)i, pushed);
	}

	return false;
}

bool Actions::loadMapping() {
	const char *tempo;
	int current_version;
	int i;
	current_version = ConfMan.getInt("action_mapping_version", domain());
	if (current_version != version())
		return false;
	tempo = ConfMan.get("action_mapping", domain()).c_str();
	if (tempo && strlen(tempo)) {
		for (i=0; i<size(); i++) {
			char x[7];
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

bool Actions::saveMapping() {
	char tempo[200];
	int i;
	tempo[0] = '\0';
	ConfMan.set("action_mapping_version", version(), domain());
	for (i=0; i<size(); i++) {
		char x[10];
		sprintf(x, "%.4x ", _action_mapping[i]);
		strcat(tempo, x);
	}
	ConfMan.set("action_mapping", tempo, domain());
	ConfMan.flushToDisk();
	return true;
}

unsigned int Actions::getMapping(ActionType action) {
	return _action_mapping[action];
}


void Actions::setMapping(ActionType action, unsigned int keyCode) {
	int i;

	for (i=0; i<size(); i++) {
		if (_action_mapping[i] == keyCode)
			_action_mapping[i] = 0;
	}

	_action_mapping[action] = keyCode;
}

Key& Actions::getKeyAction(ActionType action)
{
	return _key_action[action];
}

// Game detector
GameDetector& Actions::gameDetector(){
	return *_detector;
}
Actions *Actions::_instance = NULL;


} // namespace GUI
