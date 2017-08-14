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
#include "common/tokenizer.h"

#include "backends/keymapper/action.h"
#include "backends/keymapper/hardware-input.h"

#define KEYMAP_KEY_PREFIX "keymap_"

namespace Common {

Keymap::Keymap(KeymapType type, const String &name) :
		_type(type),
		_name(name),
		_configDomain(nullptr),
		_enabled(true),
		_hardwareInputSet(nullptr) {

}

Keymap::~Keymap() {
	for (ActionArray::iterator it = _actions.begin(); it != _actions.end(); ++it)
		delete *it;
}

void Keymap::addAction(Action *action) {
	if (findAction(action->id))
		error("Action with id %s already in KeyMap", action->id);

	_actions.push_back(action);
}

void Keymap::registerMapping(Action *action, const HardwareInput *hwInput) {
	ActionArray &actionArray = _hwActionMap.getVal(hwInput);

	// Don't allow an input to map to the same action multiple times
	ActionArray::const_iterator found = find(actionArray.begin(), actionArray.end(), action);
	if (found == actionArray.end()) {
		actionArray.push_back(action);
	}
}

void Keymap::unregisterMapping(Action *action) {
	// Remove the action from all the input mappings
	for (HardwareActionMap::iterator itInput = _hwActionMap.begin(); itInput != _hwActionMap.end(); itInput++) {
		for (ActionArray::iterator itAction = itInput->_value.begin(); itAction != itInput->_value.end(); itAction++) {
			if (*itAction == action) {
				itInput->_value.erase(itAction);
				break;
			}
		}
		if (itInput->_value.empty()) {
			_hwActionMap.erase(itInput);
		}
	}
}

void Keymap::resetMapping(Action *action) {
	unregisterMapping(action);

	const Array<String> &hwInputIds = action->getDefaultInputMapping();
	registerMappings(action, hwInputIds);
}

Array<const HardwareInput *> Keymap::getActionMapping(Action *action) const {
	Array<const HardwareInput *> inputs;

	for (HardwareActionMap::iterator itInput = _hwActionMap.begin(); itInput != _hwActionMap.end(); itInput++) {
		for (ActionArray::iterator itAction = itInput->_value.begin(); itAction != itInput->_value.end(); itAction++) {
			if (*itAction == action) {
				inputs.push_back(itInput->_key);
				break;
			}
		}
	}

	return inputs;
}

const Action *Keymap::findAction(const char *id) const {
	for (ActionArray::const_iterator it = _actions.begin(); it != _actions.end(); ++it) {
		if (strcmp((*it)->id, id) == 0)
			return *it;
	}

	return nullptr;
}

const Keymap::ActionArray &Keymap::getMappedActions(const HardwareInput *hardwareInput) const {
	return _hwActionMap[hardwareInput];
}

void Keymap::setConfigDomain(ConfigManager::Domain *configDomain) {
	_configDomain = configDomain;
}

void Keymap::setHardwareInputs(HardwareInputSet *hardwareInputSet) {
	_hardwareInputSet = hardwareInputSet;
}

void Keymap::loadMappings() {
	assert(_configDomain);
	assert(_hardwareInputSet);

	if (_actions.empty()) {
		return;
	}

	String prefix = KEYMAP_KEY_PREFIX + _name + "_";

	_hwActionMap.clear();
	for (ActionArray::const_iterator it = _actions.begin(); it != _actions.end(); ++it) {
		Action *action = *it;
		String confKey = prefix + action->id;

		Array<String> hwInputIds;
		if (_configDomain->contains(confKey)) {
			// The configuration value is a list of space separated hardware input ids
			StringTokenizer hwInputTokenizer = _configDomain->getVal(confKey);

			String hwInputId;
			while ((hwInputId = hwInputTokenizer.nextToken()) != "") {
				hwInputIds.push_back(hwInputId);
			}
		} else {
			// If the configuration key was not found, use the default mapping
			hwInputIds = action->getDefaultInputMapping();
		}

		registerMappings(action, hwInputIds);
	}
}

void Keymap::registerMappings(Action *action, const Array <String> &hwInputIds) {
	assert(_hardwareInputSet);

	for (uint i = 0; i < hwInputIds.size(); i++) {
			const HardwareInput *hwInput = _hardwareInputSet->findHardwareInput(hwInputIds[i].c_str());

			if (!hwInput) {
				// Silently ignore unknown hardware ids because the current device may not have inputs matching the defaults
				debug(1, "HardwareInput with ID '%s' not known", hwInputIds[i].c_str());
				continue;
			}

			// map the key
			registerMapping(action, hwInput);
		}
}

void Keymap::saveMappings() {
	if (!_configDomain)
		return;

	String prefix = KEYMAP_KEY_PREFIX + _name + "_";

	for (ActionArray::const_iterator it = _actions.begin(); it != _actions.end(); it++) {
		Action *action = *it;
		Array<const HardwareInput *> mappedInputs = getActionMapping(action);

		if (areMappingsIdentical(mappedInputs, action->getDefaultInputMapping())) {
			// If the current mapping is the default, don't write anything to the config manager
			_configDomain->erase(prefix + action->id);
			continue;
		}

		// The configuration value is a list of space separated hardware input ids
		String confValue;
		for (uint j = 0; j < mappedInputs.size(); j++) {
			if (!confValue.empty()) {
				confValue += " ";
			}

			confValue += mappedInputs[j]->id;
		}

		_configDomain->setVal(prefix + action->id, confValue);
	}
}

bool Keymap::areMappingsIdentical(const Array<const HardwareInput *> &inputs, const Array<String> &mapping) {
	if (inputs.size() != mapping.size()) {
		return false;
	}

	// Assumes array values are not duplicated, but registerMapping and addDefaultInputMapping ensure that

	uint foundCount = 0;
	for (uint i = 0; i < inputs.size(); i++) {
		for (uint j = 0; j < mapping.size(); j++) {
			if (inputs[i]->id == mapping[j]) {
				foundCount++;
				break;
			}
		}
	}

	return foundCount == inputs.size();
}

} // End of namespace Common

#endif // #ifdef ENABLE_KEYMAPPER
