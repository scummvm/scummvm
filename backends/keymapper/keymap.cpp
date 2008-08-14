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

void Keymap::addAction(Action *action) {
	if (findAction(action->id))
		error("Action with id %d already in KeyMap!", action->id);
	action->setParent(this);
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

Action *Keymap::getAction(int32 id) {
	return findAction(id);
}

Action *Keymap::findAction(int32 id) {
	List<Action*>::iterator it;
	for (it = _actions.begin(); it != _actions.end(); it++) {
		if ((*it)->id == id)
			return *it;
	}
	return 0;
}

const Action *Keymap::findAction(int32 id) const {
	List<Action*>::const_iterator it;
	for (it = _actions.begin(); it != _actions.end(); it++) {
		if ((*it)->id == id)
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
	String prefix = "km_" + _name + "_";
	for (it = _configDomain->begin(); it != _configDomain->end(); it++) {
		const String& key = it->_key;
		if (!key.hasPrefix(prefix.c_str()))
			continue;

		// parse Action ID
		const char *actionIdStart = key.c_str() + prefix.size();
		char *err;
		int32 actionId = (int32) strtol(actionIdStart, &err, 0);
		if (err == actionIdStart) {
			warning("'%s' is not a valid Action ID", err);
			continue;
		}
		Action *ua = getAction(actionId);
		if (!ua) {
			warning("'%s' keymap does not contain Action with ID %d", 
				_name.c_str(), (int)actionId);
			_configDomain->erase(key);
			continue;
		}

		// parse HardwareKey ID
		int32 hwKeyId = (int32) strtol(it->_value.c_str(), &err, 0);
		if (err == it->_value.c_str()) {
			warning("'%s' is not a valid HardwareKey ID", err);
			continue;
		}
		const HardwareKey *hwKey = hwKeys->findHardwareKey(hwKeyId);
		if (!hwKey) {
			warning("HardwareKey with ID %d not known", (int)hwKeyId);
			_configDomain->erase(key);
			continue;
		}

		ua->mapKey(hwKey);
	}
}

void Keymap::saveMappings() {
	if (!_configDomain) return;
	List<Action*>::const_iterator it;
	char buf[12];
	String prefix = "km_" + _name + "_";
	for (it = _actions.begin(); it != _actions.end(); it++) {
		sprintf(buf, "%d", (*it)->id);
		String key = prefix + buf;
		if ((*it)->getMappedKey())
			sprintf(buf, "%d", (*it)->getMappedKey()->id);
		else
			strcpy(buf, "");
		_configDomain->setVal(key, buf);
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
