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

#include "common/system.h"
#include "common/tokenizer.h"

#include "backends/keymapper/action.h"
#include "backends/keymapper/hardware-input.h"
#include "backends/keymapper/keymapper-defaults.h"

#define KEYMAP_KEY_PREFIX "keymap_"

namespace Common {

Keymap::Keymap(KeymapType type, const String &id, const U32String &description) :
		_type(type),
		_id(id),
		_description(description),
		_enabled(true),
		_configDomain(nullptr),
		_hardwareInputSet(nullptr),
		_backendDefaultBindings(nullptr) {

}

Keymap::Keymap(KeymapType type, const String &id, const String &description) :
		_type(type),
		_id(id),
		_description(U32String(description)),
		_enabled(true),
		_configDomain(nullptr),
		_hardwareInputSet(nullptr),
		_backendDefaultBindings(nullptr) {

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

void Keymap::registerMapping(Action *action, const HardwareInput &hwInput) {
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

	StringArray hwInputIds = getActionDefaultMappings(action);
	registerMappings(action, hwInputIds);
}

struct HardwareInputTypeIdComparator {
	bool operator()(const HardwareInput &x, const HardwareInput &y) const {
		if (x.type != y.type) {
			return x.type < y.type;
		}
		return x.id.compareTo(y.id);
	}
};

Array<HardwareInput> Keymap::getActionMapping(Action *action) const {
	Array<HardwareInput> inputs;

	for (HardwareActionMap::iterator itInput = _hwActionMap.begin(); itInput != _hwActionMap.end(); itInput++) {
		for (ActionArray::iterator itAction = itInput->_value.begin(); itAction != itInput->_value.end(); itAction++) {
			if (*itAction == action) {
				inputs.push_back(itInput->_key);
				break;
			}
		}
	}

	// Sort the inputs by type and then id for the remap dialog
	Common::sort(inputs.begin(), inputs.end(), HardwareInputTypeIdComparator());

	return inputs;
}

const Action *Keymap::findAction(const char *id) const {
	for (ActionArray::const_iterator it = _actions.begin(); it != _actions.end(); ++it) {
		if (strcmp((*it)->id, id) == 0)
			return *it;
	}

	return nullptr;
}

Keymap::KeymapMatch Keymap::getMappedActions(const Event &event, ActionArray &actions) const {
	switch (event.type) {
	case EVENT_KEYDOWN:
	case EVENT_KEYUP: {
		KeyState normalizedKeystate = KeyboardHardwareInputSet::normalizeKeyState(event.kbd);
		HardwareInput hardwareInput = HardwareInput::createKeyboard("", normalizedKeystate, U32String());
		actions.push_back(_hwActionMap[hardwareInput]);
		if (!actions.empty()) {
			return kKeymapMatchExact;
		}

		if (normalizedKeystate.flags & KBD_NON_STICKY) {
			// If no matching actions and non-sticky keyboard modifiers are down,
			// check again for matches without the exact keyboard modifiers
			for (HardwareActionMap::const_iterator itInput = _hwActionMap.begin(); itInput != _hwActionMap.end(); ++itInput) {
				if (itInput->_key.type == kHardwareInputTypeKeyboard && itInput->_key.key.keycode == normalizedKeystate.keycode) {
					int flags = itInput->_key.key.flags;
					if (flags & KBD_NON_STICKY && (flags & normalizedKeystate.flags) == flags) {
						actions.push_back(itInput->_value);
						return kKeymapMatchPartial;
					}
				}
			}

			// Lastly check again for matches no non-sticky keyboard modifiers
			normalizedKeystate.flags &= ~KBD_NON_STICKY;
			hardwareInput = HardwareInput::createKeyboard("", normalizedKeystate, U32String());
			actions.push_back(_hwActionMap[hardwareInput]);
			return actions.empty() ? kKeymapMatchNone : kKeymapMatchPartial;
		}
		break;
	}
	case EVENT_LBUTTONDOWN:
	case EVENT_LBUTTONUP: {
		HardwareInput hardwareInput = HardwareInput::createMouse("", MOUSE_BUTTON_LEFT, U32String());
		actions.push_back(_hwActionMap[hardwareInput]);
		break;
	}
	case EVENT_RBUTTONDOWN:
	case EVENT_RBUTTONUP: {
		HardwareInput hardwareInput = HardwareInput::createMouse("", MOUSE_BUTTON_RIGHT, U32String());
		actions.push_back(_hwActionMap[hardwareInput]);
		break;
	}
	case EVENT_MBUTTONDOWN:
	case EVENT_MBUTTONUP: {
		HardwareInput hardwareInput = HardwareInput::createMouse("", MOUSE_BUTTON_MIDDLE, U32String());
		actions.push_back(_hwActionMap[hardwareInput]);
		break;
	}
	case Common::EVENT_WHEELUP: {
		HardwareInput hardwareInput = HardwareInput::createMouse("", MOUSE_WHEEL_UP, U32String());
		actions.push_back(_hwActionMap[hardwareInput]);
		break;
	}
	case Common::EVENT_WHEELDOWN: {
		HardwareInput hardwareInput = HardwareInput::createMouse("", MOUSE_WHEEL_DOWN, U32String());
		actions.push_back(_hwActionMap[hardwareInput]);
		break;
	}
	case EVENT_X1BUTTONDOWN:
	case EVENT_X1BUTTONUP: {
		HardwareInput hardwareInput = HardwareInput::createMouse("", MOUSE_BUTTON_X1, U32String());
		actions.push_back(_hwActionMap[hardwareInput]);
		break;
	}
	case EVENT_X2BUTTONDOWN:
	case EVENT_X2BUTTONUP: {
		HardwareInput hardwareInput = HardwareInput::createMouse("", MOUSE_BUTTON_X2, U32String());
		actions.push_back(_hwActionMap[hardwareInput]);
		break;
	}
	case EVENT_JOYBUTTON_DOWN:
	case EVENT_JOYBUTTON_UP: {
		HardwareInput hardwareInput = HardwareInput::createJoystickButton("", event.joystick.button, U32String());
		actions.push_back(_hwActionMap[hardwareInput]);
		break;
	}
	case EVENT_JOYAXIS_MOTION: {
		if (event.joystick.position != 0) {
			bool positiveHalf = event.joystick.position >= 0;
			HardwareInput hardwareInput = HardwareInput::createJoystickHalfAxis("", event.joystick.axis, positiveHalf, U32String());
			actions.push_back(_hwActionMap[hardwareInput]);
		} else {
			// Axis position zero is part of both half axes, and triggers actions bound to both
			HardwareInput hardwareInputPos = HardwareInput::createJoystickHalfAxis("", event.joystick.axis, true, U32String());
			HardwareInput hardwareInputNeg = HardwareInput::createJoystickHalfAxis("", event.joystick.axis, false, U32String());
			actions.push_back(_hwActionMap[hardwareInputPos]);
			actions.push_back(_hwActionMap[hardwareInputNeg]);
		}
		break;
	}
	case EVENT_CUSTOM_BACKEND_HARDWARE: {
		HardwareInput hardwareInput = HardwareInput::createCustom("", event.customType, U32String());
		actions.push_back(_hwActionMap[hardwareInput]);
		break;
	}
	default:
		break;
	}

	return actions.empty() ? kKeymapMatchNone : kKeymapMatchExact;
}

void Keymap::setConfigDomain(ConfigManager::Domain *configDomain) {
	_configDomain = configDomain;
}

void Keymap::setHardwareInputs(HardwareInputSet *hardwareInputSet) {
	_hardwareInputSet = hardwareInputSet;
}

void Keymap::setBackendDefaultBindings(const KeymapperDefaultBindings *backendDefaultBindings) {
	_backendDefaultBindings = backendDefaultBindings;
}

StringArray Keymap::getActionDefaultMappings(Action *action) {
	// Backend default mappings overrides keymap default mappings, so backends can resolve mapping conflicts.
	// Empty mappings are valid and mean the action should not be mapped by default.
	if (_backendDefaultBindings) {
		KeymapperDefaultBindings::const_iterator it = _backendDefaultBindings->findDefaultBinding(_id, action->id);
		if (it != _backendDefaultBindings->end()) {
			if (it->_value.empty()) {
				return StringArray();
			}
			return StringArray(1, it->_value);
		}

		// If no keymap-specific default mapping was found, look for a standard action binding
		it = _backendDefaultBindings->findDefaultBinding(kStandardActionsKeymapName, action->id);
		if (it != _backendDefaultBindings->end()) {
			if (it->_value.empty()) {
				return StringArray();
			}
			return StringArray(1, it->_value);
		}
	}

	return action->getDefaultInputMapping();
}

void Keymap::loadMappings() {
	assert(_configDomain);
	assert(_hardwareInputSet);

	if (_actions.empty()) {
		return;
	}

	String prefix = KEYMAP_KEY_PREFIX + _id + "_";

	_hwActionMap.clear();
	for (ActionArray::const_iterator it = _actions.begin(); it != _actions.end(); ++it) {
		Action *action = *it;
		String confKey = prefix + action->id;

		StringArray hwInputIds;
		if (_configDomain->contains(confKey)) {
			// The configuration value is a list of space separated hardware input ids
			StringTokenizer hwInputTokenizer = _configDomain->getVal(confKey);

			while (!hwInputTokenizer.empty()) {
				hwInputIds.push_back(hwInputTokenizer.nextToken());
			}
		} else {
			// If the configuration key was not found, use the default mapping
			hwInputIds = getActionDefaultMappings(action);
		}

		registerMappings(action, hwInputIds);
	}
}

void Keymap::registerMappings(Action *action, const StringArray &hwInputIds) {
	assert(_hardwareInputSet);

	for (uint i = 0; i < hwInputIds.size(); i++) {
			HardwareInput hwInput = _hardwareInputSet->findHardwareInput(hwInputIds[i]);

			if (hwInput.type == kHardwareInputTypeInvalid) {
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

	String prefix = KEYMAP_KEY_PREFIX + _id + "_";

	for (ActionArray::const_iterator it = _actions.begin(); it != _actions.end(); it++) {
		Action *action = *it;
		Array<HardwareInput> mappedInputs = getActionMapping(action);

		if (areMappingsIdentical(mappedInputs, getActionDefaultMappings(action))) {
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

			confValue += mappedInputs[j].id;
		}

		_configDomain->setVal(prefix + action->id, confValue);
	}
}

bool Keymap::areMappingsIdentical(const Array<HardwareInput> &mappingsA, const StringArray &mappingsB) {
	// Assumes array values are not duplicated, but registerMapping and addDefaultInputMapping ensure that

	uint foundCount = 0;
	uint validDefaultMappings = 0;
	for (uint i = 0; i < mappingsB.size(); i++) {
		// We resolve the hardware input to make sure it is not a default for some hardware we don't have currently
		HardwareInput mappingB = _hardwareInputSet->findHardwareInput(mappingsB[i]);
		if (mappingB.type == kHardwareInputTypeInvalid) continue;
		validDefaultMappings++;

		for (uint j = 0; j < mappingsA.size(); j++) {
			if (mappingsA[j].id == mappingB.id) {
				foundCount++;
				break;
			}
		}
	}

	return foundCount == mappingsA.size() && foundCount == validDefaultMappings;
}

} // End of namespace Common
