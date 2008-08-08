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
#include "common/config-manager.h"
namespace Common {

Keymapper::Keymapper(EventManager *evtMgr) {
	_eventMan = evtMgr;
	_keymapMan = new KeymapManager();
	_enabled = true;
}

Keymapper::~Keymapper() {
	delete _keymapMan;
}

void Keymapper::registerHardwareKeySet(HardwareKeySet *keys) {
	_keymapMan->registerHardwareKeySet(keys);
}

void Keymapper::addGlobalKeymap(Keymap *keymap) {
	_keymapMan->registerGlobalKeymap(keymap);
}

void Keymapper::addGameKeymap(Keymap *keymap) {
	if (ConfMan.getActiveDomain() == 0)
		error("Call to Keymapper::addGameKeymap when no game loaded");
		
	_keymapMan->registerGameKeymap(keymap);
}

bool Keymapper::pushKeymap(const String& name, bool inherit) {
	Keymap *newMap = _keymapMan->getKeymap(name);
	if (!newMap) {
		warning("Keymap '%s' not registered", name.c_str());
		return false;
	}
	pushKeymap(newMap, inherit);
	return true;
}

void Keymapper::pushKeymap(Keymap *newMap, bool inherit) {
	MapRecord mr;
	mr.inherit = inherit;
	mr.keymap = newMap;
	_activeMaps.push(mr);
}

void Keymapper::popKeymap() {
	if (!_activeMaps.empty())
		_activeMaps.pop();
}

bool Keymapper::mapKeyDown(const KeyState& key) {
	return mapKey(key, true);
}

bool Keymapper::mapKeyUp(const KeyState& key) {
	return mapKey(key, false);
}

bool Keymapper::mapKey(const KeyState& key, bool isKeyDown) {
	if (!_enabled) return false;
	if (_activeMaps.empty()) return false;

	Action *action = 0;
	for (int i = _activeMaps.size() - 1; !action && i >= 0; i++) {
		MapRecord mr = _activeMaps[i];
		action = mr.keymap->getMappedAction(key);
		if (mr.inherit == false) break;
	}
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

const HardwareKey *Keymapper::getHardwareKey(const KeyState& key) {
	HardwareKeySet *keyset = _keymapMan->getHardwareKeySet();
	if (!keyset) return 0;
	return keyset->findHardwareKey(key);
}

} // end of namespace Common
