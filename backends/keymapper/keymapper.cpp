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

#include "backends/keymapper/action.h"
#include "backends/keymapper/hardware-input.h"
#include "backends/keymapper/keymapper-defaults.h"

#include "common/system.h"

namespace Common {

// These magic numbers are provided by fuzzie and WebOS
static const uint32 kDelayKeyboardEventMillis = 250;
static const uint32 kDelayMouseEventMillis = 50;

Keymapper::Keymapper(EventManager *eventMan) :
		_eventMan(eventMan),
		_hardwareInputs(nullptr),
		_backendDefaultBindings(nullptr),
		_delayedEventSource(new DelayedEventSource()),
		_enabled(true),
		_enabledKeymapType(Keymap::kKeymapTypeGlobal) {
	_eventMan->getEventDispatcher()->registerSource(_delayedEventSource, true);
}

Keymapper::~Keymapper() {
	for (KeymapArray::iterator it = _keymaps.begin(); it != _keymaps.end(); it++) {
		delete *it;
	}
	delete _backendDefaultBindings;
	delete _hardwareInputs;
}

void Keymapper::registerHardwareInputSet(HardwareInputSet *inputs) {
	if (_hardwareInputs)
		error("Hardware input set already registered");

	if (!inputs) {
		warning("No hardware input were defined, using defaults");
		CompositeHardwareInputSet *compositeInputs = new CompositeHardwareInputSet();
		compositeInputs->addHardwareInputSet(new MouseHardwareInputSet(defaultMouseButtons));
		compositeInputs->addHardwareInputSet(new KeyboardHardwareInputSet(defaultKeys, defaultModifiers));
		inputs = compositeInputs;
	}

	_hardwareInputs = inputs;
}

void Keymapper::registerBackendDefaultBindings(KeymapperDefaultBindings *backendDefaultBindings) {
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
		warning("No hardware inputs were registered yet (%s)", keymap->getId().c_str());
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

Keymap *Keymapper::getKeymap(const String &id) const {
	for (KeymapArray::const_iterator it = _keymaps.begin(); it != _keymaps.end(); it++) {
		if ((*it)->getId() == id) {
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

List<Event> Keymapper::mapEvent(const Event &ev) {
	if (!_enabled) {
		List<Event> originalEvent;
		originalEvent.push_back(ev);
		return originalEvent;
	}

	hardcodedEventMapping(ev);

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

		debug(5, "Keymapper::mapKey keymap: %s", _keymaps[i]->getId().c_str());

		const Keymap::ActionArray &actions = _keymaps[i]->getMappedActions(ev);
		for (Keymap::ActionArray::const_iterator it = actions.begin(); it != actions.end(); it++) {
			mappedEvents.push_back(executeAction(*it, incomingEventType));
		}
		if (!actions.empty()) {
			// If we found actions matching this input in a keymap, no need to look at the other keymaps.
			// An input resulting in actions from system and game keymaps would lead to unexpected user experience.
			break;
		}
	}

	// Ignore keyboard repeat events. Repeat event are meant for text input,
	// the keymapper / keymaps are supposed to be disabled during text input.
	// TODO: Add a way to keep repeat events if needed.
	if (!mappedEvents.empty() && ev.type == EVENT_KEYDOWN && ev.kbdRepeat) {
		return List<Event>();
	}

	if (mappedEvents.empty()) {
		// if it didn't get mapped, just pass it through
		mappedEvents.push_back(ev);
	}

	return mappedEvents;
}

Keymapper::IncomingEventType Keymapper::convertToIncomingEventType(const Event &ev) const {
	if (ev.type == EVENT_CUSTOM_BACKEND_HARDWARE) {
		return kIncomingEventInstant;
	} else if (ev.type == EVENT_KEYDOWN
	           || ev.type == EVENT_LBUTTONDOWN
	           || ev.type == EVENT_RBUTTONDOWN
	           || ev.type == EVENT_MBUTTONDOWN
	           || ev.type == EVENT_JOYBUTTON_DOWN) {
		return kIncomingEventStart;
	} else {
		return kIncomingEventEnd;
	}
}

Event Keymapper::executeAction(const Action *action, IncomingEventType incomingType) {
	Event evt = Event(action->event);
	EventType convertedType = convertStartToEnd(evt.type);

	// hardware keys need to send up instead when they are up
	if (incomingType == kIncomingEventEnd) {
		evt.type = convertedType;
	}

	evt.mouse = _eventMan->getMousePos();

	// Check if the event is coming from a non-key hardware event
	// that is mapped to a key event
	if (incomingType == kIncomingEventInstant && convertedType != EVENT_INVALID) {
		// WORKAROUND: Delay the down events coming from non-key hardware events
		// with a zero delay. This is to prevent DOWN1 DOWN2 UP1 UP2.
		_delayedEventSource->scheduleEvent(evt, 0);

		// non-keys need to send up as well
		// WORKAROUND: Delay the up events coming from non-key hardware events
		// This is for engines that run scripts that check on key being down
		evt.type = convertedType;
		const uint32 delay = (convertedType == EVENT_KEYUP ? kDelayKeyboardEventMillis : kDelayMouseEventMillis);
		_delayedEventSource->scheduleEvent(evt, delay);
	}

	return evt;
}

EventType Keymapper::convertStartToEnd(EventType type) {
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
	case EVENT_JOYBUTTON_DOWN:
		result = EVENT_JOYBUTTON_UP;
		break;
	case EVENT_CUSTOM_BACKEND_ACTION_START:
		result = EVENT_CUSTOM_BACKEND_ACTION_END;
		break;
	case EVENT_CUSTOM_ENGINE_ACTION_START:
		result = EVENT_CUSTOM_ENGINE_ACTION_END;
		break;
	default:
		break;
	}
	return result;
}

HardwareInput Keymapper::findHardwareInput(const Event &event) {
	return _hardwareInputs->findHardwareInput(event);
}

void Keymapper::hardcodedEventMapping(Event ev) {
	// TODO: Either add support for long presses to the keymapper
	// or move this elsewhere as an event observer + source
#ifdef ENABLE_VKEYBD
	// Trigger virtual keyboard on long press of more than 1 second
	// of middle mouse button.
	const uint32 vkeybdTime = 1000;

	static uint32 vkeybdThen = 0;

	if (ev.type == EVENT_MBUTTONDOWN) {
		vkeybdThen = g_system->getMillis();
	}

	if (ev.type == EVENT_MBUTTONUP) {
		if ((g_system->getMillis() - vkeybdThen) >= vkeybdTime) {
			Event vkeybdEvent;
			vkeybdEvent.type = EVENT_VIRTUAL_KEYBOARD;

			// Avoid blocking event from engine.
			_delayedEventSource->scheduleEvent(vkeybdEvent, 100);
		}
	}
#endif
}

void DelayedEventSource::scheduleEvent(const Event &ev, uint32 delayMillis) {
	if (_delayedEvents.empty()) {
		_delayedEffectiveTime = g_system->getMillis() + delayMillis;
		delayMillis = 0;
	}
	DelayedEventsEntry entry = DelayedEventsEntry(delayMillis, ev);
	_delayedEvents.push(entry);
}

bool DelayedEventSource::pollEvent(Event &event) {
	if (_delayedEvents.empty()) {
		return false;
	}

	uint32 now = g_system->getMillis();

	if (now >= _delayedEffectiveTime) {
		event = _delayedEvents.pop().event;

		if (!_delayedEvents.empty()) {
			_delayedEffectiveTime += _delayedEvents.front().timerOffset;
		}

		return true;
	}

	return false;
}

bool DelayedEventSource::allowMapping() const {
	return false; // Events from this source have already been mapped, and should not be mapped again
}

} // End of namespace Common
