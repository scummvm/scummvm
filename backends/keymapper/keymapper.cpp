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

#include "backends/keymapper/keymapper.h"

#ifdef ENABLE_KEYMAPPER

#include "backends/keymapper/action.h"
#include "backends/keymapper/hardware-input.h"

#include "common/system.h"

namespace Common {

// These magic numbers are provided by fuzzie and WebOS
static const uint32 kDelayKeyboardEventMillis = 250;
static const uint32 kDelayMouseEventMillis = 50;

Keymapper::Keymapper(EventManager *evtMgr) :
		_eventMan(evtMgr),
		_enabled(true),
		_enabledKeymapType(Keymap::kKeymapTypeGlobal),
		_hardwareInputs(nullptr),
		_backendDefaultBindings(nullptr) {
}

Keymapper::~Keymapper() {
	delete _hardwareInputs;
	for (KeymapArray::iterator it = _keymaps.begin(); it != _keymaps.end(); it++) {
		delete *it;
	}
}

void Keymapper::registerHardwareInputSet(HardwareInputSet *inputs) {
	if (_hardwareInputs)
		error("Hardware input set already registered");

	if (!inputs) {
		warning("No hardware input were defined, using defaults");
		inputs = new HardwareInputSet(true);
	}

	_hardwareInputs = inputs;
}

void Keymapper::registerBackendDefaultBindings(const Common::KeymapperDefaultBindings *backendDefaultBindings) {
	if (!_keymaps.empty())
		error("Backend default bindings must be defined before adding keymaps");

	_backendDefaultBindings = backendDefaultBindings;
}

void Keymapper::addGlobalKeymap(Keymap *keymap) {
	assert(keymap->getType() == Keymap::kKeymapTypeGlobal
	       || keymap->getType() == Keymap::kKeymapTypeGui);

	ConfigManager::Domain *keymapperDomain = ConfMan.getDomain(ConfigManager::kKeymapperDomain);
	initKeymap(keymap, keymapperDomain);

	// Global keymaps have the lowest priority, they need to be first in the array
	_keymaps.insert_at(0, keymap);
}

void Keymapper::addGameKeymap(Keymap *keymap) {
	assert(keymap->getType() == Keymap::kKeymapTypeGame);

	ConfigManager::Domain *gameDomain = ConfMan.getActiveDomain();

	if (!gameDomain) {
		error("Call to Keymapper::addGameKeymap when no game loaded");
	}

	initKeymap(keymap, gameDomain);
	_keymaps.push_back(keymap);
}

void Keymapper::initKeymap(Keymap *keymap, ConfigManager::Domain *domain) {
	if (!_hardwareInputs) {
		warning("No hardware inputs were registered yet (%s)", keymap->getName().c_str());
		return;
	}

	keymap->setConfigDomain(domain);
	keymap->setHardwareInputs(_hardwareInputs);
	keymap->setBackendDefaultBindings(_backendDefaultBindings);
	keymap->loadMappings();
}

void Keymapper::cleanupGameKeymaps() {
	// Flush all game specific keymaps
	KeymapArray::iterator it = _keymaps.begin();
	while (it != _keymaps.end()) {
		if ((*it)->getType() == Keymap::kKeymapTypeGame) {
			delete *it;
			it = _keymaps.erase(it);
		} else {
			it++;
		}
	}
}

Keymap *Keymapper::getKeymap(const String &name) {
	for (KeymapArray::const_iterator it = _keymaps.begin(); it != _keymaps.end(); it++) {
		if ((*it)->getName() == name) {
			return *it;
		}
	}

	return nullptr;
}

void Keymapper::reloadAllMappings() {
	for (uint i = 0; i < _keymaps.size(); i++) {
		_keymaps[i]->loadMappings();
	}
}

void Keymapper::setEnabledKeymapType(Keymap::KeymapType type) {
	_enabledKeymapType = type;
}

List<Event> Keymapper::mapEvent(const Event &ev, EventSource *source) {
	if (!_enabled) {
		return DefaultEventMapper::mapEvent(ev, source);
	}
	if (source && !source->allowMapping()) {
		return DefaultEventMapper::mapEvent(ev, source);
	}

	const HardwareInput *hwInput = findHardwareInput(ev);
	if (!hwInput) {
		return DefaultEventMapper::mapEvent(ev, source);
	}

	IncomingEventType incomingEventType = convertToIncomingEventType(ev);

	List<Event> mappedEvents;
	for (int i = _keymaps.size() - 1; i >= 0; --i) {
		if (!_keymaps[i]->isEnabled()) {
			continue;
		}

		Keymap::KeymapType keymapType = _keymaps[i]->getType();
		if (keymapType != _enabledKeymapType && keymapType != Keymap::kKeymapTypeGlobal) {
			continue; // Ignore GUI keymaps while in game and vice versa
		}

		debug(5, "Keymapper::mapKey keymap: %s", _keymaps[i]->getName().c_str());

		const Keymap::ActionArray &actions = _keymaps[i]->getMappedActions(hwInput);
		for (Keymap::ActionArray::const_iterator it = actions.begin(); it != actions.end(); it++) {
			mappedEvents.push_back(executeAction(*it, incomingEventType));
		}
		if (!actions.empty()) {
			// If we found actions matching this input in a keymap, no need to look at the other keymaps.
			// An input resulting in actions from system and game keymaps would lead to unexpected user experience.
			break;
		}
	}

	if (mappedEvents.empty()) {
		return DefaultEventMapper::mapEvent(ev, source);
	}

	return mappedEvents;
}

Keymapper::IncomingEventType Keymapper::convertToIncomingEventType(const Event &ev) const {
	if (ev.type == EVENT_CUSTOM_BACKEND_HARDWARE) {
		return kIncomingNonKey;
	} else if (ev.type == EVENT_KEYDOWN) {
		return kIncomingKeyDown;
	} else {
		return kIncomingKeyUp;
	}
}

Event Keymapper::executeAction(const Action *action, IncomingEventType incomingType) {
	Event evt = Event(action->event);
	EventType convertedType = convertDownToUp(evt.type);

	// hardware keys need to send up instead when they are up
	if (incomingType == kIncomingKeyUp) {
		evt.type = convertedType;
	}

	evt.mouse = _eventMan->getMousePos();

	// Check if the event is coming from a non-key hardware event
	// that is mapped to a key event
	if (incomingType == kIncomingNonKey && convertedType != EVENT_INVALID) {
		// WORKAROUND: Delay the down events coming from non-key hardware events
		// with a zero delay. This is to prevent DOWN1 DOWN2 UP1 UP2.
		addDelayedEvent(0, evt);

		// non-keys need to send up as well
		// WORKAROUND: Delay the up events coming from non-key hardware events
		// This is for engines that run scripts that check on key being down
		evt.type = convertedType;
		const uint32 delay = (convertedType == EVENT_KEYUP ? kDelayKeyboardEventMillis : kDelayMouseEventMillis);
		addDelayedEvent(delay, evt);
	}

	return evt;
}

EventType Keymapper::convertDownToUp(EventType type) {
	EventType result = EVENT_INVALID;
	switch (type) {
	case EVENT_KEYDOWN:
		result = EVENT_KEYUP;
		break;
	case EVENT_LBUTTONDOWN:
		result = EVENT_LBUTTONUP;
		break;
	case EVENT_RBUTTONDOWN:
		result = EVENT_RBUTTONUP;
		break;
	case EVENT_MBUTTONDOWN:
		result = EVENT_MBUTTONUP;
		break;
	default:
		break;
	}
	return result;
}

const HardwareInput *Keymapper::findHardwareInput(const Event &event) {
	switch (event.type) {
		case EVENT_KEYDOWN:
		case EVENT_KEYUP:
			return _hardwareInputs->findHardwareInput(event.kbd);
		case EVENT_CUSTOM_BACKEND_HARDWARE:
			return _hardwareInputs->findHardwareInput(event.customType);
		default:
			return nullptr;
	}
}

} // End of namespace Common

#endif // #ifdef ENABLE_KEYMAPPER
