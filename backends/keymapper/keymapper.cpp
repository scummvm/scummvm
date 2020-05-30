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
		_enabledKeymapType(Keymap::kKeymapTypeGame) {
	_eventMan->getEventDispatcher()->registerSource(_delayedEventSource, true);
	resetInputState();
}

Keymapper::~Keymapper() {
	clear();
}

void Keymapper::clear() {
	for (KeymapArray::iterator it = _keymaps.begin(); it != _keymaps.end(); it++) {
		delete *it;
	}
	_keymaps.clear();

	delete _backendDefaultBindings;
	_backendDefaultBindings = nullptr;

	delete _hardwareInputs;
	_hardwareInputs = nullptr;
}

void Keymapper::registerHardwareInputSet(HardwareInputSet *inputs, KeymapperDefaultBindings *backendDefaultBindings) {
	bool reloadMappings = false;
	if (_hardwareInputs) {
		reloadMappings = true;
		delete _hardwareInputs;
	}
	if (_backendDefaultBindings) {
		reloadMappings = true;
		delete _backendDefaultBindings;
	}

	if (!inputs) {
		warning("No hardware input were defined, using defaults");
		CompositeHardwareInputSet *compositeInputs = new CompositeHardwareInputSet();
		compositeInputs->addHardwareInputSet(new MouseHardwareInputSet(defaultMouseButtons));
		compositeInputs->addHardwareInputSet(new KeyboardHardwareInputSet(defaultKeys, defaultModifiers));
		inputs = compositeInputs;
	}

	_hardwareInputs = inputs;
	_backendDefaultBindings = backendDefaultBindings;

	if (reloadMappings) {
		reloadAllMappings();
	}
}

void Keymapper::addGlobalKeymap(Keymap *keymap) {
	assert(keymap->getType() == Keymap::kKeymapTypeGlobal
	       || keymap->getType() == Keymap::kKeymapTypeGui);

	ConfigManager::Domain *keymapperDomain = ConfMan.getDomain(ConfigManager::kKeymapperDomain);
	initKeymap(keymap, keymapperDomain);
	_keymaps.push_back(keymap);
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
	reloadKeymapMappings(keymap);
}

void Keymapper::reloadKeymapMappings(Keymap *keymap) {
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
		reloadKeymapMappings(_keymaps[i]);
	}
}

void Keymapper::setEnabledKeymapType(Keymap::KeymapType type) {
	assert(type == Keymap::kKeymapTypeGui || type == Keymap::kKeymapTypeGame);
	_enabledKeymapType = type;
}

List<Event> Keymapper::mapEvent(const Event &ev) {
	if (!_enabled) {
		List<Event> originalEvent;
		originalEvent.push_back(ev);
		return originalEvent;
	}

	hardcodedEventMapping(ev);

	Keymap::ActionArray actions;
	Keymap::KeymapMatch match = getMappedActions(ev, actions, _enabledKeymapType);
	if (match != Keymap::kKeymapMatchExact) {
		// If we found exact matching actions this input in the game / gui keymaps,
		// no need to look at the global keymaps. An input resulting in actions
		// from system and game keymaps would lead to unexpected user experience.
		Keymap::ActionArray globalActions;
		match = getMappedActions(ev, globalActions, Keymap::kKeymapTypeGlobal);
		if (match == Keymap::kKeymapMatchExact || actions.empty()) {
			actions = globalActions;
		}
	}

	bool matchedAction = !actions.empty();
	List<Event> mappedEvents;
	for (Keymap::ActionArray::const_iterator it = actions.begin(); it != actions.end(); it++) {
		Event mappedEvent = executeAction(*it, ev);
		if (mappedEvent.type == EVENT_INVALID) {
			continue;
		}

		// In case we mapped a mouse event to something else, we need to generate an artificial
		// mouse move event so event observers can keep track of the mouse position.
		// Makes it possible to reliably use the mouse position from EventManager when consuming
		// custom action events.
		if (isMouseEvent(ev) && !isMouseEvent(mappedEvent)) {
			Event fakeMouseEvent;
			fakeMouseEvent.type  = EVENT_MOUSEMOVE;
			fakeMouseEvent.mouse = ev.mouse;

			mappedEvents.push_back(fakeMouseEvent);
		}

		mappedEvents.push_back(mappedEvent);
	}

	if (ev.type == EVENT_JOYAXIS_MOTION && ev.joystick.axis < ARRAYSIZE(_joystickAxisPreviouslyPressed)) {
		if (ABS<int32>(ev.joystick.position) >= kJoyAxisPressedTreshold) {
			_joystickAxisPreviouslyPressed[ev.joystick.axis] = true;
		} else if (ABS<int32>(ev.joystick.position) < kJoyAxisUnpressedTreshold) {
			_joystickAxisPreviouslyPressed[ev.joystick.axis] = false;
		}
	}

	if (!matchedAction) {
		// if it didn't get mapped, just pass it through
		mappedEvents.push_back(ev);
	}

	return mappedEvents;
}

