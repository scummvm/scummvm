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

#include "backends/common/keymap.h"
#include "backends/common/hardware-key.h"

namespace Common {

Keymap::Keymap(const Keymap& km) : _actions(km._actions), _keymap() {
	init();
	for (uint i = 0; i < _actions.size(); i++) {
		const HardwareKey *hwKey = _actions[i].getMappedKey();
		if (hwKey) {
			_keymap[hwKey->key] = &_actions[i];
		}
	}
}

void Keymap::init() {
	_actions.reserve(20);
}

void Keymap::addAction(UserAction& action) {
	if (findUserAction(action.id))
		error("UserAction with id %d already in KeyMap!", action.id);
	action.setParent(this);
	_actions.push_back(action);
}

void Keymap::registerMapping(UserAction *action, const HardwareKey *hwKey) {
	HashMap<KeyState, UserAction*>::iterator it;
	it = _keymap.find(hwKey->key);
	// if key is already mapped to an action then un-map it
	if (it != _keymap.end())
		it->_value->mapKey(0);

	_keymap[hwKey->key] = action;
}

void Keymap::unregisterMapping(UserAction *action) {
	const HardwareKey *hwKey = action->getMappedKey();
	if (hwKey)
		_keymap[hwKey->key] = 0;
}

UserAction *Keymap::getUserAction(int32 id) {
	return findUserAction(id);
}

UserAction *Keymap::findUserAction(int32 id) {
	Array<UserAction>::iterator it;
	for (it = _actions.begin(); it != _actions.end(); it++) {
		if (it->id == id)
			return &*it;
	}
	return 0;
}

const UserAction *Keymap::findUserAction(int32 id) const {
	Array<UserAction>::const_iterator it;
	for (it = _actions.begin(); it != _actions.end(); it++) {
		if (it->id == id)
			return &*it;
	}
	return 0;
}

UserAction *Keymap::getMappedAction(const KeyState& ks) const {
	HashMap<KeyState, UserAction*>::iterator it;
	it = _keymap.find(ks);
	if (it == _keymap.end())
		return 0;
	else
		return it->_value;
}

} // end of namespace Common
