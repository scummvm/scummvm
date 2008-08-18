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
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*
* $URL$
* $Id$
*
*/

#include "backends/keymapper/keymap.h"
#include "backends/keymapper/hardware-key.h"

#define KEYMAP_KEY_PREFIX "keymap_"

namespace Common {

Keymap::Keymap(const Keymap& km) : _actions(km._actions), _keymap(), _configDomain(0) {
	List<Action*>::iterator it;
	for (it = _actions.begin(); it != _actions.end(); it++) {
		const HardwareKey *hwKey = (*it)->getMappedKey();
		if (hwKey) {
			_keymap[hwKey->key] = *it;
		}
	}
}

Keymap::~Keymap() {
	List<Action*>::iterator it;
	for (it = _actions.begin(); it != _actions.end(); it++)
		delete *it;
}

void Keymap::addAction(Action *action) {
	if (findAction(action->id))
		error("Action with id %d already in KeyMap!", action->id);
	_actions.push_back(action);
}

void Keymap::registerMapping(Action *action, const HardwareKey *hwKey) {
	HashMap<KeyState, Action*>::iterator it;
	it = _keymap.find(hwKey->key);
	// if key is already mapped to a different action then un-map it
	if (it != _keymap.end() && action != it->_value) {
		it->_value->mapKey(0);
	}

	_keymap[hwKey->key] = action;
}

void Keymap::unregisterMapping(Action *action) {
	const HardwareKey *hwKey = action->getMappedKey();
	if (hwKey) {
		_keymap.erase(hwKey->key);
	}
}

Action *Keymap::getAction(const char *id) {
	return findAction(id);
}

Action *Keymap::findAction(const char *id) {
	List<Action*>::iterator it;
	for (it = _actions.begin(); it != _actions.end(); it++) {
		if (strncmp((*it)->id, id, ACTION_ID_SIZE) == 0)
			return *it;
	}
	return 0;
}

const Action *Keymap::findAction(const char *id) const {
	List<Action*>::const_iterator it;
	for (it = _actions.begin(); it != _actions.end(); it++) {
		if (strncmp((*it)->id, id, ACTION_ID_SIZE) == 0)
			return *it;
	}
	return 0;
}

Action *Keymap::getMappedAction(const KeyState& ks) const {
	HashMap<KeyState, Action*>::iterator it;
	it = _keymap.find(ks);
	if (it == _keymap.end())
		return 0;
	else
		return it->_value;
}

void Keymap::setConfigDomain(ConfigManager::Domain *dom) {
	_configDomain = dom;
}

void Keymap::loadMappings(const HardwareKeySet *hwKeys) {
	if (!_configDomain) return;
	ConfigManager::Domain::iterator it;	
	String prefix = KEYMAP_KEY_PREFIX + _name + "_";
	for (it = _configDomain->begin(); it != _configDomain->end(); it++) {
		const String& key = it->_key;
		if (!key.hasPrefix(prefix.c_str()))
			continue;

		// parse Action ID
		const char *actionId = key.c_str() + prefix.size();
		Action *ua = getAction(actionId);
		if (!ua) {
			warning("'%s' keymap does not contain Action with ID %s", 
				_name.c_str(), actionId);
			_configDomain->erase(key);
			continue;
		}

		const HardwareKey *hwKey = hwKeys->findHardwareKey(it->_value.c_str());
		if (!hwKey) {
			warning("HardwareKey with ID %s not known", it->_value.c_str());
			_configDomain->erase(key);
			continue;
		}

		ua->mapKey(hwKey);
	}
}

void Keymap::saveMappings() {
	if (!_configDomain) return;
	List<Action*>::const_iterator it;
	String prefix = KEYMAP_KEY_PREFIX + _name + "_";
	for (it = _actions.begin(); it != _actions.end(); it++) {
		uint actIdLen = strnlen((*it)->id, ACTION_ID_SIZE);
		String actId((*it)->id, (*it)->id + actIdLen);
		if ((*it)->getMappedKey()) {
			uint hwIdLen = strnlen((*it)->getMappedKey()->id, HWKEY_ID_SIZE);
			String hwId((*it)->getMappedKey()->id, (*it)->getMappedKey()->id + hwIdLen);
			_configDomain->setVal(prefix + actId, hwId);
		} else {
			_configDomain->setVal(prefix + actId, "");
		}
	}
}

bool Keymap::isComplete(const HardwareKeySet *hwKeys) {
	List<Action*>::iterator it;
	bool allMapped = true;
	uint numberMapped = 0;
	for (it = _actions.begin(); it != _actions.end(); it++) {
		if ((*it)->getMappedKey()) {
			numberMapped++;
		} else {
			allMapped = false;
		}
	}
	return allMapped || (numberMapped == hwKeys->count());
}

} // end of namespace Common
