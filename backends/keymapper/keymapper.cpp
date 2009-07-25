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

#include "backends/keymapper/keymapper.h"

#ifdef ENABLE_KEYMAPPER

#include "common/config-manager.h"

namespace Common {

void Keymapper::Domain::addKeymap(Keymap *map) {
	iterator it = find(map->getName());

	if (it != end())
		delete it->_value;

	setVal(map->getName(), map);
}

void Keymapper::Domain::deleteAllKeyMaps() {
	for (iterator it = begin(); it != end(); it++)
		delete it->_value;

	clear();
}

Keymap *Keymapper::Domain::getKeymap(const String& name) {
	iterator it = find(name);

	if (it != end())
		return it->_value;
	else
		return 0;
}

Keymapper::Keymapper(EventManager *evtMgr)
	: _eventMan(evtMgr), _enabled(true), _hardwareKeys(0) {
	ConfigManager::Domain *confDom = ConfMan.getDomain(ConfigManager::kKeymapperDomain);

	_globalDomain.setConfigDomain(confDom);
}

Keymapper::~Keymapper() {
	delete _hardwareKeys;
}

void Keymapper::registerHardwareKeySet(HardwareKeySet *keys) {
	if (_hardwareKeys)
		error("Hardware key set already registered");

	if (!keys) {
		warning("No hardware keys are supplied");
		return;
	}

	_hardwareKeys = keys;
}

void Keymapper::addGlobalKeymap(Keymap *keymap) {
	initKeymap(_globalDomain, keymap);
}

void Keymapper::addGameKeymap(Keymap *keymap) {
	if (ConfMan.getActiveDomain() == 0)
		error("Call to Keymapper::addGameKeymap when no game loaded");

	// Detect whether the active game changed since last call.
	// If so, flush the game key configuration.
	if (_gameDomain.getConfigDomain() != ConfMan.getActiveDomain()) {
		cleanupGameKeymaps();
		_gameDomain.setConfigDomain(ConfMan.getActiveDomain());
	}

	initKeymap(_gameDomain, keymap);
}

void Keymapper::initKeymap(Domain &domain, Keymap *map) {
	if (!_hardwareKeys) {
		warning("No hardware keys were registered yet (%s)", map->getName().c_str());
		return;
	}

	map->setConfigDomain(domain.getConfigDomain());
	map->loadMappings(_hardwareKeys);

	if (map->isComplete(_hardwareKeys) == false) {
		map->automaticMapping(_hardwareKeys);
		map->saveMappings();
		ConfMan.flushToDisk();
	}

	domain.addKeymap(map);
}

void Keymapper::cleanupGameKeymaps() {
	// Flush all game specific keymaps
	_gameDomain.deleteAllKeyMaps();

	// Now restore the stack of active maps. Re-add all global keymaps, drop
	// the game specific (=deleted) ones.
	Stack<MapRecord> newStack;

	for (int i = 0; i < _activeMaps.size(); i++) {
		if (_activeMaps[i].global)
			newStack.push(_activeMaps[i]);
	}

	_activeMaps = newStack;
}

Keymap *Keymapper::getKeymap(const String& name, bool &global) {
	Keymap *keymap = _gameDomain.getKeymap(name);
	global = false;

	if (!keymap) {
		keymap = _globalDomain.getKeymap(name);
		global = true;
	}

	return keymap;
}

bool Keymapper::pushKeymap(const String& name, bool inherit) {
	bool global;
	Keymap *newMap = getKeymap(name, global);

	if (!newMap) {
		warning("Keymap '%s' not registered", name.c_str());
		return false;
	}

	pushKeymap(newMap, inherit, global);

	return true;
}

void Keymapper::pushKeymap(Keymap *newMap, bool inherit, bool global) {
	MapRecord mr = {newMap, inherit, global};

	_activeMaps.push(mr);
}

void Keymapper::popKeymap() {
	if (!_activeMaps.empty())
		_activeMaps.pop();
}

bool Keymapper::notifyEvent(const Common::Event &ev) {
	if (ev.type == Common::EVENT_KEYDOWN)
		return mapKeyDown(ev.kbd);
	else if (ev.type == Common::EVENT_KEYUP)
		return mapKeyUp(ev.kbd);
	else
		return false;
}

bool Keymapper::mapKeyDown(const KeyState& key) {
	return mapKey(key, true);
}

bool Keymapper::mapKeyUp(const KeyState& key) {
	return mapKey(key, false);
}

bool Keymapper::mapKey(const KeyState& key, bool keyDown) {
	if (!_enabled || _activeMaps.empty())
		return false;

	Action *action = 0;

	if (keyDown) {
		// Search for key in active keymap stack
		for (int i = _activeMaps.size() - 1; i >= 0; --i) {
			MapRecord mr = _activeMaps[i];

			action = mr.keymap->getMappedAction(key);

			if (action || mr.inherit == false)
				break;
		}

		if (action)
			_keysDown[key] = action;
	} else {
		HashMap<KeyState, Action*>::iterator it = _keysDown.find(key);

		if (it != _keysDown.end()) {
			action = it->_value;
			_keysDown.erase(key);
		}
	}

	if (!action)
		return false;

	executeAction(action, keyDown);

	return true;
}

Action *Keymapper::getAction(const KeyState& key) {
	Action *action = 0;

	return action;
}

void Keymapper::executeAction(const Action *action, bool keyDown) {
	List<Event>::const_iterator it;

	for (it = action->events.begin(); it != action->events.end(); ++it) {
		Event evt = *it;

		switch (evt.type) {
		case EVENT_KEYDOWN:
			if (!keyDown) evt.type = EVENT_KEYUP;
			break;
		case EVENT_KEYUP:
			if (keyDown) evt.type = EVENT_KEYDOWN;
			break;
		case EVENT_LBUTTONDOWN:
			if (!keyDown) evt.type = EVENT_LBUTTONUP;
			break;
		case EVENT_LBUTTONUP:
			if (keyDown) evt.type = EVENT_LBUTTONDOWN;
			break;
		case EVENT_RBUTTONDOWN:
			if (!keyDown) evt.type = EVENT_RBUTTONUP;
			break;
		case EVENT_RBUTTONUP:
			if (keyDown) evt.type = EVENT_RBUTTONDOWN;
			break;
		case EVENT_MBUTTONDOWN:
			if (!keyDown) evt.type = EVENT_MBUTTONUP;
			break;
		case EVENT_MBUTTONUP:
			if (keyDown) evt.type = EVENT_MBUTTONDOWN;
			break;
		default:
			// don't deliver other events on key up
			if (!keyDown) continue;
		}

		evt.mouse = _eventMan->getMousePos();
		addEvent(evt);
	}
}

const HardwareKey *Keymapper::findHardwareKey(const KeyState& key) {
	return (_hardwareKeys) ? _hardwareKeys->findHardwareKey(key) : 0;
}

} // end of namespace Common

#endif // #ifdef ENABLE_KEYMAPPER
