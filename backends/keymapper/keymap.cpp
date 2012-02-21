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
*/

#include "backends/keymapper/keymap.h"

#ifdef ENABLE_KEYMAPPER

#include "common/system.h"

#include "backends/keymapper/hardware-key.h"
#include "backends/keymapper/keymapper-defaults.h"

#define KEYMAP_KEY_PREFIX "keymap_"

namespace Common {

Keymap::Keymap(const Keymap& km) : _actions(km._actions), _keymap(), _configDomain(0) {
	List<Action *>::iterator it;

	for (it = _actions.begin(); it != _actions.end(); ++it) {
		const HardwareKey *hwKey = (*it)->getMappedKey();

		if (hwKey) {
			_keymap[hwKey->key] = *it;
		}
	}
}

Keymap::~Keymap() {
	List<Action *>::iterator it;

	for (it = _actions.begin(); it != _actions.end(); ++it)
		delete *it;
}

void Keymap::addAction(Action *action) {
	if (findAction(action->id))
		error("Action with id %s already in KeyMap", action->id);

	_actions.push_back(action);
}

void Keymap::registerMapping(Action *action, const HardwareKey *hwKey) {
	HashMap<KeyState, Action *>::iterator it;

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
	List<Action *>::iterator it;

	for (it = _actions.begin(); it != _actions.end(); ++it) {
		if (strncmp((*it)->id, id, ACTION_ID_SIZE) == 0)
			return *it;
	}
	return 0;
}

const Action *Keymap::findAction(const char *id) const {
	List<Action *>::const_iterator it;

	for (it = _actions.begin(); it != _actions.end(); ++it) {
		if (strncmp((*it)->id, id, ACTION_ID_SIZE) == 0)
			return *it;
	}

	return 0;
}

Action *Keymap::getMappedAction(const KeyState& ks) const {
	HashMap<KeyState, Action *>::iterator it;

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
	if (!_configDomain)
		return;

	if (_actions.empty())
		return;

	Common::KeymapperDefaultBindings *defaults = g_system->getKeymapperDefaultBindings();

	HashMap<String, const HardwareKey *> mappedKeys;
	List<Action*>::iterator it;
	String prefix = KEYMAP_KEY_PREFIX + _name + "_";

	for (it = _actions.begin(); it != _actions.end(); ++it) {
		Action* ua = *it;
		String actionId(ua->id);
		String confKey = prefix + actionId;

		String hwKeyId = _configDomain->getVal(confKey);

		bool defaulted = false;
		// fall back to the platform-specific defaults
		if (hwKeyId.empty() && defaults) {
			hwKeyId = defaults->getDefaultBinding(_name, actionId);
			if (!hwKeyId.empty())
				defaulted = true;
		}
		// there's no mapping
		if (hwKeyId.empty())
			continue;

		const HardwareKey *hwKey = hwKeys->findHardwareKey(hwKeyId.c_str());

		if (!hwKey) {
			warning("HardwareKey with ID '%s' not known", hwKeyId.c_str());
			continue;
		}

		if (defaulted) {
			if (mappedKeys.contains(hwKeyId)) {
				debug(1, "Action [%s] not falling back to hardcoded default value [%s] because the key is in use", confKey.c_str(), hwKeyId.c_str());
				continue;
			}
			warning("Action [%s] fell back to hardcoded default value [%s]", confKey.c_str(), hwKeyId.c_str());
		}

		mappedKeys.setVal(hwKeyId, hwKey);
		// map the key
		ua->mapKey(hwKey);
	}
}

void Keymap::saveMappings() {
	if (!_configDomain)
		return;

	List<Action *>::const_iterator it;
	String prefix = KEYMAP_KEY_PREFIX + _name + "_";

	for (it = _actions.begin(); it != _actions.end(); ++it) {
		uint actIdLen = strlen((*it)->id);

		actIdLen = (actIdLen > ACTION_ID_SIZE) ? ACTION_ID_SIZE : actIdLen;

		String actId((*it)->id, (*it)->id + actIdLen);
		char hwId[HWKEY_ID_SIZE+1];

		memset(hwId, 0, HWKEY_ID_SIZE+1);

		if ((*it)->getMappedKey()) {
			memcpy(hwId, (*it)->getMappedKey()->hwKeyId, HWKEY_ID_SIZE);
		}
		_configDomain->setVal(prefix + actId, hwId);
	}
}

bool Keymap::isComplete(const HardwareKeySet *hwKeys) {
	List<Action *>::iterator it;
	bool allMapped = true;
	uint numberMapped = 0;

	for (it = _actions.begin(); it != _actions.end(); ++it) {
		if ((*it)->getMappedKey()) {
			numberMapped++;
		} else {
			allMapped = false;
		}
	}

	return allMapped || (numberMapped == hwKeys->size());
}

} // End of namespace Common

#endif // #ifdef ENABLE_KEYMAPPER
