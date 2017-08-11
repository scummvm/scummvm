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

#include "backends/keymapper/action.h"
#include "backends/keymapper/hardware-input.h"

#define KEYMAP_KEY_PREFIX "keymap_"

namespace Common {

Keymap::Keymap(const Keymap& km) : _actions(km._actions), _hwActionMap(), _configDomain(0) {
}

Keymap::~Keymap() {
	for (ActionList::iterator it = _actions.begin(); it != _actions.end(); ++it)
		delete *it;
}

void Keymap::addAction(Action *action) {
	if (findAction(action->id))
		error("Action with id %s already in KeyMap", action->id);

	_actions.push_back(action);
}

void Keymap::registerMapping(Action *action, const HardwareInput *hwInput) {
	unregisterMapping(action);

	_hwActionMap[hwInput] = action;
}

void Keymap::unregisterMapping(Action *action) {
	for (HardwareActionMap::iterator it = _hwActionMap.begin(); it != _hwActionMap.end(); it++) {
		if (it->_value == action) {
			_hwActionMap.erase(it);
		}
	}
}

const HardwareInput *Keymap::getActionMapping(Action *action) const {
	for (HardwareActionMap::const_iterator it = _hwActionMap.begin(); it != _hwActionMap.end(); it++) {
		if (it->_value == action) {
			return it->_key;
		}
	}

	return nullptr;
}

const Action *Keymap::findAction(const char *id) const {
	for (ActionList::const_iterator it = _actions.begin(); it != _actions.end(); ++it) {
		if (strncmp((*it)->id, id, ACTION_ID_SIZE) == 0)
			return *it;
	}

	return nullptr;
}

Action *Keymap::getMappedAction(const HardwareInput *hardwareInput) const {
	return _hwActionMap[hardwareInput];
}

void Keymap::setConfigDomain(ConfigManager::Domain *dom) {
	_configDomain = dom;
}

void Keymap::loadMappings(const HardwareInputSet *hwKeys) {
	if (!_configDomain)
		return;

	if (_actions.empty())
		return;

	String prefix = KEYMAP_KEY_PREFIX + _name + "_";

	for (ActionList::iterator it = _actions.begin(); it != _actions.end(); ++it) {
		Action* ua = *it;
		String actionId(ua->id);
		String confKey = prefix + actionId;

		String hwInputId = _configDomain->getVal(confKey);

		// there's no mapping
		if (hwInputId.empty())
			continue;

		const HardwareInput *hwInput = hwKeys->findHardwareInput(hwInputId.c_str());

		if (!hwInput) {
			warning("HardwareInput with ID '%s' not known", hwInputId.c_str());
			continue;
		}

		// map the key
		_hwActionMap[hwInput] = ua;
	}
}

void Keymap::saveMappings() {
	if (!_configDomain)
		return;

	String prefix = KEYMAP_KEY_PREFIX + _name + "_";

	for (HardwareActionMap::iterator it = _hwActionMap.begin(); it != _hwActionMap.end(); it++) {
		const Action *action = it->_value;
		const HardwareInput *input = it->_key;

		uint actIdLen = strlen(action->id);

		actIdLen = (actIdLen > ACTION_ID_SIZE) ? ACTION_ID_SIZE : actIdLen;

		String actId(action->id, action->id + actIdLen);

		_configDomain->setVal(prefix + actId, input->id);
	}
}

} // End of namespace Common

#endif // #ifdef ENABLE_KEYMAPPER