Keymap::KeymapMatch Keymapper::getMappedActions(const Event &event, Keymap::ActionArray &actions, Keymap::KeymapType keymapType) const {
	Keymap::KeymapMatch match = Keymap::kKeymapMatchNone;

	for (uint i = 0; i < _keymaps.size(); i++) {
		if (!_keymaps[i]->isEnabled() || _keymaps[i]->getType() != keymapType) {
			continue;
		}

		Keymap::ActionArray array;
		Keymap::KeymapMatch match2 = _keymaps[i]->getMappedActions(event, array);
		if (match2 == match) {
			actions.push_back(array);
		} else if (match2 > match) {
			match = match2;
			actions.clear();
			actions.push_back(array);
		}
	}
	return match;
}

Keymapper::IncomingEventType Keymapper::convertToIncomingEventType(const Event &ev) const {
	if (ev.type == EVENT_CUSTOM_BACKEND_HARDWARE
	           || ev.type == EVENT_WHEELDOWN
	           || ev.type == EVENT_WHEELUP) {
		return kIncomingEventInstant;
	} else if (ev.type == EVENT_JOYAXIS_MOTION) {
		if (ev.joystick.axis >= ARRAYSIZE(_joystickAxisPreviouslyPressed)) {
			return kIncomingEventIgnored;
		}

		if (!_joystickAxisPreviouslyPressed[ev.joystick.axis] && ABS<int32>(ev.joystick.position) >= kJoyAxisPressedTreshold) {
			return kIncomingEventStart;
		} else if (_joystickAxisPreviouslyPressed[ev.joystick.axis] && ABS<int32>(ev.joystick.position) < kJoyAxisUnpressedTreshold) {
			return kIncomingEventEnd;
		} else {
			return kIncomingEventIgnored;
		}
	} else if (ev.type == EVENT_KEYDOWN
	           || ev.type == EVENT_LBUTTONDOWN
	           || ev.type == EVENT_RBUTTONDOWN
	           || ev.type == EVENT_MBUTTONDOWN
	           || ev.type == EVENT_X1BUTTONDOWN
	           || ev.type == EVENT_X2BUTTONDOWN
	           || ev.type == EVENT_JOYBUTTON_DOWN) {
		return kIncomingEventStart;
	} else {
		return kIncomingEventEnd;
	}
}

Event Keymapper::executeAction(const Action *action, const Event &incomingEvent) {
	Event outgoingEvent = Event(action->event);

	IncomingEventType incomingType = convertToIncomingEventType(incomingEvent);

	if (outgoingEvent.type == EVENT_JOYAXIS_MOTION
	        || outgoingEvent.type == EVENT_CUSTOM_BACKEND_ACTION_AXIS) {
		if (incomingEvent.type == EVENT_JOYAXIS_MOTION) {
			// At the moment only half-axes can be bound to actions, hence taking
			//  the absolute value. If full axes were to be mappable, the action
			//  could carry the information allowing to distinguish cases here.
			outgoingEvent.joystick.position = ABS(incomingEvent.joystick.position);
		} else if (incomingType == kIncomingEventStart) {
			outgoingEvent.joystick.position = JOYAXIS_MAX;
		} else if (incomingType == kIncomingEventEnd) {
			outgoingEvent.joystick.position = 0;
		}

		return outgoingEvent;
	}

	if (incomingType == kIncomingEventIgnored) {
		outgoingEvent.type = EVENT_INVALID;
		return outgoingEvent;
	}

	if (incomingEvent.type == EVENT_KEYDOWN && incomingEvent.kbdRepeat && !action->shouldTriggerOnKbdRepeats()) {
		outgoingEvent.type = EVENT_INVALID;
		return outgoingEvent;
	}

	EventType convertedType = convertStartToEnd(outgoingEvent.type);

	// hardware keys need to send up instead when they are up
	if (incomingType == kIncomingEventEnd) {
		outgoingEvent.type = convertedType;
	}

	if (outgoingEvent.type == EVENT_KEYDOWN && incomingEvent.type == EVENT_KEYDOWN) {
		outgoingEvent.kbdRepeat = incomingEvent.kbdRepeat;
	}

	if (isMouseEvent(outgoingEvent)) {
		if (isMouseEvent(incomingEvent)) {
			outgoingEvent.mouse = incomingEvent.mouse;
		} else {
			outgoingEvent.mouse = _eventMan->getMousePos();
		}
	}

	// Check if the event is coming from a non-key hardware event
	// that is mapped to a key event
	if (incomingType == kIncomingEventInstant && convertedType != EVENT_INVALID) {
		// WORKAROUND: Delay the down events coming from non-key hardware events
		// with a zero delay. This is to prevent DOWN1 DOWN2 UP1 UP2.
		_delayedEventSource->scheduleEvent(outgoingEvent, 0);

		// non-keys need to send up as well
		// WORKAROUND: Delay the up events coming from non-key hardware events
		// This is for engines that run scripts that check on key being down
		outgoingEvent.type = convertedType;
		const uint32 delay = (convertedType == EVENT_KEYUP ? kDelayKeyboardEventMillis : kDelayMouseEventMillis);
		_delayedEventSource->scheduleEvent(outgoingEvent, delay);
	}

	return outgoingEvent;
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
	case EVENT_X1BUTTONDOWN:
		result = EVENT_X1BUTTONUP;
		break;
	case EVENT_X2BUTTONDOWN:
		result = EVENT_X2BUTTONUP;
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

void Keymapper::resetInputState() {
	for (uint i = 0; i < ARRAYSIZE(_joystickAxisPreviouslyPressed); i++) {
		_joystickAxisPreviouslyPressed[i] = false;
	}
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
