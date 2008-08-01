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

#include "backends/common/keymapper.h"
#include "backends/common/keymap-manager.h"
#include "common/config-manager.h"
namespace Common {

Keymapper::Keymapper(EventManager *evtMgr) {
	_eventMan = evtMgr;
	_keymapMan = new KeymapManager();
	_currentMap = 0;
}

void Keymapper::registerHardwareKeySet(HardwareKeySet *keys) {
	_keymapMan->registerHardwareKeySet(keys);
}

void Keymapper::addGlobalKeyMap(const String& name, Keymap *keymap) {
	_keymapMan->registerGlobalKeymap(name, keymap);
}

void Keymapper::addGameKeyMap(const String& name, Keymap *keymap) {
	if (_gameId.size() == 0) {
		initGame();
		if (_gameId.size() == 0)
			return;
	}
	_keymapMan->registerGameKeymap(name, keymap);
}

void Keymapper::initGame() {
	if (ConfMan.getActiveDomain() == 0)
		error("Call to Keymapper::initGame when no game loaded");

	if (_gameId.size() > 0)
		cleanupGame();
	_gameId = ConfMan.getActiveDomainName();
}

void Keymapper::cleanupGame() {
	_keymapMan->unregisterAllGameKeymaps();
	_gameId.clear();
}


bool Keymapper::switchKeymap(const String& name) {
	Keymap *new_map = _keymapMan->getKeymap(name);
	if (!new_map) {
		warning("Keymap '%s' not registered", name.c_str());
		return false;
	}
	_currentMap = new_map;
	return true;
}

bool Keymapper::mapKeyDown(const KeyState& key) {
	return mapKey(key, true);
}

bool Keymapper::mapKeyUp(const KeyState& key) {
	return mapKey(key, false);
}

bool Keymapper::mapKey(const KeyState& key, bool isKeyDown) {
	if (!_currentMap) return false;
	Action *action = _currentMap->getMappedAction(key);
	if (!action) return false;
	List<Event>::iterator it;
	for (it = action->events.begin(); it != action->events.end(); it++) {
		Event evt = *it;
		bool pushEvent = true;
		switch (evt.type) {
		case EVENT_KEYDOWN:
			if (!isKeyDown) evt.type = EVENT_KEYUP;
			break;
		case EVENT_KEYUP:
			if (isKeyDown) evt.type = EVENT_KEYDOWN;
			break;
		case EVENT_LBUTTONDOWN:
			if (!isKeyDown) evt.type = EVENT_LBUTTONUP;
			break;
		case EVENT_LBUTTONUP:
			if (isKeyDown) evt.type = EVENT_LBUTTONDOWN;
			break;
		case EVENT_RBUTTONDOWN:
			if (!isKeyDown) evt.type = EVENT_RBUTTONUP;
			break;
		case EVENT_RBUTTONUP:
			if (isKeyDown) evt.type = EVENT_RBUTTONDOWN;
			break;
		case EVENT_MBUTTONDOWN:
			if (!isKeyDown) evt.type = EVENT_MBUTTONUP;
			break;
		case EVENT_MBUTTONUP:
			if (isKeyDown) evt.type = EVENT_MBUTTONDOWN;
			break;
		default:
			// don't deliver other events on key up
			if (!isKeyDown) pushEvent = false;
		}
		if (pushEvent) _eventMan->pushEvent(evt);
	}
	return true;
}

} // end of namespace Common
